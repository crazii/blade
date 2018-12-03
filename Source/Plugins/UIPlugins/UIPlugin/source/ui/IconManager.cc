/********************************************************************
	created:	2010/05/23
	filename: 	IconManager.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include "stdafx.h"
#include "ui/IconManager.h"

namespace Blade
{
	const int GROW_CON_COUNT = 128;

	//////////////////////////////////////////////////////////////////////////
	IconManager::IconManager()
	{
		AFX_MANAGE_STATE(::AfxGetStaticModuleState());

		//init size list first
		mSizeList.resize(IS_COUNT);
		for(size_t i = 0; i < IS_COUNT; ++i )
			mSizeList[i] = (i+1)*8;

		mSizeIconList.resize(IS_COUNT);
		mGrayedIconList.resize(IS_COUNT);

		for(size_t i = 0; i < IS_COUNT; ++i )
		{
			size_t size = this->getIconSize( ICONSIZE(i) );
			//normal
			mSizeIconList[i] = BLADE_NEW UIImageList();
			mSizeIconList[i]->Create((int)size, (int)size, ILC_COLOR32|ILC_MASK, 0, GROW_CON_COUNT);

			//grayed
			mGrayedIconList[i] = BLADE_NEW UIImageList();
			mGrayedIconList[i]->Create((int)size, (int)size, ILC_COLOR32|ILC_MASK, 0, GROW_CON_COUNT);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	IconManager::~IconManager()
	{
	}

	/************************************************************************/
	/* IIconManager interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void		IconManager::initialize(const TString& path)
	{
		mIconPath = path;
	}

	//////////////////////////////////////////////////////////////////////////
	IconIndex	IconManager::loadSingleIconImage(const TString& image)
	{
		AFX_MANAGE_STATE(::AfxGetStaticModuleState());

		HSTREAM stream;
#if ENABLE_THEME
		const tchar* theme = ThemeManager::getSingleton().getTheme().mName;
		if(!mIconPath.empty())
			stream = IResourceManager::getSingleton().loadStream(mIconPath + theme + TEXT('/') + image );
#endif
		if(stream == NULL )
			stream = IResourceManager::getSingleton().loadStream(mIconPath + image);
		if(stream == NULL)
			stream = IResourceManager::getSingleton().loadStream(image);
		if(stream == NULL)
		{
			assert(false);
			return INVALID_ICON_INDEX;
		}

		HIMAGE img = IImageManager::getSingleton().loadImage(stream, IP_TEMPORARY, PF_ARGB, 1, IMGO_BOTTOM_UP);

		if( img != NULL )
		{
			for(size_t i = 0; i < IS_COUNT; ++i )
			{
				int iconSize = (int)this->getIconSize( (ICONSIZE)i );

				HIMAGE iconImg = IImageManager::getSingleton().scaleImage(img, SIZE2I(iconSize,iconSize), IP_TEMPORARY );
				HBITMAP hImage= this->createBitmap(iconImg);
				HBITMAP hGrayed = this->createGrayedIcon(iconImg);
				assert( hGrayed != NULL );
				::ImageList_Add( *mSizeIconList[i], hImage, NULL );
				::ImageList_Add( *mGrayedIconList[i], hGrayed, NULL );
				::DeleteObject(hImage);
				::DeleteObject(hGrayed);
			}
			return (IconIndex)mSizeIconList[IS_16]->GetImageCount()-1u;
		}
		else
			return INVALID_ICON_INDEX;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		IconManager::loadIconsImage(const TString& /*image*/, size_t /*iconSize*/, size_t /*validCount*/)
	{
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	IconIndex	IconManager::loadIconForFileType(const TString& extension)
	{
		//http://stackoverflow.com/questions/524137/get-icons-for-common-file-types
		ExtensionMap::iterator iter = mExtIcon.find(extension);
		if( iter != mExtIcon.end() )
			return iter->second;

		SHFILEINFO shfi;
		TString file = BTString("*.") + extension;
		//TODO: from MSDN: You should call this function from a background thread. Failure to do so could cause the UI to stop responding.
		DWORD_PTR ret = ::SHGetFileInfo(file.c_str(), FILE_ATTRIBUTE_NORMAL, &shfi, sizeof(SHFILEINFO), SHGFI_USEFILEATTRIBUTES | SHGFI_ICON | SHGFI_SHELLICONSIZE | SHGFI_SMALLICON);

		if( !ret )
			return INVALID_ICON_INDEX;

		HDC hDC = ::GetDC(NULL);
		HDC hMemDC = ::CreateCompatibleDC(hDC);

		for(size_t i = 0; i < IS_COUNT; ++i )
		{
			int iconSize = (int)this->getIconSize( (ICONSIZE)i );

			HBITMAP hMemBmp = ::CreateCompatibleBitmap(hDC, iconSize, iconSize);
			HGDIOBJ hOrgBMP = ::SelectObject(hMemDC, hMemBmp);

			::DrawIconEx(hMemDC, 0, 0, shfi.hIcon, iconSize, iconSize, 0, NULL, DI_NORMAL);
			::SelectObject(hMemDC, hOrgBMP);

			::ImageList_Add( *mSizeIconList[i], hMemBmp, NULL );
			//TODO: gray icon
			::ImageList_Add( *mGrayedIconList[i], hMemBmp, NULL );

			::DeleteObject(hMemBmp);
		}
		::DeleteDC(hMemDC);
		::ReleaseDC(NULL, hDC);

		::DestroyIcon(shfi.hIcon);

		IconIndex index = (IconIndex)mSizeIconList[IS_16]->GetImageCount()-1u;
		mExtIcon[extension] = index;
		return index;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t		IconManager::getIconCount() const
	{
		return (size_t)(*mSizeIconList.begin() )->GetImageCount();
	}

	//////////////////////////////////////////////////////////////////////////
	size_t		IconManager::getIconSize(ICONSIZE is) const
	{
		if( is >= IS_COUNT )
		{
			assert(false);
			return 0;
		}
		return mSizeList[is];
	}

	/************************************************************************/
	/* internal custom method                                                                     */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	CImageList*			IconManager::getIconList(ICONSIZE iconSize, bool grayed/* = false*/)
	{
		if( iconSize >= IS_COUNT )
		{
			assert(false);
			return NULL;
		}
		else
		{
			if( grayed )
				return mGrayedIconList[iconSize];
			else
				return mSizeIconList[iconSize];
		}
	}

	//////////////////////////////////////////////////////////////////////////
	HBITMAP				IconManager::createBitmap(const HIMAGE& img)
	{
		HIMAGE bitmapImg = img;
		if( img->isCompressed() )
			bitmapImg = IImageManager::getSingleton().convertToFormat(img, PF_ARGB, IP_TEMPORARY, 1);

		assert( (bitmapImg->getBPP() == 32 || bitmapImg->getBPP() == 24  || bitmapImg->getBPP() == 8) && !bitmapImg->isCompressed() );

		HDC hdc = ::GetDC(NULL);

		BITMAPINFO info;
		::memset(&info, 0 , sizeof(info));
		BITMAPINFOHEADER& header = info.bmiHeader;
		header.biSize = sizeof(header);
		header.biWidth = (LONG)bitmapImg->getWidth();
		header.biHeight = bitmapImg->getOrientation() == IMGO_BOTTOM_UP ? (LONG)bitmapImg->getHeight() : -(LONG)bitmapImg->getHeight();
		header.biPlanes = 1;
		header.biBitCount = bitmapImg->getBPP();
		header.biCompression = BI_RGB;
		header.biSizeImage = 0;
		header.biXPelsPerMeter = 2835;	//72 DPI
		header.biYPelsPerMeter = 2835;	//72 DPI
		header.biClrUsed = 0;
		header.biClrImportant = 0;
		HBITMAP hBitmap;
		//handle 8 bit bitmap ( gray scale only: no other indexed palette supported)
		if( bitmapImg->getBPP() == 8 )
		{
			static ::RGBQUAD grays[256];
			static bool inited = false;
			if(!inited)
			{
				for(int i = 0; i < 256; i++)
				{
					grays[i].rgbBlue = (BYTE)i;
					grays[i].rgbGreen = (BYTE)i;
					grays[i].rgbRed = (BYTE)i;
					grays[i].rgbReserved = 0;
				}
				inited = true;
			}
			TempBuffer buffer;
			buffer.reserve( sizeof(info)+sizeof(grays));
			BITMAPINFO* pInfo = (BITMAPINFO*)buffer.getData();
			header.biSizeImage = sizeof(grays);
			::memcpy(pInfo, &info, sizeof(info));
			::memcpy(pInfo->bmiColors, &grays, sizeof(grays) );
			hBitmap = ::CreateDIBitmap(hdc, &(pInfo->bmiHeader), CBM_INIT, pInfo->bmiColors, pInfo, DIB_RGB_COLORS);
		}
		else
			hBitmap = ::CreateDIBitmap(hdc, &header, CBM_INIT, bitmapImg->getBits(), &info, DIB_RGB_COLORS);
		::ReleaseDC(NULL, hdc);
		return hBitmap;
	}

	//////////////////////////////////////////////////////////////////////////
	void				IconManager::shutdown()
	{
		AFX_MANAGE_STATE(::AfxGetStaticModuleState());

		for(size_t i = 0; i < IS_COUNT; ++i)
		{
			BLADE_DELETE mSizeIconList[i];
			BLADE_DELETE mGrayedIconList[i];
		}
		mSizeIconList.clear();
		mGrayedIconList.clear();
	}

	//////////////////////////////////////////////////////////////////////////
	HBITMAP				IconManager::createGrayedIcon(const HIMAGE& img)
	{
		//we need a none-indexed 32 BPP image
		if( img->getBPP() != 32 )
		{
			assert(false);
			return NULL;
		}

		HIMAGE grayImg = IImageManager::getSingleton().grayscaleImage(img, IP_TEMPORARY);
		HBITMAP ret = this->createBitmap(grayImg);

		//now we get a gray scale 32BPP bit map, just increase the transparency a little bit by alpha blending

		uint32 width = img->getWidth();
		uint32 height = img->getHeight();

		BITMAPV5HEADER bi;
		ZeroMemory(&bi,sizeof(BITMAPV5HEADER));
		bi.bV5Size           = sizeof(BITMAPV5HEADER);
		bi.bV5Width          = (LONG)width;
		bi.bV5Height         = (LONG)height;
		bi.bV5Planes = 1;
		bi.bV5BitCount = 32;
		bi.bV5Compression = BI_BITFIELDS;
		bi.bV5RedMask   =  0x00FF0000;
		bi.bV5GreenMask =  0x0000FF00;
		bi.bV5BlueMask  =  0x000000FF;
		bi.bV5AlphaMask =  0xFF000000; 

		HDC hdc = ::GetDC(NULL);
		void* lpBits;
		HBITMAP hAlphaBitmap = ::CreateDIBSection(hdc, (::BITMAPINFO *)&bi, DIB_RGB_COLORS, &lpBits, NULL, 0 );
		HDC hAlphaDC = ::CreateCompatibleDC(hdc);
		HDC hGrayDC = ::CreateCompatibleDC(hdc);

		HBITMAP hOldAlphaBitmap = (HBITMAP)::SelectObject(hAlphaDC, hAlphaBitmap);
		HBITMAP hOldGrayBitmap = (HBITMAP)::SelectObject(hGrayDC,ret);

		BLENDFUNCTION bf;
		bf.BlendOp = AC_SRC_OVER;
		bf.BlendFlags = 0;
		bf.SourceConstantAlpha = 90;
		bf.AlphaFormat = AC_SRC_ALPHA;

		CRect rect(0, 0, (int)width, (int)height);
		::FillRect(hAlphaDC, &rect, (HBRUSH)::GetStockObject(GRAY_BRUSH) );
		::AlphaBlend(hAlphaDC, 0, 0, (int)width, (int)height, hGrayDC, 0, 0, (int)width, (int)height, bf);
		::BitBlt(hGrayDC, 0, 0, (int)width, (int)height, hAlphaDC, 0, 0, SRCCOPY);	//write back

		::SelectObject(hAlphaDC, hOldAlphaBitmap);
		::DeleteDC(hAlphaDC);
		::SelectObject(hGrayDC, hOldGrayBitmap);
		::DeleteDC(hGrayDC);
		::DeleteObject(hAlphaBitmap);
		::ReleaseDC(NULL,hdc);
		return ret;
	}


}//namespace Blade