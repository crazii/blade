/********************************************************************
	created:	2011/04/06
	filename: 	ITerrainTile.h
	author:		Crazii
	purpose:	internal interface, not public
*********************************************************************/
#ifndef __Blade_ITerrainTile_h__
#define __Blade_ITerrainTile_h__
#include <BladeTerrain.h>
#include <math/Vector3.h>
#include <math/Matrix44.h>
#include <utility/BladeContainer.h>
#include <interface/public/graphics/IVertexSource.h>
#include <interface/public/graphics/IIndexBuffer.h>
#include <interface/IRenderScene.h>
#include <MaterialInstance.h>
#include <interface/TerrainLayer.h>
#include <interface/ITerrainConfigManager.h>
#include <TerrainShaderShared.inl>
#include <BladeGraphics_blang.h>
#include "TerrainIndexGroup.h"

namespace Blade
{
	class TerrainBlock;
	class IRenderQueue;
	class GraphicsGeometry;

	namespace TerrainConsts
	{
		static const TString TEXTURE_QUALITY_HIGH = BXLang(BLANG_HIGH);
		static const TString TEXTURE_QUALITY_MIDDLE = BXLang(BLANG_MIDDLE);
		static const TString TEXTURE_QUALITY_LOW = BXLang(BLANG_LOW);
		static const TString EMPTY_TEXTURE_FILE = BTString("image:empty");
	}

#define COMPACT_TERRAIN_POSITION 1

	static const scalar	TERRAIN_MOD_HEIGHT = (scalar)int32(scalar(uint16(-1))*0.5f);

#if COMPACT_TERRAIN_POSITION
#	define TERRAIN_POSITION_XZ_FORMAT VET_UBYTE4

#	if BLADE_TERRAIN_ENABLE_UVSCALE
#		define TERRAIN_POSITION_Y_FORMAT VET_SHORT4
#	else
#		define TERRAIN_POSITION_Y_FORMAT VET_SHORT2
#	endif

	typedef struct STerrainPositionElementXZ
	{
		uint8 x;	//block index x
		uint8 z;	//block index z
		uint8 lx;
		uint8 lz;

		/** @brief  */
		inline size_t	getX() const
		{
			return ITerrainConfigManager::getSingleton().getTerrainBlockSize() * x + lx;
		}
		/** @brief  */
		inline size_t	getZ() const
		{
			return ITerrainConfigManager::getSingleton().getTerrainBlockSize() * z + lz;
		}

	}TERRAIN_POSITION_DATA_XZ;
	typedef struct STerrainPositionElementY
	{
		int16 height;
		int16 heightLowLOD;
#if BLADE_TERRAIN_ENABLE_UVSCALE
		uint16 scaledU;
		uint16 scaledV;
		void setUTiling(fp32 scale) { scaledU = (uint16)scale; }
		void setVTiling(fp32 scale) { scaledV = (uint16)scale; }
		fp32 getUTiling() const { return (fp32)scaledU; }
		fp32 getVTiling() const { return (fp32)scaledV; }
#endif
		fp32 getHeight() const;
		int16 getHeight16() const				{ return height; }
		void setHeight(int16 Height)			{height = Height;}
		void setHeightLowLOD(int16 lowLODHeight){heightLowLOD = lowLODHeight;}
	}TERRAIN_POSITION_DATA_Y;
#else

#	define TERRAIN_POSITION_XZ_FORMAT VET_FLOAT4

#	if BLADE_TERRAIN_ENABLE_UVSCALE
#		define TERRAIN_POSITION_Y_FORMAT VET_FLOAT2
#	else
#		define TERRAIN_POSITION_Y_FORMAT VET_FLOAT4
#	endif
	typedef struct STerrainPositionElementXZ
	{
		fp32 x;	//x
		fp32 z;	//z
		fp32 lx;
		fp32 lz;
	}TERRAIN_POSITION_DATA_XZ;
	typedef struct STerrainPositionElementY
	{
		fp32 height;
		fp32 heightLowLOD;
#if BLADE_TERRAIN_ENABLE_UVSCALE
		fp32 scaledU;
		fp32 scaledV;
		void setUTiling(fp32 scale) { scaledU = scale; }
		void setVTiling(fp32 scale) { scaledV = scale; }
		fp32 getUTiling() const { return scaledU; }
		fp32 getVTiling() const { return scaledV; }
#endif
		fp32 getHeight() const;
		int16 getHeight16() const { return height*TERRAIN_MOD_HEIGHT; }
		void setHeight(int16 Height)
		{
			height = (scalar)Height/ TERRAIN_MOD_HEIGHT;
		}
		void setHeightLowLOD(int16 lowLODHeight)
		{
			heightLowLOD = lowLODHeight/ TERRAIN_MOD_HEIGHT;
		}
	}TERRAIN_POSITION_DATA_Y;
#endif

