/********************************************************************
	created:	2012/03/29
	filename: 	TerrainBatchRenderable.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_TerrainBatchRenderable_h__
#define __Blade_TerrainBatchRenderable_h__
#include <interface/public/IRenderable.h>
#include <MaterialInstance.h>
#include "../TerrainTile.h"
#include "../TerrainConfigManager.h"

namespace Blade
{

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class TerrainBatchRenderable : public IRenderable , public TempAllocatable
	{
	public:
		TerrainBatchRenderable(TerrainTile* tile,const MaterialInstance* material)
			:mTile(tile)
			,mTransform(Matrix44::IDENTITY)
		{
			mTransform.setTranslation( mTile->getTileInfo().mWorldTransform.getTranslation() );
			mMaterial = material;
		}

		~TerrainBatchRenderable()												{}


		/************************************************************************/
		/* IRenderable interface                                                                      */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual RenderType&				getRenderType() const
		{
			return TerrainConfigManager::getRenderType();
		}

		/*
		@describe
		@param
		@return
		*/
		virtual const GraphicsGeometry&	getGeometry() const
		{
			return mGeometry;
		}

		/*
		@describe
		@param
		@return
		*/
		virtual const MaterialInstance*	getMaterial() const
		{
			return mMaterial;
		}

		/*
		@describe
		@param
		@return
		*/
		virtual const Matrix44&			getWorldTransform() const
		{
			return mTransform;
		}

		/**
		@describe get hosted content
		@param
		@return
		*/
		virtual ISpaceContent*			getSpaceContent() const { return mTile; }

		/**
		@describe 
		@param
		@return
		*/
		virtual const AABB&	getWorldBounding() const { return mWorldAABB; }

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief */
		void							setupGeometry(IIndexBuffer* pIndexBuffer,size_t indexCount,size_t indexStart = 0)
		{
			assert( pIndexBuffer != NULL );
			const TERRAIN_INFO& gti = TerrainConfigManager::getSingleton().getTerrainInfo();
			const TILE_INFO&	ti = mTile->getTileInfo();


			mGeometry.mVertexDecl = TerrainConfigManager::getRenderType().getVertexDeclaration();
			mGeometry.mVertexSource = ti.mVertexSource;
			mGeometry.mPrimitiveType = GraphicsGeometry::GPT_TRIANGLE_LIST;
			mGeometry.useIndexBuffer(true);

			mGeometry.mIndexStart = (uint32)indexStart;
			mGeometry.mIndexCount = (uint32)indexCount;
			mGeometry.mVertexStart = 0;
			mGeometry.mVertexCount = (uint32)gti.mTileVertexCount;
			mGeometry.mIndexBuffer = pIndexBuffer;
		}

		/** @brief  */
		void					addIndexCount(size_t additionalCount)
		{
			mGeometry.mIndexCount += (uint32)additionalCount;
		}

		/** @brief  */
		void					setWorldBounding(const AABB& bounding)
		{
			mWorldAABB = bounding;
		}

	protected:
		TerrainTile*				mTile;
		const MaterialInstance*		mMaterial;
		GraphicsGeometry			mGeometry;
		AABB				mWorldAABB;
		Matrix44					mTransform;
	};

	

}//namespace Blade


#endif //__Blade_TerrainBatchRenderable_h__