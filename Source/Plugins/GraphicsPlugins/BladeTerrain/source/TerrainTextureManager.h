/********************************************************************
	created:	2012/04/02
	filename: 	TerrainTextureManager.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_TerrainTextureManager_h__
#define __Blade_TerrainTextureManager_h__

#include <Singleton.h>
#include <utility/StringList.h>
#include <interface/TerrainBlendData.h>
#include <interface/public/graphics/IImage.h>
#include "TerrainConfigManager.h"
#include "TerrainAtlas.h"

//use static compiling option instead of runtime config:
//this is because these options will affect the work flow of artist
//so when you just start your project, you should know your art spec
//and select the right option you need

#define TERRAIN_TEXTURE_COMPRESSED 1

namespace Blade
{

#if TERRAIN_TEXTURE_COMPRESSED
	#define TERRAIN_PIXELFORMAT	(IGraphicsResourceManager::getSingleton().getGraphicsConfig().CompressedRGB)
	#define TERRAIN_NORMAL_FORMAT (IGraphicsResourceManager::getSingleton().getGraphicsConfig().CompressedNormal)
	#define TERRAIN_BLEND_FORMAT (PF_B5G5R5A1)
#else
	#define TERRAIN_PIXELFORMAT	(IGraphicsResourceManager::getSingleton().getGraphicsConfig().ColorRGBA)
	#define TERRAIN_NORMAL_FORMAT (TERRAIN_PIXELFORMAT)	//our atlas border fixing only can handle RGBA
	#define TERRAIN_BLEND_FORMAT (IGraphicsResourceManager::getSingleton().getGraphicsConfig().ColorRGBA)
#endif


	class TerrainTextureManager : public Singleton<TerrainTextureManager>
	{
	public:
		TerrainTextureManager();
		~TerrainTextureManager();

		/*
		@describe build atlas with texture lists
		@param
		@return
		*/
		TerrainAtlas*	buildTerrainAtlas(const TStringParam& textureList, PixelFormat format) const;

		/*
		@describe build atlas with pre-load image buffer
		@param textures input image buffer array same size as textureList
		@return 
		*/
		TerrainAtlas*	buildTerrainAtlas(const TStringParam& textureList, const HIMAGE* images, PixelFormat format) const;

		/*
		@describe 
		@param 
		@return 
		*/
		bool			updateTerrainAtlas(TerrainAtlas* atlas, const TStringParam& textureList, const HIMAGE* images) const;

		/*
		@describe 
		@param 
		@return 
		*/
		HIMAGE			prepareSubTexture(const TString& path, PixelFormat format) const;

		/*
		@describe 
		@param 
		@return the raw image data
		*/
		void*			getImageData(const HIMAGE& hImage, size_t& pitch, size_t& width, size_t& height, size_t mipLODLevel = 0) const;


		/*
		@describe
		@param
		@return
		*/
		bool			addTextureToAtlas(TerrainAtlas* atlas, const HIMAGE* images, const TStringParam& textureList) const;

		/*
		@describe
		@param
		@return
		*/
		bool			addTextureToAtlas(TerrainAtlas* atlas, const TStringParam& textureList) const;

		/*
		@describe
		@param
		@return
		*/
		bool			swapAtlasSubTexture(TerrainAtlas* atlas,LAYER_ID index0, LAYER_ID index1) const;

		/*
		@describe
		@param
		@return
		*/
		bool			setAtlasSubTexture(TerrainAtlas* atlas,LAYER_ID index, const HIMAGE& hImage, const TString& imageName) const;
		inline bool		setAtlasSubTexture(TerrainAtlas* atlas,LAYER_ID index, const TString& texture)
		{
			HIMAGE hImage = this->prepareSubTexture(texture, atlas->getAtlasTexture()->getPixelFormat());
			return this->setAtlasSubTexture(atlas, index, hImage, texture);
		}

		/*
		@describe
		@param
		@return
		*/
		HTEXTURE		createBlendTexture(const HIMAGE& blendImage) const;

		/*
		@describe 
		@param 
		@return 
		*/
		HIMAGE	createBlendImage(const uint8* blendBuffer) const;

		/*
		@describe
		@param
		@return
		*/
		bool			updateBlockBlendTexture(const HTEXTURE& hBlendTexture, const TerrainBlockBlendData& blockBlendData,const uint8* tileBlendBuffer) const;

		/*
		@describe 
		@param 
		@return 
		*/
		HTEXTURE		createNormalTexture(const HIMAGE& normalImage) const;

		/*
		@describe 
		@param 
		@return 
		*/
		HIMAGE			createNormalImage(const uint8* normalBuffer, size_t normalSize) const;

		/*
		@describe 
		@param 
		@return 
		*/
		bool			updateNormalTexture(const HTEXTURE& normalMap, const uint8* normalData, size_t size) const
		{
			assert( normalMap->getWidth() == normalMap->getHeight() );
			HIMAGE hImage = this->createNormalImage(normalData, size);
			return IGraphicsResourceManager::getSingleton().updateTexture( *hImage, *normalMap);
		}

		/*
		@describe create layer index map, note that this map cannot be mip-mapped, because
		the index(int) data is very sensitive
		@param 
		@return 
		*/
		HTEXTURE		createLayerTexture(const TERRAIN_LAYER* layerBuffer, bool convert = true) const;

		/*
		@describe 
		@param 
		@return 
		*/
		bool			updateLayerTexture(const HTEXTURE& layerMap, const TERRAIN_LAYER* layerBuffer, bool convert = true) const;

		/**
		@describe 
		@param
		@return
		*/
		bool			convertLayerBuffer(const TERRAIN_LAYER* layerBuffer, TERRAIN_LAYER* layer);

		/*
		@describe 
		@param 
		@return 
		*/
		bool			updateBlockLayerTexture(const HTEXTURE& layerMap, const TerrainBlockBlendData& blockBlendData, const TERRAIN_LAYER* layerData) const;

		/*
		@describe 
		@param 
		@return 
		*/
		inline	PixelFormat	getAtlasFormat() const		{return TERRAIN_PIXELFORMAT;}

	protected:
		/** @brief  */
		void	fixAtlasBorderPixelRGBA(const HIMAGE& hSrcImage, size_t subTextureSize, size_t pixelsize, scalar pixelRate = 1.0f/128.f) const;

		/** @brief copy left/top border pixel to right/bottom to fix atlas seams */
		void	fixAtlasBorderPixelCompressed(const HIMAGE& hSrcImage, size_t subTextureSize, size_t pixelsize, scalar pixelRate = 1.0f/128.f) const;

		/** @brief  */
		bool	updateAtlasTexture(const HTEXTURE& hTexture, const HIMAGE& hSrcImage,
			size_t subX, size_t subY, size_t subTextureSize) const;
	};//class TerrainTextureManager
	

}//namespace Blade

#endif //__Blade_TerrainTextureManager_h__