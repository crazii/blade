/********************************************************************
	created:	2014/05/09
	filename: 	Image.h
	author:		Crazii
	
	purpose:	Image class implementation
	remarks:	internal image memory layout compact(no gap), that means row pitch == row size, and slice pitch == image size
				so the direct copy (memcpy) could be applied to image data process
				and the compress & decompress operation also could be applied to2 2D / 3D image buffers directly.
*********************************************************************/
#ifndef __Blade_Image_h__
#define __Blade_Image_h__
#include <utility/BladeContainer.h>
#include <utility/FixedArray.h>
#include <math/BladeMath.h>

#include <interface/public/file/IFileDevice.h>
#include <interface/public/graphics/IImage.h>
#include <interface/public/graphics/IImageManager.h>
#include "BladeFreeImage.h"

#if BLADE_COMPILER_GNU_EXTENSION
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#pragma GCC diagnostic ignored "-Wcomment"
#endif

namespace Blade
{
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class ImageBase : public IImage
	{
	public:
		ImageBase(IMAGE_POOL pool)
		{
			mInfo.format = PF_UNDEFINED;
			mInfo.depth = 0;
			mInfo.faces = 0;
			mInfo.mipmap = 0;
			mInfo.pool = (uint8)pool;

			mLayout = IMGO_TOP_DOWN;
		}
		virtual ~ImageBase()	{ }
		/*
		@describe
		@param
		@return
		*/
		virtual const IMG_INFO&		getInfo() const
		{
			return mInfo;
		}

		/*
		@describe
		@param
		@return
		*/
		virtual IMG_ORIENT			getOrientation() const
		{
			return mLayout;
		}

		/*
		@describe
		@param
		@return
		*/
		virtual bool				setOrientation(IMG_ORIENT layout);
		/*
		@describe 
		@param [in] pxLayout: target layout to set
		@param [in] reformat: whether to change the content according layout. you can set reformat to false
		if the image is empty and you should fill the pixel data with the right layout
		@return 
		@note this is only available on 32BPP 8888 format
		*/
		virtual bool				setByteOrder(PIXEL_ORDER order, bool reformat = false);

		/*
		@describe 
		@param
		@return
		*/
		virtual bool				getPixelColor(index_t /*x*/, index_t /*y*/, Color::RGBA& /*outColor*/) const;

		/**
		@describe set linear flag. this WON'T change image data, only tagged for future texture generation
		@param
		@return
		*/
		virtual void				setLinearColorSpace(bool linear)
		{
			mInfo.format.setLinearColor(linear);
		}

		/************************************************************************/
		/*                                                                      */
		/************************************************************************/
		/** @brief  */
		virtual	bool				appendMipmap(const HIMAGE& mipImage) = 0;

		/** @brief */
		virtual bool				removeMipmaps(size_t count) = 0;

	protected:

		/** @brief  */
		virtual void			initializeImpl(size_t _width, size_t _height) = 0;

		/** @brief  */
		virtual void			clearImpl() = 0;

		/** @brief  */
		virtual void*			allocate(size_t bytes) = 0;

		/** @brief  */
		virtual void			free(void* ptr) = 0;

	public:

		/** @brief  */
		inline void			initialize(size_t width,size_t height, size_t LOD, PixelFormat format, size_t depth = 1,size_t faces = 1)
		{
			mInfo.format = format;
			mInfo.mipmap = (uint8)std::min(Math::CalcMaxMipCount(width, height),LOD);
			mInfo.depth = (uint8)depth;
			mInfo.faces = (uint8)faces;

			if( faces > 6 || faces == 0 || depth == 0 || LOD == 0)
			{
				assert(false);
				this->clear();
				return;
			}

			this->initializeImpl(width, height);
		}

		/** @brief  */
		inline void			clear()
		{
			this->clearImpl();

			mInfo.depth = 0;
			mInfo.faces = 0;
			mInfo.format = PF_UNDEFINED;
			mInfo.mipmap = 0;
		}

		/** @brief  */
		static inline ImageBase* createImage(IMAGE_POOL pool);

		/** @brief  */
		bool			load(const HSTREAM&	stream);

		/** @brief  */
		bool			create(size_t width, size_t height, PixelFormat format, IMG_ORIENT layout, int LOD, int depth, int faces);

		/** @brief  */
		ImageBase*		clone(IMAGE_POOL pool) const;

	protected:

		IMG_INFO		mInfo;
		IMG_ORIENT		mLayout;
	};//class ImageBase


