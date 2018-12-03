/********************************************************************
	created:	2011/04/13
	filename: 	ITerrainConfigManager.h
	author:		Crazii
	purpose:	this interface is  used for other modules to access som data
*********************************************************************/
#ifndef __Blade_ITerrainConfigManager_h__
#define __Blade_ITerrainConfigManager_h__
#include <interface/InterfaceSingleton.h>
#include <interface/public/graphics/IGraphicsType.h>
#include <interface/public/ISerializable.h>
#include <interface/IConfig.h>
#include <BladeTerrain.h>

namespace Blade
{
	class IGraphicsScene;
	 
	//////////////////////////////////////////////////////////////////////////
	//global data (changeable during runtime)
	typedef struct STerrainGlobalConfig : public EmptySerializable
	{
		TString				mTextureQuality;
		TString				mBatchCominer;
		TString				mTraingleIndexFormat;	//index generator
		size_t				mMaxLODLevels;			//maximum LOD level based on block size
		fp32				mBlendNormalScale;
		uint16				mTileTextureSize;
		uint8				mBlockSize;
		uint8				mLODLevels;				//current configured LOD level count, ranges [0, mMaxLODLevels]
		uint8				mFlatLODLevel;			//LOD level for flat planes: performance optimize
		uint8				mCliffLODLevel;			//LOD level for cliff, to avoid detail lost/over-deformation
	}TERRAIN_CFG;

	//global terrain info this may not frequently change if terrain tiles are created
	typedef struct STerrainInitInfo : public EmptySerializable
	{
		//runtime data
		uint32				mTileSize;
		uint32				mTileCount;				//total tiles including invisible tiles
		uint32				mVisibleTileCount;		//visible tile count per side
		bool				mEditingMode;
		bool				mEnableMultiLoading;	//switch to turn on/off multi tile dynamic loading, default:true,
													//this is used for editing mode disable loading for a while at runtime
		bool				mEnableQueryData;
		size_t				mBlocksPerTileSide;		//block count on one tile's each side
		size_t				mBlocksPerTile;			//block count on one whole tile
		size_t				mTileVertexCount;
		size_t				mBlockVertexCount;
		size_t				mMaxBlockIndexCount;	//maximum index count per block of all block LODs

		//serialization data (cannot change after create)
		uint32				mBlendBufferScale;		//blend map scale (based on tile size).
													//it will affect terrain resource data(blend buffer), thus cannot be changed
		fp32				mTerrainScale[3];		//height scale
		TString				mTerrainPath;
		TString				mTerrainResPrefix;

		//TODO: belows are not add into config through user interface yet, add them
		uint8				mTileTextureCount;		//texture per atlas side
		uint8				mTileTextureMinMip;		//minimum mip size of a sub texture (i.e. 4x4, 1x1)
		uint8				mTileUVMultiple;		//texture tiling repeat count 

		const scalar&	getMaxHeight(index_t) const { return mTerrainScale[1]; }
		bool			setMaxHeight(index_t, const scalar& maxheight) { mTerrainScale[1] = maxheight; return true; }
	}TERRAIN_INFO;


	class ITerrainConfigManager : public InterfaceSingleton<ITerrainConfigManager>
	{
	public:

		virtual ~ITerrainConfigManager()	{}
		/*
		@describe 
		@param 
		@return 
		*/
		virtual const HCONFIG&		getTerrainCreationConfig() const = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const HCONFIG&		getTerrainGlobalConfig() const = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual IGraphicsType*		getTerrainGraphicsType() const = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual void				initialize(IGraphicsScene* scene, size_t pageSize, size_t pageCount, size_t visiblePage,
			const TString& pathPrefix = TString::EMPTY, const TString& resourcePrefix = TString::EMPTY) = 0;

		/*
		@describe callback for config data through UI
		@param 
		@return 
		*/
		virtual void				notifyGlobalConfigChanged() = 0;

		/*
		@describe maximum index count for all tiles in current config
		@param 
		@return 
		*/
		virtual size_t				calculateTotalIndexCount() const = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void				setEditingMode(bool enable) = 0;

		/*
		@describe enable runtime multi thread loading or not
		this helps editor tools with a synchronous state
		@param 
		@return 
		*/
		virtual void				enableMultiLoading(bool enable) = 0;

		/*
		@describe 
		@param
		@return
		*/
		virtual bool				isUseVertexNormal() const = 0;

		/*
		@describe
		@param
		@return
		*/
		inline const TERRAIN_INFO&	getTerrainInfo() const
		{
			return mTerrainInfo;
		}

