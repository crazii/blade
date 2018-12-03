/********************************************************************
	created:	2010/05/13
	filename: 	TerrainBlock.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_TerrainBlock_h__
#define __Blade_TerrainBlock_h__
#include <interface/public/IRenderable.h>
#include <SpaceContent.h>
#include <interface/public/graphics/IVertexSource.h>
#include <interface/public/graphics/GraphicsGeometry.h>
#include <Material.h>
#include <interface/IMaterialLODUpdater.h>
#include "terrain_interface/ITerrainTile.h"
#include "TerrainConfigManager.h"
#include "BlockQueryDataCompact.h"

namespace Blade
{
	typedef BlockQueryDataRootCompact	BLOCK_QUERYDATA_ROOT;
	typedef SBlockRayQueryCompact		BLOCK_RAYQUERY;
	typedef SBlockVolumeQueryCompact	BLOCK_VOLUMEQUERY;

	class IRenderGroup;

	class TerrainBlock : public SpaceContentBase, public IRenderable, public IMaterialLODUpdatable, public Allocatable
	{
	public:
		static const uint8 INVALID_FIXED_LOD = 0xFu;
	public:
		TerrainBlock(index_t x,index_t z, ITerrainTile* parent);
		~TerrainBlock();

		/************************************************************************/
		/* IRenderable interface                                                                     */
		/************************************************************************/

		/*
		@describe 
		@param 
		@return 
		*/
		virtual RenderType&				getRenderType() const				{return TerrainConfigManager::getRenderType();}

		/*
		@describe
		@param
		@return
		*/
		virtual const GraphicsGeometry&	getGeometry() const					{return mParent->getTileInfo().mRenderGeomBuffer[ mBlockIndex.mIndex ];}

		/*
		@describe
		@param
		@return
		*/
		virtual const MaterialInstance*	getMaterial() const					{return this->getMaterialInstance();}

		/*
		@describe
		@param
		@return
		*/
		virtual const Matrix44&			getWorldTransform() const			{return mParent->getTileInfo().mWorldTransform;}

		/**
		@describe get hosted content
		@param
		@return
		*/
		virtual ISpaceContent*			getSpaceContent() const { return const_cast<ISpaceContent*>(static_cast< const ISpaceContent*>(this)); }

		/************************************************************************/
		/* SpaceContent overrides                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual uint32					getAppFlag() const	{ return TerrainConfigManager::getRenderType().getAppFlag().getMask();}

		/*
		@describe add renderables to render buffer
		@return
		@note  this function will be called if CUF_*_VISIBLE is set
		*/
		virtual void					updateRender(IRenderQueue* queue);

		/*
		@describe update LOD level based on camera pos
		@param
		@return
		*/
		void							visibleUpdateImpl(const Vector3& cameraPos);
		virtual void visibleUpdate(const ICamera* /*current*/, const ICamera* main, const ICamera::VISIBILITY /*visibility*/)
		{
			//invisible blocks in main camera may be visible in other cameras. but update LOD always using main camera's position
			return this->visibleUpdateImpl(main->getEyePosition());
		}

		/**
		@describe 
		@param
		@return
		*/
		virtual	bool					queryNearestPoint(SpaceQuery& query, scalar& distance) const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool					intersectTriangles(SpaceQuery& query, POS_VOL contentPos = PV_INTERSECTED) const;

		/************************************************************************/
		/* IMaterialLODUpdatable interface                                                                     */
		/************************************************************************/
		virtual index_t					getMaxLOD() const	{return mParent->getTileInfo().mMaterialLODQualityLimit;}
		virtual MaterialInstance*		getMatInstance()	{return this->getMaterialInstance();}
		virtual Vector3					getMatLODPosition() const	{return mWorldAABB.getCenter();}
		virtual scalar					getRadiusSQR() const
		{
			static size_t blockSize = 0;
			static scalar radius = 0;
			size_t curSize = mParent->getConfigManager()->getTerrainBlockSize();
			if( blockSize != curSize )
			{
				blockSize = curSize;
				radius = (scalar)blockSize * 1.4142f; //sqrt(2)
				radius *= radius;
			}
			return radius;
		}
		virtual void					setMaterialLOD(MATERIALLOD::LOD_INDEX LOD) { mMaterialLOD = LOD; }

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		index_t							getCurrentLODLevel() const			{return mLODLevel;}

		/** @brief  */
		LOD_DI							getLODDiffIndex() const				{return (LOD_DI)mLODDiffIndex;}

		/** @brief */
		ITerrainTile*					getParentTile() const				{return mParent;}

		/** @brief  */
		index_t							getBlockIndex() const				{return mBlockIndex.mIndex;}

		/** @brief  */
		index_t							getBlockX()	const					{return mBlockIndex.mX;}

		/** @brief  */
		index_t							getBlockZ() const					{return mBlockIndex.mZ;}

		/** @brief  */
		void							setVertexOffsetCount(index_t startVertex, size_t count);

		/** @brief update indices according to current LOD (this & neighbors) */
		/** @note should update after all blocks visibility/LOD updated, because it uses neighbor's LOD too */
		void							updateIndexBuffer(bool force = false);

		/** @brief  */
		void							updateMaterial();

		/** @brief  */
		void							cacheOut();

		/** @brief  */
		void							updateWorldAABBHeight(scalar minY,scalar maxY)
		{
			mWorldAABB.setMinY(minY);
			mWorldAABB.setMaxY(maxY);
		}

		/** @brief  */
		const HMATERIALINSTANCE&		getMaterialInstance() const		{return mParent->getTileInfo().mLODMaterials[mMaterialLOD];}

		/** @brief  */
		uint8				getFixedLOD() const { return mFixedLOD; }

		/** @brief  */
		inline bool			isFixedLOD() const { return mFixedLOD != INVALID_FIXED_LOD; }

		/** @brief  */
		inline void			setFixedLODLevel(uint8 level, uint8 cliffLOD)
		{
			mFixedLOD = (uint8)level;
			//skip blocks those are "flat" for shadows (skip culling directly)
			mSpaceFlags.updateBits(CSF_SHADOWCASTER, mFixedLOD == cliffLOD);
		}

	protected:
		BLOCK_INDEX			mBlockIndex;
		ITerrainTile*		mParent;
		index_t				mVertexStart;
		uint16				mVertexCount;
		uint8				mFixedLOD : 4;			//fixed LOD for cliff or flat plane
		uint8				mLODLevel : 4;
		uint8				mLODDiffIndex : 4;	//LOD_DI
		uint8				mMaterialLOD : 4;
#if BLADE_DEBUG
		uint8				mLastLODLevel;
		uint8				mLastLODDiffIndex;
		uint8				mQueued;
#endif
	};//class TerrainBlock
	
}//namespace Blade


#endif //__Blade_TerrainBlock_h__