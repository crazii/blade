/********************************************************************
	created:	2010/05/23
	filename: 	TextureResource.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_TextureResource_h__
#define __Blade_TextureResource_h__
#include <interface/public/graphics/ITexture.h>
#include <GraphicsResource.h>

namespace Blade
{

	class BLADE_GRAPHICS_API TextureResource : public GraphicsResource , public Allocatable
	{
	public:
		//usually a texture may not loaded both as 2D and 3D etc. in the meantime,
		//so we just use one type of texture resource
		//but more types of serializer
		static const TString TEXTURE_RESOURCE_TYPE;
		static const TString TEXTURE_1D_SERIALIZER;
		static const TString TEXTURE_2D_SERIALIZER;
		static const TString TEXTURE_3D_SERIALIZER;
		static const TString TEXTURE_CUBE_SERIALIZER;
	public:
		TextureResource();
		~TextureResource();

		/*
		@describe
		@param
		@return
		*/
		const HTEXTURE&			getTexture() const;

		/*
		@describe 
		@param 
		@return 
		*/
		void					setTexture(const HTEXTURE& hTexture);

	protected:
		HTEXTURE		mTexture;
		//saved loading option, use specified format on re-loading
		PixelFormat		mFormat;
		int32			mMipMap;
		friend class TextureResourceSerializer;
	};//class TextureResource

	typedef Handle<TextureResource> HTEXTURERES;
	
}//namespace Blade


#endif //__Blade_TextureResource_h__