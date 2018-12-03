/********************************************************************
	created:	2010/04/10
	filename: 	D3D9TypeConverter.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS

#ifndef __Blade_D3D9TypeConverter_h__
#define __Blade_D3D9TypeConverter_h__
#include <graphics/windows/D3D9/D3D9Header.h>
#include <interface/public/graphics/IGraphicsBuffer.h>
#include <interface/public/graphics/IIndexBuffer.h>
#include <interface/public/graphics/PixelFormat.h>
#include <interface/public/graphics/VertexElement.h>


namespace Blade
{

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class D3DUsageConverter
	{
	public:

		/** @brief  */
		inline D3DUsageConverter(DWORD D3DUsage)
			:mD3D9Usage(D3DUsage)
			,mUsage(IGraphicsBuffer::GBU_DEFAULT)
		{
			if( (mD3D9Usage&D3DUSAGE_DYNAMIC) )
			{
				mUsage = IGraphicsBuffer::GBUF_DYNAMIC;
			}

			if( (mD3D9Usage&D3DUSAGE_DEPTHSTENCIL) )
			{
				(int&)mUsage |= IGraphicsBuffer::GBUF_DEPTHSTENCIL;
			}

			if( (mD3D9Usage&D3DUSAGE_AUTOGENMIPMAP) )
			{
				(int&)mUsage |= IGraphicsBuffer::GBUF_AUTOMIPMAP;
			}

			if( (mD3D9Usage&D3DUSAGE_RENDERTARGET) )
			{
				(int&)mUsage |= IGraphicsBuffer::GBUF_RENDERTARGET;
			}
		}

		/** @brief  */
		inline D3DUsageConverter(IGraphicsBuffer::USAGE usage)
			:mD3D9Usage(0)
			,mUsage(usage)
		{
			if( (mUsage & (IGraphicsBuffer::GBUF_DYNAMIC | IGraphicsBuffer::GBUF_CPU_READ)) )
			{
				mD3D9Usage |= D3DUSAGE_DYNAMIC;
				mD3D9Usage |= D3DUSAGE_WRITEONLY;
			}

			if( ( mUsage & IGraphicsBuffer::GBUF_DEPTHSTENCIL) )
			{
				mD3D9Usage |= D3DUSAGE_DEPTHSTENCIL;
			}

			if( (mUsage & IGraphicsBuffer::GBUF_AUTOMIPMAP) )
			{
				mD3D9Usage |= D3DUSAGE_AUTOGENMIPMAP;
			}

			if( (mUsage &IGraphicsBuffer::GBUF_RENDERTARGET) )
			{
				mD3D9Usage |= D3DUSAGE_RENDERTARGET;
			}
		}

		/** @brief  */
		inline operator IGraphicsBuffer::USAGE()
		{
			return mUsage;
		}

		/** @brief  */
		inline operator DWORD()
		{
			return mD3D9Usage;
		}

	protected:
		DWORD					mD3D9Usage;
		IGraphicsBuffer::USAGE	mUsage;
	};//class D3DUsageConverter

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class D3DLockFlagConverter
	{
	public:
		D3DLockFlagConverter(IGraphicsBuffer::LOCKFLAGS flag)
			:mD3DLockFlags(0)
		{
			if( flag&IGraphicsBuffer::GBLF_DISCARD )
			{
				assert( !(flag&IGraphicsBuffer::GBLF_READONLY) );
				mD3DLockFlags |= D3DLOCK_DISCARD;
			}

			if( (flag&IGraphicsBuffer::GBLF_READONLY) && !(flag&IGraphicsBuffer::GBLF_WRITEONLY) )
			{
				mD3DLockFlags |= D3DLOCK_READONLY | D3DLOCK_NOSYSLOCK;
			}

			if( flag&IGraphicsBuffer::GBLF_NO_OVERWRITE )
			{
				mD3DLockFlags |= D3DLOCK_NOOVERWRITE;
			}

			if( flag& IGraphicsBuffer::GBLF_NO_DIRTY_UPDATE )
				mD3DLockFlags |= D3DLOCK_NO_DIRTY_UPDATE;
		}

		D3DLockFlagConverter(DWORD d3dlock)
			:mLockFlags(IGraphicsBuffer::GBLF_NORMAL)
		{
			if( d3dlock&D3DLOCK_DISCARD )
			{
				assert( !(d3dlock&D3DLOCK_READONLY) );
				mLockFlags |= IGraphicsBuffer::GBLF_DISCARDWRITE;
			}

			if( d3dlock&D3DLOCK_READONLY )
			{
				mLockFlags |= IGraphicsBuffer::GBLF_READONLY;
			}

			if( d3dlock&D3DLOCK_NOOVERWRITE )
			{
				mLockFlags |= IGraphicsBuffer::GBLF_NO_OVERWRITE;
			}

			if( d3dlock&D3DLOCK_NO_DIRTY_UPDATE)
				mLockFlags |= IGraphicsBuffer::GBLF_NO_DIRTY_UPDATE;
		}

		operator DWORD()
		{
			return mD3DLockFlags;
		}

		operator IGraphicsBuffer::LOCKFLAGS()
		{
			return mLockFlags;
		}

	protected:
		IGraphicsBuffer::LOCKFLAGS	mLockFlags;
		DWORD						mD3DLockFlags;

	};//class D3DUsageConverter


	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable:4063) //case '843666497' is not a valid value for switch of enum '_D3DFORMAT'
