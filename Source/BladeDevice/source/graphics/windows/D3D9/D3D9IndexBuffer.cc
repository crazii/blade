/********************************************************************
	created:	2010/04/14
	filename: 	D3D9IndexBuffer.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>

#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
#include <graphics/windows/D3D9/D3D9IndexBuffer.h>
#include <graphics/windows/D3D9/D3D9TypeConverter.h>




namespace Blade
{
	D3D9IndexBuffer::D3D9IndexBuffer(IDirect3DIndexBuffer9* ibuffer, const void* data, EIndexType indexType,size_t indexCount, IGraphicsBuffer::USAGE usage)
		:D3D9Resource<IDirect3DIndexBuffer9>(ibuffer)
	{
		mUsage = usage;

		const D3DINDEXBUFFER_DESC& desc = this->getD3D9Desc();

		//IGraphicsBuffer::USAGE test = D3DUsageConverter(desc.Usage);
		//(int&)test |= (usage.getCPUAccess()&IGraphicsBuffer::GBAF_READ_WRITE);

		mIndexType = indexType;
		mTotalBytes = desc.Size;

		mIndexCount = indexCount;

		if( this->isCacheNeeded() )
			mMemoryBuffer.bind( BLADE_NEW SOFT_INDEX(mTotalBytes, data) );

		if (data != NULL)
		{
			void* dest = NULL;
			HRESULT result = ibuffer->Lock(0, (UINT)mTotalBytes, &dest, 0);
			if (FAILED(result))
				BLADE_EXCEPT(EXC_UNKNOWN, BTString("unable to copy index buffer."));
			std::memcpy(dest, data, mTotalBytes);
			ibuffer->Unlock();
		}
	}

	D3D9IndexBuffer::~D3D9IndexBuffer()
	{
		assert( !this->isLocked() );
	}

	/************************************************************************/
	/* D3D9Resource imp                                                                     */
	/************************************************************************/
	void		D3D9IndexBuffer::rebuildImpl(IDirect3DDevice9* device)
	{
		assert( this->getD3D9Resource() == NULL && device != NULL && this->isInDefaultPool() );
		if( this->getD3D9Resource() != NULL )
		{
			BLADE_EXCEPT(EXC_INVALID_OPERATION,BTString("cannot re-create resource if previous not releaseed."));
		}

		if( this->isInDefaultPool() && device != NULL )
		{
			IDirect3DIndexBuffer9* buffer = NULL;
			const D3DINDEXBUFFER_DESC& desc = this->getD3D9Desc();
			HRESULT result = device->CreateIndexBuffer((UINT)mTotalBytes,desc.Usage,desc.Format,desc.Pool,&buffer,NULL);
			if( FAILED(result ) )
			{
				BLADE_EXCEPT(EXC_UNKNOWN,BTString("unable to rebuild indexbuffer.") );
			}
			this->setD3D9Resource(buffer);

			if( this->isCacheNeeded() )
			{
				assert(mMemoryBuffer != NULL);
				void* data = NULL;
				result = mMemoryBuffer->lockProxy( this->getD3D9Resource(), 0, (UINT)mTotalBytes, &data, IGraphicsBuffer::GBLF_WRITEONLY);
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
	void*		D3D9IndexBuffer::lock(size_t offset, size_t length, LOCKFLAGS lockflags)
	{
		if( !mIsLocked )
		{
			if( !mUsage.isReadable() && (lockflags&GBLF_READONLY) != 0 
				|| !mUsage.isWriteable() && (lockflags&GBLF_WRITEONLY) != 0 )
				BLADE_EXCEPT(EXC_OUT_OF_RANGE,BTString("invalid lock operation for buffer access.") );

			if( offset <= mTotalBytes )
			{
				if( length > mTotalBytes - offset )//no overflow check
					length = mTotalBytes - offset;
			}
			else
				BLADE_EXCEPT(EXC_OUT_OF_RANGE,BTString("lock data of out range.") );

			if (lockflags&IGraphicsBuffer::GBLF_DISCARD)
			{
				assert(mUsage.isDynamic() || mUsage.isDirectRead());
				if (length != mTotalBytes && offset != 0)
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
				BLADE_EXCEPT(EXC_API_ERROR, BTString("failed to lock vertex buffer.") );

			mIsLocked = true;
			return dest;
		}
		else
			BLADE_EXCEPT(EXC_REDUPLICATE,BTString("buffer already locked.") );
	}

	//////////////////////////////////////////////////////////////////////////
	void		D3D9IndexBuffer::unlock(void)
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