	//vertex normal
	typedef struct STerrainNormalElement
	{
		uint8 x,y,z,w;
	}TERRAIN_NORMAL_DATA;

	struct BlockAABB
	{
		Box3	aab;
		index_t fixedLOD;
	};
	typedef TempVector<BlockAABB>	BlockAABBList;
	typedef TempVector<BlockAABBList>	TileAABBList;

	enum ETerrainMaterialLOD
	{
		//highest should be 0 for LOD.
		TML_HIGH = 0,
		TML_MIDDLE,
		TML_LOW,
	};

	//terrain tile information
	typedef struct STileInfo
	{
		ITerrainConfigManager* mConfigManager;	//cache singleton object, since frequently getSingleton() is a little bit slow, especially in some critial points like rendering
		IRenderScene*		mScene;
		const TerrainQueryIndexGroup*	mQueryIndexGroup;
		const TerrainIndexGroup*		mIndexGroup;
		const TerrainFixedIndexGroup*	mFixedGroup;
		Material*			mTileMaterial;
		HMATERIALINSTANCE	mTileMaterialInstance;
		HMATERIALINSTANCE*	mLODMaterials;		//cloned instances from mTileMaterialInstance, don't need update textures/anything since its full cloned/shared.
		HVERTEXSOURCE		mVertexSource;
		IRenderQueue*		mOutputBuffer;		//single pointer

		//resource data (linkage only), not valid on runtime. only valid in edit mode
		const int16*		mHeightData;
		const uint8*		mNormalData;
		const uint8*		mBlendData;
		TERRAIN_LAYER*		mLayerData;
		//end resource data linkage

		uint8*				mUpdatedMask;		//array pool
		GraphicsGeometry*	mRenderGeomBuffer;	//array pool

		Matrix44			mWorldTransform;
		uint8				mMaterialLODQualityLimit;	//ETerrainMaterialLOD. 0 means highest LOD(no limit). otherwise limit LOD to lower level to force lower quality
		uint8				mMaterialLODCount;			//actual total LOD count used by current profile/technique: length of mLODMaterials
		uint8				mUsed;						//caching tag
	}TILE_INFO;

	class ITerrainTile
	{
	public:
		virtual ~ITerrainTile() {}

		/** @brief  */
		virtual TerrainBlock*				getTerrainBlock(int x,int y) const = 0;

		/** @brief  */
		virtual const TERRAIN_POSITION_DATA_Y*getSoftHeightPositionData() const = 0;

		/** @brief  */
		virtual void						releaseIndexBuffer() = 0;

		/** @brief  */
		virtual void						buildIndexBuffer() = 0;

		/** @brief  */
		virtual void						rebuildBlocks() = 0;

		/** @brief  */
		virtual void						updateVertexBuffer() = 0;

		/** @brief  */
		virtual void						updateMaterialLOD(bool updateMat = false) = 0;

		/** @brief  */
		inline const TILE_INFO&		getTileInfo() const	{return mTileInfo;}

		/** @brief  */
		inline ITerrainConfigManager* getConfigManager() const { return mTileInfo.mConfigManager; }

		/** @brief  */
		inline bool					isUsed() const { return mTileInfo.mUsed != 0; }

		/** @brief  */
		inline void					setUsed(bool used) { mTileInfo.mUsed = used ? 1u : 0; }

		/** @brief  */
		inline const Vector3&	getStartPos() const { return mTileInfo.mWorldTransform.getTranslation(); }

