/********************************************************************
	created:	2010/04/13
	filename: 	IRenderTarget.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_IRenderTarget_h__
#define __Blade_IRenderTarget_h__
#include <Handle.h>
#include <utility/String.h>
//#include <interface/public/graphics/IPixelBuffer.h> //moved into internal, for compatibility on dx9 /dx11
#include <interface/public/graphics/ITexture.h>
#include <interface/public/graphics/IRenderView.h>

namespace Blade
{
	class Box2i;

	class BLADE_BASE_API IRenderTarget
	{
	public:
		class IListener
		{
		public:
			//called when target output size changes
			virtual void	onTargetSizeChange(IRenderTarget* target) = 0;
		};
	public:
		virtual ~IRenderTarget()	{}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const TString&	getName() const = 0;

		/*
		@describe get the built in buffer of the render target(may be NULL)
		@param 
		@return 
		*/
		virtual const HTEXTURE&getBackBuffer() const = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual const HTEXTURE&getDepthBuffer() const = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual void			setDepthBuffer(const HTEXTURE& hDethBuffer) = 0;

		/*
		@describe if a output buffer is specified,return it ,otherwise return built in buffer
		@param 
		@return 
		*/
		virtual void			setOutputColorBuffer(index_t index, const HTEXTURE& buffer) = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const HTEXTURE&getOutputColorBuffer(index_t index) const = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual size_t			getOutputColorBufferCount() const = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual PixelFormat		getBackBufferPixelFormat() const = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual int				getBackBufferColorDepth() const = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual size_t			getBackBufferWidth() const = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual size_t			getBackBufferHeight() const = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual size_t			getOutputWidth() const = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual size_t			getOutputHeight() const = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual IListener*		setListener(IListener* listener) = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual IListener*		getListener() const = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual bool			isReady() const = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			swapBuffers() = 0;

		/*
		@describe wrap for glAddSwapHintRectWIN, may be used for multiple view ports clip
		@param 
		@return 
		*/
		virtual void			addSwapBufferRect(const Box2i& rect) = 0;

		/*
		@describe
		@param 
		@return 
		*/
		virtual bool			setSwapBufferRect(const Box2i* rect, size_t count) = 0;
	};//class IRenderTarget
	
}//namespace Blade


#endif //__Blade_IRenderTarget_h__