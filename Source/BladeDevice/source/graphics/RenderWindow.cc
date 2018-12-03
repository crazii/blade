/********************************************************************
	created:	2010/04/10
	filename: 	RenderWindow.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <graphics/RenderWindow.h>
#include <graphics/Texture.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	RenderWindow::RenderWindow(IWindow* pwin, IRenderDevice* device)
		:mRenderTarget(pwin->getUniqueName(),device, 0, 0)
		,mWindow(pwin)
	{
		assert(mWindow != NULL );
		mOverrideWindow = NULL;
		mOVerrideView = NULL;
	}

	/************************************************************************/
	/* IRenderTarget Interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	const TString&	RenderWindow::getName() const
	{
		return mRenderTarget.getName();
	}

	//////////////////////////////////////////////////////////////////////////
	const HTEXTURE&RenderWindow::getDepthBuffer() const
	{
		return mRenderTarget.getDepthBuffer();
	}

	//////////////////////////////////////////////////////////////////////////
	bool			RenderWindow::setDepthBuffer(const HTEXTURE& hDethBuffer)
	{
		return mRenderTarget.setDepthBuffer(hDethBuffer);
	}

	//////////////////////////////////////////////////////////////////////////
	bool			RenderWindow::setColorBuffer(index_t index, const HTEXTURE& buffer)
	{
		if( index == 0 )
		{
			assert(false && "cannot set color output of a render window");
			return false;
		}
		else
			return mRenderTarget.setColorBuffer(index, buffer);
	}

	//////////////////////////////////////////////////////////////////////////
	const HTEXTURE&RenderWindow::getColorBuffer(index_t index) const
	{
		return index == 0 ? mBackBuffer : HTEXTURE::EMPTY;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		RenderWindow::setViewRect(int32 left, int32 top, int32 width, int32 height)
	{
		bool ret = false;
		if (mBackBuffer != NULL)
		{
			Texture* tex = static_cast<Texture*>(mBackBuffer);
			scalar fwidth = (scalar)tex->getWidth();
			scalar fheight = (scalar)tex->getHeight();
			ret = tex->setViewRect((scalar)left/fwidth, (scalar)top/fheight, (scalar)width/fwidth, (scalar)height/fheight);
		}
		ret |= mRenderTarget.setViewRect(left, top, width, height);
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t			RenderWindow::getColorBufferCount() const
	{
		return 1;
	}

	//////////////////////////////////////////////////////////////////////////
	RenderWindow::IListener*	RenderWindow::setListener(IListener* listener)
	{
		return mRenderTarget.setListener(listener);
	}

	//////////////////////////////////////////////////////////////////////////
	RenderWindow::IListener*	RenderWindow::getListener() const
	{
		return mRenderTarget.getListener();
	}

	//////////////////////////////////////////////////////////////////////////
	bool			RenderWindow::isReady() const
	{
		return mBackBuffer != NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			RenderWindow::swapBuffers()
	{
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void			RenderWindow::setSwapBufferOverride(IWindow* ovrWin, IRenderView* ovrView/* = NULL*/)
	{
		mOverrideWindow = ovrWin;
		if( mOverrideWindow != NULL )
			mOVerrideView = ovrView;
		else
			mOVerrideView = NULL;
	}

	/************************************************************************/
	/* IRenderWindow interface                                                                     */
	/************************************************************************/

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool			RenderWindow::getFullScreen() const
	{
		return mWindow->getFullScreen();
	}

	//////////////////////////////////////////////////////////////////////////
	bool			RenderWindow::isVisible() const
	{
		return mWindow != NULL && mWindow->isVisible();
	}

	//////////////////////////////////////////////////////////////////////////
	IWindow*		RenderWindow::getWindow() const
	{
		return mWindow;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t			RenderWindow::getWindowWidth() const
	{
		return mWindow->getWidth();
	}

	//////////////////////////////////////////////////////////////////////////
	size_t			RenderWindow::getWindowHeight() const
	{
		return mWindow->getHeight();
	}

	//////////////////////////////////////////////////////////////////////////
	HTEXTURE&		RenderWindow::getDepthBuffer()
	{
		return mRenderTarget.getDepthBuffer();
	}

	
}//namespace Blade