#/********************************************************************
	created:	2014/12/20
	filename: 	GLESUtil.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_GLESUtil_h__
#define __Blade_GLESUtil_h__
#include "GLESHeader.h"
#if BLADE_USE_GLES
#include <graphics/Util.h>
#include <utility/Buffer.h>
#include <interface/public/graphics/PixelFormat.h>
#include <interface/public/graphics/IImage.h>
#include <interface/public/graphics/IGraphicsBuffer.h>
#include <interface/public/graphics/VertexElement.h>
#include <interface/public/graphics/RenderProperty.h>

#include <utility/BladeContainer.h>
#include <math/Box3i.h>
#include "public/GLESPixelFormat.h"

#ifndef COMPRESSED_RGB_S3TC_DXT1_EXT
	#define COMPRESSED_RGB_S3TC_DXT1_EXT		0x83F0
#endif
#ifndef COMPRESSED_RGBA_S3TC_DXT1_EXT
	#define COMPRESSED_RGBA_S3TC_DXT1_EXT		0x83F1
#endif
#ifndef COMPRESSED_RGBA_S3TC_DXT3_EXT
	#define COMPRESSED_RGBA_S3TC_DXT3_EXT		0x83F2
#endif
#ifndef COMPRESSED_RGBA_S3TC_DXT5_EXT
	#define COMPRESSED_RGBA_S3TC_DXT5_EXT		0x83F3
#endif

namespace Blade
{
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class GLESCache : protected SoftwareBuffer<TempStrategy>, public NonAllocatable
	{
		typedef SoftwareBuffer<TempStrategy> BaseType;
	public:
		inline GLESCache()		{}
		inline ~GLESCache()		{}

		/** @brief  */
		inline void* lock(size_t offset, size_t bytes)
		{
			BaseType::allocBuffer(bytes);
			void* ret = BaseType::getData();
			if( ret != NULL )
			{
				mOffset = offset;
				mSize = bytes;
			}
			return ret;
		}

		/** @brief  */
		inline void unlock()
		{
			BaseType::freeBuffer();
			mOffset = size_t(-1);
			mSize = size_t(-1);
		}

		/** @brief  */
		inline size_t getOffset() const	{return mOffset;}
		inline size_t getSize() const	{return mSize;}

	protected:
		size_t mOffset;
		size_t mSize;
	};//GLESCache

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class ShaderBuffer : public Buffer
	{
	public:
		ShaderBuffer(IPool* pool)
			:Buffer(pool)
		{

		}
	};

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class IImage;
	class GLESImageCache : public NonAllocatable
	{
	public:
		GLESImageCache();
		~GLESImageCache();

		/** @brief  */
		bool	lock(const Box3i& box, bool readOnlyLock, PixelFormat format, IMG_ORIENT layout);

		/** @brief clear internal image cache and all dirty regions */
		bool	unlock();

		/** @brief  */
		void*	getData(size_t* rowPitch = NULL, size_t* slicePitch = NULL) const;

		/** @brief  */
		bool	getSubData(const Box3i& box, TempBuffer& buffer) const;

		/** @brief  */
		bool	getSize(size_t& width, size_t& height) const;

		/** @brief  */
		inline bool		isReadOnly() const	{return mReadOnly;}

		/** @brief  */
		inline bool	isLocked() const
		{
			assert( !mLocked || mImage != NULL );
			return mLocked;
		}

		/** @brief add dirty region */
		inline bool		addDirtyRegion(const Box3i& region)
		{
			if( mLocked && !mReadOnly )
			{
				//bounds check
				if( !mLockedBox.contains(region) ) 
				{
					assert(false);
					return false;
				}

				for(size_t i = 0; i < mDirtyRegions.size(); ++i)
				{
					if( mDirtyRegions[i].contains(region) )
						return true;
				}
				mDirtyRegions.push_back(region);
				return true;
			}
			else
				return false;
		}

		/** @brief  */
		inline size_t	getDirtyCount() const {return mDirtyRegions.size();
		/** @brief  */}
		inline const Box3i& getDirtyRegion(index_t index)
		{
			assert( index < mDirtyRegions.size() );
			return index < mDirtyRegions.size() ? mDirtyRegions[index] : Box3i::EMPTY;
		}

