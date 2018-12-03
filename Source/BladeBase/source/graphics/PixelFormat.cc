/********************************************************************
	created:	2010/04/13
	filename: 	PixelFormat.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <interface/public/graphics/PixelFormat.h>

namespace Blade
{
	static const TString UNDEFINED = BTString("UNDEFINED");
	static TString A8 = BTString("A8");
	static TString R8 = BTString("R8");
	static TString R8G8 = BTString("R8G8");

	static TString R8G8B8 = BTString("R8G8B8");
	static TString B8G8R8 = BTString("B8G8R8");

	static TString R8G8B8A8 = BTString("R8G8B8A8");
	static TString B8G8R8A8 = BTString("B8G8R8A8");
	static TString A8B8G8R8 = BTString("A8B8G8R8");
	static TString A8R8G8B8 = BTString("A8R8G8B8");
	static TString R8G8B8X8 = BTString("X8R8G8B8");
	static TString B8G8R8X8 = BTString("X8B8G8R8");

	static TString R16 = BTString("R16");
	static TString R16G16 = BTString("R16G16");
	static TString R16G16B16A16 = BTString("R16G16B16A16");

	static TString R16F = BTString("R16F");
	static TString R16G16F = BTString("R16G16F");
	static TString R16G16B16F = BTString("R16G16B16F");
	static TString R16G16B16A16F = BTString("R16G16B16A16F");
	static TString R32F = BTString("R32F");
	static TString R32G32F = BTString("R32G32F");
	static TString R32G32B32F = BTString("R32G32B32F");
	static TString R32G32B32A32F = BTString("R32G32B32A32F");

	static TString RGB = BTString("RGB");
	static TString RGB565 = BTString("RGB565");
	static TString BGR = BTString("BGR");
	static TString BGR565 = BTString("BGR565");
	static TString ARGB = BTString("ARGB");
	static TString BGRA = BTString("BGRA");
	static TString RGBA = BTString("RGBA");
	static TString ABGR = BTString("ABGR");
	static TString XRGB = BTString("XRGB");
	static TString XBGR = BTString("XBGR");

	static TString R5G6B5 = BTString("R5G6B5");
	static TString B5G6R5 = BTString("B5G6R5");
	static TString B10G10R10A2 = BTString("B10G10R10A2");
	static TString B4G4R4A4 = BTString("B4G4R4A4");
	static TString B5G5R5A1 = BTString("B5G5R5A1");

	static TString DEPTH16 = BTString("D16");
	static TString DEPTH24S8 = BTString("D24S8");
	static TString DEPTH32 = BTString("D32");
	static TString DEPTH32F = BTString("D32F");

	static TString BC1 = BTString("BC1");
	static TString BC2 = BTString("BC2");
	static TString BC3 = BTString("BC3");
	static TString BC4 = BTString("BC4");
	static TString BC5 = BTString("BC5");
	static TString BC6H = BTString("BC6H");

	static TString DXT1 = BTString("DXT1");
	static TString DXT2 = BTString("DXT2");
	static TString DXT3 = BTString("DXT3");
	static TString DXT4 = BTString("DXT4");
	static TString DXT5 = BTString("DXT5");

	static const TString RGB_ETC2 = BTString("RGB_ETC2");
	static const TString RGBA_ETC2EAC = BTString("RGBA_ETC2EAC");
	static const TString R_EAC = BTString("R_EAC");
	static const TString RG_EAC = BTString("RG_EAC");

	typedef StaticTStringMap<EPixelFormat> StringFormatMap;
	extern StringFormatMap NAME_MAP;

	typedef StaticMap<EPixelFormat, TString> FormatStringMap;
	extern FormatStringMap FMT_MAP;

	static StaticLock sSafeLock;

	//////////////////////////////////////////////////////////////////////////
	PixelFormat::PixelFormat(const TString& fmtString)
	{
		ScopedLock lock(sSafeLock);

		if( NAME_MAP.size() == 0 )
		{
			NAME_MAP[A8] = PF_A8;
			NAME_MAP[R8] = PF_R8;
			NAME_MAP[R8G8] = PF_R8G8;

			NAME_MAP[R8G8B8] = PF_R8G8B8;
			NAME_MAP[B8G8R8] = PF_B8G8R8;

			NAME_MAP[R8G8B8A8] = PF_R8G8B8A8;
			NAME_MAP[B8G8R8A8] = PF_B8G8R8A8;
			NAME_MAP[A8B8G8R8] = PF_A8B8G8R8;
			NAME_MAP[A8R8G8B8] = PF_A8R8G8B8;
			NAME_MAP[R8G8B8X8] = PF_R8G8B8X8;
			NAME_MAP[B8G8R8X8] = PF_B8G8R8X8;

			NAME_MAP[R16] = PF_R16;
			NAME_MAP[R16G16] = PF_R16G16;
			NAME_MAP[R16G16B16A16] = PF_R16G16B16A16;

			NAME_MAP[R16F] = PF_R16F;
			NAME_MAP[R16G16F] = PF_R16G16F;
			NAME_MAP[R16G16B16F] = PF_R16G16B16F;
			NAME_MAP[R16G16B16A16F] = PF_R16G16B16A16F;
			NAME_MAP[R32F] = PF_R32F;
			NAME_MAP[R32G32F] = PF_R32G32F;
			NAME_MAP[R32G32B32F] = PF_R32G32B32F;
			NAME_MAP[R32G32B32A32F] = PF_R32G32B32A32F;
			NAME_MAP[R32G32B32A32F] = PF_R32G32B32A32F;

			NAME_MAP[RGB] = PF_RGB;
			NAME_MAP[RGB565] = PF_RGB565;
			NAME_MAP[BGR] = PF_BGR;
			NAME_MAP[BGR565] = PF_BGR565;
			NAME_MAP[ARGB] = PF_ARGB;
			NAME_MAP[BGRA] = PF_BGRA;
			NAME_MAP[RGBA] = PF_RGBA;
			NAME_MAP[ABGR] = PF_ABGR;
			NAME_MAP[XRGB] = PF_XRGB;
			NAME_MAP[XBGR] = PF_XBGR;

			NAME_MAP[R5G6B5] = PF_R5G6B5;
			NAME_MAP[B5G6R5] = PF_B5G6R5;
			NAME_MAP[B10G10R10A2] = PF_B10G10R10A2;
			NAME_MAP[B4G4R4A4] = PF_B4G4R4A4;
			NAME_MAP[B5G5R5A1] = PF_B5G5R5A1;

			NAME_MAP[DEPTH16] = PF_DEPTH16;
			NAME_MAP[DEPTH24S8] = PF_DEPTH24S8;
			NAME_MAP[DEPTH32] = PF_DEPTH32;
			NAME_MAP[DEPTH32F] = PF_DEPTH32F;

			NAME_MAP[BC1] = PF_BC1;
			NAME_MAP[BC2] = PF_BC2;
			NAME_MAP[BC3] = PF_BC3;
			NAME_MAP[BC4] = PF_BC4;
			NAME_MAP[BC5] = PF_BC5;
			NAME_MAP[BC6H] = PF_BC6H;

			NAME_MAP[DXT1] = PF_DXT1;
			NAME_MAP[DXT2] = PF_DXT2;
			NAME_MAP[DXT3] = PF_DXT3;
			NAME_MAP[DXT4] = PF_DXT4;
			NAME_MAP[DXT5] = PF_DXT5;

			NAME_MAP[RGB_ETC2] = PF_RGB_ETC2;
			NAME_MAP[RGBA_ETC2EAC] = PF_RGBA_ETC2EAC;
			NAME_MAP[R_EAC] = PF_R_EAC;
			NAME_MAP[RG_EAC] = PF_RG_EAC;
		}

		StringFormatMap::const_iterator i = NAME_MAP.find( fmtString );
		if( i == NAME_MAP.end() )
			mFormat = PF_UNDEFINED;
		else
			mFormat = i->second;
	}

	const TString&	PixelFormat::getString() const
	{
		ScopedLock lock(sSafeLock);

		if( FMT_MAP.size() == 0 )
		{
			FMT_MAP[PF_A8] = A8;
			FMT_MAP[PF_R8] = R8;
			FMT_MAP[PF_R8G8] = R8G8;

			FMT_MAP[PF_R8G8B8] = R8G8B8;
			FMT_MAP[PF_B8G8R8] = B8G8R8;

			FMT_MAP[PF_R8G8B8A8] = R8G8B8A8;
			FMT_MAP[PF_B8G8R8A8] = B8G8R8A8;
			FMT_MAP[PF_A8B8G8R8] = A8B8G8R8;
			FMT_MAP[PF_A8R8G8B8] = A8R8G8B8;
			FMT_MAP[PF_R8G8B8X8] = R8G8B8X8;
			FMT_MAP[PF_B8G8R8X8] = B8G8R8X8;

			FMT_MAP[PF_R16] = R16;
			FMT_MAP[PF_R16G16] = R16G16;
			FMT_MAP[PF_R16G16B16A16] = R16G16B16A16;

			FMT_MAP[PF_R16F] = R16F;
			FMT_MAP[PF_R16G16F] = R16G16F;
			FMT_MAP[PF_R16G16B16F] = R16G16B16F;
			FMT_MAP[PF_R16G16B16A16F] = R16G16B16A16F;
			FMT_MAP[PF_R32F] = R32F;
			FMT_MAP[PF_R32G32F] = R32G32F;
			FMT_MAP[PF_R32G32B32F] = R32G32B32F;
			FMT_MAP[PF_R32G32B32A32F] = R32G32B32A32F;
			FMT_MAP[PF_R32G32B32A32F] = R32G32B32A32F;

			FMT_MAP[PF_R5G6B5] = R5G6B5;
			FMT_MAP[PF_B5G6R5] = B5G6R5;

			FMT_MAP[PF_B10G10R10A2] = B10G10R10A2;
			FMT_MAP[PF_B4G4R4A4] = B4G4R4A4;
			FMT_MAP[PF_B5G5R5A1] = B5G5R5A1;

			FMT_MAP[PF_DEPTH16] = DEPTH16;
			FMT_MAP[PF_DEPTH24S8] = DEPTH24S8;
			FMT_MAP[PF_DEPTH32] = DEPTH32;
			FMT_MAP[PF_DEPTH32F] = DEPTH32F;

			FMT_MAP[PF_BC1] = BC1;
			FMT_MAP[PF_BC2] = BC2;
			FMT_MAP[PF_BC3] = BC3;
			FMT_MAP[PF_BC4] = BC4;
			FMT_MAP[PF_BC5] = BC5;
			FMT_MAP[PF_BC6H] = BC6H;

			FMT_MAP[PF_DXT1] = DXT1;
			FMT_MAP[PF_DXT2] = DXT2;
			FMT_MAP[PF_DXT3] = DXT3;
			FMT_MAP[PF_DXT4] = DXT4;
			FMT_MAP[PF_DXT5] = DXT5;

			FMT_MAP[PF_RGB_ETC2] = RGB_ETC2;
			FMT_MAP[PF_RGBA_ETC2EAC] = RGBA_ETC2EAC;
			FMT_MAP[PF_R_EAC] = R_EAC;
			FMT_MAP[PF_RG_EAC] = RG_EAC;
		}

		FormatStringMap::const_iterator i = FMT_MAP.find( mFormatEnum );
		if( i == FMT_MAP.end() )
			return UNDEFINED;
		else
			return i->second;
	}
	
}//namespace Blade