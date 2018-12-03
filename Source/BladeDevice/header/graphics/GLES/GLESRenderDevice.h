/********************************************************************
	created:	2014/01/14
	filename: 	GLESRenderDevice.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_GLESRenderDevice_h__
#define __Blade_GLESRenderDevice_h__
#include "GLESHeader.h"
#if BLADE_USE_GLES
#include <BladeDevice.h>
#include "GLESContext.h"
#include <interface/public/graphics/IRenderDevice.h>
#include <interface/public/graphics/IRenderTarget.h>
#include <interface/public/graphics/IRenderWindow.h>
#include <utility/BladeContainer.h>

namespace Blade
{
	typedef Handle<IRenderTarget> HRENDERTARGET;

	typedef Handle<IRenderWindow> HRENDERWINDOW;

	class GLESVertexDeclaration;
	class GLESIndexBuffer;
	class GLESStateBlock;

	class GLESRenderDevice : public IRenderDevice
	{
	public:
		GLESRenderDevice();
		~GLESRenderDevice();

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

		typedef	TStringMap<HRENDERTARGET>	RenderTargetMap;
		typedef	TStringMap<HRENDERWINDOW>	RenderWindowMap;

		RenderTargetMap		mRenderTargetList;
		RenderWindowMap		mRenderWindowList;

		HSHADER				mLastProgram;
		IRenderTarget*		mLastTarget;
		const GLESIndexBuffer* mLastIndex;

		//note: generally for GL/GLES there will be multiple context for rendering,
		//typically one context per thread, or one context per window.
		//but our graphics task are executed in linearly order, though there will be more than 1 graphics tasks,
		//and we will possibly have only one window, even if more, they should have the same config and could share the same context
		//so actually we only need one context
		GLESContext*		mContext;
		mutable Lock		mThreadLock;
		RENDERDEV_CAPS		mCaps;
		SAMPLER_OPTION		mGlobalFilter;
		GLESStateBlock*		mStateBlock;
	};//class GLESRenderDevice
	
}//namespace Blade

#endif//BLADE_USE_GLES

#endif //  __Blade_GLESRenderDevice_h__