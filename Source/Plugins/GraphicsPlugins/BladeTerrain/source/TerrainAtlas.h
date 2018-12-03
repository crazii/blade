/********************************************************************
	created:	2012/04/02
	filename: 	TerrainAtlas.h
	author:		Crazii
	purpose:	terrain texture atlas definition
*********************************************************************/
#ifndef __Blade_TerrainAtlas_h__
#define __Blade_TerrainAtlas_h__
#include <utility/StringList.h>
#include <interface/public/graphics/ITexture.h>

#include <BladeTerrain.h>

namespace Blade
{
	//@note the terrain atlas implemented here has the sub textures of the same size.

	class TerrainAtlas : public Allocatable
	{
	public:
		TerrainAtlas(size_t atlasSize, size_t subSize)
		{
			mAtlasSize = atlasSize;
			mSubSize = subSize;
			mSubCountPerSide = atlasSize/mSubSize;
			mMaxMipLevel = 0;
		}
		~TerrainAtlas()	{}

		/*
		@describe
		@param
		@return
		*/
		const HTEXTURE&		getAtlasTexture() const
		{
			return mAtlas;
		}

		/*
		@describe
		@param
		@return
		*/
		void				setAtlasTexture(const HTEXTURE& atlas)
		{
			mAtlas = atlas;
		}

		/*
		@describe
		@param
		@return
		*/
		size_t				getAtlasSize() const
		{
			return mAtlasSize;
		}

		/*
		@describe
		@param
		@return
		*/
		size_t				getSubTextureSize() const
		{
			return mSubSize;
		}

		/*
		@describe
		@param
		@return
		*/
		size_t				getSubTextureCountPerSide() const
		{
			return mSubCountPerSide;
		}

		/*
		@describe
		@param
		@return
		*/
		size_t				getSubTextureCount() const
		{
			return mTextureList.size();
		}

		/*
		@describe
		@param
		@return
		*/
		size_t				getMaxMipLevel() const
		{
			return mMaxMipLevel;
		}

		/*
		@describe
		@param
		@return
		*/
		void				setMaxMipLevel(size_t maxMip)
		{
			mMaxMipLevel = maxMip;
		}
		

		/** @brief  */
		bool				addTexture(const TString& texture)
		{
			if( mTextureList.find(texture) == INVALID_INDEX )
			{
				mTextureList.push_back(texture);
				return true;
			}
			else
				return false;
		}

		/** @brief  */
		LAYER_ID			findTexture(const TString& texture) const
		{
			return (LAYER_ID)mTextureList.find( texture );
		}

		/** @brief  */
		const TStringList&	getTextureList() const
		{
			return mTextureList;
		}

		/** @brief  */
		bool				swapTexture(index_t index0 ,index_t index1)
		{
			if( index0 < mTextureList.size() && index1 < mTextureList.size() )
			{
				mTextureList[index0].swap( mTextureList[index1] );
				return true;
			}
			return false;
		}

		/** @brief  */
		bool				setTexture(LAYER_ID layer,const TString& texture)
		{
			if( layer >= mTextureList.size() || texture == TString::EMPTY )
				return false;
			mTextureList[layer] = texture;
			return true;
		}

		/** @brief  */
		LAYER_ID		removeLastTexture()
		{
			if( mTextureList.size() == 0 )
				return INVALID_LAYER_ID;
			else
			{
				LAYER_ID id = (LAYER_ID )( mTextureList.size()-1 );
				mTextureList.erase( id );
				return id;
			}
		}

	protected:

		HTEXTURE	mAtlas;
		TStringList	mTextureList;
		size_t		mAtlasSize;
		size_t		mSubSize;
		size_t		mSubCountPerSide;
		size_t		mMaxMipLevel;
	};

	typedef Handle<TerrainAtlas> HTERRAINATLAS;
	

}//namespace Blade


#endif //__Blade_TerrainAtlas_h__