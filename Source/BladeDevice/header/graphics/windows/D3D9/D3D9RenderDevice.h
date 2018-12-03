/********************************************************************
	created:	2009/02/22
	filename: 	D3D9RenderDevice.h
	author:		Crazii
	
	purpose:	the d3d9render-device
*********************************************************************/
#ifndef __Blade_D3D9RenderDevice_h__
#define __Blade_D3D9RenderDevice_h__
#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
#include <graphics/windows/D3D9/D3D9Header.h>

#include <BladeDevice.h>
#include <interface/public/graphics/IRenderDevice.h>
#include <interface/public/graphics/IRenderTarget.h>
#include <interface/public/graphics/IRenderWindow.h>
#include <graphics/RenderTarget.h>
#include <utility/BladeContainer.h>

namespace Blade
{
	typedef Handle<IRenderTarget> HRENDERTARGET;
	typedef Handle<IRenderWindow> HRENDERWINDOW;

	class D3D9StateBlock;
	class D3D9VertexDeclaration;
	class IVertexSource;
	class IIndexBuffer;
	
	class D3D9RenderDevice : public IRenderDevice, public StaticAllocatable
	{
	public:
		//ctor & dector
		D3D9RenderDevice();
		virtual ~D3D9RenderDevice();

		/************************************************************************/
		/* common IDevice specs                                                                     */
		/************************************************************************/
		/** @brief open a device */
		virtual bool	open();

		/** @brief check if it is open */
		virtual bool	isOpen() const;

		/** @brief close the device */
		virtual void	close();

		/** @brief reset the device */
		virtual bool	reset();

		/** @brief update data(I/O) */
		virtual bool	update();

		/** @brief  */
		virtual void	getDeviceOption(ParamList& defVal, Options& outOption);

		/** @brief  */
		virtual bool	applyDeviceOption(const ParamList& result);

		/************************************************************************/
		/* IRenderDevice specs                                                                     */
		/************************************************************************/

		/*
		@describe
		@param
		@return
		*/
		virtual const RENDERDEV_CAPS&	getDeviceCaps() const;

		/*
		@describe add one render window to the device
		@param 
		@return 
		@remark uses the pwin 's unique name as the created RenderWindow's name
		*/
		virtual IRenderWindow*	createRenderWindow(IWindow* pwin, bool depthStencil = false, PixelFormat depthFormat = PF_DEPTH24S8);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual IRenderWindow*	getRenderWindow(const TString& name) const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual size_t			getNumRenderWindows() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			notifyRenderWindowChange(IRenderWindow* rw);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual IRenderTarget*	createRenderTarget(const TString& name,size_t width,size_t height,
			bool creatColorBuffer = false,bool createDepthBuffer = false,
			PixelFormat format = PF_UNDEFINED,PixelFormat depthFormat = PF_UNDEFINED);

		/*
		@describe
		@param
		@return
		*/
		virtual const SAMPLER_OPTION&	getGlobalSamplerOption() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			setGlobalSamplerOption(const SAMPLER_OPTION& filter);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual IRenderTarget*	getRenderTarget(const TString& name) const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual	size_t			getNumRenderTargets() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			destroyRenderTarget(const TString& name);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			setRenderTarget(IRenderTarget* target);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			beginScene();

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			endScene();

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			clear(uint32 frameBufferType,const Color& Color,scalar depth = 1.0,uint16 stencil = 0);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			setRenderView(IRenderView* view);

		/*
		@describe
		@param
		@return
		*/
		virtual void			setRenderProperty(const RenderPropertySet& propSet);

		/*
		@describe
		@param
		@return
		*/
		virtual void			setTexture(EShaderType type, index_t samplerIndex, const HTEXTURE& texture, const Sampler* sampler = NULL);

		/*
		@describe
		@param
		@return
		*/
		virtual void			setShader(const HSHADER& program);

		/*
		@describe
		@param
		@return
		*/
		virtual void			setShaderConstant(const IShader::CONSTANT_KEY& key, size_t bytes, const float* pData);

		/*
		@describe
		@param
		@return
		*/
		virtual void			setShaderConstant(const IShader::CONSTANT_KEY& key, size_t bytes, const int* pData);

		/*
		@describe
		@param
		@return
		*/
		virtual void			setShaderConstant(const IShader::CONSTANT_KEY& key, size_t bytes, const SC_BOOL* pData);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			renderGeometry(const GraphicsGeometry& geom);

	protected:
		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		void			createDevice(HWND hwnd,int depth,UINT width,UINT height,bool fullscreen = false);

		/*
		@describe 
		@param 
		@return 
		*/
		void			clearResourceBindings();


		/*
		@describe
		@param
		@return
		*/
		void			setTextureAddressMode(index_t samplerIndex,TEXADDR_MODE u,TEXADDR_MODE v,TEXADDR_MODE w,Color::COLOR border = 0);

		/*
		@describe
		@param
		@return
		*/
		void			setTextureFilterMode(index_t samplerIndex,TEXFILER_MODE mag,TEXFILER_MODE min,TEXFILER_MODE mip);

		/**
		@describe 
		@param
		@return
		*/
		HTEXTURE		createEmptyRenderTarget(size_t width, size_t height);

		typedef	TStringMap<HRENDERTARGET>	RenderTargetMap;
		typedef	TStringMap<HRENDERWINDOW>	RenderWindowMap;

		UINT					mAdapterID;
		UINT					mAdapterCount;

		D3DPRESENT_PARAMETERS	mD3DPP;
		LPDIRECT3DDEVICE9		mD3D9Device;
		LPDIRECT3D9				mD3D9;

		RenderTargetMap			mRenderTargetList;
		RenderWindowMap			mRenderWindowList;

		size_t					mMaxSourceID;
		size_t					mMaxUsedTarget;
		RENDERDEV_CAPS			mDevCaps;

		D3D9StateBlock*			mStateBlock;
		IRenderTarget*			mLastTarget;
		const D3D9VertexDeclaration*	mLastDecl;
		const IVertexSource*			mLastStreams;
		const IIndexBuffer*				mLastIndices;
		IDirect3DVertexShader9*	mLastVertexShader;
		IDirect3DPixelShader9*	mLastPixelShader;
		HTEXTURE				mNULLColor;
		HTEXTURE				mCurrentTargets[RenderTarget::MAX_MRT_COUNT+1];	//max MRT + depth stencil, cached to avoid redundant set
		mutable Lock			mThreadLock;
		bool					mFullScreen;
		bool					mVSync;
	};//class D3D9RenderDevice

}//namespace Blade

#endif//#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS

#endif // __Blade_D3D9RenderDevice_h__