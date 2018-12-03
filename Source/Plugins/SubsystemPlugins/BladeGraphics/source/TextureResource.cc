/********************************************************************
	created:	2010/05/23
	filename: 	TextureResource.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <TextureResource.h>

namespace Blade
{
	const TString TextureResource::TEXTURE_RESOURCE_TYPE = BTString("TextureResource");
	const TString TextureResource::TEXTURE_1D_SERIALIZER = BTString("TexSerializer1D");
	const TString TextureResource::TEXTURE_2D_SERIALIZER = BTString("TexSerializer2D");
	const TString TextureResource::TEXTURE_3D_SERIALIZER = BTString("TexSerializer3D");
	const TString TextureResource::TEXTURE_CUBE_SERIALIZER = BTString("TexSerializerCube");


	//////////////////////////////////////////////////////////////////////////
	TextureResource::TextureResource()
		:GraphicsResource(TEXTURE_RESOURCE_TYPE)
	{
		mFormat = PF_UNDEFINED;
		mMipMap = -1;
	}

	//////////////////////////////////////////////////////////////////////////
	TextureResource::~TextureResource()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	const HTEXTURE&			TextureResource::getTexture() const
	{
		return mTexture;
	}

	//////////////////////////////////////////////////////////////////////////
	void					TextureResource::setTexture(const HTEXTURE& hTexture)
	{
		mTexture = hTexture;
	}
	
}//namespace Blade