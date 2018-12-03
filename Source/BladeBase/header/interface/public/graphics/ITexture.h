/********************************************************************
	created:	2010/04/15
	filename: 	ITexture.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_ITexture_h__
#define __Blade_ITexture_h__
#include <Handle.h>
#include "IGraphicsBuffer.h"
#include "PixelFormat.h"
#include "GraphicsBaseEnums.h"

namespace Blade
{
	typedef struct STextureRegion
	{
		index_t mFace;
		index_t	mMipLevel;
		Box3i	mBox;
		mutable	size_t mRowPitch;
		mutable size_t mSlicePitch;

		STextureRegion()
			:mBox(Box3i::EMPTY)
		{
			mFace = 0;
			mMipLevel = 0;
			mRowPitch = 0;
			mSlicePitch = 0;
		}

		STextureRegion(const Box3i& src)
			:mBox(src)
		{
			mFace = 0;
			mMipLevel = 0;
			mRowPitch = 0;
			mSlicePitch = 0;
		}

	}TEXREGION;

	enum ETextureType
	{
		TT_1DIM,
		TT_2DIM,
		TT_3DIM,	///a volume texture
		TT_CUBE,	///a cube texture
	};

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class BLADE_BASE_API ITexture
	{
	public:
		enum
		{
			MAX_TEXTURE_COUNT = 16,		///shader model 2.0-3.0
			MAX_TEXCOORD_COUNT = 4,		///texture coordinate limit
		};
	public:
		typedef Handle<ITexture> HTEXTURE;
	public:
		virtual ~ITexture()		{}

		typedef struct STextureDesc
		{
			size_t				mWidth;
			size_t				mHeight;
			size_t				mDepth;
			size_t				mMipmapCount;
			PixelFormat			mPixelFormat;
			IGraphicsBuffer::USAGE	mUsage;
			ETextureType		mTextureType : 8;
			IMG_ORIENT			mLayout : 8;
			IGraphicsBuffer::ACCESS mGpuAccess : 8;
		}DESC;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const DESC&	getDesc() const = 0;

		inline ETextureType	getType() const
		{
			return this->getDesc().mTextureType;
		}

		inline IMG_ORIENT getLayout() const
		{
			return this->getDesc().mLayout;
		}

		inline size_t			getMipmapCount() const
		{
			return this->getDesc().mMipmapCount;
		}

		inline size_t			getWidth() const
		{
			return this->getDesc().mWidth;
		}

		inline size_t			getHeight() const
		{
			return this->getDesc().mHeight;
		}

		inline size_t			getDepth() const
		{
			return this->getDesc().mDepth;
		}

		inline IGraphicsBuffer::USAGE	getTextureUsage() const
		{
			return this->getDesc().mUsage;
		}

		/**
		@describe get the access of the texture, the access indicates the type of the texture
		READ		WRITE		indicated type
		------------------------------------------------------------------
		true		true		render target, readable depth stencil
		true		false		default texture
		false		true		back buffer, depth stencil
		false		false		N/A
		@param
		@return
		*/
		inline  IGraphicsBuffer::ACCESS	getGpuAccess() const
		{
			return this->getDesc().mGpuAccess;
		}

		inline PixelFormat		getPixelFormat() const
		{
			return this->getDesc().mPixelFormat;
		}

		inline bool			hasAlpha() const
		{
			return this->getPixelFormat().hasAlpha();
		}

		inline bool			isCubeMap() const
		{
			return this->getDesc().mTextureType == TT_CUBE;
		}

		inline size_t		getFaceCount() const
		{
			return this->isCubeMap() ? 6u : 1u;
		}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool			isLocked() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual HTEXTURE		cloneTexture(size_t newWidth = 0,size_t newHeight = 0,PixelFormat newFormat = PF_UNDEFINED) = 0;

		/**
		@describe common lock method
		@param 
		@return 
		*/
		virtual void*			lock(const TEXREGION& region,IGraphicsBuffer::LOCKFLAGS lockflags) = 0;

		/** @brief  lock for cube textures */
		inline void*			lock(index_t face, size_t& rowPitch,IGraphicsBuffer::LOCKFLAGS lockflags, const Box3i& box = Box3i::EMPTY)
		{
			TEXREGION region(box);
			region.mFace = face;
			void* ret = this->lock(region,lockflags);
			rowPitch = region.mRowPitch;
			return ret;
		}

		/** @brief lock for 3D(volume) textures  */
		inline void*			lock(size_t& rowPitch,size_t& slicePitch,IGraphicsBuffer::LOCKFLAGS lockflags,const Box3i& box= Box3i::EMPTY)
		{
			TEXREGION region(box);
			void* ret = this->lock(region,lockflags);
			rowPitch = region.mRowPitch;
			slicePitch = region.mSlicePitch;
			return ret;
		}

		/** @brief lock for 2D textures */
		inline void*			lock(size_t& rowPitch,IGraphicsBuffer::LOCKFLAGS lockflags, const Box3i& box = Box3i::EMPTY)
		{
			TEXREGION region(box);
			void* ret = this->lock(region,lockflags);
			rowPitch = region.mRowPitch;
			return ret;
		}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void			unlock() = 0;

		/**
		@describe
		@param box the region to update. EMPTY means the entire texture, or use the sub rectangle of the texture to generate mipmap.this is used for texture atlas.
		@param mipLevel the target mip level to update, the miplevel 0 must be filled already, and other level will update from it.\n
		set the param to 0 to update all miplevels
		@return
		*/
		virtual bool			updateMipMap(const TEXREGION& region,size_t mipLevel = 0) = 0;

		inline bool				updateMipMap(index_t mipLevel = 0, const Box3i& box = Box3i::EMPTY, index_t face = 0)
		{
			TEXREGION region(box);
			region.mFace = face;
			return this->updateMipMap(region,mipLevel);
		}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool			addDirtyRegion(const TEXREGION& region) = 0;

		inline bool				addDirtyRegion(const Box3i& dirtyBox, index_t face = 0)
		{
			TEXREGION region(dirtyBox);
			region.mFace = face;
			return this->addDirtyRegion(region);
		}

		/**
		@describe set texture color space. by default textures uses sRGB spaces (linear=false).
		@param
		@return
		*/
		virtual bool			setLinearColorSpace(bool linear) = 0;

		/**
		@describe get null-able sampler object
		@param
		@return
		*/
		virtual const Sampler&		getSampler() const = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual void				setSampler(const Sampler& sampler) = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual HSAMPLER			shareSampler(const HSAMPLER& share = HSAMPLER::EMPTY) = 0;

	};//class ITexture

	typedef ITexture::HTEXTURE HTEXTURE;


	class IRenderTexture : public ITexture
	{
	public:
		/**
		@describe record view size for current render texture. 
		if current texture usage is not render target (GBUF_RENDERTARGET/GBUF_DEPTHSTENCIL), return false
		@param
		@return
		*/
		virtual bool				setViewRect(scalar left, scalar top, scalar width, scalar height) = 0;

		/**
		@describe get view size for current render texture.
		if current texture usage is not render target (GBUF_RENDERTARGET/GBUF_DEPTHSTENCIL), return false
		@param
		@return
		*/
		virtual bool				getViewRect(scalar& outLeft, scalar& outTop, scalar& outWidth, scalar& outHeight) const = 0;

		inline bool		getViewRect(scalar* rect) const
		{
			return this->getViewRect(rect[0], rect[1], rect[2], rect[3]);
		}
	};
	
	
}//namespace Blade



#endif //__Blade_ITexture_h__