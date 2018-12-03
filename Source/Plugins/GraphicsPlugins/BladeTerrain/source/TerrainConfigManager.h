/********************************************************************
	created:	2011/04/13
	filename: 	TerrainConfigManager.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_TerrainConfigManager_h__
#define __Blade_TerrainConfigManager_h__
#include <utility/BladeContainer.h>
#include <utility/IOBuffer.h>
#include <interface/IConfig.h>
#include <interface/public/graphics/IVertexDeclaration.h>
#include <interface/ITerrainConfigManager.h>
#include "TerrainType.h"
#include "terrain_interface/ITerrainTile.h"
#include "terrain_interface/ITerrainBatchCombiner.h"

namespace Blade
{
	class Event;

	class LODDiff
	{
	public:
		LODDiff() { diff = 0; }

		/** @brief  */
		inline void	setUpDiff()				{up = 0xFF;}

		/** @brief  */
		inline void	clearUpDiff()			{up = 0x0;}

		/** @brief  */
		inline bool	isUpDiff() const		{return (up&0x01) != 0;}

		/** @brief  */
		inline void	setLeftDiff()			{left = 0xFF;}

		/** @brief  */
		inline void	clearLeftDiff()			{left = 0x0;}

		/** @brief  */
		inline bool	isLeftDiff() const		{return (left&0x02) != 0;}

		/** @brief  */
		inline void	setDownDiff()			{down = 0xFF;}

		/** @brief  */
		inline void	clearDownDiff()			{down = 0x0;}

		/** @brief  */
		inline bool	isDownDiff() const		{return (down&0x04) != 0;}

		/** @brief  */
		inline void	setRightDiff()			{right = 0xFF;}

		/** @brief  */
		inline void	clearRightDiff()		{right = 0x0;}

		/** @brief  */
		inline bool	isRightDiff() const		{return (right&0x08) != 0;}


		/** @brief  */
		inline bool		hasLevelDifference() const
		{
			return this->isUpDiff() || this->isLeftDiff() || this->isRightDiff() || this->isDownDiff();
		}

		/** @brief  */
		inline size_t	getDiffSideCount() const
		{
			size_t count = 0;
			if(this->isUpDiff() )
				++count;
			if(this->isLeftDiff() )
				++count;
			if(this->isDownDiff() )
				++count;
			if(this->isRightDiff() )
				++count;
			return count;
		}

		/** @brief */
		static inline LOD_DI	generateDifferenceIndex(const LODDiff& difference)
		{
			return LOD_DI((difference.up & 0x1) | (difference.left & 0x2) | (difference.down & 0x4) | (difference.right & 0x8));
		}

		/** @brief */
		static inline LODDiff	generateLODDifference(LOD_DI LODIndex)
		{
			assert(LODIndex >= LODDI_NONE && LODIndex <= LODDI_ALL);
			LODDiff diff;
			diff.up = LODIndex & 0x01u;
			diff.left = LODIndex & 0x02u;
			diff.down = LODIndex & 0x04u;
			diff.right = LODIndex & 0x08u;
			return diff;
		}

	protected:
		union
		{
			struct
			{
				uint8 up;
				uint8 left;
				uint8 down;
				uint8 right;
			};
			uint32	diff;
		};
	};

	class TerrainTile;

	class TerrainConfigManager : public ITerrainConfigManager, public Material::IListener, public Singleton<TerrainConfigManager>
	{
	public:
		static const TString GLOBAL_CONFIG_TYPENAME;
		static const TString CREATION_CONFIG_TYPENAME;
	public:
		using Singleton<TerrainConfigManager>::getSingleton;
		using Singleton<TerrainConfigManager>::getSingletonPtr;
	public:
		TerrainConfigManager();
		~TerrainConfigManager();

		/************************************************************************/
		/* ITerrainConfigManager interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual const HCONFIG&		getTerrainCreationConfig() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const HCONFIG&		getTerrainGlobalConfig() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual IGraphicsType*		getTerrainGraphicsType() const
		{
			return &msTerrainType;
		}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void				initialize(IGraphicsScene* scene, size_t pageSize, size_t pageCount, size_t visiblePage,
			const TString& pathPrefix = TString::EMPTY, const TString& resourcePrefix = TString::EMPTY);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void				notifyGlobalConfigChanged();

		/*
		@describe 
		@param 
		@return 
		*/
		virtual size_t				calculateTotalIndexCount() const;

		/** @brief  */
		virtual void				setEditingMode(bool enable)
		{
			mTerrainInfo.mEditingMode = enable;
		}

		/** @brief  */
		virtual void				enableMultiLoading(bool enable)
		{
			mTerrainInfo.mEnableMultiLoading = enable;
		}

		/** @brief  */
		virtual bool				isUseVertexNormal() const;

		/************************************************************************/
		/* Material::IListener                                                                     */
		/************************************************************************/
		/** @brief  */
		void						onActivateTechnique(Technique* old, Technique* now);

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param
		@return
		*/
		inline void			setTextureQuality(const TString& quality)
		{
			assert(quality == TerrainConsts::TEXTURE_QUALITY_HIGH || quality == TerrainConsts::TEXTURE_QUALITY_MIDDLE || quality == TerrainConsts::TEXTURE_QUALITY_LOW);
			mTerrainConfig.mTextureQuality = quality;
		}

		/*
		@describe get tile material if there is any tile instance added (assumes all tiles have the same material)
		@param 
		@return NULL if there is no tile instances recorded
		*/
		const Material*		getTileMaterial() const;

		/** @brief  */
		inline void			updateGlobalTerrainInfo(const TERRAIN_INFO& info)
		{
			static_cast<TERRAIN_INFO&>(mTerrainInfo) = info;
		}

		/** @brief */
		IIndexGenerator*getIndexGenerator() const	{return mLODComparator;}

		/** @brief  */
		void			clearBatchCombiner()		{mBatchCominer = NULL;}

		/** @brief  */
		ITerrainBatchCombiner*	getBatchCombiner();

		/** @brief */
		uint8			getLODLevelBySquaredDistance(scalar fSquaredDistance);

		/** @brief */
		uint8			getLODLevelByDistance(scalar fDistance);

		/** @brief  */
		void			onRenderDeviceReady(const Event& evt);

		/** @brief  */
		void			onRenderDeviceClose(const Event& evt);

		/** @brief get used & unused(cached) tile count */
		size_t			getTileCount() const { return mTiles.size(); }

		/** @brief get used tile count */
		size_t			getCurrentTileCount() const;

		/** @brief  */
		TerrainTile*	allocTile();

		/** @brief return if any tile is in use */
		bool			freeTile(TerrainTile* tile);

		/** @brief get used tile */
		TerrainTile*	getTile(index_t x, index_t z);

		/** @brief  */
		void			releaseTiles();

		/** @brief  */
		static TerrainType& getRenderType() { return msTerrainType; }

		/** @brief  */
		inline uint32* getBlockPartitions()
		{
			assert(mBlockSpacePartitions != NULL);
			return mBlockSpacePartitions;
		}

	protected:
		/** @brief  */
		void			resetBlockPartitions(bool doReset);

		/** @brief  */
		bool			setIndexGenerator(const TString& name);

		/** @brief  */
		bool			setBatchCombiner(const TString& name);

	protected:
		typedef StaticHandle<IIndexGenerator> HLC;
		typedef StaticHandle<ITerrainBatchCombiner> HBC;
		typedef Set<ITerrainTile*>	TileSet;

		TileSet		mTiles;

		//real time data
		TString		mBatchCombinationName;
		TString		mIndexGeneratorName;
		HLC			mLODComparator;
		HBC			mBatchCominer;

		HCONFIG		mGlobalConfig;
		HCONFIG		mCreationConfig;

		uint32*		mBlockSpacePartitions;
		TString		mLastPartitionSpace;
		size_t		mLastParitionBlock;
		bool		mLastVertexNormal;

		static TerrainType	msTerrainType;

		friend class TerrainPlugin;
	};//class TerrainConfigManager

}//namespace Blade



#endif // __Blade_TerrainConfigManager_h__