	protected:
		typedef TempVector<Box3i>	DirtyRegionList;
		DirtyRegionList mDirtyRegions;
		HIMAGE	mImage;
		Box3i	mLockedBox;
		bool	mLocked;
		bool	mReadOnly;
	};

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class GLESBufferUsageConverter : public NonAllocatable
	{
	public:
		inline GLESBufferUsageConverter(IGraphicsBuffer::USAGE usage)
			:mUsage(usage)
		{
			if( mUsage.isDynamic() )
			{
				if( (mUsage.getCPUAccess()&IGraphicsBuffer::GBAF_READ) != 0 )
					mGLUsage = GL_DYNAMIC_COPY;
				else
					mGLUsage = GL_DYNAMIC_DRAW;
			}
			else
			{
				if( (mUsage.getCPUAccess()&IGraphicsBuffer::GBAF_READ) != 0 )
					mGLUsage = GL_STATIC_COPY;
				else
					mGLUsage = GL_STATIC_DRAW;
			}
		}

		inline GLESBufferUsageConverter(GLenum usage)
			:mGLUsage(usage)
		{
			int mask = 0;
			if( usage == GL_DYNAMIC_COPY || usage == GL_DYNAMIC_DRAW )
				mask |= IGraphicsBuffer::GBUF_DYNAMIC;

			if( usage == GL_STATIC_COPY || usage == GL_DYNAMIC_COPY )
				mask |= IGraphicsBuffer::GBAF_READ_WRITE;
			else
				mask |= IGraphicsBuffer::GBAF_WRITE;

			mUsage = mask;
		}

		/** @brief  */
		inline operator IGraphicsBuffer::USAGE() const {return mUsage;}
		/** @brief  */
		inline operator GLenum() const {return mGLUsage;}

	protected:
		GLenum	mGLUsage;
		IGraphicsBuffer::USAGE mUsage;
	};//GLESBufferUsageConverter

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class GLESLockFlagsConverter
	{
	public:
		GLESLockFlagsConverter(IGraphicsBuffer::LOCKFLAGS flags)
			:mFLags(flags)
		{
			mGLFlags = 0;
			if( (flags&IGraphicsBuffer::GBLF_READONLY) )
				mGLFlags |= GL_MAP_READ_BIT;

			if( (flags&IGraphicsBuffer::GBLF_WRITEONLY) )
			{
				mGLFlags |= GL_MAP_WRITE_BIT;

				if( (flags&IGraphicsBuffer::GBLF_DISCARD) )
					mGLFlags |= GL_MAP_INVALIDATE_RANGE_BIT;

				if ((flags&IGraphicsBuffer::GBLF_NO_DIRTY_UPDATE))
					mGLFlags |= GL_MAP_FLUSH_EXPLICIT_BIT;

				if ((flags&IGraphicsBuffer::GBLF_NO_OVERWRITE))
					mGLFlags |= GL_MAP_UNSYNCHRONIZED_BIT;
			}
		}

		GLESLockFlagsConverter(GLbitfield  GLFlags)
			:mGLFlags(GLFlags)
		{
			mFLags = IGraphicsBuffer::GBLF_UNDEFINED;

			if( (mGLFlags&GL_MAP_READ_BIT) )
				mFLags |= IGraphicsBuffer::GBLF_READONLY;

			if( (mGLFlags&GL_MAP_WRITE_BIT) )
			{
				mFLags |= IGraphicsBuffer::GBLF_WRITEONLY;

				if( (mGLFlags&GL_MAP_INVALIDATE_RANGE_BIT) )
					mFLags |= IGraphicsBuffer::GBLF_DISCARD;

				if ((mGLFlags&GL_MAP_FLUSH_EXPLICIT_BIT))
					mFLags |= IGraphicsBuffer::GBLF_NO_DIRTY_UPDATE;

				if ((mGLFlags&GL_MAP_UNSYNCHRONIZED_BIT))
					mFLags |= IGraphicsBuffer::GBLF_NO_OVERWRITE;
			}
		}

		/** @brief  */
		inline operator IGraphicsBuffer::LOCKFLAGS() const	{return mFLags;}
		/** @brief  */
		inline operator GLbitfield() const					{return mGLFlags;}

	protected:
		IGraphicsBuffer::LOCKFLAGS	mFLags;
		GLbitfield					mGLFlags;
	};//GLESLockFlagsConverter

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class GLESVertexAttrib
	{
	public:
		GLint		GLcount;
		GLenum		GLtype;
		GLboolean	GLnormalize;
		inline bool operator==(const GLESVertexAttrib& rhs)
		{
			//return ::memcmp(this, &rhs, sizeof(*this) ) == 0; //note: memcmp is wrong when extra data existing for alignment
			return GLcount == rhs.GLcount && GLtype == rhs.GLtype && GLnormalize == rhs.GLnormalize;
		}
	};

