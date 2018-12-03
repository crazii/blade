/********************************************************************
	created:	2012/11/28
	filename: 	D3D9Util.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>

#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
#include "D3D9Util.h"

namespace Blade
{
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	HRESULT	D3DSystemMemorySurface::lockProxy(IDirect3DTexture9* texture, index_t mipLevel, D3DLOCKED_RECT* locked_rect, const RECT* rect,DWORD flags)
	{
		if( (size_t)rect->left > mWidth || (size_t)rect->right > mWidth
			|| (size_t)rect->top > mHeight || (size_t)rect->bottom > mHeight )
		{
			assert(false);
			return E_FAIL;
		}

		bool readonly = (flags&D3DLOCK_READONLY) != 0;
		if( !readonly || mipLevel != 0)
		{
			assert(!(flags&D3DLOCK_NO_DIRTY_UPDATE));	//dynamic texture is always discard, so doesn't support partial update

			HRESULT result = texture->LockRect( (UINT)mipLevel, locked_rect, NULL, flags | D3DLOCK_DISCARD);//note: pRect must be NULL, even pRect covers the whole texture, this call will fail
			if( FAILED(result) )
				return result;
		}

		mReadOnly = readonly;
		mLockedRect = *locked_rect;
		if( mipLevel != 0)
			return S_OK;
		
		locked_rect->Pitch = (INT)this->getPitch();
		locked_rect->pBits = mBuffer.getData() + (rect->top*mWidth + rect->left)*mFormat.getSizeBytes();
		return NOERROR;
	}

	//////////////////////////////////////////////////////////////////////////
	HRESULT		D3DSystemMemorySurface::unlockProxy(IDirect3DTexture9* texture, index_t mipLevel)
	{
		mBuffer.check( mWidth*mHeight*mFormat.getSizeBytes() );
		assert(mLockedRect.pBits != NULL);

		HRESULT result = NOERROR;
		if( !mReadOnly || mipLevel != 0)
		{
			//note: dynamic texture need fully discarding & update, or we get a performance penalty to read data back
			char* dest = (char*)mLockedRect.pBits;
			const char* source = mBuffer.getData();
			for (size_t j = 0; j < mHeight; ++j)
			{
				std::memcpy(dest, source, mWidth*mFormat.getSizeBytes());
				source += mWidth*mFormat.getSizeBytes();
				dest += mLockedRect.Pitch;
			}
			result = texture->UnlockRect( (UINT)mipLevel);
		}
		std::memset(&mLockedRect,0,sizeof(D3DLOCKED_RECT));
		mReadOnly = false;
		return result;
	}

	//////////////////////////////////////////////////////////////////////////
	HRESULT	D3DSystemMemorySurface::lockProxy(IDirect3DCubeTexture9* texture, index_t mipLevel, D3DCUBEMAP_FACES face, D3DLOCKED_RECT* locked_rect, const RECT* rect,DWORD flags)
	{
		if( (size_t)rect->left > mWidth || (size_t)rect->right > mWidth
			|| (size_t)rect->top > mHeight || (size_t)rect->bottom > mHeight )
		{
			assert(false);
			return E_FAIL;
		}

		bool readonly = (flags&D3DLOCK_READONLY) != 0;
		if( !readonly || mipLevel != 0)
		{
			assert(!(flags&D3DLOCK_NO_DIRTY_UPDATE));	//dynamic texture is always discard, so doesn't support partial update

			//note: dynamic texture need fully discarding & update, or we get a performance penalty to read data back
			HRESULT result = texture->LockRect(face, (UINT)mipLevel, locked_rect, NULL, flags | D3DLOCK_DISCARD);
			if( FAILED(result) )
				return result;
		}

		mReadOnly = readonly;
		mLockedRect = *locked_rect;

		if( mipLevel != 0 )
			return S_OK;

		locked_rect->Pitch = (INT)this->getPitch();
		locked_rect->pBits = mBuffer.getData() + (rect->top*mWidth + rect->left)*mFormat.getSizeBytes();
		return NOERROR;
	}

	//////////////////////////////////////////////////////////////////////////
	HRESULT		D3DSystemMemorySurface::unlockProxy(IDirect3DCubeTexture9* texture, index_t mipLevel, D3DCUBEMAP_FACES face)
	{
		mBuffer.check( mWidth*mHeight*mFormat.getSizeBytes() );
		assert(mLockedRect.pBits != NULL);

		HRESULT result = NOERROR;
		if( !mReadOnly || mipLevel != 0 )
		{
			//note: dynamic texture need fully discarding & update, or we get a performance penalty to read data back
			char* dest = (char*)mLockedRect.pBits;
			const char* source = mBuffer.getData();
			for (size_t j = 0; j < mHeight; ++j)
			{
				std::memcpy(dest, source, mWidth*mFormat.getSizeBytes());
				source += mWidth*mFormat.getSizeBytes();
				dest += mLockedRect.Pitch;
			}

			result = texture->UnlockRect(face, (UINT)mipLevel);
		}

		std::memset(&mLockedRect,0,sizeof(D3DLOCKED_RECT));
		mReadOnly = false;
		return result;
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	HRESULT	D3DSystemMemoryVolume::lockProxy(IDirect3DVolumeTexture9* texture, index_t mipLevel, D3DLOCKED_BOX* locked_box, const D3DBOX* box, DWORD flags)
	{
		if( box->Left > mWidth || box->Right > mWidth
			|| box->Top > mHeight || box->Bottom > mHeight 
			|| box->Front > mDepth || box->Back > mDepth )
		{
			assert(false);
			return E_FAIL;
		}

		bool readonly = (flags&D3DLOCK_READONLY) != 0;

		if( !readonly || mipLevel != 0)
		{
			assert(!(flags&D3DLOCK_NO_DIRTY_UPDATE));
			HRESULT result = texture->LockBox( (UINT)mipLevel, locked_box, NULL, flags | D3DLOCK_DISCARD);
			if( FAILED(result) )
				return result;
		}

		mReadOnly = readonly;
		mLockedBox = *locked_box;

		if( mipLevel != 0)
			return S_OK;

		locked_box->RowPitch = (INT)this->getRowPitch();
		locked_box->SlicePitch = (INT)this->getSlicePitch();
		locked_box->pBits = mBuffer.getData() + (box->Front*mWidth*mHeight + box->Top*mWidth+box->Left)*mFormat.getSizeBytes();
		return NOERROR;
	}

	//////////////////////////////////////////////////////////////////////////
	HRESULT		D3DSystemMemoryVolume::unlockProxy(IDirect3DVolumeTexture9* texture, index_t mipLevel)
	{
		mBuffer.check( mWidth*mHeight*mDepth*mFormat.getSizeBytes() );

		HRESULT result = NOERROR;
		if( !mReadOnly || mipLevel != 0)
		{
			size_t rowBytes = mWidth*mFormat.getSizeBytes();
			size_t sourceRowPitch = this->getRowPitch();
			size_t sourceSlicePitch = this->getSlicePitch();

			const char* source = mBuffer.getData();
			char* dest = (char*)mLockedBox.pBits;

			for (size_t n = 0; n < mDepth; ++n)
			{
				const char* sliceSource = source;
				char* sliceDest = dest;
				for (size_t j = 0; j < mWidth; ++j)
				{
					std::memcpy(sliceDest, sliceSource, rowBytes);
					sliceSource += sourceRowPitch;
					sliceDest += mLockedBox.RowPitch;
				}

				source += sourceSlicePitch;
				dest += mLockedBox.SlicePitch;
			}
			result = texture->UnlockBox( (UINT)mipLevel);
		}
		std::memset(&mLockedBox,0,sizeof(D3DLOCKED_BOX));
		mReadOnly = false;
		return result;
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	HRESULT	D3DSystemMemoryBuffer::lockProxy(IDirect3DVertexBuffer9* buffer,UINT offset, UINT length, void** data, DWORD flag)
	{
		bool readOnly = (flag&D3DLOCK_READONLY) != 0;
		HRESULT result = NOERROR;
		if( !readOnly )
		{
			//note: dynamic index/vertex buffer is always set write only (@see D3DUsageConverter), we can lock & write partial data
			result = buffer->Lock(offset, length, &mLockData, flag);
			if( FAILED(result) )
				return result;
		}

		*data = this->getData() + offset;
		mReadOnly = readOnly;
		mOffset = offset;
		mLength = length;
		return result;
	}

	//////////////////////////////////////////////////////////////////////////
	HRESULT	D3DSystemMemoryBuffer::unlockProxy(IDirect3DVertexBuffer9* buffer)
	{
		HRESULT result = NOERROR;
		if( !mReadOnly )
		{
			std::memcpy(mLockData, this->getData()+mOffset, mLength);
			result = buffer->Unlock();
		}

		mLockData = NULL;
		mOffset = 0;
		mLength = 0;
		return result;
	}

	//////////////////////////////////////////////////////////////////////////
	HRESULT	D3DSystemMemoryBuffer::lockProxy(IDirect3DIndexBuffer9* buffer,UINT offset, UINT length, void** data, DWORD flag)
	{
		bool readOnly = (flag&D3DLOCK_READONLY) != 0;
		HRESULT result = NOERROR;
		if( !readOnly )
		{
			//note: dynamic index/vertex buffer is always set write only (@see D3DUsageConverter), we can lock & write partial data
			result = buffer->Lock(offset,length,&mLockData,flag);
			if( FAILED(result) )
				return result;
		}

		*data = this->getData() + offset;
		mReadOnly = readOnly;
		mOffset = offset;
		mLength = length;
		return result;
	}

	//////////////////////////////////////////////////////////////////////////
	HRESULT	D3DSystemMemoryBuffer::unlockProxy(IDirect3DIndexBuffer9* buffer)
	{
		HRESULT result = NOERROR;
		if( !mReadOnly )
		{
			std::memcpy(mLockData, this->getData()+mOffset, mLength);
			result = buffer->Unlock();
		}

		mLockData = NULL;
		mOffset = 0;
		mLength = 0;
		return result;
	}
	
}//namespace Blade

#endif //BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS