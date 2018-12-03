/********************************************************************
	created:	2010/05/04
	filename: 	TerrainTile.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_Terrain_h__
#define __Blade_Terrain_h__
#include <utility/BladeContainer.h>
#include <math/Vector2.h>
#include <interface/ISpace.h>
#include <interface/TerrainBlendData.h>	//internal terrain doesn't need state things, only element need parallel state
#include "TerrainBlock.h"
#include "TerrainAtlas.h"

namespace Blade
{

	class Material;
	class TerrainIndexGroup;

	//////////////////////////////////////////////////////////////////////////
	class TerrainTile : public ITerrainTile, public SpaceContent, public Allocatable
	{
	public:
		TerrainTile();
		~TerrainTile();

		/************************************************************************/
		/* ITerrainTile interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual TerrainBlock*		getTerrainBlock(int x,int y) const;

		/** @brief  */
		virtual const TERRAIN_POSITION_DATA_Y*	getSoftHeightPositionData() const;

		/** @brief  */
		virtual void				releaseIndexBuffer();

		/** @brief  */
		virtual void				buildIndexBuffer();

		/** @brief  */
		virtual void				rebuildBlocks();

		/** @brief  */
		virtual void				updateVertexBuffer()
		{
			size_t blockCount = TerrainConfigManager::getSingleton().getBlocksPerTileSide();
			size_t blockSize = TerrainConfigManager::getSingleton().getTerrainBlockSize();
			return this->updateVertexBuffer(0, 0, blockCount*blockSize, blockCount*blockSize);
		}

		/** @brief  */
		virtual void				updateMaterialLOD(bool updateMat = false);

		/************************************************************************/
		/* SpaceContent overrides                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual uint32	getAppFlag() const	{ return TerrainConfigManager::getRenderType().getAppFlag().getMask();}

		/**
		@describe 
		@param
		@return
		*/
		virtual void	visibleUpdate(const ICamera* current, const ICamera* main, const ICamera::VISIBILITY /*visibility*/);

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		@note 
		*/
		void			setup(const int16* height, const uint8*	blendData, const uint8* normalData, TERRAIN_LAYER* layerData,
			size_t size, const Vector3& startPos, const TileAABBList& localAABBList,
			const HVBUFFER& position, const HVBUFFER& normal,
			const HTEXTURE& blendMap, const HTEXTURE& normalMap, const HTEXTURE& layerMap,
			const HTERRAINATLAS& atlas, const HTERRAINATLAS& normalAtlas,
			bool textureInited, bool rebuildBuffer = false, bool rebuildTexture = false);

		/** @brief  */
		inline Vector2			getHorizontalPos() const
		{
			const Vector3& pos = this->getStartPos();
			return Vector2(pos.x,pos.z);
		}

		/** @brief  */
		inline scalar			getBaseHeight() const
		{
			const Vector3& pos = this->getStartPos();
			return pos.y;
		}

		/** @brief  */
		inline TerrainBlock*		getTerrainBlockUnsafe(int x, int y) const
		{
			return mBlockData[(index_t)y][(index_t)x];
		}

		/*
		@describe
		@param
		@return
		*/
		void			loadMaterial();

		/*
		@describe 
		@param 
		@return 
		*/
		void			updateVertexBuffer(size_t StartX,size_t StartZ,size_t SizeX,size_t SizeZ);

		/*
		@describe 
		@param 
		@return 
		*/
		void			updateNormalBuffer(const uint8* normalData);

		/*
		@describe 
		@param 
		@return 
		*/
		void			updateBlockBlendBuffer(const TerrainBlockBlendData& blockBlendData);

		/*
		@describe 
		@param 
		@return 
		*/
		void			setTileTexture(index_t index, const TString& texture);

		/**
		@describe 
		@param
		@return
		*/
		void			cacheOut();

		/**
		@describe 
		@param
		@return
		*/
		void			cacheIn();

		/**
		@describe 
		@param
		@return
		*/
		void			updateMaterialInstances();

		/** @brief  */
		const TStringList& getTileTextureList() const	{return mAtlas->getTextureList();}

		/** @brief  */
		inline bool		isTileTextureInited() const		{return mTextureInited;}

		/** @brief  */
		inline void				setTileIndex(uint16 x, uint16 z)
		{
			mTileX = x;
			mTileZ = z;
		}

		/** @brief  */
		inline uint16			getTileIndexX() const {return mTileX;}
		/** @brief  */
		inline uint16			getTileIndexZ() const {return mTileZ;}

	protected:
		/*
		@describe
		@param
		@return
		*/
		virtual void	notifySpaceChange();

		/*
		@describe add renderables to render buffer
		@return
		@note  this function will be called if CUF_*_VISIBLE is set
		*/
		virtual void	updateRender(IRenderQueue* /*buffer*/)	{}

		/*
		@describe
		@param
		@return
		*/
		void			cleanUp();

		/*
		@describe 
		@param 
		@return 
		*/
		void			initTextureData(const HTEXTURE& blendMap, const HTEXTURE& normalMap, const HTEXTURE& layerMap,
			const HTERRAINATLAS& atlas, const HTERRAINATLAS& normalAtlas, 
			const uint8* blendData, const uint8* normalData, TERRAIN_LAYER* layerData,
			bool textureInited, bool rebuildTexture);

		/** @brief  */
		TerrainTile*	getNeighbor(int16 offsetX, int16 offsetZ) const;

		/** @brief  */
		void			setupShaderParameters();

		/** @brief  */
		void loopBegin(const Event&)
		{
			if (mTileInfo.mUpdatedMask != NULL)
			{
				const size_t blockPerSide = this->getConfigManager()->getBlocksPerTileSide();
				std::memset(mTileInfo.mUpdatedMask, 0, blockPerSide*blockPerSide);
			}
			msPerframeConstantUpdated = false;
		}

		typedef Vector<TerrainBlock*>	BlockRowData;
		typedef Vector<BlockRowData>	BlockData;
		typedef Vector<HMATERIALINSTANCE> MaterialLODInstances;

		BlockData				mBlockData;
		MaterialLODInstances	mMaterials;

		HTERRAINATLAS	mAtlas;
		HTERRAINATLAS	mNormalAtlas;
		uint16			mTileX;
		uint16			mTileZ;

		bool			mTextureInited;

		static bool		msPerframeConstantUpdated;
		static ShaderVariableMap::Handles msCameraPosHandle;
	};//class TerrainTile
	
}//namespace Blade


#endif //__Blade_Terrain_h__