		/*
		@describe
		@param
		@return
		*/
		inline const TERRAIN_CFG&	getTerrainConfig() const
		{
			return mTerrainConfig;
		}

		/** @brief  */
		inline bool			isEditingModeEnabled() const
		{
			return this->getTerrainInfo().mEditingMode;
		}

		/** @brief  */
		inline bool			isMultiLoadingEnabled() const
		{
			return this->getTerrainInfo().mEnableMultiLoading;
		}

		/** @brief  */
		inline bool			isQueryDataEnabled() const
		{
			return this->getTerrainInfo().mEnableQueryData;
		}

		/** @brief  */
		inline size_t		getTerrainTileSize() const
		{
			return this->getTerrainInfo().mTileSize;
		}

		/** @brief  */
		inline size_t		getTerrainTileCount() const
		{
			return this->getTerrainInfo().mTileCount;
		}

		/** @brief  */
		inline size_t		getVisibleTileCount() const
		{
			return this->getTerrainInfo().mVisibleTileCount;
		}

		/** @brief  */
		inline size_t		getBlocksPerTileSide() const
		{
			return this->getTerrainInfo().mBlocksPerTileSide;
		}

		/** @brief  */
		inline size_t		getBlocksPerTile() const
		{
			return this->getTerrainInfo().mBlocksPerTile;
		}

		/** @brief  */
		inline size_t		getTileVertexCount() const
		{
			return this->getTerrainInfo().mTileVertexCount;
		}

		/** @brief  */
		inline size_t		getMaxBlockIndexCount() const
		{
			return this->getTerrainInfo().mMaxBlockIndexCount;
		}

		/** @brief  */
		inline size_t		getBlendBufferSize() const
		{
			return size_t(this->getTerrainInfo().mBlendBufferScale * this->getTerrainTileSize());
		}

		/** @brief */
		inline scalar		getBlendNormalScale() const
		{
			return this->getTerrainConfig().mBlendNormalScale;
		}

		/** @brief  */
		inline size_t		getTextureSize() const
		{
			return this->getTerrainConfig().mTileTextureSize;
		}

		/** @brief */
		inline size_t		getTerrainBlockSize() const
		{
			return this->getTerrainConfig().mBlockSize;
		}

		/** @brief maximum LOD based on block size */
		inline index_t		getMaxLODLevel() const
		{
			return this->getTerrainConfig().mMaxLODLevels;
		}
		/** @brief  */
		inline const TString&	getTextureQuality() const
		{
			return this->getTerrainConfig().mTextureQuality;
		}

		/** @brief current max LOD limit */
		inline index_t		getLODLevel() const
		{
			return this->getTerrainConfig().mLODLevels;
		}

		/** @brief  */
		inline index_t		getFlatLODLevel() const
		{
			return this->getTerrainConfig().mFlatLODLevel;
		}

		/** @brief  */
		inline index_t		getCliffLODLevel() const
		{
			return this->getTerrainConfig().mCliffLODLevel;
		}

		/** @brief */
		inline size_t		getTerrainBlockVertexCount() const
		{
			return this->getTerrainInfo().mBlockVertexCount;
		}

		/** @brief  */
		inline const TString& getTerrainResourcePrefix() const
		{
			return this->getTerrainInfo().mTerrainResPrefix;
		}

		/** @brief  */
		inline const TString& getTerrainPath() const
		{
			return this->getTerrainInfo().mTerrainPath;
		}

		/** @brief  */
		inline size_t		getTileUVMultiple() const
		{
			return this->getTerrainInfo().mTileUVMultiple * this->getTerrainConfig().mBlockSize / 16u;
		}

		/** @brief  */
		inline size_t		getTextureCount() const
		{
			return this->getTerrainInfo().mTileTextureCount;
		}
		/** @brief  */
		inline size_t		getMinMipSize() const
		{
			return this->getTerrainInfo().mTileTextureMinMip;
		}

		/** @brief  */
		inline const float*		getTerrainScale() const
		{
			return this->getTerrainInfo().mTerrainScale;
		}

		/** @brief  */
		inline fp32			getTerrainHeightScale() const
		{
			return this->getTerrainInfo().mTerrainScale[1];
		}

	protected:
		TERRAIN_CFG		mTerrainConfig;
		TERRAIN_INFO	mTerrainInfo;
	};//class ITerrainConfigManager

	extern template class BLADE_TERRAIN_API Factory<ITerrainConfigManager>;

}//namespace Blade



#endif // __Blade_ITerrainConfigManager_h__
