/********************************************************************
	created:	2010/05/14
	filename: 	TerrainBufferManager.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_TerrainBufferManager_h__
#define __Blade_TerrainBufferManager_h__
#include <Singleton.h>
#include <RefCount.h>
#include <utility/BladeContainer.h>
#include <utility/FixedArray.h>
#include <interface/public/graphics/IIndexBuffer.h>
#include <interface/public/graphics/IVertexBuffer.h>
#include "TerrainConfigManager.h"


namespace Blade
{
	//deprecated
	typedef union UBlockAtlas
	{
		uint32 mVertexAtlas;
		struct  
		{
			//for layers
			uint8 mAtlasIndex0;
			uint8 mAtlasIndex1;
			uint8 mAtlasIndex2;
			uint8 mAtlasIndex3;
		};
		uint8	mLayerAtlas[4];
	}BLOCK_ATLAS;

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class TerrainBufferManager : public Singleton<TerrainBufferManager>
	{
	public:
		TerrainBufferManager();
		~TerrainBufferManager();

		/*
		@describe
		@param
		@return
		*/
		const TerrainIndexGroup*		addIndexGroup();

		/*
		@describe 
		@param 
		@return 
		*/
		const TerrainQueryIndexGroup*	addQueryIndexGroup();

		/**
		@describe 
		@param
		@return
		*/
		const TerrainFixedIndexGroup*	addFixedIndexGroup();

		/*
		@describe
		@param
		@return
		*/
		void			releaseIndexGroup(const TerrainIndexGroup* group);

		/*
		@describe 
		@param 
		@return 
		*/
		void			releaseQueryIndexGroup(const TerrainQueryIndexGroup* group);

		/**
		@describe 
		@param
		@return
		*/
		void			releaseFixedIndexGroup(const TerrainFixedIndexGroup* group);

		/*
		@describe 
		@param 
		@return 
		*/
		bool			createVertexBuffer(IGraphicsResourceManager& manager,HVBUFFER& position,HVBUFFER& normal,
			size_t TileSize, TileAABBList& localAABB,
			const int16* heightBuffer, const uint8* normalBuffer) const;

		/*
		@describe 
		@param 
		@return 
		*/
		const HVBUFFER&	getHorizontalPositionBuffer();

		/*
		@describe 
		@param 
		@return 
		*/
		void			releaseHorizontalPositionBuffer();

		/*
		@describe 
		@param 
		@return 
		*/
		const TERRAIN_POSITION_DATA_XZ*	getSoftHorizontalPositionBuffer();

	protected:
		TerrainIndexGroup*				mIndexGroup;
		TerrainQueryIndexGroup*			mQueryIndexGroup;
		TerrainFixedIndexGroup*			mFixedIndexGroup;
		HVBUFFER						mPositionXZBuffer;
	};//class TerrainBufferManager
	
}//namespace Blade


#endif //__Blade_TerrainBufferManager_h__