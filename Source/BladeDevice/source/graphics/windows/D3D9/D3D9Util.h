/********************************************************************
	created:	2012/11/28
	filename: 	D3D9Util.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_D3D9Util_h__
#define __Blade_D3D9Util_h__
#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
#include <graphics/windows/D3D9/D3D9Header.h>
#include <graphics/Util.h>

#include <BladeException.h>
#include <Handle.h>
#include <interface/public/graphics/PixelFormat.h>
#include <math/Box3i.h>
#include <utility/BladeContainer.h>


namespace Blade
{
	typedef SoftwareBuffer<DefaultStrategy> D3DBuffer;

	class D3DSystemMemorySurface : public Allocatable
	{
	public:
		D3DSystemMemorySurface(size_t width,size_t height,PixelFormat format)
			:mWidth(width),mHeight(height),mFormat(format),mReadOnly(false)
		{
			std::memset(&mLockedRect,0,sizeof(D3DLOCKED_RECT));

			mBuffer.allocBuffer( mWidth*mHeight*mFormat.getSizeBytes() );
		}
		~D3DSystemMemorySurface()		{}

		inline size_t	getPitch() const
		{
			return mWidth*mFormat.getSizeBytes();
		}

		/** @brief  */
		HRESULT	lockProxy(IDirect3DTexture9* texture, index_t mipLevel, D3DLOCKED_RECT* locked_rect, const RECT* rect,DWORD flags);

		/** @brief  */
		HRESULT		unlockProxy(IDirect3DTexture9* texture, index_t mipLevel);

		/** @brief  */
		HRESULT	lockProxy(IDirect3DCubeTexture9* texture, index_t mipLevel, D3DCUBEMAP_FACES face, D3DLOCKED_RECT* locked_rect, const RECT* rect,DWORD flags);

		/** @brief  */
		HRESULT		unlockProxy(IDirect3DCubeTexture9* texture, index_t mipLevel, D3DCUBEMAP_FACES face);

	protected:
		typedef TempVector<RECT>	DiryRectList;
		D3DBuffer		mBuffer;
		size_t			mWidth;
		size_t			mHeight;
		PixelFormat		mFormat;
		bool			mReadOnly;
		D3DLOCKED_RECT	mLockedRect;
	};//D3DSystemMemorySurface
	typedef D3DSystemMemorySurface SOFT_SURFACE;
	typedef Handle<SOFT_SURFACE> HSOFTSURFACE;


	class D3DSystemMemoryVolume : public Allocatable
	{
	public:
		D3DSystemMemoryVolume(size_t width, size_t height, size_t depth, PixelFormat format)
			:mWidth(width),mHeight(height),mDepth(depth),mFormat(format),mReadOnly(false)
		{
			std::memset(&mLockedBox,0,sizeof(D3DLOCKED_BOX));
			mBuffer.allocBuffer( mWidth*mHeight*mDepth*mFormat.getSizeBytes() );
		}

		~D3DSystemMemoryVolume()	{}

		/** @brief  */
		inline size_t getRowPitch() const
		{
			return mWidth*mFormat.getSizeBytes();
		}

		/** @brief  */
		inline size_t getSlicePitch() const
		{
			return mHeight*mWidth*mFormat.getSizeBytes();
		}

		/** @brief  */
		HRESULT	lockProxy(IDirect3DVolumeTexture9* texture, index_t mipLevel, D3DLOCKED_BOX* locked_box, const D3DBOX* box, DWORD flags);

		/** @brief  */
		HRESULT	unlockProxy(IDirect3DVolumeTexture9* texture, index_t mipLevel);

	protected:
		typedef TempVector<D3DBOX> DirtyBoxList;

		D3DBuffer		mBuffer;
		size_t			mWidth;
		size_t			mHeight;
		size_t			mDepth;
		PixelFormat		mFormat;
		bool			mReadOnly;
		D3DLOCKED_BOX	mLockedBox;
	};//D3DSystemMemoryVolume

	typedef D3DSystemMemoryVolume SOFT_VOLUME;
	typedef Handle<SOFT_VOLUME> HSFOTVOLUME;


	class D3DSystemMemoryBuffer : protected D3DBuffer, public Allocatable
	{
	public:
		D3DSystemMemoryBuffer(size_t total, const void* data)
			:mLockData(NULL),mOffset(0),mLength(0)
		{
			this->allocBuffer(total);
			if (data != NULL)
				std::memcpy(this->getData(), data, total);
		}

		/** @brief  */
		HRESULT	lockProxy(IDirect3DVertexBuffer9* buffer,UINT offset, UINT length, void** data, DWORD flag);

		/** @brief  */
		HRESULT	unlockProxy(IDirect3DVertexBuffer9* buffer);

		/** @brief  */
		HRESULT	lockProxy(IDirect3DIndexBuffer9* buffer,UINT offset, UINT length, void** data, DWORD flag);

		/** @brief  */
		HRESULT	unlockProxy(IDirect3DIndexBuffer9* buffer);

	protected:
		void*	mLockData;
		size_t	mOffset;
		size_t	mLength;
		bool	mReadOnly;
	};
	typedef D3DSystemMemoryBuffer SOFT_VERTEX;
	typedef D3DSystemMemoryBuffer SOFT_INDEX;

	typedef Handle<SOFT_VERTEX>	HSOFTVERTEX;
	typedef Handle<SOFT_INDEX>	HSOFTINDEX;


}//namespace Blade


#endif//BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS

#endif//__Blade_D3D9Util_h__