/********************************************************************
	created:	2010/04/14
	filename: 	D3D9VertexBuffer.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_D3D9VertexBuffer_h__
#define __Blade_D3D9VertexBuffer_h__

#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
#include "D3D9Resource.h"
#include <graphics/VertexBuffer.h>
#include "D3D9Util.h"

namespace Blade
{
	class D3D9VertexBuffer : public VertexBuffer, public D3D9Resource<IDirect3DVertexBuffer9> , public Allocatable
	{
	public:
		D3D9VertexBuffer(IDirect3DVertexBuffer9* vbuffer, const void* data, size_t vertexSize,size_t vertexCount, IGraphicsBuffer::USAGE usage);
		~D3D9VertexBuffer();

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
		/* custom method                                                                     */
		/************************************************************************/
		/** @brief  */
		inline bool		isCacheNeeded() const
		{
			return (mUsage.isDynamic() || mUsage.isDirectRead()) && mUsage.isReadable();
		}
		HSOFTINDEX	mMemoryBuffer;
	};
	
}//namespace Blade


#endif //BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS


#endif //__Blade_D3D9VertexBuffer_h__