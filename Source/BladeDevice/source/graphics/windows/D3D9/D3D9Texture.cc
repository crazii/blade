/********************************************************************
	created:	2010/04/15
	filename: 	D3D9Texture.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>

#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
#include <graphics/windows/D3D9/D3D9TypeConverter.h>
#include <graphics/windows/D3D9/D3D9Texture.h>
#include <graphics/windows/D3D9/D3D9PixelBuffer.h>
#include <graphics/windows/dxerr.h>

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(disable:4127)// conditional expression is constant
#endif

namespace Blade
{

	D3D9Texture::D3D9Texture(IDirect3DTexture9* texture,IGraphicsBuffer::ACCESS CpuAccess, IMG_ORIENT layout)
		:Texture(TT_2DIM, layout)
		,D3D9Resource<IDirect3DTexture9>(texture)
	{
		mDesc.mWidth		= this->getD3D9Desc().Width;
		mDesc.mHeight		= this->getD3D9Desc().Height;
		mDesc.mPixelFormat= D3DFormatConverter( this->getD3D9Desc().Format);
		mDesc.mUsage		= D3DUsageConverter( this->getD3D9Desc().Usage );
		(int&)mDesc.mUsage	|= (CpuAccess&IGraphicsBuffer::GBAF_READ_WRITE);

		mDesc.mGpuAccess	= IGraphicsBuffer::GBAF_READ;
		if( mDesc.mUsage.isRenderTarget() || mDesc.mUsage.isDepthStencil() )
			mDesc.mGpuAccess = IGraphicsBuffer::GBAF_READ_WRITE;
		mDesc.mDepth		= 1;
		mDesc.mMipmapCount= this->getD3D9Resource()->GetLevelCount();

		//TODO: fill mPxBufferList with surfaces
		//mPxBufferList.resize(mDesc.mMipmapCount);

		if( this->isCacheNeeded() )
			mMemorySurface.bind( BLADE_NEW SOFT_SURFACE(mDesc.mWidth,mDesc.mHeight,mDesc.mPixelFormat)  );
	}

	D3D9Texture::~D3D9Texture()
	{
		//base ('D3D9Resource') will not call virtuals in dector
		this->release();
	}

	/************************************************************************/
	/* ITexture Interface                                                                     */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	HTEXTURE		D3D9Texture::cloneTexture(size_t newWidth/* = 0*/,size_t newHeight/* = 0*/,PixelFormat newFormat/* = PF_UNDEFINED*/)
	{
		assert( this->getD3D9Resource() == NULL );
		if( this->getD3D9Resource() == NULL )
		{
			BLADE_EXCEPT(EXC_INVALID_OPERATION,BTString("invalid texture resource,may be due to device lost.") );
		}

		if( newWidth == 0 )
			newWidth = mDesc.mWidth;
		if( newHeight == 0 )
			newHeight = mDesc.mHeight;
		if( newFormat == PF_UNDEFINED )
			newFormat == mDesc.mPixelFormat;

		IDirect3DDevice9* device = NULL;
		HRESULT result = this->getD3D9Resource()->GetDevice(&device);
		if( FAILED(result) )
			BLADE_EXCEPT(EXC_UNKNOWN,BTString("unable to get device from resource.") );

		UINT levels = this->getD3D9Resource()->GetLevelCount();
		const D3DSURFACE_DESC& desc = this->getD3D9Desc();

		IDirect3DTexture9* texture = NULL;
		result = device->CreateTexture((UINT)mDesc.mWidth,(UINT)mDesc.mHeight,(UINT)levels,desc.Usage,desc.Format,desc.Pool,&texture,NULL);
		device->Release();
		device = NULL;
		if( FAILED(result) )
			BLADE_EXCEPT(EXC_UNKNOWN,BTString("unable to create texture resrouce.") );

		return HTEXTURE(BLADE_NEW D3D9Texture(texture, mDesc.mUsage.getCPUAccess(), mDesc.mLayout));
	}

	//////////////////////////////////////////////////////////////////////////
	void*			D3D9Texture::lockImpl(const TEXREGION& region,IGraphicsBuffer::LOCKFLAGS lockflags)
	{
		const Box3i& box = region.mBox;
		assert(region.mFace == 0);

		if( mDesc.mUsage.isRenderTarget() || mDesc.mUsage.isDepthStencil() ||
			this->getD3D9Desc().Pool == D3DPOOL_DEFAULT && !mDesc.mUsage.isDynamic() )
		{
			BLADE_EXCEPT(EXC_INVALID_OPERATION,BTString("none-lockable render target or depth stencil.") );
		}

		if (lockflags&IGraphicsBuffer::GBLF_DISCARD)
		{
			assert(mDesc.mUsage.isDynamic() || mDesc.mUsage.isDirectRead());
			assert(mDesc.mUsage != IGraphicsBuffer::GBU_DYNAMIC_WRITE || mMemorySurface == NULL);
			assert(mMemorySurface != NULL || mDesc.mUsage != IGraphicsBuffer::GBU_DYNAMIC_WRITE);
		}

		RECT rect;
		if( &box == &Box3i::EMPTY || box == Box3i::EMPTY )
		{
			rect.left = 0;
			rect.top = 0;
			rect.right = (LONG)mDesc.mWidth;
			rect.bottom = (LONG)mDesc.mHeight;
		}
		else
		{
			rect.left = (LONG)box.getLeft();
			rect.top = (LONG)box.getTop();
			rect.right = (LONG)box.getRight();
			rect.bottom = (LONG)box.getBottom();
		}
		D3DLOCKED_RECT locked_rect;
		HRESULT result;
		//dynamic + readable texture, lock use cached data, then update it
		if( this->isCacheNeeded() )
		{
			assert( this->getD3D9Desc().Pool == D3DPOOL_DEFAULT);
			assert(mMemorySurface != NULL);
			result = mMemorySurface->lockProxy( this->getD3D9Resource(), region.mMipLevel, &locked_rect, &rect, (DWORD)D3DLockFlagConverter(lockflags) );
		}
		else
			result = this->getD3D9Resource()->LockRect( (UINT)region.mMipLevel, &locked_rect, &rect,(DWORD)D3DLockFlagConverter(lockflags) );

		if (FAILED(result))
		{
			tchar Desc[1024];
			std::memset(Desc, 0, sizeof(Desc));
			DXGetErrorDescription(result, Desc, 1024);
			BLADE_EXCEPT(EXC_INVALID_OPERATION, BTString("unable to lock texture: ") + BTString("\nDescription: ") + Desc);
		}
			
		region.mRowPitch = (size_t)locked_rect.Pitch;
		region.mSlicePitch = 0;
		return locked_rect.pBits;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			D3D9Texture::unlockImpl()
	{
		index_t mipLevel = this->getLockedMip();

		HRESULT result;
		if( this->isCacheNeeded() )
		{
			assert(mMemorySurface != NULL);
			result =  mMemorySurface->unlockProxy( this->getD3D9Resource(), mipLevel);	//update cache data into dynamic texture
		}
		else
			result = this->getD3D9Resource()->UnlockRect((UINT)mipLevel);
		return SUCCEEDED(result);
		//PixelBuffer* pxBuffer = static_cast<PixelBuffer*>(mPxBufferList[mipmap]);
		//if( pxBuffer != NULL )
		//	pxBuffer->setLocked(false);
	}

	//////////////////////////////////////////////////////////////////////////
	bool			D3D9Texture::updateMipMap(const TEXREGION& region,index_t mipLevel/* = 0*/)
	{
		const Box3i& box = region.mBox;
		assert(region.mFace == 0);
		if( this->isLocked() )
		{
			assert(false);
			return false;
		}

		//cannot generate mip-maps for render target or depth stencil
		if( mDesc.mGpuAccess&IGraphicsBuffer::GBAF_WRITE )
			return false;

		if( mDesc.mUsage.isAutoMipmap() )
		{
			this->getD3D9Resource()->SetAutoGenFilterType(D3DTEXF_LINEAR);
			this->getD3D9Resource()->GenerateMipSubLevels();
			mDesc.mMipmapCount = this->getD3D9Resource()->GetLevelCount();
			return true;
		}

		if( mipLevel >= mDesc.mMipmapCount || mDesc.mMipmapCount <= 1)
			return false;

		HRESULT result;
		RECT rect;

		if( &box == &Box3i::EMPTY || box == Box3i::EMPTY )
		{
			rect.left = 0;
			rect.top = 0;
			rect.right = (LONG)mDesc.mWidth;
			rect.bottom = (LONG)mDesc.mHeight;
		}
		else
		{
			rect.left = (LONG)box.getLeft();
			rect.right = (LONG)box.getRight();
			rect.bottom = (LONG)box.getBottom();
			rect.top = (LONG)box.getTop();
		}

		IDirect3DSurface9* SrcSurface = NULL;
		HRESULT hr = this->getD3D9Resource()->GetSurfaceLevel( mipLevel == 0 ? 0 : (UINT)mipLevel - 1, &SrcSurface );
		if( FAILED(hr) )
		{
			BLADE_EXCEPT(EXC_API_ERROR,BTString("unable to get texture.") );
		}

		IDirect3DDevice9* device = NULL;
		result = this->getD3D9Resource()->GetDevice(&device);
		if( FAILED(result) )
			BLADE_EXCEPT(EXC_UNKNOWN,BTString("unable to get device from resource.") );

		//DO not always use level 0 as source, because it consumes too much time

		RECT targetRect;
		std::memcpy(&targetRect,&rect,sizeof(rect) );
		for(size_t i = 1; i < mDesc.mMipmapCount; ++i )
		{
			targetRect.left /= 2;
			targetRect.top /= 2;
			targetRect.right /= 2;
			targetRect.bottom /= 2;

			if( targetRect.left == targetRect.right || targetRect.bottom == targetRect.top )
				break;

			if( mipLevel != i && mipLevel != 0 )
				continue;

			IDirect3DSurface9* mipSurface = NULL;
			hr = this->getD3D9Resource()->GetSurfaceLevel( (UINT)i, &mipSurface);
			if( FAILED(hr) )
				BLADE_EXCEPT(EXC_API_ERROR,BTString("unable to get mip level.") );
			hr = ::D3DXLoadSurfaceFromSurface(mipSurface, NULL, &targetRect, SrcSurface, NULL, &rect, D3DX_FILTER_BOX/*|D3DX_FILTER_DITHER*/, 0);
			if( FAILED(hr) )
				BLADE_DETAIL_LOG(Error, BTString("unable to update mip level."));
			//ULONG count = mipSurface->Release();
			ULONG count = SrcSurface->Release();
			SrcSurface = mipSurface;
			//assert(count == 0);
			BLADE_UNREFERENCED(count);
			rect = targetRect;
		}

		ULONG count = SrcSurface->Release();
		//assert(count == 0);
		count = device->Release();
		BLADE_UNREFERENCED(count);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			D3D9Texture::addDirtyRegion(const TEXREGION& region)
	{
		const Box3i& dirtyBox = region.mBox;
		assert(region.mFace == 0);

		//note: dynamic texture need fully discarding & update, so partial update
		if (this->isCacheNeeded())
		{
			assert(mMemorySurface != NULL);
			return true;
		}

		RECT rect;
		rect.left = (LONG)dirtyBox.getLeft();
		rect.right = (LONG)dirtyBox.getRight();
		rect.bottom = (LONG)dirtyBox.getBottom();
		rect.top = (LONG)dirtyBox.getTop();
		HRESULT hr = this->getD3D9Resource()->AddDirtyRect( &rect );
		if( FAILED(hr) )
		{
			BLADE_EXCEPT(EXC_UNKNOWN,BTString("unable to add dirty region to pixel buffer.") );
		}
		return true;
	}


	/************************************************************************/
	/* D3D9Resource imp                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void			D3D9Texture::rebuildImpl(IDirect3DDevice9* device)
	{
		assert( this->getD3D9Resource() == NULL && device != NULL && this->isInDefaultPool() );
		if( this->getD3D9Resource() != NULL )
			BLADE_EXCEPT(EXC_INVALID_OPERATION,BTString("cannot re-create resource if previous not releaseed."));

		if( this->isInDefaultPool() && device != NULL )
		{
			IDirect3DTexture9* texture = NULL;
			const D3DSURFACE_DESC& desc = this->getD3D9Desc();
			HRESULT result = device->CreateTexture(desc.Width,desc.Height,(UINT)mDesc.mMipmapCount,desc.Usage,desc.Format,desc.Pool,&texture,NULL);
			if( FAILED(result) )
			{
				BLADE_EXCEPT(EXC_UNKNOWN, BTString("unable to rebuild texture."));
			}
			this->setD3D9Resource(texture);

			if( this->isCacheNeeded() )
			{
				assert(mMemorySurface != NULL);
				//update cache data into d3d texture
				RECT rect;
				rect.left = rect.top = 0;
				rect.right = (LONG)mDesc.mWidth;
				rect.bottom = (LONG)mDesc.mHeight;

				D3DLOCKED_RECT locked_rect;
				result = mMemorySurface->lockProxy(this->getD3D9Resource(), 0, &locked_rect, &rect, 0 );
				if( FAILED(result) )
					BLADE_EXCEPT(EXC_UNKNOWN, BTString("unable to rebuild texture."));

				result = mMemorySurface->unlockProxy( this->getD3D9Resource(), 0 );
				if( FAILED(result) )
					BLADE_EXCEPT(EXC_UNKNOWN, BTString("unable to rebuild texture."));
				this->ITexture::updateMipMap();
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	/** @brief  */
	void	D3D9Texture::onSet(IDirect3DTexture9* source)
	{
		assert( this->getD3D9Resource() == NULL );

		const D3DSURFACE_DESC& desc = this->getD3D9Desc();
		D3DSURFACE_DESC	newDesc;
		source->GetLevelDesc(0,&newDesc);

		//if the source is new,we should clear up the pixel buffer list

		//D3DSURFACE_DESC has no align gap,so use memcmp
		if( ::memcmp(&newDesc,&desc,sizeof(D3DSURFACE_DESC)) != 0 || mDesc.mMipmapCount != source->GetLevelCount() )
		{
		}
		else
		{
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void		D3D9Texture::release()
	{
		assert( !this->isLocked() );
		mPxBufferList.clear();
		D3D9Resource<IDirect3DTexture9>::release();
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	const HPXBUFFER&	D3D9Texture::getPixelBuffer(size_t mipmap/* = 0*/) const
	{
		if( mipmap < mDesc.mMipmapCount )
		{
			if( mPxBufferList.size() <= mipmap)
				mPxBufferList.resize( mipmap+1 );

			HPXBUFFER& buffer = mPxBufferList[mipmap];
			if( buffer != NULL )
				return buffer;

			IDirect3DSurface9* surface = NULL;
			HRESULT result = this->getD3D9Resource()->GetSurfaceLevel((UINT)mipmap,&surface);
			if( FAILED(result) )
				BLADE_EXCEPT(EXC_UNKNOWN,BTString("cannot get mipmap level.") );

			IPixelBuffer::ELocation eLoc = mDesc.mTextureType == TT_1DIM ? IPixelBuffer::PBL_TEXTURE_1D : IPixelBuffer::PBL_TEXTURE_2D;

			buffer.bind( BLADE_NEW D3D9PixelBuffer(surface,eLoc) );
			return buffer;
		}
		else
			BLADE_EXCEPT(EXC_OUT_OF_RANGE,BTString("mipmap index out of range.") );
	}

	
}//namespace Blade

#endif //BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