#endif
	class D3DFormatConverter
	{
	public:

		/** @brief  */
		inline D3DFormatConverter(D3DFORMAT d3dformat)
		{
			switch(d3dformat)
			{
			case D3DFMT_A8R8G8B8:
				mFormat = PF_ARGB;
				break;
			case D3DFMT_A8B8G8R8:
				mFormat = PF_ABGR;
				break;
			case D3DFMT_X8R8G8B8:
				mFormat = PF_XRGB;
				break;
			case D3DFMT_X8B8G8R8:
				mFormat = PF_XBGR;
				break;
			case D3DFMT_R5G6B5:
				mFormat = PF_R5G6B5;
				break;
			case D3DFMT_A1R5G5B5:
				mFormat = PF_B5G5R5A1;
				break;
			case D3DFMT_X1R5G5B5:
				mFormat = PF_B5G5R5A1;
				break;
			case D3DFMT_A4R4G4B4:
				mFormat = PF_B4G4R4A4;
				break;
			case D3DFMT_X4R4G4B4:
				mFormat = PF_B4G4R4A4;
				break;
			case D3DFMT_A2R10G10B10:
				mFormat = PF_B10G10R10A2;
				break;
			case D3DFMT_A2B10G10R10:
				assert(false && "unspported");
				mFormat = PF_UNDEFINED;
				break;
			case D3DFMT_D16_LOCKABLE:
			case D3DFMT_D16:
			case MAKEFOURCC('D','F','1','6'):
				mFormat = PF_DEPTH16;
				break;
			case D3DFMT_D15S1:
				mFormat = (EPixelFormat)(PF_FLAG_16BIT | PF_FLAG_DEPTH);
				break;
			case D3DFMT_D24S8:
				mFormat = PF_DEPTH24S8;
				break;
			case D3DFMT_D24X8:
			case D3DFMT_D24X4S4:
			case MAKEFOURCC('I', 'N', 'T', 'Z'):
				mFormat = PF_DEPTH24S8;
				break;
			case D3DFMT_D32:
			case D3DFMT_D32_LOCKABLE:
				mFormat = PF_DEPTH32;
				break;
			case D3DFMT_D32F_LOCKABLE:
				mFormat = PF_DEPTH32F;
				break;
			case D3DFMT_D24FS8:
				assert(false && "unsupported");
				mFormat = PF_UNDEFINED;
				break;
			case D3DFMT_R16F:
				mFormat = PF_R16F;
				break;
			case D3DFMT_G16R16F:
				mFormat = PF_R16G16F;
				break;
			case D3DFMT_A16B16G16R16F:
				mFormat = PF_R16G16B16A16F;
				break;
			case D3DFMT_R32F:
				mFormat = PF_R32F;
				break;
			case D3DFMT_G32R32F:
				mFormat = PF_R32G32F;
				break;
			case D3DFMT_A32B32G32R32F:
				mFormat = PF_R32G32B32A32F;
				break;
			case D3DFMT_A8:
				mFormat = PF_A8;
				break;
			case D3DFMT_L8:
				mFormat = PF_R8;
				break;
			case D3DFMT_A8L8:
				mFormat = PF_R8G8;
				break;
			case D3DFMT_L16:
				mFormat = PF_R16;
				break;
			case D3DFMT_DXT1:
				mFormat = PF_DXT1;
				break;
			case D3DFMT_DXT2:
				mFormat = PF_DXT2;
				break;
			case D3DFMT_DXT3:
				mFormat = PF_DXT3;
				break;
			case D3DFMT_DXT4:
				mFormat = PF_DXT4;
				break;
			case D3DFMT_DXT5:
				mFormat = PF_DXT5;
				break;
			case (D3DFORMAT)MAKEFOURCC('A','T','I','1'):
				mFormat = PF_BC4;
				break;
			case (D3DFORMAT)MAKEFOURCC('A','T','I','2'):
				mFormat = PF_BC5;
				break;
			case D3DFMT_INDEX16:
				mIndexType = IIndexBuffer::IT_16BIT;
				break;
			case D3DFMT_INDEX32:
				mIndexType = IIndexBuffer::IT_32BIT;
				break;
			case (D3DFORMAT)MAKEFOURCC('N','U','L','L'):
				mFormat = D3DFormatConverter(msBackBufferFormat);
				break;
			default:
				assert(false);
				mFormat = PF_UNDEFINED;
			}
		}

		/** @brief  */
		inline D3DFormatConverter(PixelFormat format)
		{
			switch(format)
			{
			case PF_B8G8R8A8:
				mD3DFormat = D3DFMT_A8R8G8B8;
				break;
			case PF_R8G8B8A8:
				mD3DFormat = D3DFMT_A8B8G8R8;
				break;
			case PF_XRGB:
				mD3DFormat = D3DFMT_X8R8G8B8;
				break;
			case PF_XBGR:
				mD3DFormat = D3DFMT_X8B8G8R8;
				break;
			case PF_RGB:
				mD3DFormat = D3DFMT_R8G8B8;
				break;
			case PF_R5G6B5:
				mD3DFormat = D3DFMT_R5G6B5;
				break;
			case PF_B5G5R5A1:
				mD3DFormat = D3DFMT_A1R5G5B5;
				break;
			case PF_B4G4R4A4:
				mD3DFormat = D3DFMT_A4R4G4B4;
				break;
			case PF_B10G10R10A2:
				mD3DFormat = D3DFMT_A2B10G10R10;
				break;	
			case PF_DEPTH16:
				mD3DFormat = D3DFMT_D16;
				break;
			case PF_DEPTH24S8:
				mD3DFormat = D3DFMT_D24S8;
				break;
			case PF_DEPTH32:
				mD3DFormat = D3DFMT_D32;
				break;
			case PF_DEPTH32F:
				mD3DFormat = D3DFMT_D32F_LOCKABLE;
				break;
			case PF_R16F:
				mD3DFormat = D3DFMT_R16F;
				break;
			case PF_R16G16F:
				mD3DFormat = D3DFMT_G16R16F;
				break;
			case PF_R16G16B16A16F:
				mD3DFormat = D3DFMT_A16B16G16R16F;
				break;
			case PF_R32F:
				mD3DFormat = D3DFMT_R32F;
				break;
			case PF_R32G32F:
				mD3DFormat = D3DFMT_G32R32F;
				break;
			case PF_R32G32B32A32F:
				mD3DFormat = D3DFMT_A32B32G32R32F;
				break;
			case PF_A8:
				mD3DFormat = D3DFMT_A8;
				break;
			case PF_R8:
				mD3DFormat = D3DFMT_L8;
				break;
			case PF_R8G8:
				mD3DFormat = D3DFMT_A8L8;
				break;
			case PF_R16:
				mD3DFormat = D3DFMT_L16;
				break;
			case PF_DXT1:
				mD3DFormat = D3DFMT_DXT1;
				break;
			case PF_DXT2:
				mD3DFormat = D3DFMT_DXT2;
				break;
			case PF_DXT3:
				mD3DFormat = D3DFMT_DXT3;
				break;
			case PF_DXT4:
				mD3DFormat = D3DFMT_DXT4;
				break;
			case PF_DXT5:
				mD3DFormat = D3DFMT_DXT5;
				break;
			case PF_BC4:
				mD3DFormat = (D3DFORMAT)MAKEFOURCC('A','T','I','1');
				break;
			case PF_BC5:
				mD3DFormat = (D3DFORMAT)MAKEFOURCC('A','T','I','2');
				break;
			default:
				assert(false);
				mD3DFormat = D3DFMT_UNKNOWN;
			}

		}

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(pop)
#endif

		inline D3DFormatConverter(IIndexBuffer::EIndexType type)
		{
			mD3DFormat = type == IIndexBuffer::IT_16BIT?D3DFMT_INDEX16:D3DFMT_INDEX32;
		}

		/** @brief  */
		inline operator D3DFORMAT() const
		{
			return mD3DFormat;
		}

		/** @brief  */
		inline operator PixelFormat() const
		{
			return mFormat;
		}

		/** @brief  */
		inline operator IIndexBuffer::EIndexType() const
		{
			return mIndexType;
		}

		/** @brief  */
		static inline void	setBackBuferFormat(D3DFORMAT fmt)
		{
			msBackBufferFormat = fmt;
		}

		static D3DFORMAT	getBackBufferFormat()
		{
			return msBackBufferFormat;
		}

	protected:
		static D3DFORMAT	msBackBufferFormat;


		PixelFormat	mFormat;
		D3DFORMAT	mD3DFormat;
		IIndexBuffer::EIndexType	mIndexType;
	};//class D3DFormatConverter


	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//well uses array map need that the order of enum must not change,not quite flexible.
	class D3DVertexDeclarationTypeConverter
	{
	public:
		inline D3DVertexDeclarationTypeConverter(EVertexElementType type)
			:mElementType(type)
		{
			assert( type >= VET_BEGIN && type < VET_COUNT );
			static const D3DDECLTYPE typemap[VET_COUNT] =
			{
				D3DDECLTYPE_FLOAT1,
				D3DDECLTYPE_FLOAT2,
				D3DDECLTYPE_FLOAT3,
				D3DDECLTYPE_FLOAT4,
				D3DDECLTYPE_D3DCOLOR,
				D3DDECLTYPE_SHORT2,
				D3DDECLTYPE_SHORT4,
				D3DDECLTYPE_UBYTE4,
				D3DDECLTYPE_UBYTE4N,
				D3DDECLTYPE_SHORT2N,
				D3DDECLTYPE_SHORT4N,
				D3DDECLTYPE_USHORT2N,
				D3DDECLTYPE_USHORT4N,
				D3DDECLTYPE_UDEC3,
				D3DDECLTYPE_DEC3N,
				D3DDECLTYPE_FLOAT16_2,
				D3DDECLTYPE_FLOAT16_4,
			};
			mD3DDeclType = typemap[type];
		}

		inline D3DVertexDeclarationTypeConverter(D3DDECLTYPE d3ddecl_type)
			:mD3DDeclType(d3ddecl_type)
		{
			static const EVertexElementType typemap[] = 
			{
				VET_FLOAT1,
				VET_FLOAT2,
				VET_FLOAT3,
				VET_FLOAT4,
				VET_COLOR,
				VET_UBYTE4,
				VET_SHORT2,
				VET_SHORT4,
				VET_UBYTE4N,
				VET_SHORT2N,
				VET_SHORT4N,
				VET_USHORT2N,
				VET_USHORT4N,
				VET_UDEC3,	//D3DDECLTYPE_UDEC3 for dx9, DXGI_FORMAT_R10G10B10A2_UINT for dx11, GL_UNSIGNED_INT_2_10_10_10_REV for GLES
				VET_DEC3N,
				VET_HALF2,	//float16x2
				VET_HALF4,	//float16x4
			};

			//undefined types
			if( d3ddecl_type >= VET_COUNT )
			{
				assert(false);
				mElementType =  VET_FLOAT3;
			}
			else
				mElementType = typemap[d3ddecl_type];
		}

		inline operator EVertexElementType()
		{
			return mElementType;
		}

		inline operator D3DDECLTYPE()
		{
			return mD3DDeclType;
		}

		EVertexElementType	mElementType;
		D3DDECLTYPE			mD3DDeclType;
	};//class D3DVertexDeclarationTypeConverter


	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class D3DVertexDeclarationUsageConverter
	{
	public:
		inline D3DVertexDeclarationUsageConverter(EVertexUsage usage)
			:mElementUsage(usage)
		{
			assert( usage >= VU_POSITION && usage <= VU_TANGENT );

			static const D3DDECLUSAGE usagemap[VU_COUNT]=
			{
				D3DDECLUSAGE_POSITION,
				D3DDECLUSAGE_BLENDWEIGHT,
				D3DDECLUSAGE_BLENDINDICES,
				D3DDECLUSAGE_NORMAL,
				D3DDECLUSAGE_COLOR,
				D3DDECLUSAGE_TEXCOORD,
				D3DDECLUSAGE_BINORMAL,
				D3DDECLUSAGE_TANGENT,
			};
			mD3DDeclUsage = usagemap[usage];
		}

		inline D3DVertexDeclarationUsageConverter(D3DDECLUSAGE d3dusage)
			:mD3DDeclUsage(d3dusage)
		{
			static const EVertexUsage usagemap[]=
			{
				VU_POSITION,
				VU_BLEND_WEIGHTS,
				VU_BLEND_INDICES,
				VU_NORMAL,
				VU_UNDEFINED,	//D3DDECLUSAGE_PSIZE
				VU_TEXTURE_COORDINATES,
				VU_TANGENT,
				VU_BINORMAL,
				VU_UNDEFINED,	//D3DDECLUSAGE_TESSFACTOR
				VU_UNDEFINED,	//D3DDECLUSAGE_POSITIONT
				VU_COLOR,
				VU_UNDEFINED,	//D3DDECLUSAGE_FOG
				VU_UNDEFINED,	//D3DDECLUSAGE_DEPTH
				VU_UNDEFINED,	//D3DDECLUSAGE_SAMPLE
			};

			if( d3dusage > D3DDECLUSAGE_SAMPLE ||d3dusage < D3DDECLUSAGE_POSITION)
				mElementUsage = VU_UNDEFINED;
			else
				mElementUsage = usagemap[d3dusage];
		}

		inline operator EVertexUsage()
		{
			return mElementUsage;
		}

		inline operator D3DDECLUSAGE()
		{
			return mD3DDeclUsage;
		}

		EVertexUsage	mElementUsage;
		D3DDECLUSAGE	mD3DDeclUsage;
	};//class D3DVertexDeclarationUsageConverter
	
}//namespace Blade


#endif //__Blade_D3D9TypeConverter_h__

#endif//BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS