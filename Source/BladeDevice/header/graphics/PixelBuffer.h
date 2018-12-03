/********************************************************************
	created:	2010/04/09
	filename: 	PixelBuffer.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_PixelBuffer_h__
#define __Blade_PixelBuffer_h__
#include <BladeDevice.h>
#include <Handle.h>
#include "IPixelBuffer.h"

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#	pragma warning(push)
#	pragma warning(disable:4275)
#endif


namespace Blade
{
	class BLADE_DEVICE_API PixelBuffer : public IPixelBuffer
	{
	public:
		PixelBuffer(ELocation location);
		PixelBuffer(ELocation location,USAGE usage,PixelFormat format,size_t width,size_t height,size_t depth);

		virtual ~PixelBuffer()	{}

		/************************************************************************/
		/*IGraphicsBuffer interface                                                                      */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual void*		lock(size_t offset,size_t length,LOCKFLAGS lockflags);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual USAGE		getUsage() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool		isLocked() const;


		/************************************************************************/
		/* IPixelBuffer interface                                                                    */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual PixelFormat		getPixelFormat() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual size_t			getWidth() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual size_t			getHeight() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual size_t			getDepth() const;

		/*
		@describe tell the where buffer is from
		@param 
		@return 
		*/
		virtual ELocation		getLocation() const;

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/

		/** @brief this is used by parent texture to set lock flag when the whole textrure is locked */
		inline void				setLocked(bool locked)
		{
			mIsLocked = locked;
		}

	protected:

		size_t			mSizeBytes;
		size_t			mWidth;
		size_t			mHeight;
		size_t			mDepth;
		//size_t			mRowPitch;
		//size_t			mSlicePitch;
		PixelFormat		mFormat;
		USAGE			mUsage;
		ELocation		mLocation;

		bool			mIsLocked;
	};//class PixelBuffer

	
}//namespace Blade


#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#	pragma warning(pop)
#endif


#endif //__Blade_PixelBuffer_h__