		/** @brief  */
		inline IIndexBuffer*				getIndexBuffer(index_t LODLevel, LOD_DI diffIndex, size_t blockIndex, uint32& IndexStart, uint32& IndexCount) const
		{
			assert(LODLevel <= this->getConfigManager()->getMaxLODLevel());
			return mTileInfo.mIndexGroup->getIndexBuffer(LODLevel, diffIndex, blockIndex, IndexStart, IndexCount);
		}

		/** @brief  */
		inline IIndexBuffer*				getFixedIndexBuffer(index_t LODLevel, FixedLODDiff diff, size_t blockIndex, uint32& IndexStart, uint32& IndexCount) const
		{
			assert(LODLevel == this->getConfigManager()->getCliffLODLevel() || LODLevel == this->getConfigManager()->getFlatLODLevel());
			return mTileInfo.mFixedGroup->getIndexBuffer(*(this->getConfigManager()), LODLevel, diff, blockIndex, IndexStart, IndexCount);
		}

		/** @brief  */
		inline const TerrainQueryIndexGroup*	getQueryIndexBuffer() const
		{
			return mTileInfo.mQueryIndexGroup;
		}

		/** @brief get fixed LOD using bounding size. either local or world bounding is OK */
		static inline bool		getFixedLOD(const Box3& blockBounding, index_t& oriLOD)
		{
			index_t flat = ITerrainConfigManager::getSingleton().getFlatLODLevel();
			index_t cliff = ITerrainConfigManager::getSingleton().getCliffLODLevel();
			if (flat <= cliff)
				return false;

			static const float MIN_HALF_HEIGHT = 0.02f / 2.0f;
			static const float MIN_DYDX = 0.6f;
			scalar halfHeight = blockBounding.getHalfSize().y;
			scalar halfX = blockBounding.getHalfSize().x;
			if (halfHeight <= MIN_HALF_HEIGHT)
			{
				oriLOD = flat;
			}
			else if (halfHeight / halfX >= MIN_DYDX)
			{
				//use fixed LOD.
				//Note: another method is to set max LOD level (min detail) limit and recursive add 1 as max LOD for neighbors (decrease min detail limit).
				//but it has few problems
				//1.it won't work for flat LOD blocks: you cannot force lower block details, but only can upper the details
				//so float LOD blocks still have to use fixed LOD special indices.

				//2.a group of discrete max LOD blocks will cause a large range of high detail blocks, which
				//may cause performance hit:
				//
				// X is the min LOD level set, and M-n is recursively LOD. the whole range will be in high detail
				//
				//  M+1 |  X  | M+1 | M+2 | M+3
				// -----+-----+-----+-----+-----
				//  M+2 | M+1 | M+2 | M+1 | M+2
				// -----+-----+-----+-----+-----
				//  M+1 | M+2 | M+1 |  X  | M+1
				// -----+-----+-----+-----+-----
				//   X  | M+1 | M+2 | M+1 |  X
				// -----+-----+-----+-----+-----
				//  M+1 | M+2 | M+3 | M+2 | M+1
				//
				oriLOD = cliff;
				return true;
			}
			return false;
		}

	protected:
		TILE_INFO		mTileInfo;
	};

	//////////////////////////////////////////////////////////////////////////
	class ILODComparator
	{
	public:
		virtual ~ILODComparator() {}
		/*
		@describe
		@param
		@return true if need set LOD diff
		*/
		virtual indexdiff_t	compareLOD(index_t neighborLOD, index_t selfLOD) = 0;
	};

	class IIndexGenerator : public ILODComparator
	{
	public:
		virtual ~IIndexGenerator() {}

		/*
		@describe
		@param
		@return
		*/
		virtual TerrainIndexGroup*	createTileIndexBuffer() = 0;

		/*
		@describe create software index buffer for space query
		@param
		@return
		*/
		virtual TerrainQueryIndexGroup*	createBlockQueryIndexBuffer() = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual TerrainFixedIndexGroup*	createFixedIndexBuffer() = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual size_t				getLODMaxIndexCount(index_t LOD) const = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual int16				getLowLODHeight(const int16* tileHeightaMap, size_t& LOD,
			index_t blockX, index_t blockZ, index_t x, index_t z) const = 0;
	};

	extern template class Factory<IIndexGenerator>;
	typedef Factory<IIndexGenerator> IndexGeneratorFactory;

}//namespace Blade



#endif // __Blade_ITerrainTile_h__