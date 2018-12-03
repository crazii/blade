/********************************************************************
	created:	2010/05/23
	filename: 	TextureResourceSerializer.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "TextureResourceSerializer.h"
#include <utility/MemoryStream.h>
#include <interface/public/graphics/IGraphicsResourceManager.h>
#include <TextureResource.h>
#include <interface/IGraphicsSystem.h>

namespace Blade
{

	//////////////////////////////////////////////////////////////////////////
	TextureResourceSerializer::TextureResourceSerializer(ETextureType type)
		:mTextureType(type)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	TextureResourceSerializer::~TextureResourceSerializer()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	bool	TextureResourceSerializer::loadResource(IResource* res,const HSTREAM& stream, const ParamList& params)
	{
		if( res == NULL || stream == NULL || !stream->isValid() )
			BLADE_EXCEPT(EXC_INVALID_PARAM,BTString("resource or stream data corrupted.") );
		else if( res->getType() != TextureResource::TEXTURE_RESOURCE_TYPE )
			BLADE_EXCEPT(EXC_INVALID_PARAM,BTString("resource type mismatch.") );
		TextureResource* resource = static_cast<TextureResource*>(res);

		BLADE_LOG(Information, TEXT("Loading texture: \"") 
			<< stream->getName() 
			<< TEXT("\"."));

		
		//IMG_ORIENT textureDir = IGraphicsResourceManager::getSingleton().getGraphicsConfig().TextureDir;
		IMG_ORIENT textureDir = IMGO_DEFAULT;
		PIXEL_ORDER byteOrder = IGraphicsResourceManager::getSingleton().getGraphicsConfig().DesiredOrder;

		PixelFormat format = PF_UNDEFINED;
		if( params[BTString("FORMAT")].isValid() )
		{
			format = (EPixelFormat)(int)params[BTString("FORMAT")];
			if( format.hasByteOrder() )
				format.setByteOrder( byteOrder );
			//record options
			resource->mFormat = format;
		}

		int LODlevel = IImage::MIPLOD_MAX;
		if( params[BTString("MIPMAP")].isValid() )
		{
			LODlevel = params[BTString("MIPMAP")];
			//record options
			if( LODlevel != IImage::MIPLOD_DEFAULT)
				resource->mMipMap = (int32)LODlevel;
		}

		bool linearColorSpace = false;
		if (params[BTString("LINEAR_SPACE")].isValid())
			linearColorSpace = params[BTString("LINEAR_SPACE")];
		
		if( format != PF_UNDEFINED )
			mImage = IImageManager::getSingleton().loadImage( stream, IP_TEMPORARY, format, LODlevel, textureDir );
		else
			mImage = IImageManager::getSingleton().loadImage( stream, IP_TEMPORARY, byteOrder, textureDir, LODlevel);

		if( mImage == NULL)
		{
			assert(false);
			return false;
		}

		if( LODlevel == IImage::MIPLOD_DEFAULT)
			resource->mMipMap = (int32)LODlevel;

		if( format == PF_UNDEFINED && !mImage->isCompressed() && mImage->getBPP() == 24)
			mImage = IImageManager::getSingleton().convertToFormat(mImage, IGraphicsResourceManager::getSingleton().getGraphicsConfig().ColorRGBA, IP_TEMPORARY);

		const IMG_INFO& infoHeader = mImage->getInfo();
		//if( mTextureType == TT_3DIM && header->mDepth == 1)

		if( infoHeader.depth > 1 && infoHeader.faces > 1)
		{
			assert(false);
			return false;
		}

		if( mTextureType != TT_3DIM && infoHeader.depth > 1 )
		{
			assert(false);
			return false;
		}

		if( mTextureType != TT_CUBE && infoHeader.faces > 1)
		{
			assert(false);
			return false;
		}

		if( mTextureType == TT_CUBE && (infoHeader.faces != 6 || infoHeader.depth != 1) )
		{
			assert(false);
			return false;
		}

		mImage->setLinearColorSpace(linearColorSpace);

		return true;
	}

	////////////////////////////////////////////////////////////////////////////
	//bool	TextureResourceSerializer::preLoadResource(const TString& path,IResource* res)
	//{
	//	return false;
	//}

	//////////////////////////////////////////////////////////////////////////
	void	TextureResourceSerializer::postProcessResource(IResource* resource)
	{
		if(!BLADE_TS_CHECK_GRAPHICS_CONTEXT())
			BLADE_EXCEPT(EXCE_STATE_ERROR, BTString("cannot post process texture resource without graphics context."));

		//TODO: make it loading various texture type,i.e. cube map
		TextureResource* texRes = static_cast<TextureResource*>(resource);
		texRes->mTexture = IGraphicsResourceManager::getSingleton().createTexture( *mImage );
		mImage.clear();
	}

	//////////////////////////////////////////////////////////////////////////
	bool	TextureResourceSerializer::saveResource(const IResource* /*res*/, const HSTREAM& /*stream*/)
	{
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	TextureResourceSerializer::createResource(IResource* res, ParamList& params)
	{
		if(res == NULL || res->getType() != TextureResource::TEXTURE_RESOURCE_TYPE)
		{
			assert(false);
			return false;
		}

		TextureResource* texRes = static_cast<TextureResource*>(res);
		Variant widthVal = params[BTString("WIDTH")];
		Variant  heightVal = params[BTString("HEIGHT")];
		Variant  formatVal = params[BTString("FORMAT")];
		Variant  mipmapVal = params[BTString("MIPMAP")];
		Variant  linearVal = params[BTString("LINEAR_SPACE")];
		if( !widthVal.isValid() || !heightVal.isValid() || !formatVal.isValid())
		{
			assert(false);
			return false;
		}
		if(!BLADE_TS_CHECK_GRAPHICS_CONTEXT())
			BLADE_EXCEPT(EXCE_STATE_ERROR, BTString("cannot create texture resource without graphics context."));

		uint32 width = widthVal;
		uint32 height = heightVal;
		PixelFormat format = (EPixelFormat)(int)formatVal;

		int mipLevel = IImage::MIPLOD_MAX;
		if( mipmapVal.isValid() )
			mipLevel = mipmapVal;

		HTEXTURE hTex = IGraphicsResourceManager::getSingleton().createTexture(width, height, format, mipLevel);
		texRes->setTexture(hTex);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	TextureResourceSerializer::reloadResource(IResource* resource, const HSTREAM& stream, const ParamList& /*params*/)
	{
		BLADE_LOG(Information, TEXT("Reloading texture: \"") 
			<< stream->getName() 
			<< TEXT("\"."));
		TextureResource* texRes = static_cast<TextureResource*>(resource);

		PixelFormat format = texRes->mFormat;
		IMG_ORIENT textureDir = IGraphicsResourceManager::getSingleton().getGraphicsConfig().TextureDir;

		if( format != PF_UNDEFINED )
		{
			mImage = IImageManager::getSingleton().loadImage( stream, IP_TEMPORARY, 
				format, texRes->mMipMap, textureDir );
		}
		else
		{
			PIXEL_ORDER byteOrder = IGraphicsResourceManager::getSingleton().getGraphicsConfig().DesiredOrder;
			//use image's format (maybe changed by other apps), but need adjust byte order needed by API
			mImage = IImageManager::getSingleton().loadImage( stream, IP_TEMPORARY, byteOrder, textureDir, texRes->mMipMap);
		}

		return mImage != NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	TextureResourceSerializer::reprocessResource(IResource* resource)
	{
		if( mImage == NULL )
		{
			assert(false);
			return false;
		}

		if(!BLADE_TS_CHECK_GRAPHICS_CONTEXT())
			BLADE_EXCEPT(EXCE_STATE_ERROR, BTString("cannot cache texture resource without graphics context."));

		TextureResource* texRes = static_cast<TextureResource*>(resource);
		if( texRes->mTexture == NULL )
		{
			this->TextureResourceSerializer::postProcessResource(resource);
			return true;
		}
		else
		{
			bool ret = IGraphicsResourceManager::getSingleton().updateTexture( *mImage, *(texRes->mTexture) );
			mImage.clear();
			return ret;
		}
	}


	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	Texture1DSerializer::Texture1DSerializer()
		:TextureResourceSerializer(TT_1DIM)
	{

	}


	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	Texture2DSerializer::Texture2DSerializer()
		:TextureResourceSerializer(TT_2DIM)
	{

	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	Texture3DSerializer::Texture3DSerializer()
		:TextureResourceSerializer(TT_3DIM)
	{

	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	TextureCubeSerializer::TextureCubeSerializer()
		:TextureResourceSerializer(TT_CUBE)
	{

	}


}//namespace Blade