	class GLESVertexElementConverter
	{
	public:
		static const GLESVertexAttrib msTable[VET_COUNT];

		GLESVertexElementConverter(GLESVertexAttrib type)
			:mGLAttrib(type)
		{
			mType = VET_FLOAT1;
			bool isDefined = false;
			for(int i = VET_BEGIN; i < VET_COLOR; ++i)
			{
				if( type == msTable[i] )
				{
					mType = EVertexElementType(i);
					isDefined = true;
					break;
				}
			}
			assert( isDefined && "enum for vertex atribute not defined in public header" );
		}

		GLESVertexElementConverter(GLint count, GLenum type, GLboolean normalize)
		{
			GLESVertexAttrib attrib = {count ,type, normalize};
			*this = GLESVertexElementConverter(attrib);
		}

		GLESVertexElementConverter(EVertexElementType type)
			:mType(type)
		{
			if( type >= VET_BEGIN && type < VET_COUNT)
				mGLAttrib = msTable[type];
			else
			{
				assert(false);
				mGLAttrib = msTable[0];
			}
		}

		/** @brief  */
		inline operator EVertexElementType() const	{return mType;}
		/** @brief  */
		inline operator const GLESVertexAttrib&() const	{return mGLAttrib;}
		/** @brief  */
		inline GLenum		getGLType() const		{return mGLAttrib.GLtype;}
		/** @brief get component count [1,4] */
		inline GLint		getGLCount() const		{return mGLAttrib.GLcount;}
		/** @brief  */
		inline GLboolean	getGLNormalize() const	{return mGLAttrib.GLnormalize;}

