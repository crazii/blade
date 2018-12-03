/********************************************************************
	created:	2010/04/13
	filename: 	IRenderTarget.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_IRenderTarget_h__
#define __Blade_IRenderTarget_h__
#include <utility/String.h>
#include "ITexture.h"

namespace Blade
{
	class Box2i;

	class BLADE_BASE_API IRenderTarget
	{
	public:
		class IListener
		{
		public:
			///called when target output size changes
			virtual void	onTargetSizeChange(IRenderTarget* target) = 0;
		};
	public:
		virtual ~IRenderTarget()	{}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const TString&	getName() const = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual const HTEXTURE&getDepthBuffer() const = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual bool			setDepthBuffer(const HTEXTURE& hDethBuffer) = 0;

		/**
		@describe if a output buffer is specified,return it ,otherwise return built in buffer
		@param 
		@return 
		*/
		virtual bool			setColorBuffer(index_t index, const HTEXTURE& buffer) = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual bool			setColorBufferCount(index_t count) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const HTEXTURE&getColorBuffer(index_t index = 0) const = 0;

		/** @brief  */
		inline int		getColorDepth(index_t index = 0) const
		{
			const HTEXTURE& color = this->getColorBuffer(index);
			return color != NULL ? (int)color->getPixelFormat().getSizeBytes()*8 : 0;
		}

		/** @brief  */
		inline PixelFormat	getPixelFormat(index_t index = 0) const
		{
			const HTEXTURE& color = this->getColorBuffer(index);
			return color != NULL ? color->getPixelFormat() : PixelFormat(PF_UNDEFINED);
		}

		/** @brief  */
		inline size_t		getWidth(index_t index = 0) const
		{
			const HTEXTURE& color = this->getColorBuffer(index);
			return color != NULL ? color->getWidth() : 0;
		}

		/** @brief  */
		inline size_t		getHeight(index_t index = 0) const
		{
			const HTEXTURE& color = this->getColorBuffer(index);
			return color != NULL ? color->getHeight() : 0;
		}

		/**
		@describe used internally
		@param
		@return
		*/
		virtual bool		activate()	{return true;}

		/**
		@describe 
		@param
		@return
		*/
		virtual size_t		getViewWidth() const = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual size_t		getViewHeight() const = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual bool		setViewWidth(size_t width) = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual bool		setViewHeight(size_t height) = 0;

		/**
		@describe set active view rect, the rect must be with the view width & height set by setViewWidth/Height
		@param
		@return
		*/
		virtual bool		setViewRect(int32 left, int32 top, int32 width, int32 height) = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual size_t		getColorBufferCount() const = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual IListener*	setListener(IListener* listener) = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual IListener*	getListener() const = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual bool		isReady() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool		swapBuffers() = 0;
	};//class IRenderTarget
	
}//namespace Blade


#endif //__Blade_IRenderTarget_h__