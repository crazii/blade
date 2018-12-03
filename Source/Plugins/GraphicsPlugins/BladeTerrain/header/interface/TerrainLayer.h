/********************************************************************
	created:	2013/05/21
	filename: 	TerrainLayer.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_TerrainLayer_h__
#define __Blade_TerrainLayer_h__
#include <BladeTerrain.h>

namespace Blade
{

	struct STerrainLayer
	{
	public:
		static const int	BLEND_TEXTURE_INDEX_OFFSET = 1;
		static const size_t MAX_LAYER_COUNT = 4;
		static const LAYER_ID MAX_LAYER_ID = 15;
		union
		{
			uint16	mVertexAtlas;
			struct  
			{
				uint8 mAtlasIndex0 : 4;
				uint8 mAtlasIndex1 : 4;
				uint8 mAtlasIndex2 : 4;
				uint8 mAtlasIndex3 : 4;
			};
		};

		/** @brief  */
		inline size_t	getLayerCount()	const
		{
			if( mVertexAtlas == uint16(-1) )
				return 0;
			else if( mVertexAtlas == 0 )
				return 1;
			else
			{
				assert( !(mAtlasIndex0 == mAtlasIndex1
					&& mAtlasIndex1 == mAtlasIndex2
					&& mAtlasIndex2 == mAtlasIndex3) );
				if( mAtlasIndex1 == mAtlasIndex2 && mAtlasIndex2 == mAtlasIndex3 )
					return 2;
				else if( mAtlasIndex2 == mAtlasIndex3 )
					return 3;
				else
					return 4;
			}
		}

		/** @brief get layer ID by layer index */
		inline LAYER_ID	getLayer(index_t index) const
		{
			if( index > MAX_LAYER_COUNT )
				return INVALID_LAYER_ID;
			if( index == 0 )
				return mAtlasIndex0;
			else if( index == 1 )
				return mAtlasIndex1;
			else if( index == 2)
				return mAtlasIndex2;
			else
				return mAtlasIndex3;
		}

		/** @brief get layer index (0-3) from layer ID */
		inline index_t	findLayer(LAYER_ID layerID) const
		{
			assert(layerID <= 0xF );
			size_t count = this->getLayerCount();
			for(size_t i = 0; i < count; ++i)
			{
				if( this->getLayer(i) == layerID )
					return i;
			}
			return INVALID_INDEX;
		}

		/** @brief  */
		inline index_t			addLayer(LAYER_ID layerID)
		{
			if( layerID == INVALID_LAYER_ID)
				return INVALID_INDEX;
			if( layerID > 0xF )
			{
				assert(false);
				return INVALID_INDEX;
			}

			size_t count = this->getLayerCount();
			if( count < MAX_LAYER_COUNT && this->findLayer(layerID) == INVALID_INDEX )
			{
				if( count == 0 )
				{
					assert(layerID == 0);
					mAtlasIndex0 = 0;
				}
				else if( count == 1)
					mAtlasIndex1 = mAtlasIndex2 = mAtlasIndex3 = layerID;
				else if( count == 2 )
					mAtlasIndex2 = mAtlasIndex3 = layerID;
				else /*if( count == 3 )*/
					mAtlasIndex3 = layerID;
				return count;
			}
			else
			{
				return INVALID_INDEX;
			}
		}

		/** @brief  */
		inline bool			removeLayer(LAYER_ID id)
		{
			size_t count = this->getLayerCount();
			if (count == 0)
				return false;
			index_t index = this->findLayer(id);
			if (index == INVALID_INDEX)
				return false;

			assert(index < count);
			while (index + 1 < count)
			{
				if (index == 0)
					mAtlasIndex0 = mAtlasIndex1;
				else if (index == 1)
					mAtlasIndex1 = mAtlasIndex2;
				else if (index == 2)
					mAtlasIndex2 = mAtlasIndex3;
			}

			//fill last layer
			assert(index + 1 == count);
			if (index == 0)
				mVertexAtlas = uint16(-1);
			else if (index == 1)
				mAtlasIndex1 = mAtlasIndex2 = mAtlasIndex3 = mAtlasIndex0;
			else if (index == 2)
				mAtlasIndex2 = mAtlasIndex3 = mAtlasIndex1;
			else if (index == 3)
				mAtlasIndex3 = mAtlasIndex2;
			else
				assert(false);

			assert(this->getLayerCount() + 1 == count);

			return true;
		}

	};

	typedef STerrainLayer TERRAIN_LAYER;
	static_assert(sizeof(TERRAIN_LAYER) == 2, "size error");
	
}//namespace Blade

#endif//__Blade_TerrainLayer_h__