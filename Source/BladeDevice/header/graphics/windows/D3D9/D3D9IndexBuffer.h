/********************************************************************
	created:	2010/04/14
	filename: 	D3D9IndexBuffer.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_D3D9IndexBuffer_h__
#define __Blade_D3D9IndexBuffer_h__

#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
#include "D3D9Resource.h"
#include <graphics/IndexBuffer.h>
#include "D3D9Util.h"


namespace Blade
{
	class D3D9IndexBuffer : public IndexBuffer, public D3D9Resource<IDirect3DIndexBuffer9> , public Allocatable
	{
	public:
		D3D9IndexBuffer(IDirect3DIndexBuffer9* ibuffer, const void* data, EIndexType indexType, size_t indexCount, IGraphicsBuffer::USAGE usage);
		~D3D9IndexBuffer();


		/************************************************************************/
		/* D3D9Resource imp                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual void		rebuildImpl(IDirect3DDevice9* device);
	protected:

		/************************************************************************/
		/* IGraphicsBuffer interface                                                                    */
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
		virtual void		unlock(void);

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		inline bool		isCacheNeeded() const
		{
			return (mUsage.isDynamic() || mUsage.isDirectRead()) && mUsage.isReadable();
		}

	protected:
		HSOFTINDEX	mMemoryBuffer;
	};
	
}//namespace Blade



#endif //BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS


#endif //__Blade_D3D9IndexBuffer_h__