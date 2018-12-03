/********************************************************************
	created:	2010/04/10
	filename: 	D3D9PixelBuffer.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
#include <graphics/windows/D3D9/D3D9PixelBuffer.h>
#include <graphics/windows/D3D9/D3D9TypeConverter.h>

//#include <math/Box3.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	D3D9PixelBuffer::D3D9PixelBuffer(IDirect3DSurface9* pSurface,ELocation location)
		:PixelBuffer(location)
		,D3D9Resource<IDirect3DSurface9>(pSurface)
	{
		assert( pSurface!= NULL );
		const D3DSURFACE_DESC& desc = this->getD3D9Desc();
		mUsage = D3DUsageConverter(desc.Usage);
		mFormat= D3DFormatConverter(desc.Format);
		mWidth = desc.Width;
		mHeight = desc.Height;
		mDepth = 1;
	}

	//////////////////////////////////////////////////////////////////////////
	D3D9PixelBuffer::~D3D9PixelBuffer()
	{
	}

	/************************************************************************/
	/* D3D9Resource imp                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void	D3D9PixelBuffer::rebuildImpl(IDirect3DDevice9* device)
	{
		if( !this->isInDefaultPool() || device == NULL )
		{
			assert(false);
			return;
		}

		if( mLocation == PBL_BACKBUFFER )
		{
			//it is the swap chain is duty to rebuild pixel buffer
			return;
		}
		else if( (mLocation & PBL_TEXTURE_MASK) != 0 )
		{
			//it is the texture's duty to rebuild render texture
			return;
		}

		if( (mUsage.isRenderTarget() ) )
		{
			//the pixel buffer directly comes from a render target surface.
			if( this->getD3D9Resource() != NULL )
			{
				assert(false);
				BLADE_EXCEPT(EXC_INVALID_OPERATION,BTString("cannot re-create resource if previous not releaseed."));
			}

			IDirect3DSurface9* surface = NULL;
			HRESULT result = device->CreateRenderTarget((UINT)mWidth,(UINT)mHeight,D3DFormatConverter(mFormat),D3DMULTISAMPLE_NONE,0,FALSE,&surface,NULL);
			if( FAILED(result) )
			{
				BLADE_EXCEPT(EXC_UNKNOWN,BTString("unable to create rendertarget.") );
			}
			this->setD3D9Resource(surface);
		}
		else if( (mUsage&GBUF_DEPTHSTENCIL) )
		{
			if( this->getD3D9Resource() != NULL )
			{
				assert(false);
				BLADE_EXCEPT(EXC_INVALID_OPERATION,BTString("cannot re-create resource if previous not releaseed."));
			}

			IDirect3DSurface9* surface = NULL;
			const D3DSURFACE_DESC& desc = this->getD3D9Desc();
			HRESULT result = device->CreateDepthStencilSurface(desc.Width,desc.Height,desc.Format,desc.MultiSampleType,desc.MultiSampleQuality,FALSE,&surface,NULL);
			if( FAILED(result) )
			{
				BLADE_EXCEPT(EXC_UNKNOWN,BTString("unable to create rendertarget.") );
			}
			this->setD3D9Resource(surface);
		}
		else
		{
			//assert( false );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void		D3D9PixelBuffer::onSet(IDirect3DSurface9* source)
	{
		D3DSURFACE_DESC desc;
		source->GetDesc(&desc);
		mUsage = D3DUsageConverter(desc.Usage);
		mFormat = D3DFormatConverter(desc.Format);
		mWidth = desc.Width;
		mHeight = desc.Height;
		mDepth = 1;
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void*		D3D9PixelBuffer::lock(const Box3i& box,size_t& outPitch,LOCKFLAGS lockflags)
	{
		if( mIsLocked )
		{
			if( !mUsage.isRenderTarget() && !mUsage.isDepthStencil() )
			{
				mIsLocked = true;

				HRESULT result;
				D3DLOCKED_RECT locked_rect;

				if( &box == &Box3i::EMPTY || box == Box3i::EMPTY )
					result = this->getD3D9Resource()->LockRect(&locked_rect,NULL,(DWORD)D3DLockFlagConverter(lockflags));
				else
				{
					RECT rect;
					rect.left = (LONG)box.getLeft();
					rect.right = (LONG)box.getRight();
					rect.bottom = (LONG)box.getBottom();
					rect.top = (LONG)box.getTop();
					result = this->getD3D9Resource()->LockRect(&locked_rect,&rect,(DWORD)D3DLockFlagConverter(lockflags) );
				}

				if( FAILED(result) )
					BLADE_EXCEPT(EXC_UNKNOWN,BTString("unable to lock pixel buffer.") );
				outPitch = (size_t)locked_rect.Pitch;
				return locked_rect.pBits;
			}
			else
				BLADE_EXCEPT(EXC_INVALID_OPERATION,BTString("none-lockabe render target or depth stencil.") );
		}
		else
			BLADE_EXCEPT(EXC_REDUPLICATE,BTString("buffer already locked,cannot lock twice.") );
	}

	//////////////////////////////////////////////////////////////////////////
	void		D3D9PixelBuffer::unlock(void)
	{
		this->getD3D9Resource()->UnlockRect();
		mIsLocked = false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		D3D9PixelBuffer::addDirtyRegion(const Box3i& dirtyBox)
	{
		HRESULT result;
		if( (mLocation & PBL_TEXTURE_MASK) )
		{
			switch( mLocation )
			{
			case PBL_TEXTURE_1D:
			case PBL_TEXTURE_2D:
				{
					void *pContainer = NULL;
					result = this->getD3D9Resource()->GetContainer(::IID_IDirect3DTexture9,&pContainer);

					if( SUCCEEDED(result) )
					{
						IDirect3DTexture9* pTexture = (IDirect3DTexture9 *)pContainer;
						RECT rect;
						rect.left = (LONG)dirtyBox.getLeft();
						rect.right = (LONG)dirtyBox.getRight();
						rect.bottom = (LONG)dirtyBox.getBottom();
						rect.top = (LONG)dirtyBox.getTop();
						result = pTexture->AddDirtyRect( &rect );
					}
					break;
				}
			case PBL_TEXTURE_3D:
				{
					BLADE_EXCEPT(EXC_INVALID_OPERATION,BTString("not implemented.") );
					break;
				}
			case PBL_TEXTURE_CUBE:
				{
					BLADE_EXCEPT(EXC_INVALID_OPERATION,BTString("not implemented.") );
					break;
				}
			default:
				BLADE_EXCEPT(EXC_UNKNOWN,BTString("code not supposed to run to here.") );
			}
		}
		else
			BLADE_EXCEPT(EXC_UNKNOWN,BTString("cannot add dirty region of a pixel buffer that doesn't belong to a texture.") );

		if( FAILED(result) )
			BLADE_EXCEPT(EXC_UNKNOWN,BTString("unable to add dirty region to pixel buffer.") );
		return true;
	}
	
}//namespace Blade

#endif //BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
