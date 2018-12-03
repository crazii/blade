/********************************************************************
	created:	2010/04/10
	filename: 	D3D9RenderWindow.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
#include <graphics/windows/D3D9/D3D9Header.h>	//must be the first
#include <interface/public/graphics/IRenderDevice.h>
#include <interface/public/graphics/IRenderView.h>
#include <graphics/windows/D3D9/D3D9RenderWindow.h>
#include <graphics/windows/D3D9/D3D9PixelBuffer.h>
#include <graphics/windows/D3D9/D3D9RenderSurface.h>
#include <graphics/windows/D3D9/D3D9TypeConverter.h>
#include <interface/public/graphics/PixelColorFormatter.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	D3D9RenderWindow::D3D9RenderWindow(IDirect3DSwapChain9* pSwapChain,IWindow* window,IRenderDevice* device, bool bMain)
		:RenderWindow(window, device)
		,D3D9Resource<IDirect3DSwapChain9>(pSwapChain)
		,mIsMain(bMain)
	{
		assert( pSwapChain != NULL );
		if( mIsMain )
			mHwnd = (HWND)(void*)mWindow->getNativeHandle();

		mIsFullScreen = window->getFullScreen();

		this->onSet( this->getD3D9Resource() );
	}

	//////////////////////////////////////////////////////////////////////////
	D3D9RenderWindow::~D3D9RenderWindow()
	{
		//virtual function not called from base at dector
		this->release();
	}

	/************************************************************************/
	/* RenderWindow interface                                                          */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool			D3D9RenderWindow::isReady() const
	{
		//update view
		if( !this->isVisible() || this->getD3D9Resource() == NULL )
			return false;
		else
			return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			D3D9RenderWindow::swapBuffers()
	{
		//device lost?
		if( this->getD3D9Resource() == NULL )
			return true;

		if( mOverrideWindow != NULL )
			return this->swapBuffers(mOverrideWindow, mOVerrideView);

		RECT source = {0, 0, (LONG)this->D3D9RenderWindow::getViewWidth(), (LONG)this->D3D9RenderWindow::getViewHeight()};

		HRESULT result = this->getD3D9Resource()->Present(&source, NULL, mHwnd, NULL, 0);
		return SUCCEEDED(result);
	}

	//////////////////////////////////////////////////////////////////////////
	bool			D3D9RenderWindow::swapBuffers(IWindow* destWindow, IRenderView* sourceView/* = NULL*/)
	{
		//device lost?
		if( this->getD3D9Resource() == NULL)
			return true;
		HWND target;
		if( destWindow == NULL )
			target = mHwnd;
		else
			target = (HWND)(void*)destWindow->getNativeHandle();

		assert(::IsWindow(target));
		RECT sourceRect; 

		if( sourceView != NULL )
		{
			sourceView->calculateSizeInPixel( this );
			sourceRect.left = sourceView->getPixelLeft();
			sourceRect.right = sourceView->getPixelRight();
			sourceRect.top = sourceView->getPixelTop();
			sourceRect.bottom = sourceView->getPixelBottom();
		}
		else
		{
			sourceRect.left = 0;
			sourceRect.top = 0;
			sourceRect.right = (LONG)this->D3D9RenderWindow::getViewWidth();
			sourceRect.bottom = (LONG)this->D3D9RenderWindow::getViewHeight();
		}

		HRESULT result = this->getD3D9Resource()->Present(&sourceRect, NULL, target, NULL, 0);
		return SUCCEEDED(result);
	}

	//////////////////////////////////////////////////////////////////////////
	bool			D3D9RenderWindow::swapBuffers(IImage* destImage, IRenderView* sourceView/* = NULL*/)
	{
		if (destImage == NULL || destImage->isCompressed())
			return false;

		bool ret = this->swapBuffers((IWindow*)NULL, sourceView);
		if (ret)
		{
			const HTEXTURE& backBuffer = this->getBackBuffer();
			HRESULT result;

			LPDIRECT3DDEVICE9 device;
			result = this->getD3D9Resource()->GetDevice(&device);
			if (!SUCCEEDED(result))
				return false;
			ComPtr<IDirect3DDevice9> dptr(device);

			UINT surfaceWidth, surfaceHeight;
#if 0
			if (this->mIsFullScreen)
			{
				surfaceWidth = (UINT)backBuffer->getWidth();
				surfaceHeight = (UINT)backBuffer->getHeight();
			}
			else
			{
				HWND desktop = ::GetDesktopWindow();
				RECT r;
				::GetWindowRect(desktop, &r);
				surfaceWidth = (UINT)(r.right - r.left);
				surfaceHeight = (UINT)(r.bottom - r.top);
			}

			LPDIRECT3DSURFACE9 surface;
			result = device->CreateOffscreenPlainSurface(surfaceWidth, surfaceHeight, desc.Format, D3DPOOL_SYSTEMMEM, &surface, NULL);
			if (!SUCCEEDED(result))
				return false;

			ComPtr<IDirect3DSurface9> sptr(surface);
			result = this->getD3D9Resource()->GetFrontBufferData(surface);
			if (!SUCCEEDED(result))
				return false;

			D3DSURFACE_DESC desc;
			result = surface->GetDesc(&desc);
			if (!SUCCEEDED(result))
				return false;
#else
			LPDIRECT3DSURFACE9 bkSurface = static_cast<D3D9PixelBuffer*>( static_cast<D3D9RenderSurface*>(backBuffer)->getPixelBuffer(0))->getD3D9Resource();
			D3DSURFACE_DESC desc;
			result = bkSurface->GetDesc(&desc);
			if (!SUCCEEDED(result))
				return false;
			surfaceWidth = desc.Width;
			surfaceHeight = desc.Height;

			LPDIRECT3DSURFACE9 surface;
			result = device->CreateOffscreenPlainSurface(surfaceWidth, surfaceHeight, desc.Format, D3DPOOL_SYSTEMMEM, &surface, NULL);
			if (!SUCCEEDED(result))
				return false;
			ComPtr<IDirect3DSurface9> sptr(surface);
			result = device->GetRenderTargetData(bkSurface, surface);
			if (!SUCCEEDED(result))
				return false;
#endif
			RECT sourceRect;
			if (sourceView != NULL)
			{
				sourceView->calculateSizeInPixel(this);
				sourceRect.left = sourceView->getPixelLeft();
				sourceRect.right = sourceView->getPixelRight();
				sourceRect.top = sourceView->getPixelTop();
				sourceRect.bottom = sourceView->getPixelBottom();
			}
			else
			{
				sourceRect.left = 0;
				sourceRect.top = 0;
				sourceRect.right = (LONG)desc.Width;
				sourceRect.bottom = (LONG)desc.Height;
			}
			assert(sourceRect.left >= 0 && (UINT)sourceRect.right <= desc.Width);
			assert(sourceRect.top >= 0 && (UINT)sourceRect.bottom <= desc.Height);
			D3DLOCKED_RECT rect;
			result = surface->LockRect(&rect, &sourceRect, D3DLOCK_READONLY);
			if (!SUCCEEDED(result))
				return false;
			LONG height = sourceRect.bottom - sourceRect.top;
			LONG width = sourceRect.right - sourceRect.left;
			assert(destImage->getWidth() >=(size_t)width);
			assert(destImage->getHeight() >= (size_t)height);

			uint8* data = destImage->getBits();
			D3DFORMAT srcFmtD3D9 = desc.Format;
			const PixelFormat srcFmt = D3DFormatConverter(srcFmtD3D9);

			PixelFormat destFmt = destImage->getFormat();
			D3DFORMAT destFmtD3D9 = D3DFormatConverter(destFmt);
			for (LONG i = 0; i < height; ++i)
			{
				if(destFmtD3D9 == srcFmtD3D9)
					std::memcpy(data, rect.pBits, width*destFmt.getSizeBytes());
				else
				{
					Color::RGBA c;
					for (LONG j = 0; j < width; ++j)
					{
						ColorFormatter::readColorLDR((char*)rect.pBits + j * 4,  c, srcFmt);
						ColorFormatter::writeColorLDR(data + j*destFmt.getSizeBytes(), c, destFmt);
					}
				}
				(char*&)rect.pBits += rect.Pitch;
				data += destImage->getPitch();
			}
			surface->UnlockRect();
		}
		return ret;
	}

	/************************************************************************/
	/* IWindowEventHandler interface                                                           */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool			D3D9RenderWindow::onWindowResizing(IWindow* pwin,size_t width,size_t height)
	{
		BLADE_UNREFERENCED(width);
		BLADE_UNREFERENCED(height);

		assert( pwin == mWindow );
		if( pwin->getFullScreen() && mRenderTarget.getDevice()->getNumRenderWindows() > 1 )
			return false;

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void			D3D9RenderWindow::onWindowResized(IWindow* pwin)
	{
		assert( pwin == mWindow );

		//minimize
		if( pwin->getWidth() == 0 || pwin->getHeight() == 0 )
			return;

		mRenderTarget.getDevice()->notifyRenderWindowChange(this);
		mRenderTarget.notifySizeChange(this);
		mIsFullScreen = pwin->getFullScreen();
	}

	//////////////////////////////////////////////////////////////////////////
	void			D3D9RenderWindow::onWindowDestroy(IWindow* pwin)
	{
		assert( pwin == mWindow );
		BLADE_UNREFERENCED(pwin);
		pwin->removeEventHandler(this);
		mWindow = NULL;
	}

	/************************************************************************/
	/*ID3D9Resource interface                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void			D3D9RenderWindow::release()
	{
		Texture* surfaceTexture = static_cast<Texture*>( this->getBackBuffer() );
		D3D9PixelBuffer* buffer = static_cast<D3D9PixelBuffer*>( surfaceTexture->getPixelBuffer() );
		buffer->release();
		D3D9Resource::release();
	}

	//////////////////////////////////////////////////////////////////////////
	bool					D3D9RenderWindow::isInDefaultPool() const
	{
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void					D3D9RenderWindow::rebuildImpl(IDirect3DDevice9* device)
	{
		assert( this->getD3D9Resource() == NULL && device != NULL);

		if( this->getD3D9Resource() != NULL )
		{
			BLADE_EXCEPT(EXC_INVALID_OPERATION,BTString("cannot re-create resource if previous not releaseed."));
		}

		//assert( mRenderTarget.getDevice() == device );
		if( device == NULL )
			return;

		IDirect3DSwapChain9* swapChain = NULL;

		if( this->isMainWindow() )
		{
			HRESULT result = device->GetSwapChain(0,&swapChain);
			if( FAILED(result) )
			{
				BLADE_EXCEPT(EXC_API_ERROR,BTString("unable to get implicit swap chain.") );
			}
		}
		else
		{
			D3DPRESENT_PARAMETERS d3dpp = this->getD3D9Desc();
			HRESULT result = device->CreateAdditionalSwapChain(&d3dpp,&swapChain);
			if( FAILED(result) )
			{
				BLADE_EXCEPT(EXC_API_ERROR,BTString("unable to create addtional swap chain.") );
			}
		}

		this->setD3D9Resource(swapChain);
	}

	//////////////////////////////////////////////////////////////////////////
	void					D3D9RenderWindow::onSet(IDirect3DSwapChain9* source)
	{
		IDirect3DSurface9* surface = NULL;
		if( source == NULL )
			return;

		HRESULT result = source->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &surface);
		if( FAILED(result) )
			BLADE_EXCEPT(EXC_API_ERROR,BTString("unable to get back buffer of swap chain.") );

		if( this->getBackBuffer() == NULL )
		{
			D3D9PixelBuffer* buffer = BLADE_NEW D3D9PixelBuffer(surface,IPixelBuffer::PBL_BACKBUFFER);
			D3D9RenderSurface* surfaceTexture = BLADE_NEW D3D9RenderSurface(buffer);
			surfaceTexture->setLinearColorSpace(false);	//back buffer uses sRGB color space
			this->getBackBuffer().bind(surfaceTexture);
		}
		else
		{
			D3D9RenderSurface* surfaceTexture = static_cast<D3D9RenderSurface*>(this->getBackBuffer());
			D3D9PixelBuffer* buffer = static_cast<D3D9PixelBuffer*>( surfaceTexture->getPixelBuffer() );
			buffer->setD3D9Resource(surface);
			surfaceTexture->setSize(buffer->getWidth(), buffer->getHeight());
		}
	}

	/************************************************************************/
	/* custom method                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool					D3D9RenderWindow::isMainWindow() const
	{
		return mIsMain;
	}

}//namespace Blade

#endif //BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
	