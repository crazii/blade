/********************************************************************
	created:	2010/04/15
	filename: 	D3D9CubeTexture.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
#include <graphics/windows/D3D9/D3D9TypeConverter.h>
#include <graphics/windows/D3D9/D3D9CubeTexture.h>
#include <graphics/windows/D3D9/D3D9PixelBuffer.h>

namespace Blade
{

	D3D9CubeTexture::D3D9CubeTexture(IDirect3DCubeTexture9* texture, IGraphicsBuffer::ACCESS CpuAccess, IMG_ORIENT layout)
		:Texture(TT_CUBE,layout)
		,D3D9Resource<IDirect3DCubeTexture9>(texture)
	{
		mDesc.mWidth		= this->getD3D9Desc().Width;
		mDesc.mHeight		= this->getD3D9Desc().Height;
		mDesc.mPixelFormat= D3DFormatConverter(this->getD3D9Desc().Format);
		mDesc.mUsage		= D3DUsageConverter( this->getD3D9Desc().Usage );
		(int&)mDesc.mUsage	|= (CpuAccess&IGraphicsBuffer::GBAF_READ_WRITE);
		mDesc.mGpuAccess	= IGraphicsBuffer::GBAF_READ;
		mDesc.mDepth		= 1;
		mDesc.mMipmapCount	= this->getD3D9Resource()->GetLevelCount();

		if( mDesc.mUsage.isDepthStencil() || mDesc.mUsage.isRenderTarget() )
			mDesc.mGpuAccess = IGraphicsBuffer::GBAF_READ_WRITE;

		//TODO: fill mPxBufferList with surfaces
		//mPxBufferList.resize(mDesc.mMipmapCount);

		if( this->isCacheNeeded() )
		{
			mMemorySurfaces.resize(6);
			for(size_t i = 0; i < 6; ++i)
				mMemorySurfaces[i].bind( BLADE_NEW SOFT_SURFACE(mDesc.mWidth,mDesc.mHeight,mDesc.mPixelFormat) );
		}
	}



	D3D9CubeTexture::~D3D9CubeTexture()
	{
		//base ('D3D9Resource') will not call virtuals in dector
		release();
	}

	/************************************************************************/
	/* ITexture Interface                                                                     */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	HTEXTURE		D3D9CubeTexture::cloneTexture(size_t newWidth/* = 0*/,size_t newHeight/* = 0*/,PixelFormat newFormat/* = PF_UNDEFINED*/)
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
		{
			BLADE_EXCEPT(EXC_UNKNOWN,BTString("unable to get device from resource.") );
		}
		if( mDesc.mTextureType == TT_2DIM || mDesc.mTextureType == TT_1DIM )
		{
			UINT levels = this->getD3D9Resource()->GetLevelCount();
			const D3DSURFACE_DESC& desc = this->getD3D9Desc();

			IDirect3DCubeTexture9* texture = NULL;
			result = device->CreateCubeTexture((UINT)mDesc.mWidth, (UINT)levels,desc.Usage,desc.Format,desc.Pool,&texture,NULL);
			device->Release();
			device = NULL;
			if( FAILED(result) )
			{
				BLADE_EXCEPT(EXC_UNKNOWN,BTString("unable to create texture resrouce.") );
			}
			return HTEXTURE(BLADE_NEW D3D9CubeTexture(texture, mDesc.mUsage.getCPUAccess(), mDesc.mLayout));
		}
		else
		{
			BLADE_EXCEPT(-1,BTString("not supposed run into here.") );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void*			D3D9CubeTexture::lockImpl(const TEXREGION& region,IGraphicsBuffer::LOCKFLAGS lockflags)
	{
		const Box3i& box = region.mBox;
		index_t face = region.mFace;

		if( mDesc.mUsage.isRenderTarget() || mDesc.mUsage.isDepthStencil() || 
			this->getD3D9Desc().Pool == D3DPOOL_DEFAULT && !mDesc.mUsage.isDynamic() 
			)
			BLADE_EXCEPT(EXC_INVALID_OPERATION,BTString("none-lockabe render target or depth stencil.") );

		if (lockflags&IGraphicsBuffer::GBLF_DISCARD)
		{
			assert(mDesc.mUsage.isDynamic() || mDesc.mUsage.isDirectRead());
			assert(mDesc.mUsage != IGraphicsBuffer::GBU_DYNAMIC_WRITE || mMemorySurfaces.size() == 0);
			assert(mMemorySurfaces.size() != 0 || mDesc.mUsage != IGraphicsBuffer::GBU_DYNAMIC_WRITE);
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
			assert(mMemorySurfaces.size() == 6);

			//record locked data and get cache data instead
			result = mMemorySurfaces[face]->lockProxy( this->getD3D9Resource(), region.mMipLevel, (D3DCUBEMAP_FACES)face, &locked_rect, &rect, (DWORD)D3DLockFlagConverter(lockflags) );
		}
		else
			result = this->getD3D9Resource()->LockRect( (D3DCUBEMAP_FACES)face, (UINT)region.mMipLevel, &locked_rect, &rect,(DWORD)D3DLockFlagConverter(lockflags) );

		region.mRowPitch = (size_t)locked_rect.Pitch;
		region.mSlicePitch = 0;
		return locked_rect.pBits;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			D3D9CubeTexture::unlockImpl()
	{
		index_t mipLevel = this->getLockedMip();
		index_t face = this->getLockedFace();
		assert( face < 6 );

		HRESULT result;
		if( this->isCacheNeeded())
		{
			assert(mMemorySurfaces.size() == 6);
			//update cache data into dynamic texture
			result = mMemorySurfaces[face]->unlockProxy( this->getD3D9Resource(), mipLevel, (D3DCUBEMAP_FACES)face);
		}
		else
			result = this->getD3D9Resource()->UnlockRect( (D3DCUBEMAP_FACES)face, (UINT)mipLevel);
		return SUCCEEDED(result);

		//PixelBuffer* pxBuffer = static_cast<PixelBuffer*>(mPxBufferList[mipmap]);
		//if( pxBuffer != NULL )
		//	pxBuffer->setLocked(false);
	}

	//////////////////////////////////////////////////////////////////////////
	bool			D3D9CubeTexture::updateMipMap(const TEXREGION& region,index_t mipLevel/* = 0*/)
	{
		const Box3i& box = region.mBox;
		index_t face = region.mFace;
		if( face >= 6 || this->isLocked() )
		{
			assert(false);
			return false;
		}

		if( mDesc.mGpuAccess&IGraphicsBuffer::GBAF_WRITE)
			return false;

		if( mDesc.mUsage & IGraphicsBuffer::GBUF_AUTOMIPMAP )
		{
			this->getD3D9Resource()->SetAutoGenFilterType(D3DTEXF_LINEAR);
			this->getD3D9Resource()->GenerateMipSubLevels();
			mDesc.mMipmapCount = this->getD3D9Resource()->GetLevelCount();
			return true;
		}

		if(mipLevel >= mDesc.mMipmapCount)
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
		HRESULT hr = this->getD3D9Resource()->GetCubeMapSurface( (D3DCUBEMAP_FACES)face, mipLevel == 0 ? 0 : (UINT)mipLevel - 1, &SrcSurface );
		if( FAILED(hr) )
		{
			BLADE_EXCEPT(EXC_API_ERROR,BTString("unable to get texture.") );
		}

		IDirect3DDevice9* device = NULL;
		result = this->getD3D9Resource()->GetDevice(&device);
		if( FAILED(result) )
		{
			BLADE_EXCEPT(EXC_UNKNOWN,BTString("unable to get device from resource.") );
		}

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
			hr = this->getD3D9Resource()->GetCubeMapSurface( (D3DCUBEMAP_FACES)face, (UINT)i, &mipSurface);
			if( FAILED(hr) )
			{
				BLADE_EXCEPT(EXC_API_ERROR,BTString("unable to get mip level.") );
			}
			hr = ::D3DXLoadSurfaceFromSurface(mipSurface, NULL, &targetRect, SrcSurface, NULL, &rect, D3DX_FILTER_BOX, 0);
			if( FAILED(hr) )
			{
				BLADE_EXCEPT(EXC_API_ERROR,BTString("unable to update mip level.") );
			}
			//mipSurface->Release();
			SrcSurface->Release();
			SrcSurface = mipSurface;
			rect = targetRect;
		}

		SrcSurface->Release();

		device->Release();

		//debug
		//::D3DXSaveTextureToFile(TEXT("Z:\\watch.tga"),D3DXIFF_TGA,this->getD3D9Resource(),NULL);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			D3D9CubeTexture::addDirtyRegion(const TEXREGION& region)
	{
		const Box3i& dirtyBox = region.mBox;
		index_t face = region.mFace;
		if(face >= 6)
		{
			assert(false);
			return false;
		}

		//note: dynamic texture need fully discarding & update, so partial update
		if (this->isCacheNeeded())
		{
			assert(mMemorySurfaces.size() == 6 && mMemorySurfaces[face] != NULL);
			return true;
		}

		RECT rect;
		rect.left = (LONG)dirtyBox.getLeft();
		rect.right = (LONG)dirtyBox.getRight();
		rect.bottom = (LONG)dirtyBox.getBottom();
		rect.top = (LONG)dirtyBox.getTop();
		HRESULT hr = this->getD3D9Resource()->AddDirtyRect( (D3DCUBEMAP_FACES)face, &rect );
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
	void			D3D9CubeTexture::rebuildImpl(IDirect3DDevice9* device)
	{
		assert( this->getD3D9Resource() == NULL && device != NULL && this->isInDefaultPool() );
		if( this->getD3D9Resource() != NULL )
		{
			BLADE_EXCEPT(EXC_INVALID_OPERATION,BTString("cannot re-create resource if previous not releaseed."));
		}

		if( isInDefaultPool() && device != NULL )
		{
			IDirect3DCubeTexture9* texture = NULL;
			const D3DSURFACE_DESC& desc = this->getD3D9Desc();
			HRESULT result = device->CreateCubeTexture(desc.Width, (UINT)mDesc.mMipmapCount,desc.Usage,desc.Format,desc.Pool,&texture,NULL);
			if( FAILED(result) )
				BLADE_EXCEPT(EXC_UNKNOWN,BTString("unable to rebuild texture.") );

			this->setD3D9Resource(texture);


			if( this->isCacheNeeded() )
			{
				D3DLOCKED_RECT locked_rect;
				RECT rect;
				rect.left = rect.top = 0;
				rect.right = (LONG)mDesc.mWidth;
				rect.bottom = (LONG)mDesc.mHeight;
				for(size_t i = 0; i < 6; ++i)
				{
					//update data from cache after reset
					result = mMemorySurfaces[i]->lockProxy(this->getD3D9Resource(), 0, (D3DCUBEMAP_FACES)i,&locked_rect,&rect, 0);
					if( FAILED(result) )
						BLADE_EXCEPT(EXC_UNKNOWN,BTString("unable to rebuild texture.") );

					result = mMemorySurfaces[i]->unlockProxy(this->getD3D9Resource(), 0, (D3DCUBEMAP_FACES)i);
					if( FAILED(result) )
						BLADE_EXCEPT(EXC_UNKNOWN,BTString("unable to rebuild texture.") );
				}
				this->ITexture::updateMipMap();
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	/** @brief  */
	void	D3D9CubeTexture::onSet(IDirect3DCubeTexture9* source)
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
	void		D3D9CubeTexture::release()
	{
		D3D9Resource<IDirect3DCubeTexture9>::release();
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/

	
}//namespace Blade

#endif //BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
