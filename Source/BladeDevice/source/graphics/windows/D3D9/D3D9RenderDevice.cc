/********************************************************************
	created:	2009/02/22
	filename: 	D3D9RenderDevice.cc
	author:		Crazii
	
	purpose:	D3D9RenderDevice implementation
*********************************************************************/
#include <BladePCH.h>

#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
#include <graphics/windows/D3D9/D3D9RenderDevice.h>

#include <BladeBase_blang.h>

#include <interface/public/graphics/IRenderView.h>
#include <interface/public/graphics/GraphicsGeometry.h>
#include <interface/public/graphics/IVertexSource.h>

#include <graphics/windows/D3D9/D3D9GraphicsResourceManager.h>
#include <graphics/windows/D3D9/D3D9TypeConverter.h>
#include <graphics/windows/D3D9/D3D9Resource.h>
#include <graphics/windows/D3D9/D3D9IndexBuffer.h>
#include <graphics/windows/D3D9/D3D9VertexBuffer.h>
#include <graphics/windows/D3D9/D3D9PixelBuffer.h>
#include <graphics/windows/D3D9/D3D9Texture.h>
#include <graphics/windows/D3D9/D3D9VolumeTexture.h>
#include <graphics/windows/D3D9/D3D9CubeTexture.h>
#include <graphics/windows/D3D9/D3D9RenderSurface.h>
#include <graphics/windows/D3D9/D3D9VertexDeclaration.h>
#include <graphics/windows/D3D9/D3D9RenderWindow.h>
#include <graphics/windows/D3D9/D3D9Shader.h>
#include <graphics/windows/D3D9/ID3D9Helper.h>
#include "D3D9StateBlock.h"

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#	pragma comment(lib,"d3d9.lib")
#	pragma comment(lib,"d3dx9.lib")
#endif

namespace Blade
{
	D3DFORMAT D3DFormatConverter::msBackBufferFormat;

	namespace D3D9OptionString
	{
		static const TString ADAPTER_COUNT = BXLang(BLANG_ADAPTER_COUNT);
		static const TString ADAPTER_LIST = BXLang(BLANG_ADATPER);
		static const TString VSYNC = BXLang(BLANG_VSYNC);
	}

