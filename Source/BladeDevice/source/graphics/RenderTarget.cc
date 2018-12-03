/********************************************************************
	created:	2010/04/10
	filename: 	RenderTarget.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <graphics/RenderTarget.h>
#include <interface/public/graphics/IRenderDevice.h>
#include <graphics/Texture.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	RenderTarget::RenderTarget(const TString& name,IRenderDevice* device, size_t viewWidth, size_t viewHeight)
		:mName(name)
		,mDevice(device)
		,mListener(NULL)
		,mViewWidth(viewWidth)
		,mViewHeight(viewHeight)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	RenderTarget::~RenderTarget()
	{

	}


	/************************************************************************/
	/* IRenderTarget interface                                                                     */
	/************************************************************************/


	//////////////////////////////////////////////////////////////////////////
	const TString&			RenderTarget::getName() const
	{
		return mName;
	}

	//////////////////////////////////////////////////////////////////////////
	const HTEXTURE&		RenderTarget::getDepthBuffer() const
	{
		return mDepthBuffer;
	}

	//////////////////////////////////////////////////////////////////////////
	bool					RenderTarget::setDepthBuffer(const HTEXTURE& hDethBuffer)
	{
		if( hDethBuffer != NULL && !hDethBuffer->getPixelFormat().isDepth() && !hDethBuffer->getPixelFormat().isDepthStencil() )
		{
			assert(false);
			return false;
		}
		mDepthBuffer = hDethBuffer;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool					RenderTarget::setColorBuffer(index_t index, const HTEXTURE& buffer)
	{
		if( index >= mDevice->getDeviceCaps().mMaxMRT )
		{
			assert(false);
			return false;
		}
		assert( index == 0 || buffer == NULL || buffer->getWidth() == this->getWidth() );
		assert( index == 0 || buffer == NULL || buffer->getHeight() == this->getHeight() );

		if( index < mOutputBuffers.size() )
			mOutputBuffers[index] = buffer;
		else if( index == mOutputBuffers.size() )
			mOutputBuffers.push_back(buffer);
		else
		{
			assert(false);
			return false;
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			RenderTarget::setColorBufferCount(index_t count)
	{
		if( count <= mDevice->getDeviceCaps().mMaxMRT )
		{
			mOutputBuffers.resize(count);
			return true;
		}
		assert(false);
		return false;
	}


	//////////////////////////////////////////////////////////////////////////
	const HTEXTURE&		RenderTarget::getColorBuffer(index_t index) const
	{
		if( index < mOutputBuffers.size() )
			return mOutputBuffers[index];
		else
			return HTEXTURE::EMPTY;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				RenderTarget::setViewRect(int32 left, int32 top, int32 width, int32 height)
	{
		bool ret = false;
		for (size_t i = 0; i < mOutputBuffers.size(); ++i)
		{
			Texture* tex = static_cast<Texture*>(mOutputBuffers[i]);
			if (tex != NULL)
			{
				scalar fwidth = (scalar)tex->getWidth();
				scalar fheight = (scalar)tex->getHeight();

				ret |= tex->setViewRect((scalar)left/fwidth, (scalar)top/fheight, std::min<scalar>(1.0f, width / fwidth), std::min<scalar>(1.0f, (scalar)height / fheight));
			}
		}

		if (mDepthBuffer != NULL)
		{
			Texture* tex = static_cast<Texture*>(mDepthBuffer);
			if (tex != NULL)
			{
				scalar fwidth = (scalar)tex->getWidth();
				scalar fheight = (scalar)tex->getHeight();
				ret |= tex->setViewRect((scalar)left / fwidth, (scalar)top / fheight, std::min<scalar>(1.0f, width / fwidth), std::min<scalar>(1.0f, (scalar)height / fheight));
			}
			
		}
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t				RenderTarget::getColorBufferCount() const
	{
		return mOutputBuffers.size();
	}

	//////////////////////////////////////////////////////////////////////////
	RenderTarget::IListener*RenderTarget::setListener(IListener* listener)
	{
		IListener* old = mListener;
		mListener = listener;
		return old;
	}

	RenderTarget::IListener*RenderTarget::getListener() const
	{
		return mListener;
	}

	//////////////////////////////////////////////////////////////////////////
	bool					RenderTarget::isReady() const
	{
		return this->getColorBuffer(0) != NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	bool					RenderTarget::swapBuffers()
	{
		return true;
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void					RenderTarget::notifySizeChange(IRenderTarget* target)
	{
		if( mListener != NULL )
			mListener->onTargetSizeChange(target);
	}

	
}//namespace Blade