	/************************************************************************/
	/* memory traits                                                                     */
	/************************************************************************/
	static const size_t HEADGUARD  = 0xB1900D09;
	static const size_t TAILGUARD = 0xBAD0BABE;
	template<IMAGE_POOL POOL>
	class IMGMemTraits
	{
	public:
		typedef IMG_MIPINFO MIPINFO;
		typedef void* MipInfoList;
		static void*	alloc(size_t /*bytes*/)	{return NULL;}
		static void		free(void* /*ptr*/)		{}
	};

	template<>
	class IMGMemTraits<IP_DEFAULT>
	{
	public:
		typedef struct DefaultInfo : public IMG_MIPINFO, public Allocatable	{}MIPINFO;
		typedef Vector<MIPINFO*> MipInfoList;
		static void*	alloc(size_t bytes)
		{
#if BLADE_DEBUG
			static_assert(sizeof(size_t) <= IImage::IMAGE_ALIGNMENT, "overflow");
			char* data = (char*)BLADE_IMAGE_ALIGN_ALLOC(bytes+IImage::IMAGE_ALIGNMENT*3, IImage::IMAGE_ALIGNMENT);

			*(size_t*)data = bytes;
			data += IImage::IMAGE_ALIGNMENT;

			*(size_t*)data = HEADGUARD;
			data += IImage::IMAGE_ALIGNMENT;

			*(size_t*)( data + bytes ) = TAILGUARD;
			return data;
#else
			return BLADE_IMAGE_ALIGN_ALLOC(bytes, IImage::IMAGE_ALIGNMENT);
#endif
		}
		static void		free(void* ptr)
		{
#if BLADE_DEBUG
			const char* data = (const char*)ptr;
			assert( *(size_t*)(data - IImage::IMAGE_ALIGNMENT) == HEADGUARD );
			size_t* bytes = (size_t*)(data - IImage::IMAGE_ALIGNMENT*2);
			assert( *(size_t*)( data + *bytes ) == TAILGUARD );
			return BLADE_IMAGE_ALIGN_FREE(bytes);
#else
			return BLADE_IMAGE_ALIGN_FREE(ptr);
#endif
		}
	};

	template<>
	class IMGMemTraits<IP_TEMPORARY>
	{
	public:
		typedef struct TempInfo : public IMG_MIPINFO, public TempAllocatable	{}MIPINFO;
		typedef TempVector<MIPINFO*> MipInfoList;
		static void*	alloc(size_t bytes)
		{
#if BLADE_DEBUG
			static_assert(sizeof(size_t) <= IImage::IMAGE_ALIGNMENT, "overflow");
			char* data = (char*)BLADE_TMP_ALIGN_ALLOC(bytes+IImage::IMAGE_ALIGNMENT*3, IImage::IMAGE_ALIGNMENT);

			*(size_t*)data = bytes;
			data += IImage::IMAGE_ALIGNMENT;

			*(size_t*)data = HEADGUARD;
			data += IImage::IMAGE_ALIGNMENT;

			*(size_t*)( data + bytes ) = TAILGUARD;
			return data;
#else
			return BLADE_TMP_ALIGN_ALLOC(bytes, IImage::IMAGE_ALIGNMENT);
#endif
		}
		static void		free(void* ptr)
		{
#if BLADE_DEBUG
			const char* data = (const char*)ptr;
			assert( *(size_t*)(data - IImage::IMAGE_ALIGNMENT) == HEADGUARD );
			size_t* bytes = (size_t*)(data - IImage::IMAGE_ALIGNMENT*2);
			assert( *(size_t*)( data + *bytes ) == TAILGUARD );
			return BLADE_TMP_ALIGN_FREE(bytes);
#else
			return BLADE_TMP_ALIGN_FREE(ptr);
#endif
		}
	};


	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	template<IMAGE_POOL POOL>
	class ImageImpl : public ImageBase, public IMGMemTraits<POOL>
	{
	public:
		typedef IMGMemTraits<POOL> Traits;
		typedef typename Traits::MIPINFO MIPINFO;
		typedef typename Traits::MipInfoList MipInfoList;
		typedef FixedVector<MipInfoList,6> FaceMipInfoList;