	protected:
		GLESVertexAttrib	mGLAttrib;
		EVertexElementType	mType;
	};//GLESVertexElementConverter

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class GLESBlendModeConverter
	{
	public:
		GLESBlendModeConverter(BLEND_MODE mode)
		{
			assert(mode >= BM_ZERO && mode <= BM_INV_DEST_COLOR);
			assert(mode >= 1 && mode <= 10);	//in case enum value changes
			static GLenum GL_BLEND_MODE[11] =
			{
				GL_ZERO,	//0: unused

				GL_ZERO,	//1:BM_ZERO
				GL_ONE,		//	BM_ONE
				GL_SRC_COLOR,
				GL_ONE_MINUS_SRC_COLOR,
				GL_SRC_ALPHA,
				GL_ONE_MINUS_SRC_ALPHA,
				GL_DST_ALPHA,
				GL_ONE_MINUS_DST_ALPHA,
				GL_DST_COLOR,
				GL_ONE_MINUS_DST_COLOR
			};
			mBlendMode = GL_BLEND_MODE[mode];
		}

		/** @brief  */
		inline operator GLenum() const	{return mBlendMode;}

		GLenum mBlendMode;
	};

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class GLESBlendOpConverter
	{
	public:
		GLESBlendOpConverter(BLEND_OP op)
		{
			assert( op >= BO_ADD && op <= BO_MAX );
			assert( op >= 1 && op <= 5);
			static GLenum GL_BLEND_OP[6] = 
			{
				GL_FUNC_ADD,	//0

				GL_FUNC_ADD,
				GL_FUNC_SUBTRACT,
				GL_FUNC_REVERSE_SUBTRACT,
				GL_MIN,
				GL_MAX,
			};
			mBlendOp = GL_BLEND_OP[op];
		}
		/** @brief  */
		inline operator GLenum() const	{return mBlendOp;}
	protected:
		GLenum mBlendOp;
	};

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class GLESDepthSencilFuncConverter
	{
	public:
		GLESDepthSencilFuncConverter(TEST_FUNC func)
		{
			assert( func >= TF_NEVER && func <= TF_ALWAYS );
			assert( func >= 1 && func <= 8);
			static GLenum GL_TEST_FUNC[9] = 
			{
				GL_NEVER,	//0

				GL_NEVER,
				GL_LESS,
				GL_EQUAL,
				GL_LEQUAL,
				GL_GREATER,
				GL_NOTEQUAL,
				GL_GEQUAL,
				GL_ALWAYS,
			};
			mGLFuc = GL_TEST_FUNC[func];
		}
		/** @brief  */
		inline operator GLenum() const	{return mGLFuc;}
		GLenum mGLFuc;
	};

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class GLESStencilOpConverter
	{
	public:
		GLESStencilOpConverter(STENCIL_OP op)
		{
			assert( op >= SOP_KEEP && op <= SOP_DECRESE_WRAP);
			assert( op >= 1 && op <= 8 );
			static GLenum GL_STENCIL_OP[9] = 
			{
				GL_KEEP,	//0

				GL_KEEP,
				GL_ZERO,
				GL_REPLACE,
				GL_INCR,
				GL_DECR,
				GL_INVERT,
				GL_INCR_WRAP,
				GL_DECR_WRAP,
			};
			mStencilOp = GL_STENCIL_OP[op];
		}
		/** @brief  */
		inline operator GLenum() const	{return mStencilOp;}
	protected:
		GLenum mStencilOp;
	};

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class GLESTexAddressModeConverter
	{
	public:
		GLESTexAddressModeConverter(TEXADDR_MODE mode)
		{
			assert( mode >= TAM_WRAP && mode <= TAM_CLAMP );
			assert( mode >= 1 && mode <= 3 );
			static GLint GL_TEXADDR_MODE[4] = 
			{
				GL_REPEAT,	//0

				GL_REPEAT,
				GL_MIRRORED_REPEAT,
				GL_CLAMP_TO_EDGE,
			};
			mMode = GL_TEXADDR_MODE[mode];
		}
		/** @brief  */
		inline operator GLint() const	{return mMode;}
	protected:
		GLint mMode;
	};

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class GLESTexFilterConverter
	{
	public:
		GLESTexFilterConverter(TEXFILER_MODE mode)
		{
			assert( mode >= TFM_NONE && mode <= TFM_ANISOTROPIC);
			assert( mode >= 0 && mode <= 3);
			static GLint GL_FILTER_MODE[4] =
			{
				GL_NEAREST,	//TFM_NONE
				GL_NEAREST, //TFM_POINT
				GL_LINEAR,
				GL_LINEAR,	//anisotropy not supported
			};
			mFilter = GL_FILTER_MODE[mode];
		}

		GLESTexFilterConverter(TEXFILER_MODE min, TEXFILER_MODE mip)
		{
			assert( min >= TFM_NONE && min <= TFM_ANISOTROPIC);
			assert( min >= 0 && min <= 3);
			assert( mip >= TFM_NONE && mip <= TFM_ANISOTROPIC);
			assert( mip >= 0 && mip <= 3);

			static GLint GL_FILTER_MODE[4][4] =
			{
				//min: none
				GL_NEAREST_MIPMAP_NEAREST,	//TFM_NONE
				GL_NEAREST_MIPMAP_NEAREST, //TFM_POINT
				GL_NEAREST_MIPMAP_LINEAR,
				GL_NEAREST_MIPMAP_LINEAR,	//anisotropy not supported
				//min: point
				GL_NEAREST_MIPMAP_NEAREST,	//TFM_NONE
				GL_NEAREST_MIPMAP_NEAREST, //TFM_POINT
				GL_NEAREST_MIPMAP_LINEAR,
				GL_NEAREST_MIPMAP_LINEAR,	//anisotropy not supported
				//min: linear
				GL_LINEAR_MIPMAP_NEAREST,
				GL_LINEAR_MIPMAP_NEAREST,
				GL_LINEAR_MIPMAP_LINEAR,
				GL_LINEAR_MIPMAP_LINEAR,
				//min: anisotropy
				GL_LINEAR_MIPMAP_NEAREST,
				GL_LINEAR_MIPMAP_NEAREST,
				GL_LINEAR_MIPMAP_LINEAR,
				GL_LINEAR_MIPMAP_LINEAR,
			};
			mFilter = GL_FILTER_MODE[min][mip];
		}

		inline operator GLint() const	{return mFilter;}
	protected:
		GLint mFilter;
	};
	
}//namespace Blade

#endif


#endif // __Blade_GLESUtil_h__