/********************************************************************
	created:	2010/04/14
	filename: 	D3D9VertexBuffer.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>

#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
#include <graphics/windows/D3D9/D3D9TypeConverter.h>
#include <graphics/windows/D3D9/D3D9VertexBuffer.h>


namespace Blade
{

	//////////////////////////////////////////////////////////////////////////
	D3D9VertexBuffer::D3D9VertexBuffer(IDirect3DVertexBuffer9* vbuffer, const void* data, size_t vertexSize,size_t vertexCount, IGraphicsBuffer::USAGE usage)
		:D3D9Resource<IDirect3DVertexBuffer9>(vbuffer)
	{
		mUsage = usage;

		const D3DVERTEXBUFFER_DESC& desc = this->getD3D9Desc();

		mSizeBytes = desc.Size;
		mVertexCount = vertexCount;
		mVertexSize = vertexSize;

		if( this->isCacheNeeded() )
			mMemoryBuffer.bind( BLADE_NEW SOFT_VERTEX(mSizeBytes, data) );

		if (data != NULL)
		{
			void* dest = NULL;
			HRESULT result = vbuffer->Lock(0, (UINT)mSizeBytes, &dest, 0);
			if (FAILED(result))
				BLADE_EXCEPT(EXC_UNKNOWN, BTString("unable to copy vertex buffer."));
			std::memcpy(dest, data, mSizeBytes);
			vbuffer->Unlock();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	D3D9VertexBuffer::~D3D9VertexBuffer()
	{
		assert( !this->isLocked() );
	}

	/************************************************************************/
	/* D3D9Resource imp                                                                     */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	void		D3D9VertexBuffer::rebuildImpl(IDirect3DDevice9* device)
	{
		assert( this->getD3D9Resource() == NULL && device != NULL && this->isInDefaultPool() );
		if( this->getD3D9Resource() != NULL )
		{
			BLADE_EXCEPT(EXC_INVALID_OPERATION,BTString("cannot re-create resource if previous not releaseed."));
		}

		if( this->isInDefaultPool() && device != NULL )
		{
			IDirect3DVertexBuffer9* buffer = NULL;
			const D3DVERTEXBUFFER_DESC& desc = this->getD3D9Desc();
			HRESULT result = device->CreateVertexBuffer((UINT)mSizeBytes,desc.Usage,desc.FVF,desc.Pool,&buffer,NULL);
			if( FAILED(result ) )
				BLADE_EXCEPT(EXC_UNKNOWN,BTString("unable to rebuild vertex buffer.") );
			this->setD3D9Resource(buffer);

			if( this->isCacheNeeded() )
			{
				assert(mMemoryBuffer != NULL);
				void* data = NULL;
				result = mMemoryBuffer->lockProxy( this->getD3D9Resource(), 0, (UINT)mSizeBytes, &data, IGraphicsBuffer::GBLF_WRITEONLY);
				if( FAILED(result ) )
					BLADE_EXCEPT(EXC_UNKNOWN,BTString("unable to rebuild vertex buffer.") );
				result = mMemoryBuffer->unlockProxy( this->getD3D9Resource() );
				if( FAILED(result ) )
					BLADE_EXCEPT(EXC_UNKNOWN,BTString("unable to rebuild vertex buffer.") );
			}

		}
	}

	/************************************************************************/
	/* IGraphicsBuffer interface                                                                    */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void*		D3D9VertexBuffer::lock(size_t offset,size_t length,LOCKFLAGS lockflags)
	{
		if( mIsLocked )
			BLADE_EXCEPT(EXC_REDUPLICATE,BTString("buffer already locked.") );

		if( !mUsage.isReadable() && (lockflags&GBLF_READONLY) != 0 
			|| !mUsage.isWriteable() && (lockflags&GBLF_WRITEONLY) != 0 )
			BLADE_EXCEPT(EXC_OUT_OF_RANGE, BTString("invalid lock operation for buffer access.") );
		
		if( offset <= mSizeBytes )
		{
			if( length > mSizeBytes - offset )	//no overflow check
				length = mSizeBytes - offset;
		}
		else
			BLADE_EXCEPT(EXC_OUT_OF_RANGE,BTString("lock data of out range.") );

		if (lockflags&IGraphicsBuffer::GBLF_DISCARD)
		{
			assert(mUsage.isDynamic() || mUsage.isDirectRead());
			if(length != mSizeBytes && offset != 0)
				BLADE_EXCEPT(EXC_INVALID_OPERATION, BTString("discarding buffer need a full lock."));
		}

		void* dest = NULL;
		HRESULT result;
		if( !this->isCacheNeeded() )
			result = this->getD3D9Resource()->Lock((UINT)offset, (UINT)length, &dest, (DWORD)D3DLockFlagConverter(lockflags) );
		else
		{
			assert(mMemoryBuffer != NULL);
			result = mMemoryBuffer->lockProxy(this->getD3D9Resource(), (UINT)offset, (UINT)length, &dest, (DWORD)D3DLockFlagConverter(lockflags));
		}

		if( FAILED(result) )
			BLADE_EXCEPT(EXC_API_ERROR,BTString("failed to lock vertex buffer.") );

		mIsLocked = true;
		return dest;
	}

	//////////////////////////////////////////////////////////////////////////
	void		D3D9VertexBuffer::unlock(void)
	{
		if( !mIsLocked )
			BLADE_EXCEPT(EXC_REDUPLICATE,BTString("buffer not locked.") );

		HRESULT result;
		if( !this->isCacheNeeded() )
			result = this->getD3D9Resource()->Unlock();
		else
		{
			assert(mMemoryBuffer != NULL);
			result = mMemoryBuffer->unlockProxy( this->getD3D9Resource() );
		}

		if( FAILED(result) )
			BLADE_EXCEPT(EXC_OUT_OF_RANGE,BTString("failed to un-lock vertex buffer.") );

		mIsLocked = false;
	}

}//namespace Blade

#endif //BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