		ImageImpl() :ImageBase(POOL)	{}
		~ImageImpl()					{this->clear();}

		/*
		@describe 
		@param
		@return
		*/
		virtual const IMG_MIPINFO*	getMipInfo(index_t LOD, size_t face = 0) const
		{
			assert( mInfo.faces == 0 || mInfo.depth == 1);

			//validate
			if( mInfo.faces != mMipInfo.size() )
			{
				assert(false);
				return NULL;
			}

			//check face index
			if( (uint8)face >= mInfo.faces )
			{
				assert(face < 6 );
				return NULL;
			}

			const MipInfoList& mipInfo = mMipInfo[face];
			if(!( mInfo.mipmap == mipInfo.size() ) )
			{
				assert(false);
				return NULL;
			}

			//check LOD index
			if(!( LOD < mInfo.mipmap ) )
				return NULL;			
			return mipInfo[LOD];
		}

	protected:

		/** @brief  */
		virtual	bool			appendMipmap(const HIMAGE& mipImage)
		{
			assert( this->getMipmapCount() < Math::CalcMaxMipCount(this->getWidth(), this->getHeight()) );

			//no images, init
			if( (mipImage != NULL && mInfo.mipmap == 0) || mInfo.faces == 0 )
			{
				assert( mInfo.mipmap == 0 && mInfo.faces == 0 );
				assert( mInfo.format == PF_UNDEFINED );
				this->initialize( mipImage->getImageWidth(), mipImage->getImageHeight(), 1, mipImage->getFormat(), mipImage->getDepth(), mipImage->getFaceCount() );
				mLayout = mipImage->getOrientation();

				for(size_t f = 0; f < this->getFaceCount(); ++f)
				{
					const IMG_MIPINFO* src = mipImage->getMipInfo(0, f);
					if( src == NULL || src->data == NULL )
					{
						assert(false);
						return false;
					}

					const IMG_MIPINFO* dest = mMipInfo[0][mInfo.mipmap-1u];
					assert( dest != NULL );
					assert( dest != NULL && dest->data != NULL );
					uint32 width = dest->width;
					uint32 height = dest->height;
					uint32 oriWidth = dest->oriWidth;
					uint32 oriHeight = dest->oriHeight;
					assert( width == src->width && height == src->height && oriWidth == src->oriWidth && oriHeight == src->oriHeight );
					assert( dest->bytes == src->bytes );
					BLADE_UNREFERENCED(oriWidth);
					BLADE_UNREFERENCED(oriHeight);
					BLADE_UNREFERENCED(width);
					BLADE_UNREFERENCED(height);
					std::memcpy(dest->data, src->data, dest->bytes);
				}
				return true;
			}
			else if( mipImage != NULL && this->getFormat() == mipImage->getFormat() && this->getDepth() == mipImage->getDepth() && this->getFaceCount() == mipImage->getFaceCount() )
			{
				for(size_t f = 0; f < this->getFaceCount(); ++f)
				{
					const IMG_MIPINFO* src = mipImage->getMipInfo(0, f);
					if( src == NULL || src->data == NULL )
					{
						for(size_t i = 0; i < f; ++i)
						{
							IMG_MIPINFO* dest = mMipInfo[i][mInfo.mipmap-1u];
							Traits::free(dest->data);
							BLADE_DELETE dest;
							mMipInfo[i].pop_back();
						}
						assert(false);
						return false;
					}

					const IMG_MIPINFO* lastMip = this->getMipInfo(this->getMipmapCount()-1u, f);
					assert( lastMip != NULL && lastMip->data != NULL );
					uint32 width = lastMip->width/2;
					uint32 height = lastMip->height/2;
					uint32 oriWidth = lastMip->oriWidth/2;
					uint32 oriHeight = lastMip->oriHeight/2;
					assert( width == src->width && height == src->height && oriWidth == src->oriWidth && oriHeight == src->oriHeight );

					mMipInfo[f].push_back( BLADE_NEW MIPINFO() );
					assert( mMipInfo[f].size() == (size_t)mInfo.mipmap+1 );
					IMG_MIPINFO* dest = mMipInfo[f][mInfo.mipmap];

					dest->width = width;
					dest->height = height;
					dest->oriWidth = oriWidth;
					dest->oriHeight = oriHeight;
					dest->rowPitch = src->rowPitch;
					dest->slicePitch = src->slicePitch;
					dest->bytes = src->bytes;
					dest->data = (uint8*)Traits::alloc( dest->bytes );
					std::memcpy(dest->data, src->data, dest->bytes);
				}
				++mInfo.mipmap;
				return true;
			}
			assert(false);
			return false;
		}

