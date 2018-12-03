/********************************************************************
	created:	2010/04/10
	filename: 	D3D9PixelBuffer.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_D3D9PixelBuffer_h__
#define __Blade_D3D9PixelBuffer_h__

#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
#include "D3D9Resource.h"
#include <graphics/PixelBuffer.h>


namespace Blade
{

	class D3D9PixelBuffer : public PixelBuffer, public D3D9Resource<IDirect3DSurface9>, public Allocatable
	{
	public:
		D3D9PixelBuffer(IDirect3DSurface9* pSurface,ELocation location);
		~D3D9PixelBuffer();

		/************************************************************************/
		/* D3D9Resource imp                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual void		rebuildImpl(IDirect3DDevice9* device);

		/**
		@describe 
		@param
		@return
		*/
		virtual	void		onSet(IDirect3DSurface9* source);

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/


	protected:
		/************************************************************************/
		/* IGraphicsBuffer interface                                                                    */
		/************************************************************************/

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void*		lock(const Box3i& box,size_t& outPitch,LOCKFLAGS lockflags);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void		unlock(void);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool		addDirtyRegion(const Box3i& dirtyBox);

	};//class D3D9PixelBuffer
	
}//namespace Blade

#endif //BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS

#endif //__Blade_D3D9PixelBuffer_h__