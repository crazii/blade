/********************************************************************
	created:	2009/02/22
	filename: 	IRenderDevice.h
	author:		Crazii
	
	purpose:	render device abstraction
*********************************************************************/

#ifndef __Blade_IRenderDevice_h__
#define __Blade_IRenderDevice_h__
#include <interface/public/IDevice.h>
#include "RenderProperty.h"

#include "PixelFormat.h"
#include "Color.h"
#include "ITexture.h"
#include "IShader.h"
#include "RenderDeviceCaps.h"

namespace Blade
{
	class IWindow;
	class IRenderTarget;
	class IRenderWindow;
	class IRenderView;
	class GraphicsGeometry;
	class RenderPropertySet;

	enum EFrameBufferType
	{
		FBT_NONE	= 0x0,
		FBT_COLOR	= 0x1,
		FBT_DEPTH	= 0x2,
		FBT_STENCIL	= 0x4,
		FBT_ALL		= 0x7,
	};

	typedef struct SSamplerOption
	{
		TEXFILER_MODE			mag,min,mip;
		ETextureAddressMode		u,v,w;
		unsigned int			maxAnisotropy;
	}SAMPLER_OPTION;

	class IRenderDevice :public IDevice
	{
	public:
		virtual ~IRenderDevice()	{}

		/**
		@describe
		@param
		@return
		*/
		virtual const RENDERDEV_CAPS&	getDeviceCaps() const = 0;

		/**
		@describe add one render window to the device
		@param 
		@return 
		@remark uses the pwin 's unique name as the created RenderWindow's name
		*/
		virtual IRenderWindow*	createRenderWindow(IWindow* pwin, bool depthStencil = false, PixelFormat depthFormat = PF_DEPTH24S8) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual IRenderWindow*	getRenderWindow(const TString& name) const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual size_t			getNumRenderWindows() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void			notifyRenderWindowChange(IRenderWindow* rw) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual IRenderTarget*	createRenderTarget(const TString& name,size_t width,size_t height,
			bool creatColorBuffer = false,bool createDepthBuffer = false,
			PixelFormat format = PF_UNDEFINED,PixelFormat depthFormat = PF_UNDEFINED) = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual const SAMPLER_OPTION&	getGlobalSamplerOption() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void			setGlobalSamplerOption(const SAMPLER_OPTION& filter) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual IRenderTarget*	getRenderTarget(const TString& name) const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual	size_t			getNumRenderTargets() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool			destroyRenderTarget(const TString& name) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void			setRenderTarget(IRenderTarget* target) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool			beginScene() = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool			endScene() = 0;

		/**
		@describe 
		@param [in] frameBufferType combination of EFrameBufferType masks
		@return 
		*/
		virtual bool			clear(uint32 frameBufferType,const Color& Color,scalar depth = 1.0,uint16 stencil = 0) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void			setRenderView(IRenderView* view) = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual void			setRenderProperty(const RenderPropertySet& propSet) = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual void			setTexture(EShaderType type, index_t samplerIndex, const HTEXTURE& texture, const Sampler* sampler = NULL) = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual void			setShader(const HSHADER& program) = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual void			setShaderConstant(const IShader::CONSTANT_KEY& key, size_t bytes, const float* pData) = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual void			setShaderConstant(const IShader::CONSTANT_KEY& key, size_t bytes, const int* pData) = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual void			setShaderConstant(const IShader::CONSTANT_KEY& key, size_t bytes, const SC_BOOL* pData) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void			renderGeometry(const GraphicsGeometry& geom) = 0;

	};//class IRenderDevice

	typedef Handle<IRenderDevice> HRENDERDEVICE;

	extern template class BLADE_BASE_API Factory<IRenderDevice>;
	typedef Factory<IRenderDevice> RenderDeviceFactory;

}//namespace Blade


#endif // __Blade_IRenderDevice_h__