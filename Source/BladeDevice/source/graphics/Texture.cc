/********************************************************************
	created:	2010/04/15
	filename: 	Texture.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <graphics/Texture.h>

namespace Blade
{
	////////////////////////////////////////////////////////////////////////////
	Texture::Texture(ETextureType type,IMG_ORIENT layout)
		:mViewTop(0)
		,mViewLeft(0)
		,mViewWidth(0)
		,mViewHeight(0)
		,mLockedMip(-1)
		,mLockedFace(-1)
		,mIsLocked(false)
		,mSharedSampler(false)
	{
		mDesc.mTextureType = type;
		mDesc.mLayout = layout;
	}

	//////////////////////////////////////////////////////////////////////////
	Texture::Texture(ETextureType type,IMG_ORIENT layout,
		size_t width,size_t height,PixelFormat format,
		IGraphicsBuffer::USAGE usage,IGraphicsBuffer::ACCESS GpuAccess,size_t depth/* = 1*/,size_t mipmap/* = 0*/)
		:mLockedMip(-1)
		,mLockedFace(-1)
		,mIsLocked(false)
		,mSharedSampler(false)
	{
		mDesc.mTextureType = type;
		mDesc.mLayout = layout;
		mDesc.mWidth = width;
		mDesc.mHeight = height;
		mDesc.mDepth = depth;
		mDesc.mPixelFormat = format;
		mDesc.mMipmapCount = mipmap;
		mDesc.mUsage = usage;
		mDesc.mGpuAccess = GpuAccess;
	}

	//////////////////////////////////////////////////////////////////////////
	/*static */const HSAMPLER& Texture::DEFAULT_RTT_SAMPLER()
	{
#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(disable:4640)
#endif
		static HSAMPLER sampler;
		if(sampler == NULL)
		{
			sampler.bind(BLADE_NEW Sampler(Sampler::DEFAULT_RTT));
		}
		return sampler;
	}

	//////////////////////////////////////////////////////////////////////////
	/*static */const HSAMPLER& Texture::DEFAULT_DEPTH_RTT_SAMPLER()
	{
#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(disable:4640)
#endif
		static HSAMPLER sampler;
		if (sampler == NULL)
		{
			sampler.bind(BLADE_NEW Sampler(Sampler::DEFAULT_RTT_DEPTH));
		}
		return sampler;
	}
	
}//namespace Blade