		//////////////////////////////////////////////////////////////////////////
		bool					removeMipmaps(size_t count)
		{
			if( mInfo.mipmap > 1 && count > 0 )
			{
				for(size_t f = 0; f < mInfo.faces; ++f)
				{
					MipInfoList& mipInfo = mMipInfo[f];
					size_t n = 0;
					size_t fcount = count;
					for(int i = (int)mipInfo.size() - 1; i > 0 && fcount > 0; --i)
					{
						Traits::free( mipInfo[(size_t)i]->data );
						BLADE_DELETE mipInfo[(size_t)i];
						mipInfo[(size_t)i] = NULL;
						--fcount;
						++n;
					}
					assert( n < mipInfo.size() );
					mipInfo.resize(mipInfo.size() - n);
					assert(mipInfo.size() >= 1);
				}
				mInfo.mipmap = (uint8)mMipInfo[0].size();
				return true;
			}
			return false;
		}

		/** @brief  */
		virtual void			initializeImpl(size_t _width, size_t _height)
		{
			this->clearImpl();
			assert( mInfo.faces == 1 || mInfo.depth == 1);

			mMipInfo.resize( mInfo.faces );
			for(size_t i = 0; i < mInfo.faces; ++i)
			{
				MipInfoList& mipInfo = mMipInfo[i];
				mipInfo.reserve( mInfo.mipmap );
				size_t width = _width;
				size_t height = _height;

				for(size_t j = 0; j < mInfo.mipmap; ++j )
				{
					mipInfo.push_back( BLADE_NEW MIPINFO() );
					MIPINFO* infoHeader = mipInfo.back();
					infoHeader->bytes  = IImageManager::getSingleton().calcImageSize(width, height, mInfo.depth, mInfo.format);
					assert(infoHeader->bytes != 0);
					infoHeader->oriWidth = (uint32)width;
					infoHeader->oriHeight = (uint32)height;
					if( mInfo.format.isCompressed() )
					{
						infoHeader->width = (uint32)((width+3)/4);
						infoHeader->height = (uint32)((height+3)/4);
					}
					else
					{
						infoHeader->width = (uint32)width;
						infoHeader->height = (uint32)height;
					}
					infoHeader->rowPitch = std::max<uint32>( 1, infoHeader->width ) * (uint32)mInfo.format.getSizeBytes();
					infoHeader->slicePitch = infoHeader->rowPitch * infoHeader->height;
					//TODO: pooling?
					infoHeader->data = (uint8*)Traits::alloc(infoHeader->bytes);
					std::memset(infoHeader->data, 0xFF, infoHeader->bytes);

					width /= 2;
					height /= 2;

					if (width < 1)
						width = 1;
					if (height < 1)
						height = 1;
				}
			}
		}

		/** @brief  */
		virtual void			clearImpl()
		{
			assert( mInfo.faces == 1 || mInfo.depth == 1);
			if( mMipInfo.size() == 0 )
				return;

			for(size_t i = 0; i < mInfo.faces; ++i)
			{
				MipInfoList& mipInfo = mMipInfo[i];
				for(size_t j = 0; j < mipInfo.size(); ++j )
				{
					Traits::free( mipInfo[j]->data );
					BLADE_DELETE mipInfo[j];
				}
				mipInfo.clear();
			}
			mMipInfo.clear();
		}

		/** @brief  */
		virtual void*			allocate(size_t bytes)
		{
			return Traits::alloc(bytes);
		}

		/** @brief  */
		virtual void			free(void* ptr)
		{
			return Traits::free(ptr);
		}
	protected:

		FaceMipInfoList	mMipInfo;
	};

	class Image : public ImageImpl<IP_DEFAULT>, public Allocatable{};
	class TempImage : public ImageImpl<IP_TEMPORARY>, public TempAllocatable{};

	inline ImageBase* ImageBase::createImage(IMAGE_POOL pool)
	{
		return (pool == IP_TEMPORARY) ? (ImageBase*)(BLADE_NEW TempImage()) : (ImageBase*)(BLADE_NEW Image());
	}

}//namespace Blade


#endif // __Blade_Image_h__