	//////////////////////////////////////////////////////////////////////////
	D3D9RenderDevice::D3D9RenderDevice()
		:mAdapterID(0)
		,mD3D9Device(NULL)
		,mD3D9(NULL)
		,mMaxSourceID(0)
		,mMaxUsedTarget(0)
		,mStateBlock(NULL)
		,mLastTarget(NULL)
		,mLastVertexShader(NULL)
		,mLastPixelShader(NULL)
		,mFullScreen(false)
		,mVSync(true)
	{
		ZeroMemory(&(this->mD3DPP),sizeof(D3DPRESENT_PARAMETERS));
		this->mD3D9 = ::Direct3DCreate9(D3D_SDK_VERSION);
		assert( mD3D9 );
		mAdapterCount = mD3D9->GetAdapterCount();

		mDevCaps.mMaxMRT = 1;
		mDevCaps.mMaxTextures = 1;
		mDevCaps.mMaxStreams = 1;
		mStateBlock = BLADE_NEW D3D9StateBlock();
		mLastDecl = NULL;
		mLastStreams = NULL;
		mLastIndices = NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	D3D9RenderDevice::~D3D9RenderDevice()
	{
		this->close();

		assert( mD3D9Device == NULL );

		if( mD3D9 != NULL )
		{
			ULONG count = mD3D9->Release();
			assert(count == 0);
			BLADE_UNREFERENCED(count);
			mD3D9 = NULL;
		}

		BLADE_DELETE mStateBlock;
	}

	/************************************************************************/
	/* common IDevice specs                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool	D3D9RenderDevice::open()
	{
		//note: sometimes D3D9GraphicsResourceManager needed before device really created. i.e. for shader compiling tool
		//replace the old graphics resource manager,if exist
		IGraphicsResourceManager::interchange(D3D9GraphicsResourceManager::TYPE);
		return mD3D9Device != NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	D3D9RenderDevice::isOpen() const
	{
		return mD3D9Device != NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	void	D3D9RenderDevice::close()
	{
		ID3D9LostResourceManager::getSingleton().releaseDefaultResource();
		//remove the current graphics resource manager
		IGraphicsResourceManager::resetSingleton();
		mRenderTargetList.clear();
		mRenderWindowList.clear();

		if( mD3D9 != NULL )
		{
			if( mD3D9Device != NULL )
			{
				this->clearResourceBindings();
				ULONG count = mD3D9Device->Release();
				assert(count == 0);	//check resource leaks
				BLADE_UNREFERENCED(count);
				mD3D9Device = NULL;
			}
		}
		else
		{
			assert( mD3D9Device == NULL );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool	D3D9RenderDevice::reset()
	{
		assert( mD3D9Device );

		mD3DPP.Windowed = !mFullScreen;

		const int MAX_TRY_COUNT = 5;
		int tryCount = 0;

		this->clearResourceBindings();

		HRESULT result = mD3D9Device->Reset( &mD3DPP );
		if( FAILED(result) )
		{
			do 
			{
				::Sleep(100);
				result = mD3D9Device->Reset( &mD3DPP );
				++tryCount;
			} while ( FAILED(result) && tryCount < MAX_TRY_COUNT );
		}
		
		if( FAILED(result) )
		{
			tchar Desc[1024];
			std::memset(Desc, 0, sizeof(Desc));
			DXGetErrorDescription(result, Desc, 1024);
			BLADE_EXCEPT(EXC_API_ERROR,BTString("unable to reset device: ") + DXGetErrorString(result) + BTString("\nDescription: ") + Desc);
		}

		ID3D9LostResourceManager::getSingleton().rebuildDefaultResource();

		mStateBlock->resetState();
		mLastTarget = NULL;
		mNULLColor = this->createEmptyRenderTarget((size_t)1u, (size_t)1u);
		mLastDecl = NULL;
		mLastStreams = NULL;
		mLastIndices = NULL;
		mLastVertexShader = NULL;
		mLastPixelShader = NULL;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	D3D9RenderDevice::update()
	{
		if(mD3D9Device != NULL)
		{
			//note: device lost should be handled inside the device module,
			//because other type of devices may not have such a bad status
			//according to DX9 doc, reset must be performed on the window thread.
			if( D3DERR_DEVICELOST == mD3D9Device->TestCooperativeLevel() )
			{
				//ID3D9Helper::getSingleton().onDeviceLost();
				ID3D9Helper::getSingleton().outputString(BTString("D3D9 Device lost."));
				ID3D9LostResourceManager::getSingleton().releaseDefaultResource();

				return false;
			}
			else if( mD3D9Device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET )
			{
				//on some systems (some Windows 10) we get a D3DERR_DEVICENOTRESET without device lost,
				//probably caused by application's render state/target error,
				//perform clean up anyway
				ID3D9LostResourceManager::getSingleton().releaseDefaultResource();

				if( !this->reset() )
				{
					::Sleep(100);
					return false;
				}
				else
				{
					//ID3D9Helper::getSingleton().onDeviceReset();
					ID3D9Helper::getSingleton().outputString(BTString("D3D9 Device reset."));
					return true;
				}
			}
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void	D3D9RenderDevice::getDeviceOption(ParamList& defVal, Options& outOption)
	{
		TempTStringSet adapterSet;
		TStringParam adapters;

		for( UINT id = 0 ; id < mAdapterCount; ++id )
		{
			D3DADAPTER_IDENTIFIER9 adapterId;
			std::memset(&adapterId, 0, sizeof(adapterId));
			mD3D9->GetAdapterIdentifier(id,0,&adapterId);

			TString AdapterDesc_tstr = StringConverter::StringToTString(adapterId.Description);
			AdapterDesc_tstr = TStringHelper::trim( AdapterDesc_tstr );
			if( adapterSet.insert(AdapterDesc_tstr).second )
				adapters.push_back( AdapterDesc_tstr );
		}
		mAdapterCount = (UINT)adapterSet.size();

		Variant& v = defVal[D3D9OptionString::ADAPTER_COUNT];
		v = mAdapterCount;
		v.setReadOnly(true);

		defVal[D3D9OptionString::VSYNC] = mVSync;

		defVal[D3D9OptionString::ADAPTER_LIST] = *adapterSet.begin();
		outOption[D3D9OptionString::ADAPTER_LIST] = adapters;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	D3D9RenderDevice::applyDeviceOption(const ParamList& result)
	{
		const Variant& val = result[ D3D9OptionString::ADAPTER_COUNT ];
		if(val.isValid())
		{
			UINT AdapterCount = val;

			//saved config is old,because current value is not as the saved.
			//this may be caused by a new(second) adapter installed on the system
			//return false will give a chance for user to choose  when adapter changes.
			if( AdapterCount != mAdapterCount )
				return false;
		}

		const Variant& adapter = result[ D3D9OptionString::ADAPTER_LIST ];
		if(adapter.isValid())
		{
			mAdapterID = UINT(-1);
			for( UINT id = 0 ; id < mAdapterCount; ++id )
			{
				const TString& adapterDesc = adapter;

				D3DADAPTER_IDENTIFIER9 adapterId;
				std::memset(&adapterId, 0, sizeof(adapterId));
				mD3D9->GetAdapterIdentifier(id,0,&adapterId);

				TString AdapterDesc_tstr = StringConverter::StringToTString(adapterId.Description);
				AdapterDesc_tstr = TStringHelper::trim( AdapterDesc_tstr );

				if( adapterDesc == AdapterDesc_tstr )
				{
					mAdapterID = id;
					break;
				}
			}
		}

		//adapter has changed.
		if (mAdapterID == UINT(-1))
		{
			mAdapterID = 0;
			return false;
		}

		const Variant& vsync = result[D3D9OptionString::VSYNC];
		if (vsync.isValid())
			mVSync = vsync;

		return true;
	}

	/************************************************************************/
	/* IRenderDevice specs                                                                     */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	const RENDERDEV_CAPS&	D3D9RenderDevice::getDeviceCaps() const
	{
		return mDevCaps;
	}

	//////////////////////////////////////////////////////////////////////////
	IRenderWindow*	D3D9RenderDevice::createRenderWindow(IWindow* pwin, bool depthStencil/* = false*/, PixelFormat depthFormat/* = PF_DEPTH24S8*/)
	{
		assert( pwin != NULL );
		const TString& WinName = pwin->getUniqueName();
		ScopedLock lock(mThreadLock);

		HWND hwnd = (HWND)(void*)pwin->getNativeHandle();
		if( !::IsWindow(hwnd) )
			BLADE_EXCEPT(EXC_INVALID_PARAM,BTString("invalid window handle.") );

		HRENDERTARGET& rt = mRenderTargetList[WinName];
		HRENDERWINDOW& RenderWin = mRenderWindowList[WinName];
		if( RenderWin != NULL || rt != NULL )
			BLADE_EXCEPT(EXC_REDUPLICATE,BTString("unable to create renderwindow -\"")+WinName+BTString("\", a instance with the same name already exist.")  );

		size_t nWidth = pwin->getWidth();
		size_t nHeight = pwin->getHeight();

		IDirect3DSwapChain9* pSwapChain = NULL;
		HRESULT result;
		bool MainWindow = false;

		D3D9RenderWindow* renderwindow = NULL;
		if( mD3D9Device == NULL )
		{
			MainWindow = true;
			this->createDevice(hwnd, pwin->getColorDepth(), (UINT)nWidth, (UINT)nHeight, pwin->getFullScreen() );
			result = mD3D9Device->GetSwapChain(0,&pSwapChain);
		}

		//no window exist, size()== 1 means the newly added one: this one, that to be created
		//the window that the device used on creation is closed,but the implicit swap chain exist,so
		//reuse the swap chain
		else if( mRenderWindowList.size() == 1 )
		{
			assert( mRenderTargetList.size() == 1);
			MainWindow = true;
			result = mD3D9Device->GetSwapChain(0,&pSwapChain);
			if( FAILED(result) )
				BLADE_EXCEPT(EXC_API_ERROR,BTString("unable to get the device implicit swap chain.") );
		}
		else
		{
			if( mFullScreen || pwin->getFullScreen() )
				BLADE_EXCEPT(EXC_API_ERROR,BTString("cannot create a second render window in full screen mode.") );

			D3DPRESENT_PARAMETERS d3dpp;
			::ZeroMemory(&d3dpp,sizeof(D3DPRESENT_PARAMETERS));

			d3dpp.BackBufferWidth = 0;
			d3dpp.BackBufferHeight = 0;
			d3dpp.AutoDepthStencilFormat = D3DFMT_UNKNOWN;
			d3dpp.BackBufferCount = 1;
			d3dpp.BackBufferFormat = D3DFormatConverter::getBackBufferFormat();
			d3dpp.Windowed   = TRUE;
			d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
			d3dpp.hDeviceWindow = hwnd;
			d3dpp.EnableAutoDepthStencil = FALSE;
			if (mVSync)
			{
				d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;	//VSync
				d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
			}
			else
			{
				d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
				d3dpp.FullScreen_RefreshRateInHz = 0;
			}

			result = mD3D9Device->CreateAdditionalSwapChain(&d3dpp,&pSwapChain);
			if( FAILED(result) )
				BLADE_EXCEPT(EXC_API_ERROR,BTString("unable to create additional swap chain.") );
		}

		renderwindow = BLADE_NEW D3D9RenderWindow(pSwapChain, pwin, this, MainWindow);

		if( depthStencil )
		{
			HTEXTURE hDepthBuffer = IGraphicsResourceManager::getSingleton().createDepthStencilBuffer(nWidth, nHeight, depthFormat);
			renderwindow->getDepthBuffer() = hDepthBuffer;
		}

		RenderWin.bind( renderwindow );
		rt = RenderWin;
		return renderwindow;
	}


	//////////////////////////////////////////////////////////////////////////
	IRenderWindow*	D3D9RenderDevice::getRenderWindow(const TString& name) const
	{
		ScopedLock lock(mThreadLock);
		RenderWindowMap::const_iterator i = mRenderWindowList.find(name);
		if( i == mRenderWindowList.end() )
			return NULL;
		else
			return i->second;
	}


	//////////////////////////////////////////////////////////////////////////
	size_t			D3D9RenderDevice::getNumRenderWindows() const
	{
		ScopedLock lock(mThreadLock);
		return mRenderWindowList.size();
	}

	//////////////////////////////////////////////////////////////////////////
	void			D3D9RenderDevice::notifyRenderWindowChange(IRenderWindow* rw)
	{
		if( rw->getFullScreen() && this->getNumRenderWindows() > 1 )
			BLADE_EXCEPT(EXC_API_ERROR,BTString("cannot set fullscreen in multi window mode."));

		D3D9RenderWindow* window = static_cast<D3D9RenderWindow*>(rw);

		if( rw->getFullScreen() )
		{
			assert( window->isMainWindow() );
			if( mFullScreen )
			{
				//full screen mode/size switch: nothing TODO
			}
			else
				mFullScreen = true;

			mD3DPP.BackBufferWidth = (UINT)rw->getWindowWidth();
			mD3DPP.BackBufferHeight = (UINT)rw->getWindowHeight();
			mD3DPP.BackBufferFormat = D3DFormatConverter( window->getPixelFormat() );
			D3DFormatConverter::setBackBuferFormat( mD3DPP.BackBufferFormat );

			UINT maxWidth = (UINT)::GetSystemMetrics(SM_CXSCREEN);
			UINT maxHeight = (UINT)::GetSystemMetrics(SM_CYSCREEN);

			if( mD3DPP.BackBufferWidth > maxWidth )
				mD3DPP.BackBufferWidth = maxWidth;
			if( mD3DPP.BackBufferHeight > maxHeight )
				mD3DPP.BackBufferHeight = maxHeight;

			ID3D9LostResourceManager::getSingleton().releaseDefaultResource();
			this->reset();
		}
		else
		{
			if( mFullScreen )//rebuild back buffer
			{
				assert( window->isMainWindow() );
				mFullScreen = false;
				mD3DPP.BackBufferWidth = (UINT)::GetSystemMetrics(SM_CXSCREEN);
				mD3DPP.BackBufferHeight = (UINT)::GetSystemMetrics(SM_CYSCREEN);

				ID3D9LostResourceManager::getSingleton().releaseDefaultResource();
				this->reset();
			}
			else
			{
				//main window is created with desktop size back buffer
				if( !window->isMainWindow() )
				{
					D3DFORMAT format = D3DFormatConverter( window->getPixelFormat() );

					const D3DPRESENT_PARAMETERS& d3dppRef = window->getD3D9Desc();
					if( d3dppRef.BackBufferWidth != window->getWindowWidth() || d3dppRef.BackBufferHeight != window->getWindowHeight() || d3dppRef.BackBufferFormat != format )
					{
						D3DPRESENT_PARAMETERS d3dpp = d3dppRef;
						d3dpp.Windowed = TRUE;
						d3dpp.BackBufferWidth = (UINT)window->getWindowWidth();
						d3dpp.BackBufferHeight = (UINT)window->getWindowHeight();
						d3dpp.BackBufferFormat = format;
						window->release();
						window->setD3D9Desc(d3dpp);
						window->rebuild(mD3D9Device);
					}
				}
			}
		}
	}


	//////////////////////////////////////////////////////////////////////////
	IRenderTarget*	D3D9RenderDevice::createRenderTarget(const TString& name, size_t width, size_t height,
		bool creatColorBuffer/* = false*/, bool createDepthBuffer/* = false*/,
		PixelFormat format/* = PF_UNDEFINED*/, PixelFormat depthFormat/* = PF_UNDEFINED*/)
	{
		assert( mD3D9Device );
		ScopedLock lock(mThreadLock);

		HRENDERTARGET& rt = mRenderTargetList[name];
		if( rt != NULL )
			BLADE_EXCEPT(EXC_REDUPLICATE,BTString("unable to create rendertarget -\"")+name+BTString("\", a instance with the same name already exist.")  );

		rt.bind( BLADE_NEW RenderTarget(name, this, width, height) );
		
		if( creatColorBuffer )
		{
			HTEXTURE hColorBuffer = IGraphicsResourceManager::getSingleton().createTexture(width, height, depthFormat, 1, 
				IGraphicsBuffer::GBUF_RENDERTARGET, TT_2DIM, IMGO_DEFAULT, 1);
			rt->setColorBuffer(0, hColorBuffer);
		}

		if( createDepthBuffer )
		{
			HTEXTURE hDepthBuffer = IGraphicsResourceManager::getSingleton().createDepthStencilBuffer(width,height,depthFormat);
			rt->setDepthBuffer( hDepthBuffer );
		}
		return rt;
	}

	//////////////////////////////////////////////////////////////////////////
	const SAMPLER_OPTION&	D3D9RenderDevice::getGlobalSamplerOption() const
	{
		return mStateBlock->getGlobalFilter();
	}

	//////////////////////////////////////////////////////////////////////////
	void			D3D9RenderDevice::setGlobalSamplerOption(const SAMPLER_OPTION& filter)
	{
		mStateBlock->setGlobalFilter( filter );
	}

	//////////////////////////////////////////////////////////////////////////
	IRenderTarget*	D3D9RenderDevice::getRenderTarget(const TString& name) const
	{
		ScopedLock lock(mThreadLock);
		RenderTargetMap::const_iterator i = mRenderTargetList.find(name);
		if( i == mRenderTargetList.end() )
			return NULL;
		else
			return i->second;
	}


	//////////////////////////////////////////////////////////////////////////
	size_t			D3D9RenderDevice::getNumRenderTargets() const
	{
		ScopedLock lock(mThreadLock);
		return mRenderTargetList.size();
	}


	//////////////////////////////////////////////////////////////////////////
	bool			D3D9RenderDevice::destroyRenderTarget(const TString& name)
	{
		ScopedLock lock(mThreadLock);
		RenderTargetMap::iterator i = mRenderTargetList.find(name);
		if( i != mRenderTargetList.end() )
		{
			mRenderWindowList.erase(name);
			mRenderTargetList.erase(i);

			if( mRenderWindowList.size() == 0 )
			{
				//no render targets if no window at all
				//assert(mRenderTargetList.size() == 0);
				//re-init the device (reopen) ?
				//this->close();
				//this->open();
			}
			return true;
		}
		else
			return false;
	}


	//////////////////////////////////////////////////////////////////////////
	void			D3D9RenderDevice::setRenderTarget(IRenderTarget* target)
	{
		if( target == NULL )
			BLADE_EXCEPT(EXC_INVALID_PARAM,BTString("invalid render target."));

		//setup MRT, if exist
		size_t count = target->getColorBufferCount();
		if( count > mDevCaps.mMaxMRT )
			BLADE_EXCEPT(EXC_INVALID_PARAM, BTString("MRT count exceeds hardware limit.") );

		//clear textures in case RTT bound to texture sampler?
		//for (size_t i = 0; i < mDevCaps.mMaxTextures; ++i)
		//	mD3D9Device->SetTexture((DWORD)i, NULL);
		bool linearTarget = false;

		for( size_t i = 0; i < count; ++i)
		{
			const HTEXTURE& hTarget = target->getColorBuffer(i);
			if( hTarget == NULL ||  !hTarget->getTextureUsage().isRenderTarget() || hTarget->getType() != TT_2DIM)
				BLADE_EXCEPT(EXC_INVALID_PARAM,BTString("invalid render target."));

			Texture* texture = static_cast<Texture*>(hTarget);
			if (i == 0)
				linearTarget = texture->getPixelFormat().isLinearColor();
			else
				assert(linearTarget == texture->getPixelFormat().isLinearColor() && "rendertargets color space mismatch, don't use back buffer as MRT.");

			if (mCurrentTargets[i] != hTarget)
			{
				const HPXBUFFER& pxBuffer = texture->getPixelBuffer();
				D3D9PixelBuffer* buffer = static_cast<D3D9PixelBuffer*>(pxBuffer);
				if (FAILED(mD3D9Device->SetRenderTarget((DWORD)i, buffer->getD3D9Resource())))
					BLADE_EXCEPT(EXC_API_ERROR, BTString("set render target failed."));
				mCurrentTargets[i] = hTarget;
			}
		}
		if( count == 0 )
		{
			assert(target->getDepthBuffer()->getWidth() >= mNULLColor->getWidth() && target->getDepthBuffer()->getHeight() >= mNULLColor->getHeight());
			const HTEXTURE& nullColor = mNULLColor;
			Texture* texture = static_cast<Texture*>(nullColor);
			const HPXBUFFER& pxBuffer = texture->getPixelBuffer();
			D3D9PixelBuffer* buffer = static_cast<D3D9PixelBuffer*>(pxBuffer);
			if( FAILED(mD3D9Device->SetRenderTarget(0, buffer->getD3D9Resource())) )
				BLADE_EXCEPT(EXC_API_ERROR,BTString("set render target failed."));
			count = 1;

			texture->setViewRect(0, 0, 1, 1);
			mCurrentTargets[0] = nullColor;
		}
		else
			mStateBlock->setRenderTargetState(linearTarget);

		for(size_t i = count; i < mMaxUsedTarget; ++i)
		{
			mD3D9Device->SetRenderTarget((DWORD)i,NULL);
			mCurrentTargets[i] = HTEXTURE::EMPTY;
		}
		mMaxUsedTarget = count;

		//setup depth
		const HTEXTURE& depthBuffer = target->getDepthBuffer();
		if(  depthBuffer  != NULL )
		{
			if( !depthBuffer->getTextureUsage().isDepthStencil() || depthBuffer->getType() != TT_2DIM )
				BLADE_EXCEPT(EXC_INVALID_PARAM,BTString("invalid depth buffer."));

			D3D9Texture* texture = static_cast<D3D9Texture*>(depthBuffer);

			if (mCurrentTargets[RenderTarget::MAX_MRT_COUNT] != depthBuffer)
			{
				const HPXBUFFER& pxBuffer = texture->getPixelBuffer();
				D3D9PixelBuffer* buffer = static_cast<D3D9PixelBuffer*>(pxBuffer);

#if BLADE_DEBUG
				D3DDISPLAYMODE mode;
				mD3D9->GetAdapterDisplayMode(mAdapterID, &mode);

				// verify that the depth format exists
				if (FAILED(mD3D9->CheckDeviceFormat(mAdapterID,
					D3DDEVTYPE_HAL,
					mode.Format,
					D3DUSAGE_DEPTHSTENCIL,
					D3DRTYPE_SURFACE,
					buffer->getD3D9Desc().Format)))
				{
					BLADE_EXCEPT(EXC_API_ERROR, BTString("invalid depth stencil format"));
				}

				// verify that the depth format is compatible
				if (FAILED(mD3D9->CheckDepthStencilMatch(D3DADAPTER_DEFAULT,
					D3DDEVTYPE_HAL,
					mode.Format,
					mD3DPP.BackBufferFormat,
					buffer->getD3D9Desc().Format)))
				{
					BLADE_EXCEPT(EXC_API_ERROR, BTString("invalid depth stencil format"));
				}
#endif

				if (FAILED(mD3D9Device->SetDepthStencilSurface(buffer->getD3D9Resource())))
					BLADE_EXCEPT(EXC_API_ERROR, BTString("set depth stencil failed."));

				mCurrentTargets[RenderTarget::MAX_MRT_COUNT] = depthBuffer;
			}
		}
		else
		{
			mD3D9Device->SetDepthStencilSurface( NULL );
			mCurrentTargets[RenderTarget::MAX_MRT_COUNT] = HTEXTURE::EMPTY;
		}

		//reset view rect, until set render view is called
		target->setViewRect(0, 0, (int32)target->getViewWidth(), (int32)target->getViewHeight());
		mLastTarget = target;
	}


	//////////////////////////////////////////////////////////////////////////
	bool			D3D9RenderDevice::beginScene()
	{
		HRESULT result = mD3D9Device->BeginScene();
		return SUCCEEDED(result);
	}

	//////////////////////////////////////////////////////////////////////////
	bool			D3D9RenderDevice::endScene()
	{
		for(size_t i = 0; i < RenderTarget::MAX_MRT_COUNT+1; ++i)
			mCurrentTargets[i] = HTEXTURE::EMPTY;

		HRESULT result = mD3D9Device->EndScene();
		return SUCCEEDED(result);
	}

	//////////////////////////////////////////////////////////////////////////
	bool			D3D9RenderDevice::clear(uint32 frameBufferType, const Color& color, scalar depth/* = 1.0*/, uint16 stencil/* = 0*/)
	{
		DWORD clearflags = 0;
		if( frameBufferType & FBT_COLOR )
			clearflags |= D3DCLEAR_TARGET;

		if( frameBufferType & FBT_DEPTH )
			clearflags |= D3DCLEAR_ZBUFFER;

		if( frameBufferType & FBT_STENCIL )
			clearflags |= D3DCLEAR_STENCIL;

		if( clearflags == 0 )
			return true;

		HRESULT result = mD3D9Device->Clear(0, NULL, clearflags, color.getBGRA(), depth, stencil);
		if( FAILED(result) )
			BLADE_EXCEPT(EXC_API_ERROR,BTString("unable to clear frame buffer:") + DXGetErrorString(result) );
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void	D3D9RenderDevice::setRenderView(IRenderView* view)
	{
		if( view == NULL )
		{
			assert(false);
			return;
		}

		D3DVIEWPORT9 vpt;
		vpt.X = (DWORD)view->getPixelLeft();
		vpt.Y = (DWORD)view->getPixelTop();
		vpt.Width = view->getPixelRight() - vpt.X;
		vpt.Height = view->getPixelBottom() - vpt.Y;
		vpt.MinZ = 0.0f;
		vpt.MaxZ = 1.0f;
		HRESULT ret = mD3D9Device->SetViewport(&vpt);

		if(!SUCCEEDED(ret))
		{
			assert(false);
			return;
		}
		if (mLastTarget != NULL)
			mLastTarget->setViewRect((int32)vpt.X, (int32)vpt.Y, (int32)vpt.Width, (int32)vpt.Height);
	}

	//////////////////////////////////////////////////////////////////////////
	void			D3D9RenderDevice::setRenderProperty(const RenderPropertySet& propSet)
	{
		mStateBlock->setFillMode( propSet.getFillMode() );
		mStateBlock->setCullMode( propSet.getCullMode() );

		mStateBlock->setColorWriteProperty( static_cast<ColorWriteProperty*>(propSet.getProperty(RPT_COLORWIRTE)) );
		mStateBlock->setAlphaBlendProperty( static_cast<AlphaBlendProperty*>(propSet.getProperty(RPT_ALPHABLEND)) );
		mStateBlock->setDepthProperty( static_cast<DepthProperty*>(propSet.getProperty(RPT_DEPTH)) );
		mStateBlock->setStencilProperty( static_cast<StencilProperty*>(propSet.getProperty(RPT_STENCIL)) );
		mStateBlock->setScissorProperty( static_cast<ScissorProperty*>(propSet.getProperty(RPT_SCISSOR)) );
	}

	//////////////////////////////////////////////////////////////////////////
	void			D3D9RenderDevice::setTexture(EShaderType type, index_t samplerIndex, const HTEXTURE& texture, const Sampler* sampler/* = NULL*/)
	{
		if( mD3D9Device == NULL )
			return;
		HRESULT result;

		if( texture == NULL )
		{
			result = mD3D9Device->SetTexture((DWORD)samplerIndex, NULL);
			if( FAILED(result) )
				BLADE_EXCEPT(EXC_API_ERROR, BTString("unable to bind texture"));
			return;
		}

		//no read access, probably a surface(back buffer render target,see @D3D9RenderSurface ), not a texture
		if( (texture->getGpuAccess() & IGraphicsBuffer::GBAF_READ) == 0 )
			BLADE_EXCEPT(EXC_INVALID_PARAM, BTString("unable to set a none-read texture.") );

		if( type == SHADER_FRAGMENT || type == SHADER_VERTEX )
		{
			if( type == SHADER_VERTEX )
				samplerIndex += D3DDMAPSAMPLER;

			//!!hard code
			ETextureType texType = texture->getType();
			if( texType == TT_2DIM || texType == TT_1DIM )
			{
				D3D9Texture* pTexture = static_cast<D3D9Texture*>(texture);
				result = mD3D9Device->SetTexture( (DWORD)samplerIndex, pTexture->getD3D9Resource() );
			}
			else if( texType == TT_CUBE )
			{
				D3D9CubeTexture* pTexture = static_cast<D3D9CubeTexture*>(texture);
				result = mD3D9Device->SetTexture( (DWORD)samplerIndex, pTexture->getD3D9Resource() );
			}
			else if( texType == TT_3DIM )
			{
				D3D9VolumeTexture* pTexture = static_cast<D3D9VolumeTexture*>(texture);
				result = mD3D9Device->SetTexture( (DWORD)samplerIndex, pTexture->getD3D9Resource() );
			}
			else
			{
				assert(false);
				result = E_FAIL;
			}
		}
		else
			result = E_FAIL;

		if( FAILED(result) )
			BLADE_EXCEPT(EXC_API_ERROR,BTString("unable to bind texture"));

		//d3d9 D3DSAMP_SRGBTEXTURE need be set after texture being bound
		//http://stackoverflow.com/questions/6509956/directx9-gamma-correction-applied-implicitely
		mStateBlock->compareSetSampler(sampler, samplerIndex, texture);
	}

	//////////////////////////////////////////////////////////////////////////
	void			D3D9RenderDevice::setShader(const HSHADER& program)
	{
		if( mD3D9Device != NULL && program != NULL )
		{
			if( program->isReady() )
			{
				HRESULT hr = S_OK;
				D3D9Shader* shader = static_cast<D3D9Shader*>(program);

				if( mLastVertexShader != shader->getVertexShader() )
				{
					hr = mD3D9Device->SetVertexShader( shader->getVertexShader() );
					if( FAILED(hr) )
						BLADE_EXCEPT(EXC_API_ERROR,BTString("set shader failed.") );
					mLastVertexShader = shader->getVertexShader();
				}

				/*if (mCurrentTargets[0] == mNULLColorMin || mCurrentTargets[0] == mNULLColorMax)
				{
					hr = mD3D9Device->SetPixelShader(NULL);
					if (FAILED(hr))
						BLADE_EXCEPT(EXC_API_ERROR, BTString("set shader failed."));
					mLastPixelShader = NULL;
				}
				else */if( mLastPixelShader != shader->getPixelShader() )
				{
					hr = mD3D9Device->SetPixelShader( shader->getPixelShader() );
					if( FAILED(hr) )
						BLADE_EXCEPT(EXC_API_ERROR,BTString("set shader failed.") );
					mLastPixelShader = shader->getPixelShader();
				}
			}
			else
				BLADE_EXCEPT(EXC_API_ERROR,BTString("invalid shader program.") );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void			D3D9RenderDevice::setShaderConstant(const IShader::CONSTANT_KEY& key, size_t bytes, const float* pData)
	{
		if( mD3D9Device != NULL )
		{
			if( pData != NULL )
			{
				HRESULT hr;
				//note: low profile shaders may have fewer constant, so key.mMaxBytes maybe smaller than bytes
				size_t validBytes = std::min<size_t>(bytes, key.mMaxBytes);
				if( key.mShaderType == SHADER_VERTEX )
					hr = mD3D9Device->SetVertexShaderConstantF( (UINT)key.mIndex, pData, (UINT)(validBytes+15)/16);
				else if( key.mShaderType == SHADER_FRAGMENT)
					hr = mD3D9Device->SetPixelShaderConstantF( (UINT)key.mIndex, pData, (UINT)(validBytes+15)/16);
				else
					hr = E_INVALIDARG;

				if( FAILED(hr) )
					BLADE_EXCEPT(EXC_API_ERROR,BTString("set shader constant failed.") );
			}
			else
				BLADE_EXCEPT(EXC_INVALID_PARAM,BTString("invalid shader constant data."));
		}
		else
			BLADE_EXCEPT(EXC_NEXIST,BTString("render device not ready."));
	}

	//////////////////////////////////////////////////////////////////////////
	void			D3D9RenderDevice::setShaderConstant(const IShader::CONSTANT_KEY& key, size_t bytes, const int* pData)
	{
		if( mD3D9Device != NULL )
		{
			if( pData != NULL )
			{
				HRESULT hr;
				size_t validBytes = std::min<size_t>(bytes, key.mMaxBytes);
				if( key.mShaderType == SHADER_VERTEX )
					hr = mD3D9Device->SetVertexShaderConstantI( (UINT)key.mIndex, pData, (UINT)(validBytes+15)/16);
				else if( key.mShaderType == SHADER_FRAGMENT)
					hr = mD3D9Device->SetPixelShaderConstantI( (UINT)key.mIndex, pData, (UINT)(validBytes+15)/16);
				else
					hr = E_INVALIDARG;

				if( FAILED(hr) )
					BLADE_EXCEPT(EXC_API_ERROR,BTString("set shader constant failed.") );
			}
			else
				BLADE_EXCEPT(EXC_INVALID_PARAM,BTString("invalid shader constant data."));
		}
		else
			BLADE_EXCEPT(EXC_NEXIST,BTString("render device not ready."));
	}

	//////////////////////////////////////////////////////////////////////////
	void			D3D9RenderDevice::setShaderConstant(const IShader::CONSTANT_KEY& key, size_t bytes, const SC_BOOL* pData)
	{
		if( mD3D9Device != NULL )
		{
			if( pData != NULL )
			{
				HRESULT hr;
				size_t validBytes = std::min<size_t>(bytes, key.mMaxBytes);
				if( key.mShaderType == SHADER_VERTEX )
					hr = mD3D9Device->SetVertexShaderConstantB( (UINT)key.mIndex, (const BOOL*)pData, (UINT)(validBytes+sizeof(SC_BOOL)-1)/sizeof(SC_BOOL) );
				else if( key.mShaderType == SHADER_FRAGMENT)
					hr = mD3D9Device->SetPixelShaderConstantB( (UINT)key.mIndex, (const BOOL*)pData, (UINT)(validBytes+sizeof(SC_BOOL)-1)/sizeof(SC_BOOL) );
				else
					hr = E_INVALIDARG;

				if( FAILED(hr) )
					BLADE_EXCEPT(EXC_API_ERROR,BTString("set shader constant failed.") );
			}
			else
				BLADE_EXCEPT(EXC_INVALID_PARAM,BTString("invalid shader constant data."));
		}
		else
			BLADE_EXCEPT(EXC_NEXIST,BTString("render device not ready."));
	}

	//////////////////////////////////////////////////////////////////////////
	void			D3D9RenderDevice::renderGeometry(const GraphicsGeometry& geom)
	{
		if( geom.mVertexCount == 0 )
			return;

		//set indexes & draw primitives
		DWORD pointCount = DWORD( geom.hasIndexBuffer() ? geom.mIndexCount : geom.mVertexCount);
		D3DPRIMITIVETYPE primitiveType;
		DWORD primitiveCount;
		switch( geom.mPrimitiveType )
		{
		case GraphicsGeometry::GPT_POINT_LIST:
			primitiveType = D3DPT_POINTLIST;
			primitiveCount = pointCount;
			break;
		case GraphicsGeometry::GPT_LINE_LIST:
			primitiveType = D3DPT_LINELIST;
			primitiveCount = pointCount/2;
			break;
		case GraphicsGeometry::GPT_LINE_STRIP:
			primitiveType = D3DPT_LINESTRIP;
			primitiveCount = pointCount - 1;
			break;
		case GraphicsGeometry::GPT_TRIANGLE_LIST:
			primitiveType = D3DPT_TRIANGLELIST;
			primitiveCount = pointCount/3;
			break;
		case GraphicsGeometry::GPT_TRIANGLE_STRIP:
			primitiveType = D3DPT_TRIANGLESTRIP;
			primitiveCount = pointCount - 2;
			break;
		case GraphicsGeometry::GPT_TRIANGLE_FAN:
			primitiveType = D3DPT_TRIANGLEFAN;
			primitiveCount = pointCount - 2;
			break;
		default:
			primitiveType = D3DPT_POINTLIST;
			primitiveCount = 0;
		}

		if( primitiveCount == 0 )
			return;

		HRESULT result;
		//set vertex decl
		const D3D9VertexDeclaration* decl = static_cast<const D3D9VertexDeclaration*>(geom.mVertexDecl);
		if(mLastDecl != decl)
		{
			result = mD3D9Device->SetVertexDeclaration(decl->getD3D9Declaration(mD3D9Device));
			if( SUCCEEDED(result) )
				mLastDecl = decl;
			else
				BLADE_EXCEPT(EXC_API_ERROR,BTString("unable to set vertex declaration.") );
		}

		//set stream source
		if (mLastStreams != geom.mVertexSource)
		{
			mLastStreams = geom.mVertexSource;
			uint16 validsource = 0;
			for (uint16 i = 0; i < geom.mVertexSource->getBufferSourceCount(); ++i)
			{
				const StreamInfo& info = geom.mVertexSource->getStreamInfo(i);
				const HVBUFFER& hvb = info.buffer;
				if (hvb == NULL)
				{
					result = mD3D9Device->SetStreamSource(static_cast<UINT>(i), NULL, 0, 0);
					assert(SUCCEEDED(result));
					result = mD3D9Device->SetStreamSourceFreq(static_cast<UINT>(i), 1);
					assert(SUCCEEDED(result));
					++validsource;
					continue;
				}
				assert(validsource == info.source);

				D3D9VertexBuffer* vb = static_cast<D3D9VertexBuffer*>(hvb);
				result = mD3D9Device->SetStreamSource((UINT)validsource, vb->getD3D9Resource(), (UINT)info.offset, (UINT)vb->getVertexSize());
				if (FAILED(result))
					BLADE_EXCEPT(EXC_API_ERROR, BTString("unable to set D3D9 stream source or frequency."));

				if (!geom.isInstancing())
					result = mD3D9Device->SetStreamSourceFreq((UINT)validsource, 1);
				else
				{
					assert(geom.hasIndexBuffer());	//hardware instancing need index buffer, or it will be slow
					UINT settings = (info.source == geom.mInstanceSourceIndex) ? (UINT)D3DSTREAMSOURCE_INSTANCEDATA : (UINT)D3DSTREAMSOURCE_INDEXEDDATA;
					UINT frequency = (info.source == geom.mInstanceSourceIndex) ? 1u : (UINT)geom.mInstanceCount;
					result = mD3D9Device->SetStreamSourceFreq((UINT)validsource, settings | frequency);
				}
				if (FAILED(result))
					BLADE_EXCEPT(EXC_API_ERROR, BTString("unable to set D3D9 stream source or frequency."));
				++validsource;
			}
			for (size_t i = validsource; i < mMaxSourceID; ++i)
			{
				result = mD3D9Device->SetStreamSource(static_cast<UINT>(i), NULL, 0, 0);
				assert(SUCCEEDED(result));
				result = mD3D9Device->SetStreamSourceFreq(static_cast<UINT>(i), 1);
				assert(SUCCEEDED(result));
			}
			mMaxSourceID = validsource;
		}

		//draw
		if(geom.hasIndexBuffer())
		{
			if (mLastIndices != geom.mIndexBuffer)
			{
				mLastIndices = geom.mIndexBuffer;
				const D3D9IndexBuffer* ib = static_cast<const D3D9IndexBuffer*>(geom.mIndexBuffer);
				result = mD3D9Device->SetIndices(ib->getD3D9Resource());
				assert(SUCCEEDED(result));
			}
			else
				result = S_OK;
			if( SUCCEEDED(result) )
				result = mD3D9Device->DrawIndexedPrimitive(primitiveType, (INT)geom.mVertexStart, 0, (UINT)geom.mVertexCount, (UINT)geom.mIndexStart, (UINT)primitiveCount);
		}
		else
			result = mD3D9Device->DrawPrimitive(primitiveType, (UINT)geom.mVertexStart, (UINT)primitiveCount);

		if(FAILED(result) )
			BLADE_EXCEPT(EXC_API_ERROR, BTString("failed to draw primitives.") + DXGetErrorString(result) );
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void			D3D9RenderDevice::createDevice(HWND hwnd,int depth,UINT width,UINT height,bool fullscreen/* = false*/)
	{
		assert( hwnd != NULL );

		D3DFORMAT format;

		if( depth >16 )
			format = D3DFMT_A8R8G8B8;
		else
			format = D3DFMT_R5G6B5;

		if( !fullscreen )
		{
			//uses maximal window size,to avoid rebuild backbuffer
			width = (UINT)::GetSystemMetrics(SM_CXSCREEN);
			height = (UINT)::GetSystemMetrics(SM_CYSCREEN);
		}

		mFullScreen = fullscreen;

		mD3DPP.BackBufferWidth = width;
		mD3DPP.BackBufferHeight = height;
		mD3DPP.AutoDepthStencilFormat = D3DFMT_UNKNOWN;
		mD3DPP.BackBufferCount = 1;
		mD3DPP.BackBufferFormat = format;
		mD3DPP.Windowed   = !fullscreen;
		mD3DPP.SwapEffect = D3DSWAPEFFECT_DISCARD;
		mD3DPP.hDeviceWindow = hwnd;
		mD3DPP.EnableAutoDepthStencil = FALSE;
		if (mVSync)
		{
			mD3DPP.PresentationInterval = D3DPRESENT_INTERVAL_ONE;	//VSync
			mD3DPP.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
		}
		else
		{
			mD3DPP.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
			mD3DPP.FullScreen_RefreshRateInHz = 0;
		}

		//the task scheduler may put rendering task in any thread on each frame,
		//but there is only one rendering task can be run at the same time.
		//we don't actually need D3DCREATE_MULTITHREADED even if the rendering is running in another thread
		//because the resource creation is always in main synchronous state (or in the thread that rendering task is running in),
		//and there's no parallel access to the device
		//but we use D3DCREATE_MULTITHREADED to disable the D3D9 debug mode runtime warning.
		//in final/release build, this flag can be removed.

		//update 11/18/2018: now graphics tasks are bound to initial main thread where device is created (ITask::TA_MAIN),
		//so D3DCREATE_MULTITHREADED is not needed even for debug mode.
#define BLADE_DX9_FLAGS (D3DCREATE_PUREDEVICE)

		if( FAILED( 
			mD3D9->CreateDevice( mAdapterID, D3DDEVTYPE_HAL, NULL,
			D3DCREATE_HARDWARE_VERTEXPROCESSING | BLADE_DX9_FLAGS, &mD3DPP, &mD3D9Device ) 
			) )
		{
			BLADE_EXCEPT(EXC_API_ERROR,BTString("Render API Error:unable to create D3D9 RenderDevice.") );
		}

		ID3D9Helper::getSingleton().outputString(BTString("D3D9 Device created."));
		ID3D9LostResourceManager::getSingleton().setD3D9Device(mD3D9Device, mD3D9, mAdapterID);

		//dirty set format
		//TODO: better ways
		D3DFormatConverter::setBackBuferFormat( format );
		//the 'NULL' render target can be created in any size, but it seems that D3D9 still need 'color surface size must be less or equal to depth surface' for 'NULL' target.
		//so just use a minimal size
		mNULLColor = this->createEmptyRenderTarget(1u, 1u);

		//get device caps
		D3DCAPS9 dev_caps;
		if( FAILED(mD3D9->GetDeviceCaps(mAdapterID, D3DDEVTYPE_HAL, &dev_caps)) )
			BLADE_EXCEPT(EXC_API_ERROR,BTString("unable to get device caps.") );
		mDevCaps.mMaxTextures = dev_caps.MaxSimultaneousTextures;
		mDevCaps.mMaxMRT = dev_caps.NumSimultaneousRTs;
		mDevCaps.mMaxStreams = dev_caps.MaxStreams;

		mStateBlock->setup( mD3D9Device, mDevCaps.mMaxTextures );

		//replace the old graphics resource manager,if exist
		IGraphicsResourceManager::interchange(D3D9GraphicsResourceManager::TYPE);
	}

	//////////////////////////////////////////////////////////////////////////
	void			D3D9RenderDevice::clearResourceBindings()
	{
		if( mD3D9Device != NULL )
		{
			for(size_t i = 1; i < mDevCaps.mMaxMRT; ++i)
				mD3D9Device->SetRenderTarget((DWORD)i, NULL);

			for(size_t i = 0; i < mDevCaps.mMaxTextures; ++i)
				mD3D9Device->SetTexture((DWORD)i, NULL);

			for(size_t i = 0; i < mDevCaps.mMaxStreams; ++i )
				mD3D9Device->SetStreamSource((DWORD)i, NULL, 0, 0);

			mD3D9Device->SetIndices(NULL);
			mD3D9Device->SetDepthStencilSurface(NULL);
			mD3D9Device->SetPixelShader(NULL);
			mD3D9Device->SetVertexShader(NULL);
			mD3D9Device->SetVertexDeclaration(NULL);
			mLastDecl = NULL;
			mNULLColor.clear();

			for(size_t i = 0; i < RenderTarget::MAX_MRT_COUNT+1; ++i)
				mCurrentTargets[i] = HTEXTURE::EMPTY;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void			D3D9RenderDevice::setTextureAddressMode(index_t samplerIndex,TEXADDR_MODE u,TEXADDR_MODE v,TEXADDR_MODE w,Color::COLOR border/* = 0*/)
	{
		if( mD3D9Device != NULL )
		{
			mD3D9Device->SetSamplerState((DWORD)samplerIndex,D3DSAMP_ADDRESSU,u);
			mD3D9Device->SetSamplerState((DWORD)samplerIndex,D3DSAMP_ADDRESSV,v);
			mD3D9Device->SetSamplerState((DWORD)samplerIndex,D3DSAMP_ADDRESSW,w);
			mD3D9Device->SetSamplerState((DWORD)samplerIndex,D3DSAMP_BORDERCOLOR,border);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void			D3D9RenderDevice::setTextureFilterMode(index_t samplerIndex,TEXFILER_MODE mag,TEXFILER_MODE min,TEXFILER_MODE mip)
	{
		if( mD3D9Device != NULL )
		{
			mD3D9Device->SetSamplerState((DWORD)samplerIndex,D3DSAMP_MAGFILTER,mag);
			mD3D9Device->SetSamplerState((DWORD)samplerIndex,D3DSAMP_MINFILTER,min);
			mD3D9Device->SetSamplerState((DWORD)samplerIndex,D3DSAMP_MIPFILTER,mip);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	HTEXTURE			D3D9RenderDevice::createEmptyRenderTarget(size_t width, size_t height)
	{
		//setup empty render target 'NULL'
		D3DFORMAT fmt = (D3DFORMAT)MAKEFOURCC('N','U','L','L');
		//mD3D9->CheckDeviceFormat( mAdapterID, D3DDEVTYPE_HAL,fmt,D3DRTYPE_SURFACE,D3DUSAGE_RENDERTARGET)
		LPDIRECT3DSURFACE9 pSurface;
		HRESULT hr = mD3D9Device->CreateRenderTarget((UINT)width, (UINT)height, fmt, D3DMULTISAMPLE_NONE, 0, FALSE, &pSurface, NULL);
		if( FAILED(hr)  )
			BLADE_EXCEPT(EXC_API_ERROR,BTString("unable to create NULL render target.") );

		D3D9PixelBuffer* pxBuffer = BLADE_NEW D3D9PixelBuffer(pSurface, IPixelBuffer::PBL_RENDERTARGET);
		return HTEXTURE( BLADE_NEW D3D9RenderSurface(pxBuffer) );
	}


}//namespace Blade

#endif//#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
