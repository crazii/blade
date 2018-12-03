/********************************************************************
	created:	2010/04/10
	filename: 	IRenderView.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_IRenderView_h__
#define __Blade_IRenderView_h__
#include <BladeTypes.h>

namespace Blade
{
	class IRenderTarget;
	class IRenderDevice;
	class ITexture;
	class Color;

	class  IRenderView
	{
	public:
		virtual ~IRenderView()		{}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual int32			getPixelLeft() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual int32			getPixelRight() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual int32			getPixelTop() const = 0;


		/**
		@describe 
		@param 
		@return 
		*/
		virtual int32			getPixelBottom() const = 0;

		/*@brief */
		inline int32	getPixelWidth() const
		{
			return this->getPixelRight() - this->getPixelLeft();
		}
		/*@brief */
		inline int32	getPixelHeight() const
		{
			return this->getPixelBottom() - this->getPixelTop();
		}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual scalar			getRelativeLeft() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual scalar			getRelativeRight() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual scalar			getRelativeTop() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual scalar			getRelativeBottom() const = 0;

		/*@brief */
		inline scalar	getRelativeWidth() const
		{
			return this->getRelativeRight() - this->getRelativeLeft();
		}
		/*@brief */
		inline scalar	getRelativeHeight() const
		{
			return this->getRelativeBottom() - this->getRelativeTop();
		}

		/**
		@describe 
		@param
		@return
		*/
		virtual void			getSizePosition(scalar relatives[4], size_t offsets[4]) const = 0;

		inline void				getSizePosition(scalar& outLeft, scalar& outTop, scalar outRight, scalar& outBottom,
			size_t& outLeftOffset, size_t& outTopOffset, size_t& outRightOffset, size_t& outBottomOffset)
		{
			scalar _relative[4]; size_t _offset[4];
			this->getSizePosition(_relative, _offset);
			outLeft = _relative[0]; outTop = _relative[1]; outRight = _relative[2]; outBottom = _relative[3];
			outLeftOffset = _offset[0]; outTopOffset = _offset[1]; outRightOffset = _offset[2]; outBottomOffset = _offset[3];
		}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const Color&	getBackgroundColor() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual uint32			getClearMode() const = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual scalar			getClearDepth() const = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual uint16			getClearStencil() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void			setSizePosition(scalar left,scalar top,scalar right,scalar bottom,
			size_t leftOffset = 0,size_t topOffset = 0,size_t rightOffset = 0, size_t bottomOffset = 0) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void			setBackgroundColor(const Color& color) = 0;

		/**
		@describe 
		@param [in]frametypeMode combination of EFrameBufferType
		@return 
		*/
		virtual void			setClearMode(uint32 frameTypeMode,scalar depth = 1.0f,uint16 stencil = 0) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void			calculateSizeInPixel(IRenderTarget* target) = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual void			calculateSizeInPixel(ITexture* rtt) = 0;

	};//class IRenderView
	
}//namespace Blade


#endif //__Blade_IRenderView_h__