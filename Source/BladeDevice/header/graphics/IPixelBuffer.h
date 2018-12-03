/********************************************************************
	created:	2010/04/13
	filename: 	IPixelBuffer.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_IPixelBuffer_h__
#define __Blade_IPixelBuffer_h__
#include <Handle.h>
#include <interface/public/graphics/PixelFormat.h>
#include <interface/public/graphics/IGraphicsBuffer.h>

namespace Blade
{
	class IPixelBuffer : public IGraphicsBuffer
	{
	public:
		enum ELocation
		{
			PBL_BACKBUFFER		= 0x1,		//default render target created automatically
			PBL_RENDERTARGET	= 0x2,		//render target created manually

			PBL_TEXTURE_MASK	= 0x10000000,//textures
			PBL_TEXTURE_1D,
			PBL_TEXTURE_2D,
			PBL_TEXTURE_3D,	
			PBL_TEXTURE_CUBE,
		};

		virtual ~IPixelBuffer()		{}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual PixelFormat		getPixelFormat() const = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual size_t			getWidth() const = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual size_t			getHeight() const = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual size_t			getDepth() const = 0;

		/*
		@describe tell the where buffer is from
		@param 
		@return 
		*/
		virtual ELocation		getLocation() const = 0;

	};//class IPixelBuffer

	typedef Handle<IPixelBuffer> HPXBUFFER;
	
}//namespace Blade


#endif //__Blade_IPixelBuffer_h__