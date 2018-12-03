/********************************************************************
	created:	2010/04/15
	filename: 	Texture.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_Texture_h__
#define __Blade_Texture_h__
#include <BladeDevice.h>
#include <interface/public/graphics/ITexture.h>
#include <interface/public/graphics/RenderProperty.h>
#include "IPixelBuffer.h"

namespace Blade
{
	class BLADE_DEVICE_API Texture : public IRenderTexture
	{
	public:
		Texture(ETextureType type,IMG_ORIENT layout);

		Texture(ETextureType type, IMG_ORIENT layout, size_t width,size_t height,PixelFormat format,
			IGraphicsBuffer::USAGE usage,IGraphicsBuffer::ACCESS GpuAccess,size_t depth = 1, size_t mipmap = 0);

		virtual ~Texture()	{}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const DESC&		getDesc() const			{return mDesc;}
	
		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			isLocked() const		{return mIsLocked;}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void*			lock(const TEXREGION& region, IGraphicsBuffer::LOCKFLAGS lockflags)
		{
			if( this->isLocked() )
				BLADE_EXCEPT(EXC_REDUPLICATE,BTString("texture already locked,cannot lock twice.") );

			if( !mDesc.mUsage.isReadable() && !mDesc.mUsage.isWriteable() )
				BLADE_EXCEPT(EXC_INVALID_OPERATION,BTString("cannot lock immutable texture.") );

			if( (!mDesc.mUsage.isReadable() && (lockflags&IGraphicsBuffer::GBLF_READONLY) != 0)
				|| (!mDesc.mUsage.isWriteable() && (lockflags&IGraphicsBuffer::GBLF_WRITEONLY) != 0) )
				BLADE_EXCEPT(EXC_INVALID_OPERATION,BTString("invalid lock operation for buffer access.") );

			if( region.mBox.getDepth() > mDesc.mDepth || region.mMipLevel >= mDesc.mMipmapCount || region.mFace >= 6)
				BLADE_EXCEPT(EXC_OUT_OF_RANGE,BTString("invalid range for miplevel/depth.") );

			void* ptr = this->lockImpl(region,lockflags);
			assert(ptr != NULL);
			mIsLocked = true;
			mLockedMip = (int16)region.mMipLevel;
			mLockedFace = (int8)region.mFace;
			return ptr;
		}

		/** @brief  */
		inline void*		lockNoCheck(const TEXREGION& region, IGraphicsBuffer::LOCKFLAGS lockflags)
		{
			if( this->isLocked() )
				BLADE_EXCEPT(EXC_REDUPLICATE,BTString("texture already locked,cannot lock twice.") );

			if( region.mBox.getDepth() > mDesc.mDepth || region.mMipLevel >= mDesc.mMipmapCount || region.mFace >= 6)
				BLADE_EXCEPT(EXC_OUT_OF_RANGE,BTString("invalid range for miplevel/depth.") );

			void* ptr = this->lockImpl(region,lockflags);
			assert(ptr != NULL);
			mIsLocked = true;
			mLockedMip = (int16)region.mMipLevel;
			mLockedFace = (int8)region.mFace;
			return ptr;
		}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			unlock()
		{
			if( !mIsLocked )
				BLADE_EXCEPT(EXC_INTERNAL_ERROR,BTString("texture not previously locked.") );

			assert( mLockedMip >= 0 );
			assert( mLockedFace >= 0 );

			if( this->unlockImpl() )
			{
				mIsLocked = false;
				mLockedMip = -1;
				mLockedFace = -1;
			}
			else
				BLADE_EXCEPT(EXC_INTERNAL_ERROR,BTString("unable to lock texture.") );
		}

		/**
		@describe 
		@param
		@return
		*/
		virtual bool			setLinearColorSpace(bool linear)
		{
			mDesc.mPixelFormat.setLinearColor(linear);
			return true;
		}

		/**
		@describe get null-able sampler object
		@param
		@return
		*/
		virtual const Sampler&		getSampler() const
		{
			return mSampler == NULL ? Sampler::DEFAULT : *mSampler;
		}

		/**
		@describe 
		@param
		@return
		*/
		virtual void				setSampler(const Sampler& sampler)
		{
			if (mSharedSampler && (*mSampler != sampler))
			{
				mSampler.clear();
				mSharedSampler = false;
			}
			if(mSampler == NULL)
				mSampler.constructInstance<Sampler>();
			*mSampler = sampler;
		}

		/**
		@describe 
		@param
		@return
		*/
		virtual HSAMPLER			shareSampler(const HSAMPLER& share = HSAMPLER::EMPTY)
		{
			HSAMPLER old = mSampler;
			if(share != NULL)
				mSampler = share;
			mSharedSampler = true;
			return old;
		}

		/************************************************************************/
		/* IRenderTexture interface                                                                     */
		/************************************************************************/
		/**
		@describe record view size for current render texture
		@param
		@return
		*/
		virtual bool				setViewRect(scalar left, scalar top, scalar width, scalar height)
		{
			if(this->getTextureUsage().isRenderTarget() || this->getTextureUsage().isDepthStencil())
			{
				assert(left >= 0 && top >= 0 && left <= 1 && top <= 1 && width <= 1 && height <= 1);
				mViewLeft = left;
				mViewTop = top;
				mViewWidth = width;
				mViewHeight = height;
				return true;
			}
			assert(false);
			return false;
		}

		/**
		@describe 
		@param
		@return
		*/
		virtual bool				getViewRect(scalar& outLeft, scalar& outTop, scalar& outWidth, scalar& outHeight) const
		{
			if(this->getTextureUsage().isRenderTarget() || this->getTextureUsage().isDepthStencil())
			{
				outLeft = mViewLeft;
				outTop = mViewTop;
				outWidth = mViewWidth;
				outHeight = mViewHeight;
				return true;
			}
			assert(false);
			outWidth = 0;
			outHeight = 0;
			return false;
		}

		/************************************************************************/
		/* internally used for render devices, for render target or depth-stencil                                                                    */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual const HPXBUFFER&	getPixelBuffer(index_t mipMap = 0) const
		{
			BLADE_UNREFERENCED(mipMap);
			return HPXBUFFER::EMPTY;
		}

		/** @brief  */
		void			setLayout(IMG_ORIENT layout)
		{
			mDesc.mLayout = layout;
		}

		/** @brief  */
		static const HSAMPLER& DEFAULT_RTT_SAMPLER();
		static const HSAMPLER& DEFAULT_DEPTH_RTT_SAMPLER();

	protected:

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void*			lockImpl(const TEXREGION& region,IGraphicsBuffer::LOCKFLAGS lockflags) = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			unlockImpl() = 0;

		/** @brief  */
		inline index_t		getLockedMip() const {return index_t(mLockedMip); }
		/** @brief  */
		inline index_t		getLockedFace() const {return index_t(mLockedFace);}

		DESC				mDesc;

	private:
		HSAMPLER			mSampler;
		scalar				mViewTop;
		scalar				mViewLeft;
		scalar				mViewWidth;
		scalar				mViewHeight;
		int16				mLockedMip;
		int8				mLockedFace;
		bool				mIsLocked;
		bool				mSharedSampler;
	};

	
}//namespace Blade


#endif //__Blade_Texture_h__