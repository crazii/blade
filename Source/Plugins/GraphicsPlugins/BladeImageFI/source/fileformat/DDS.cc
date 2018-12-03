/********************************************************************
	created:	2012/04/26
	filename: 	DDS.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "DDS.h"
#include <interface/public/ISerializer.h>

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(disable:4127)
#endif

namespace Blade
{
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))

	namespace DDSUtil
	{
		//////////////////////////////////////////////////////////////////////////
		bool loadTextureData(IOBuffer& buffer, DDS_HEADER* pHeader, size_t* faceCount, const DDS_HEADER_DXT10** ppDX10Header, uint8** ppBitData, size_t* pBitSize)
		{
			if( buffer.getData() == NULL || pHeader == NULL || faceCount == NULL || ppBitData == NULL || pBitSize == NULL )
			{
				assert(false);
				return false;
			}

			// DDS files always start with the same magic number ("DDS ")
			DWORD dwMagicNumber = 0;
			buffer.read(&dwMagicNumber);
			if( dwMagicNumber != DDS_MAGIC )
			{
				assert(false);
				return false;
			}

			buffer.read(pHeader);

			// Verify header to validate DDS file
			if( pHeader->dwSize != sizeof(DDS_HEADER)
				|| pHeader->ddspf.dwSize != sizeof(DDS_PIXELFORMAT) )
			{
				assert(false);
				return false;
			}

			// Check for DX10 extension
			bool bDXT10Header = false;
			if ( (pHeader->ddspf.dwFlags & DDS_FOURCC)
				&& (MAKEFOURCC( 'D', 'X', '1', '0' ) == pHeader->ddspf.dwFourCC) )
			{
				// Must be long enough for both headers and magic value
				if( buffer.getSize() < (sizeof(DDS_HEADER)+sizeof(DWORD)+sizeof(DDS_HEADER_DXT10)) )
				{
					assert(false);
					return false;
				}
				bDXT10Header = true;
			}

			if( bDXT10Header )
			{
				const DDS_HEADER_DXT10* pDX10Header;
				buffer.readPtr(pDX10Header);
				*ppDX10Header = pDX10Header;
			}
			else
				*ppDX10Header = NULL;

			//set valid value for depth / cube face / mipmap
			if( pHeader->dwMipMapCount == 0 )
				pHeader->dwMipMapCount = 1;

			if ( (pHeader->dwFlags & DDS_HEADER_FLAGS_VOLUME) )
				assert( pHeader->dwDepth >= 1);
			else
				pHeader->dwDepth = 1;

			if ( pHeader->dwCaps2 & DDS_CUBEMAP )
			{
				//we don't support dx9's incomplete cube
				assert( (pHeader->dwCaps2 & DDS_CUBEMAP_ALLFACES) );
				*faceCount = 6;
			}
			else
				*faceCount = 1;

			// setup the pointers in the process request
			size_t offset = sizeof( DWORD ) + sizeof( DDS_HEADER )
				+ (bDXT10Header ? sizeof( DDS_HEADER_DXT10 ) : 0);
			*ppBitData = (BYTE*)buffer.getData() + offset;
			*pBitSize = buffer.getSize() - offset;

			return true;
		}

		//////////////////////////////////////////////////////////////////////////
		bool		saveTexture(IOBuffer& buffer, const DDS_HEADER& header, const DDS_HEADER_DXT10* pDX10Header, const uint8* bitData, size_t bitSize)
		{
			if( header.dwSize != sizeof(header) || header.ddspf.dwSize != sizeof(header.ddspf) )
			{
				assert(false);
				return false;
			}
			bool hasDX10Header = (header.ddspf.dwFlags & DDS_FOURCC) != 0 && header.ddspf.dwFourCC == MAKEFOURCC('D','X','1','0');
			if( hasDX10Header && pDX10Header == NULL )
			{
				assert(false);
				return false;
			}
			assert( bitData != NULL );

			DWORD magic = DDS_MAGIC;
			buffer.write(&magic);

			buffer.write(&header);

			if( hasDX10Header )
				buffer.write(pDX10Header);

			buffer.writeData(bitData, (IStream::Size)bitSize);
			return true;
		}

		//--------------------------------------------------------------------------------------
#define ISBITMASK( r,g,b,a ) ( ddpf.dwRBitMask == r && ddpf.dwGBitMask == g && ddpf.dwBBitMask == b && ddpf.dwABitMask == a )
#define SETBITMASK(r,g,b,a)  do{ddpf.dwRBitMask = r; ddpf.dwGBitMask = g; ddpf.dwBBitMask = b; ddpf.dwABitMask = a; }while(0)

		//--------------------------------------------------------------------------------------
		//////////////////////////////////////////////////////////////////////////
		PixelFormat		getPixelFormat(const DDS_PIXELFORMAT& ddpf, const DDS_HEADER_DXT10* pDX10Header)
		{
			//DX10 header
			if( pDX10Header != NULL )
			{
				//only parse BC format for simplicity
				PixelFormat format = PF_UNDEFINED;
				switch(pDX10Header->dxgiFormat)
				{
				case DXGI_FORMAT_BC1_UNORM:
					format = PF_BC1;
					break;
				case DXGI_FORMAT_BC2_UNORM:
					format = PF_BC2;
					break;
				case DXGI_FORMAT_BC3_UNORM:
					format = PF_BC3;
					break;
				case DXGI_FORMAT_BC4_UNORM:
					format = PF_BC4;
					break;
				case DXGI_FORMAT_BC5_UNORM:
					format = PF_BC5;
					break;
				case DXGI_FORMAT_R32G32_FLOAT:
					format = PF_R32G32F;
					break;
				case DXGI_FORMAT_R32G32B32A32_FLOAT:
					format = PF_R32G32B32A32F;
					break;
				default:
					break;
				}
				if( format != PF_UNDEFINED )
					return format;
			}

			//DX9 header
			if( ddpf.dwFlags & DDS_RGB )
			{
				switch (ddpf.dwRGBBitCount)
				{
				case 32:
					if( ISBITMASK(0x00ff0000,0x0000ff00,0x000000ff,0xff000000) )
						return PF_ARGB;
					if( ISBITMASK(0x00ff0000,0x0000ff00,0x000000ff,0x00000000) )
						return PF_XRGB;
					if( ISBITMASK(0x000000ff,0x0000ff00,0x00ff0000,0xff000000) )
						return PF_ABGR;
					if( ISBITMASK(0x000000ff,0x0000ff00,0x00ff0000,0x00000000) )
						return PF_XBGR;

					// Note that many common DDS reader/writers (including D3DX) swap the
					// the RED/BLUE masks for 10:10:10:2 formats. We assumme
					// below that the 'incorrect' header mask is being used

					// For 'correct' writers this should be 0x3ff00000,0x000ffc00,0x000003ff for BGR data
					if( ISBITMASK(0x000003ff,0x000ffc00,0x3ff00000,0xc0000000) )
						return PF_B10G10R10A2;

					//// For 'correct' writers this should be 0x000003ff,0x000ffc00,0x3ff00000 for RGB data
					//if( ISBITMASK(0x3ff00000,0x000ffc00,0x000003ff,0xc0000000) )
					//	return PF_A2B10G10R10;

					if( ISBITMASK(0x0000ffff,0xffff0000,0x00000000,0x00000000) )
						return PF_R16G16F;
					if( ISBITMASK(0xffffffff,0x00000000,0x00000000,0x00000000) )
						return PF_R32F; // D3DX writes this out as a FourCC of 114
					break;

				case 24:
					if( ISBITMASK(0x00ff0000,0x0000ff00,0x000000ff,0x00000000) )
						return PF_R8G8B8;
					break;

				case 16:
					if( ISBITMASK(0x0000f800,0x000007e0,0x0000001f,0x00000000) )
						return PF_R5G6B5;
					if( ISBITMASK(0x00007c00,0x000003e0,0x0000001f,0x00008000) )
						return PF_B5G5R5A1;
					//if( ISBITMASK(0x00007c00,0x000003e0,0x0000001f,0x00000000) )
					//	return PF_B5G5R5X1;
					if( ISBITMASK(0x00000f00,0x000000f0,0x0000000f,0x0000f000) )
						return PF_B4G4R4A4;
					//if( ISBITMASK(0x00000f00,0x000000f0,0x0000000f,0x00000000) )
					//	return PF_B4G4R4A4;
					// 3:3:2, 3:3:2:8, and paletted texture formats are typically not supported on modern video cards
					break;
				}
			}
			else if( ddpf.dwFlags & DDS_LUMINANCE )
			{
				if( 8 == ddpf.dwRGBBitCount )
				{
					//remove A4L4 for DX11 compatibility
					//if( ISBITMASK(0x0000000f,0x00000000,0x00000000,0x000000f0) )
					//	return PF_A4L4;
					if( ISBITMASK(0x000000ff,0x00000000,0x00000000,0x00000000) )
						return PF_R8;
				}

				if( 16 == ddpf.dwRGBBitCount )
				{
					if( ISBITMASK(0x0000ffff,0x00000000,0x00000000,0x00000000) )
						return PF_R16;
					if( ISBITMASK(0x000000ff,0x00000000,0x00000000,0x0000ff00) )
						return PF_R8G8;
				}
			}
			else if( ddpf.dwFlags & DDS_ALPHA )
			{
				if( 8 == ddpf.dwRGBBitCount )
				{
					return PF_A8;
				}
			}
			else if( ddpf.dwFlags & DDS_FOURCC )
			{
				if( D3DFMT_DXT1 == ddpf.dwFourCC )
					return PF_DXT1;
				if( D3DFMT_DXT2 == ddpf.dwFourCC )
					return PF_DXT2;
				if( D3DFMT_DXT3 == ddpf.dwFourCC )
					return PF_DXT3;
				if( D3DFMT_DXT4 == ddpf.dwFourCC )
					return PF_DXT4;
				if( MAKEFOURCC( 'D', 'X', 'T', '5' ) == ddpf.dwFourCC )
					return PF_DXT5;

				if( MAKEFOURCC( 'A', 'T', 'I', '1' ) == ddpf.dwFourCC 
					|| MAKEFOURCC( 'B', 'C', '4', 'U' ) == ddpf.dwFourCC )
					return PF_BC4;

				if( MAKEFOURCC( 'A', 'T', 'I', '2' ) == ddpf.dwFourCC 
					|| MAKEFOURCC( 'B', 'C', '5', 'U' ) == ddpf.dwFourCC )
					return PF_BC5;

				// Check for D3DFORMAT enums being set here
				switch( ddpf.dwFourCC )
				{
				case D3DFMT_A16B16G16R16:
					//return 
					//case D3DFMT_Q16W16V16U16:
				case D3DFMT_R16F:
					return PF_R16F;
				case D3DFMT_G16R16F:
					return PF_R16G16F;
				case D3DFMT_A16B16G16R16F:
					return PF_R16G16B16A16F;
				case D3DFMT_R32F:
					return PF_R32F;
				case D3DFMT_G32R32F:
					return PF_R32G32F;
				case D3DFMT_A32B32G32R32F:
					return PF_R32G32B32A32F;
					//case D3DFMT_CxV8U8:
					//return (D3DFORMAT)ddpf.dwFourCC;
				default:
					return PF_UNDEFINED;
				}
			}

			return PF_UNDEFINED;
		}

		//////////////////////////////////////////////////////////////////////////
		DDS_PIXELFORMAT	getDDSFormat(PixelFormat format)
		{
			//DX9 header
			DDS_PIXELFORMAT ddpf;
			std::memset(&ddpf, 0, sizeof(ddpf) );
			ddpf.dwSize = sizeof(ddpf);
			if( !format.isCompressed() )
				ddpf.dwRGBBitCount = (DWORD)format.getSizeBytes()*8;

			if (!format.isFloat() && !format.isCompressed() )
			{
				if (((EPixelFormat)format& (PF_FLAG_RG | PF_FLAG_RGB | PF_FLAG_RGBX | PF_FLAG_RGBA)))
				{
					ddpf.dwFlags |= DDS_RGB;
					if (format.hasAlpha())
						ddpf.dwFlags |= DDS_RGBA;

					switch (ddpf.dwRGBBitCount)
					{
					case 32:
						if ((format&PF_FLAG_ORDER_BGRA))	//byte order
						{
							SETBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
							if ((format&PF_FLAG_RGBX))
								SETBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000);
						}

						if ((format&PF_FLAG_ORDER_BGRA)) //byte order
						{
							SETBITMASK(0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
							if ((format&PF_FLAG_RGBX))
								ISBITMASK(0x000000ff, 0x0000ff00, 0x00ff0000, 0x00000000);
						}

						// Note that many common DDS reader/writers (including D3DX) swap the
						// the RED/BLUE masks for 10:10:10:2 formats. We assumme
						// below that the 'incorrect' header mask is being used

						// For 'correct' writers this should be 0x3ff00000,0x000ffc00,0x000003ff for BGR data
						if (format == PF_B10G10R10A2)
							SETBITMASK(0x000003ff, 0x000ffc00, 0x3ff00000, 0xc0000000);

						//// For 'correct' writers this should be 0x000003ff,0x000ffc00,0x3ff00000 for RGB data
						//if( format == PF_A2B10G10R10 )
						//	SETBITMASK(0x3ff00000,0x000ffc00,0x000003ff,0xc0000000);

						if (format == PF_R16G16F)
							SETBITMASK(0x0000ffff, 0xffff0000, 0x00000000, 0x00000000);
						if (format == PF_R32F)
							SETBITMASK(0xffffffff, 0x00000000, 0x00000000, 0x00000000); // D3DX writes this out as a FourCC of 114
						break;

					case 24:
						if (format == PF_R8G8B8)
							SETBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000);
						break;

					case 16:
						if (format == PF_R5G6B5)
							SETBITMASK(0x0000f800, 0x000007e0, 0x0000001f, 0x00000000);
						if (format == PF_B5G5R5A1)
							SETBITMASK(0x00007c00, 0x000003e0, 0x0000001f, 0x00008000);
						//if( format == PF_B5G5R5X1 )
						//	SETBITMASK(0x00007c00,0x000003e0,0x0000001f,0x00000000);
						if (format == PF_B4G4R4A4)
							SETBITMASK(0x00000f00, 0x000000f0, 0x0000000f, 0x0000f000);
						//if( format == PF_B4G4R4A4 )
						//	ISBITMASK(0x00000f00,0x000000f0,0x0000000f,0x00000000);
						// 3:3:2, 3:3:2:8, and paletted texture formats are typically not supported on modern video cards
						if (format == PF_R8G8)
							SETBITMASK(0x000000ff, 0x00000000, 0x00000000, 0x0000ff00);
						break;
					}
				}
				else if ((format&PF_FLAG_R))
				{
					ddpf.dwFlags |= DDS_LUMINANCE;
					if (8 == ddpf.dwRGBBitCount)
					{
						//remove A4L4 for DX11 compatibility
						//if( ISBITMASK(0x0000000f,0x00000000,0x00000000,0x000000f0) )
						//	return PF_A4L4;
						if (format == PF_R8)
							SETBITMASK(0x000000ff, 0x00000000, 0x00000000, 0x00000000);
					}

					if (16 == ddpf.dwRGBBitCount)
					{
						if (format == PF_R16)
							SETBITMASK(0x0000ffff, 0x00000000, 0x00000000, 0x00000000);
					}
				}
				else if ((format&PF_FLAG_A))
				{
					ddpf.dwFlags |= DDS_ALPHA;
					assert(8 == ddpf.dwRGBBitCount);
				}
			}
			else
			{
				ddpf.dwFlags |= DDS_FOURCC;
				switch(format)
				{
				case PF_DXT1:
					ddpf.dwFourCC = D3DFMT_DXT1;
					break;
				case PF_DXT2:
					ddpf.dwFourCC = D3DFMT_DXT2;
					break;
				case PF_DXT3:
					ddpf.dwFourCC = D3DFMT_DXT3;
					break;
				case PF_DXT4:
					ddpf.dwFourCC = D3DFMT_DXT4;
					break;
				case PF_DXT5:
					ddpf.dwFourCC = D3DFMT_DXT5;
					break;
				case PF_BC4:
					ddpf.dwFourCC = MAKEFOURCC( 'B', 'C', '4', 'U' );
					break;
				case PF_BC5:
					ddpf.dwFourCC = MAKEFOURCC( 'B', 'C', '5', 'U' );
					break;
				case PF_R16F:
					ddpf.dwFourCC = D3DFMT_R16F;
					break;
				case PF_R16G16F:
					ddpf.dwFourCC = D3DFMT_G16R16F;
					break;
				case PF_R16G16B16A16F:
					ddpf.dwFourCC = D3DFMT_A16B16G16R16F;
					break;
				case PF_R32F:
					ddpf.dwFourCC = D3DFMT_R32F;
					break;
				case PF_R32G32F:
					ddpf.dwFourCC = D3DFMT_G32R32F;
					break;
				case PF_R32G32B32A32F:
					ddpf.dwFourCC = D3DFMT_A32B32G32R32F;
					break;
				default:
					ddpf.dwFourCC = 0;
					ddpf.dwFlags &= ~DDS_FOURCC;
					break;
				}
			}
			return ddpf;
		}

		//////////////////////////////////////////////////////////////////////////
		DXGI_FORMAT		getDXGIFormat(PixelFormat format)
		{
			DXGI_FORMAT result = DXGI_FORMAT_UNKNOWN;
			switch(format)
			{
			case PF_BC1:
				result = DXGI_FORMAT_BC1_UNORM;
				break;
			case PF_BC2:
				result = DXGI_FORMAT_BC2_UNORM;
				break;
			case PF_BC3:
				result = DXGI_FORMAT_BC3_UNORM;
				break;
			case PF_BC4:
				result = DXGI_FORMAT_BC4_UNORM;
				break;
			case PF_BC5:
				result = DXGI_FORMAT_BC5_UNORM;
				break;
			case PF_BC6H:
				result = DXGI_FORMAT_BC6H_UF16;
				break;
			case PF_R32G32F:
				result = DXGI_FORMAT_R32G32_FLOAT;
				break;
			case PF_R32G32B32A32F:
				result = DXGI_FORMAT_R32G32B32A32_FLOAT;
				break;
			default:
				break;
			}
			return result;
		}

		//--------------------------------------------------------------------------------------
		// Get surface information for a particular format
		//--------------------------------------------------------------------------------------
		//////////////////////////////////////////////////////////////////////////
		void	getSurfaceInfo(size_t width, size_t height, PixelFormat fmt, size_t* pNumBytes, size_t* pRowBytes, size_t* pNumRows)
		{
			UINT numBytes = 0;
			UINT rowBytes = 0;
			UINT numRows = 0;

			// From the DXSDK docs:
			//
			//     When computing DXTn compressed sizes for non-square textures, the 
			//     following formula should be used at each mipmap level:
			//
			//         max(1, width ?4) x max(1, height ?4) x 8(DXT1) or 16(DXT2-5)
			//
			//     The pitch for DXTn formats is different from what was returned in 
			//     Microsoft DirectX 7.0. It now refers the pitch of a row of blocks. 
			//     For example, if you have a width of 16, then you will have a pitch 
			//     of four blocks (4*8 for DXT1, 4*16 for DXT2-5.)"

			switch( fmt )
			{
			case PF_DXT1:
			case PF_DXT2:
			case PF_DXT3:
			case PF_DXT4:
			case PF_DXT5:
			case PF_BC4:
			case PF_BC5:
			case PF_BC6H:
				{
					int numBlocksWide = 0;
					if( width > 0 )
						numBlocksWide = (int)max( 1, (width + 3) / 4 );
					int numBlocksHigh = 0;
					if( height > 0 )
						numBlocksHigh = (int)max( 1, (height + 3) / 4 );

					int numBytesPerBlock = (int)fmt.getSizeBytes();
					rowBytes = (UINT)numBlocksWide * (UINT)numBytesPerBlock;
					numRows = (UINT)numBlocksHigh;
				}
				break;
			default:
				{
					UINT bpp = (UINT)fmt.getSizeBytes()*8;
					rowBytes = (UINT)(( width * bpp + 7 ) / 8); // round up to nearest byte
					numRows = (UINT)height;
				}
				break;
			}

			numBytes = rowBytes * numRows;
			if( pNumBytes != NULL )
				*pNumBytes = numBytes;
			if( pRowBytes != NULL )
				*pRowBytes = rowBytes;
			if( pNumRows != NULL )
				*pNumRows = numRows;
		}

	}//namespace DDSUtil


}//namespace Blade