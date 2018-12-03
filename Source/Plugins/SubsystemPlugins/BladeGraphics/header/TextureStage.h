/********************************************************************
	created:	2010/05/22
	filename: 	TextureStage.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_TextureStage_h__
#define __Blade_TextureStage_h__
#include <BladeGraphics.h>
#include <math/Vector4.h>
#include <interface/public/graphics/RenderProperty.h>
#include <interface/public/graphics/PixelFormat.h>
#include <interface/public/graphics/IShader.h>

//note: use texture resource instead of texture,to allow directly reload texture resource \n
//without notification.you can reload the resource at any other location,and any time you want.
#include <TextureResource.h>
#include <ResourceState.h>

namespace Blade
{

	class BLADE_GRAPHICS_API TextureStage : public SubResourceState, public Allocatable
	{
	public:
		TextureStage(const TString& name, const HTEXTURE& texture = HTEXTURE::EMPTY);
		~TextureStage();

		/**
		@describe 
		@param
		@return
		*/
		const TString&	getSamplerUniform() const { return mSamplerUniform; }

		/*
		@describe
		@param
		@return
		*/
		void			setTexture(const HTEXTURE& texture);

		/*
		@describe
		@param
		@return
		*/
		const HTEXTURE&	getTexture() const;

		/*
		@describe set params for loading
		@param 
		@return 
		*/
		void			setTargetPixelFormat(PixelFormat format)	{mFormat = format;}

		/*
		@describe set params for loading
		@param [in] mipmap : -1 means full mipmap chain, 0 means default, etc.
		@return 
		*/
		void			setTargetMipMap(int mipmap)					{mMipmap = mipmap;}

		/** @brief  */
		const scalar*	getTextureSize() const						{ return mTexureSize.getData(); }

		/**
		@describe set texture color space. by default textures uses sRGB spaces.
		non-color textures, i.e. normal maps, specular level & gloss should use linear space.
		its only used before loading. for custom bound texture, set texture format to linear before creating it
		@param 
		@return 
		*/
		void			setLinearSpace(bool linear) { mLinearSpace = linear; if (this->getTexture() != NULL) this->getTexture()->setLinearColorSpace(linear); }

		/** @brief  */
		inline void setTextureType(ETextureType type)
		{
			if (mType != type)
			{
				mType = type;
				if (mType == TT_1DIM)
					mSerializerType = TextureResource::TEXTURE_1D_SERIALIZER;
				else if (mType == TT_3DIM)
					mSerializerType = TextureResource::TEXTURE_3D_SERIALIZER;
				else if (mType == TT_CUBE)
					mSerializerType = TextureResource::TEXTURE_CUBE_SERIALIZER;
				else
					mSerializerType = TextureResource::TEXTURE_2D_SERIALIZER;
			}
		}

		inline const ETextureType getTextureType() const { return mType; }

	protected:
		/************************************************************************/
		/* ResourceState interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual void			generateParams(ParamList& params) const;
		/** @brief  */
		virtual void			postLoad(const HRESOURCE& resource);
		virtual bool			onFailed();

		/** @brief  */
		void		updateTextureSize();

		TString			mSamplerUniform;
		Vector4			mTexureSize;
		int				mMipmap;
		EPixelFormat	mFormat;
		ETextureType	mType;
		bool			mLinearSpace;
	};//class TextureStage
	
}//namespace Blade



#endif //__Blade_TextureStage_h__