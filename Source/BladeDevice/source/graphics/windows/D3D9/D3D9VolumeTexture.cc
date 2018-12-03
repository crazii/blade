/********************************************************************
	created:	2010/04/15
	filename: 	D3D9VolumeTexture.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>

#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
#include <graphics/windows/D3D9/D3D9TypeConverter.h>
#include <graphics/windows/D3D9/D3D9VolumeTexture.h>
#include <graphics/windows/D3D9/D3D9PixelBuffer.h>

namespace Blade
{

	D3D9VolumeTexture::D3D9VolumeTexture(IDirect3DVolumeTexture9* texture,IGraphicsBuffer::ACCESS CpuAccess, IMG_ORIENT layout)
		:Texture(TT_3DIM,layout)
		,D3D9Resource<IDirect3DVolumeTexture9>(texture)
	{
		mDesc.mWidth		= this->getD3D9Desc().Width;
		mDesc.mHeight		= this->getD3D9Desc().Height;
		mDesc.mPixelFormat= D3DFormatConverter(this->getD3D9Desc().Format);
		mDesc.mUsage		= D3DUsageConverter( this->getD3D9Desc().Usage );
		(int&)mDesc.mUsage	|= (CpuAccess&IGraphicsBuffer::GBAF_READ_WRITE);

		if( mDesc.mUsage.isDepthStencil() || mDesc.mUsage.isRenderTarget() )
			mDesc.mGpuAccess = IGraphicsBuffer::GBAF_READ_WRITE;

		mDesc.mDepth		= this->getD3D9Desc().Depth;
		mDesc.mMipmapCount= this->getD3D9Resource()->GetLevelCount();

		if( this->isCacheNeeded() )
			mMemoryVolume.bind( BLADE_NEW SOFT_VOLUME(mDesc.mWidth,mDesc.mHeight,mDesc.mDepth,mDesc.mPixelFormat)  );
	}


	D3D9VolumeTexture::~D3D9VolumeTexture()
	{
		//base ('D3D9Resource') will not call virtuals in dector
		this->release();
	}

	/************************************************************************/
	/* ITexture Interface                                                                     */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	HTEXTURE		D3D9VolumeTexture::cloneTexture(size_t newWidth/* = 0*/,size_t newHeight/* = 0*/,PixelFormat newFormat/* = PF_UNDEFINED*/)
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
		UINT levels = this->getD3D9Resource()->GetLevelCount();
		const D3DVOLUME_DESC& desc = this->getD3D9Desc();

		IDirect3DVolumeTexture9* texture = NULL;
		
		result = device->CreateVolumeTexture((UINT)mDesc.mWidth,(UINT)mDesc.mHeight, desc.Depth,(UINT)levels,desc.Usage,desc.Format,desc.Pool,&texture,NULL);
		device->Release();
		device = NULL;
		if( FAILED(result) )
		{
			BLADE_EXCEPT(EXC_UNKNOWN,BTString("unable to create texture resrouce.") );
		}
		return HTEXTURE(BLADE_NEW D3D9VolumeTexture(texture, mDesc.mUsage.getCPUAccess(), mDesc.mLayout));
	}

	//////////////////////////////////////////////////////////////////////////
	void*			D3D9VolumeTexture::lockImpl(const TEXREGION& region,IGraphicsBuffer::LOCKFLAGS lockflags)
	{
		const Box3i& box = region.mBox;
		index_t face= region.mFace;
		assert(face == 0);
		BLADE_UNREFERENCED(face);

		if( mDesc.mUsage.isRenderTarget() || mDesc.mUsage.isDepthStencil() ||
			this->getD3D9Desc().Pool == D3DPOOL_DEFAULT && !mDesc.mUsage.isDynamic() )
			BLADE_EXCEPT(EXC_INVALID_OPERATION,BTString("none-lockable rendertarget or depth stencil.") );

		if (lockflags&IGraphicsBuffer::GBLF_DISCARD)
		{
			assert(mDesc.mUsage.isDynamic() || mDesc.mUsage.isDirectRead());
			assert(mDesc.mUsage != IGraphicsBuffer::GBU_DYNAMIC_WRITE || mMemoryVolume == NULL);
			assert(mMemoryVolume != NULL || mDesc.mUsage != IGraphicsBuffer::GBU_DYNAMIC_WRITE);
		}

		D3DBOX d3dbox;
		if( &box == &Box3i::EMPTY || box == Box3i::EMPTY)
		{
			d3dbox.Left = 0;
			d3dbox.Top = 0;
			d3dbox.Front = 0;
			d3dbox.Right = (UINT)box.getRight();
			d3dbox.Bottom = (UINT)box.getBottom();
			d3dbox.Back = (UINT)box.getBack();
		}
		else
		{
			d3dbox.Left = (UINT)box.getLeft();
			d3dbox.Right = (UINT)box.getRight();
			d3dbox.Bottom = (UINT)box.getBottom();
			d3dbox.Top = (UINT)box.getTop();
			d3dbox.Back = (UINT)box.getBack();
			d3dbox.Front = (UINT)box.getFront();
		}

		HRESULT result;
		D3DLOCKED_BOX locked_box;

		//dynamic + readable texture, lock use cached data, then update it
		if( this->isCacheNeeded() )
		{
			assert(mMemoryVolume != NULL);
			//record locked data and get cache data instead
			result = mMemoryVolume->lockProxy( this->getD3D9Resource(), region.mMipLevel, &locked_box, &d3dbox, (DWORD)D3DLockFlagConverter(lockflags) );
		}
		else
			result = this->getD3D9Resource()->LockBox( (UINT)region.mMipLevel, &locked_box, &d3dbox, (DWORD)D3DLockFlagConverter(lockflags) );

		region.mRowPitch = (size_t)locked_box.RowPitch;
		region.mSlicePitch = (size_t)locked_box.SlicePitch;
		return locked_box.pBits;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			D3D9VolumeTexture::unlockImpl()
	{
		index_t mipLevel = this->getLockedMip();

		HRESULT result;
		if( this->isCacheNeeded() )
		{
			assert(mMemoryVolume != NULL);
			//update cache data into dynamic texture
			result = mMemoryVolume->unlockProxy( this->getD3D9Resource(), mipLevel);
		}
		else
			result = this->getD3D9Resource()->UnlockBox( (UINT)mipLevel );
		return SUCCEEDED(result);
	}

	//////////////////////////////////////////////////////////////////////////
	bool			D3D9VolumeTexture::updateMipMap(const TEXREGION& region,index_t mipLevel/* = 0*/)
	{
		assert(region.mFace == 0);
		if( this->isLocked() )
		{
			assert(false);
			return false;
		}

		const Box3i& box = region.mBox;

		if( mDesc.mGpuAccess & IGraphicsBuffer::GBAF_WRITE )
			return false;

		if( mDesc.mUsage & IGraphicsBuffer::GBUF_AUTOMIPMAP )
		{
			this->getD3D9Resource()->SetAutoGenFilterType(D3DTEXF_LINEAR);
			this->getD3D9Resource()->GenerateMipSubLevels();
			mDesc.mMipmapCount = this->getD3D9Resource()->GetLevelCount();
			return true;
		}

		if( mipLevel >= mDesc.mMipmapCount || this->isLocked() )
			return false;

		HRESULT result;
		D3DBOX d3dbox;
		d3dbox.Left = (UINT)box.getLeft();
		d3dbox.Top = (UINT)box.getTop();
		d3dbox.Right = (UINT)box.getRight();
		d3dbox.Bottom = (UINT)box.getBottom();
		d3dbox.Front = (UINT)box.getFront();
		d3dbox.Back = (UINT)box.getBack();

		IDirect3DVolume9* SrcVolume = NULL;
		HRESULT hr = this->getD3D9Resource()->GetVolumeLevel( 0, &SrcVolume );
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

		D3DBOX targetBox;
		std::memcpy(&targetBox,&d3dbox,sizeof(d3dbox) );
		for(size_t i = 1; i < mDesc.mMipmapCount; ++i )
		{
			targetBox.Left /= 2;
			targetBox.Top /= 2;
			targetBox.Right /= 2;
			targetBox.Bottom /= 2;
			targetBox.Back /= 2;
			targetBox.Front /= 2;
			if( targetBox.Left == targetBox.Right || targetBox.Bottom == targetBox.Top || targetBox.Back == targetBox.Front)
				break;

			if( mipLevel != i && mipLevel != 0 )
				continue;

			IDirect3DVolume9* DestVolume = NULL;
			hr = this->getD3D9Resource()->GetVolumeLevel( (UINT)i, &DestVolume);
			if( FAILED(hr) )
				BLADE_EXCEPT(EXC_API_ERROR,BTString("unable to get mip level.") );
			hr = ::D3DXLoadVolumeFromVolume(DestVolume, NULL, &targetBox, SrcVolume, NULL, &d3dbox, D3DX_FILTER_BOX, 0);
			if( FAILED(hr) )
				BLADE_EXCEPT(EXC_API_ERROR,BTString("unable to update mip level.") );
			//DestVolume->Release();
			SrcVolume->Release();
			SrcVolume = DestVolume;
			d3dbox = targetBox;
		}

		SrcVolume->Release();

		device->Release();

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			D3D9VolumeTexture::addDirtyRegion(const TEXREGION& region)
	{
		const Box3i& dirtyBox = region.mBox;
		assert(region.mFace == 0);

		//dynamic + readable texture, perform a fully update. skip dirty region
		if (this->isCacheNeeded())
		{
			assert(mMemoryVolume != NULL);
			return true;
		}

		D3DBOX box;
		box.Left = (UINT)dirtyBox.getLeft();
		box.Right = (UINT)dirtyBox.getRight();
		box.Bottom = (UINT)dirtyBox.getBottom();
		box.Top = (UINT)dirtyBox.getTop();
		box.Back = (UINT)dirtyBox.getBack();
		box.Front = (UINT)dirtyBox.getFront();
		HRESULT hr = this->getD3D9Resource()->AddDirtyBox( &box );
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
	void			D3D9VolumeTexture::rebuildImpl(IDirect3DDevice9* device)
	{
		assert( this->getD3D9Resource() == NULL && device != NULL && this->isInDefaultPool() );
		if( this->getD3D9Resource() != NULL )
		{
			BLADE_EXCEPT(EXC_INVALID_OPERATION,BTString("cannot re-create resource if previous not releaseed."));
		}

		if( this->isInDefaultPool() && device != NULL )
		{
			IDirect3DVolumeTexture9* texture = NULL;
			const D3DVOLUME_DESC& desc = this->getD3D9Desc();
			HRESULT result = device->CreateVolumeTexture(desc.Width,desc.Height, desc.Depth,(UINT)mDesc.mMipmapCount,desc.Usage,desc.Format,desc.Pool,&texture,NULL);
			if( FAILED(result) )
				BLADE_EXCEPT(EXC_UNKNOWN,BTString("unable to rebuild texture.") );

			this->setD3D9Resource(texture);

			if( this->isCacheNeeded() )
			{
				assert(mMemoryVolume != NULL);
				D3DLOCKED_BOX locked_box;
				//update data from cache after reset
				D3DBOX d3dbox;
				d3dbox.Left = 0;
				d3dbox.Right = (UINT)mDesc.mWidth;
				d3dbox.Top = 0;
				d3dbox.Bottom = (UINT)mDesc.mHeight;
				d3dbox.Front = 0;
				d3dbox.Back = (UINT)mDesc.mDepth;
				result = mMemoryVolume->lockProxy(this->getD3D9Resource(), 0, &locked_box, &d3dbox, 0);
				if( FAILED(result) )
					BLADE_EXCEPT(EXC_UNKNOWN,BTString("unable to lock texture.") );

				result = mMemoryVolume->unlockProxy( this->getD3D9Resource(), 0);
				if( FAILED(result) )
					BLADE_EXCEPT(EXC_UNKNOWN,BTString("unable to lock texture.") );

				this->ITexture::updateMipMap();
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	/** @brief  */
	void	D3D9VolumeTexture::onSet(IDirect3DVolumeTexture9* source)
	{
		assert( this->getD3D9Resource() == NULL );

		const D3DVOLUME_DESC& desc = this->getD3D9Desc();
		D3DVOLUME_DESC	newDesc;
		source->GetLevelDesc(0,&newDesc);

		//if the source is new,we should clear up the pixel buffer list

		//D3DVOLUME_DESC has no align gap,so use memcmp
		if( ::memcmp(&newDesc,&desc,sizeof(D3DVOLUME_DESC)) != 0 || mDesc.mMipmapCount != source->GetLevelCount() )
		{
		}
		else
		{
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void		D3D9VolumeTexture::release()
	{
		D3D9Resource<IDirect3DVolumeTexture9>::release();
	}



	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/


	
}//namespace Blade

#endif //BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
