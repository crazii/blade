//==========================================================
// Author: Borodenko Oleg
// 20/07/2009 <oktamail@gmail.com>
//==========================================================
// 
/////////////////////////////////////////////////////////////////////////////
#pragma once
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#if _MSC_VER <= 1200		// VisualStudio 6.0 and below.
	#pragma comment (lib, "Msimg32.lib")	// for GradientFill.
#endif
#pragma warning(disable : 4365)
/////////////////////////////////////////////////////////////////////////////
// 
struct Draw
{		// 
	enum SIDE
	{	SIDE_LEFT, SIDE_TOP, SIDE_RIGHT, SIDE_BOTTOM
	};
		// 
	static void DrawHalfRoundFrame(CDC *pDC, CRect const *pRect, SIDE side, int radius, COLORREF clrBorder, COLORREF clrBack)
	{	POINT pts[6];
			// 
		switch(side)
		{	case SIDE_LEFT:
				pts[0].x = pRect->right-1; pts[0].y = pRect->top;
				pts[1].x = pRect->left+radius; pts[1].y = pRect->top;
				pts[2].x = pRect->left; pts[2].y = pRect->top+radius;
				pts[3].x = pRect->left; pts[3].y = pRect->bottom-1-radius;
				pts[4].x = pRect->left+radius; pts[4].y = pRect->bottom-1;
				pts[5].x = pRect->right-1; pts[5].y = pRect->bottom-1;
				break;
			case SIDE_TOP:
				pts[0].x = pRect->left; pts[0].y = pRect->bottom-1;
				pts[1].x = pRect->left; pts[1].y = pRect->top+radius;
				pts[2].x = pRect->left+radius; pts[2].y = pRect->top;
				pts[3].x = pRect->right-1-radius; pts[3].y = pRect->top;
				pts[4].x = pRect->right-1; pts[4].y = pRect->top+radius;
				pts[5].x = pRect->right-1; pts[5].y = pRect->bottom-1;
				break;
			case SIDE_RIGHT:
				pts[0].x = pRect->left; pts[0].y = pRect->top;
				pts[1].x = pRect->right-1-radius; pts[1].y = pRect->top;
				pts[2].x = pRect->right-1; pts[2].y = pRect->top+radius;
				pts[3].x = pRect->right-1; pts[3].y = pRect->bottom-1-radius;
				pts[4].x = pRect->right-1-radius; pts[4].y = pRect->bottom-1;
				pts[5].x = pRect->left; pts[5].y = pRect->bottom-1;
				break;
			case SIDE_BOTTOM:
				pts[0].x = pRect->left; pts[0].y = pRect->top;
				pts[1].x = pRect->left; pts[1].y = pRect->bottom-1-radius;
				pts[2].x = pRect->left+radius; pts[2].y = pRect->bottom-1;
				pts[3].x = pRect->right-1-radius; pts[3].y = pRect->bottom-1;
				pts[4].x = pRect->right-1; pts[4].y = pRect->bottom-1-radius;
				pts[5].x = pRect->right-1; pts[5].y = pRect->top;
				break;
		}
			// 
		DrawFrame(pDC,pts,sizeof(pts)/sizeof(POINT),clrBorder,clrBack);
	}
	/////////////////////////////////////////////////////////////////////////////
		// 
	static void DrawFrame(CDC *pDC, POINT const *pPoints, int iCount, COLORREF clrBorder, COLORREF clrBack)
	{	CPen pen(PS_SOLID,1,clrBack);
		CPen *pOldPen = pDC->SelectObject(&pen);
			// 
		CBrush brush(clrBack);
		CBrush *pBrushOld = pDC->SelectObject(&brush);
			// 
		pDC->Polygon((POINT *)pPoints,iCount);
			// 
		pDC->SelectObject(pBrushOld);
		pDC->SelectObject(pOldPen);
			// 
		DrawFrame(pDC,pPoints,iCount,clrBorder);
	}
		// 
	static void DrawFrame(CDC *pDC, POINT const *pPoints, int iCount, COLORREF clrLine)
	{	CPen pen(PS_SOLID,1,clrLine);
		CPen *pOldPen = pDC->SelectObject(&pen);
			// 
		pDC->MoveTo(pPoints[0].x,pPoints[0].y);
		for(int i=1; i<iCount; ++i)
			pDC->LineTo(pPoints[i].x,pPoints[i].y);
		pDC->SetPixelV(pPoints[iCount-1].x,pPoints[iCount-1].y,clrLine);	// draw last pixel.
			// 
		pDC->SelectObject(pOldPen);
	}
	/////////////////////////////////////////////////////////////////////////////
		// 
	static void DrawGradient(CDC *pDC, CRect const *pRect, bool horz, COLORREF clrTopLeft, COLORREF clrBottomRight)
	{	GRADIENT_RECT gRect = {0,1};
		TRIVERTEX vert[2] = 
		{	{pRect->left,pRect->top,(COLOR16)(GetRValue(clrTopLeft)<<8),(COLOR16)(GetGValue(clrTopLeft)<<8),(COLOR16)(GetBValue(clrTopLeft)<<8),0},
			{pRect->right,pRect->bottom,(COLOR16)(GetRValue(clrBottomRight)<<8),(COLOR16)(GetGValue(clrBottomRight)<<8),(COLOR16)(GetBValue(clrBottomRight)<<8),0}
		};
		::GradientFill(pDC->m_hDC,vert,2,&gRect,1,(horz==true ? GRADIENT_FILL_RECT_H : GRADIENT_FILL_RECT_V));
	}
	/////////////////////////////////////////////////////////////////////////////
		// 
	static void DrawLine(CDC *pDC, int x1, int y1, int x2, int y2, COLORREF clrLine)
	{	CPen pen(PS_SOLID,1,clrLine);
		CPen *pOldPen = pDC->SelectObject(&pen);
		pDC->MoveTo(x1,y1);
		pDC->LineTo(x2,y2);
		pDC->SelectObject(pOldPen);
	}
		// 
	static void DrawLine(CDC *pDC, int x1, int y1, int x2, int y2)
	{	pDC->MoveTo(x1,y1);
		pDC->LineTo(x2,y2);
	}
	/////////////////////////////////////////////////////////////////////////////
		// 
	static void DrawRect(CDC *pDC, int x1, int y1, int x2, int y2, COLORREF clrLine)
	{	CPen pen(PS_SOLID,1,clrLine);
		CPen *pOldPen = pDC->SelectObject(&pen);
		pDC->MoveTo(x1,y1);
		pDC->LineTo(x1,y2);
		pDC->LineTo(x2,y2);
		pDC->LineTo(x2,y1);
		pDC->LineTo(x1,y1);
		pDC->SelectObject(pOldPen);
	}
		// 
	static void DrawRect(CDC *pDC, CRect const *pRect, COLORREF clrLine)
	{	DrawRect(pDC,pRect->left,pRect->top,pRect->right-1,pRect->bottom-1,clrLine);
	}
		// 
	static void DrawRect(CDC *pDC, int x1, int y1, int x2, int y2)
	{	pDC->MoveTo(x1,y1);
		pDC->LineTo(x1,y2);
		pDC->LineTo(x2,y2);
		pDC->LineTo(x2,y1);
		pDC->LineTo(x1,y1);
	}
		// 
	static void DrawRect(CDC *pDC, CRect const *pRect)
	{	pDC->MoveTo(pRect->left,pRect->top);
		pDC->LineTo(pRect->left,pRect->bottom-1);
		pDC->LineTo(pRect->right-1,pRect->bottom-1);
		pDC->LineTo(pRect->right-1,pRect->top);
		pDC->LineTo(pRect->left,pRect->top);
	}
	/////////////////////////////////////////////////////////////////////////////
		// 
	static void DrawBeveledRect(CDC *pDC, CRect const *pRect, int bevel)
	{	pDC->MoveTo(pRect->left,pRect->top+bevel);
		pDC->LineTo(pRect->left,pRect->bottom-bevel-1);
		pDC->LineTo(pRect->left+bevel,pRect->bottom-1);
		pDC->LineTo(pRect->right-bevel-1,pRect->bottom-1);
		pDC->LineTo(pRect->right-1,pRect->bottom-bevel-1);
		pDC->LineTo(pRect->right-1,pRect->top+bevel);
		pDC->LineTo(pRect->right-bevel-1,pRect->top);
		pDC->LineTo(pRect->left+bevel,pRect->top);
		pDC->LineTo(pRect->left,pRect->top+bevel);
	}
	/////////////////////////////////////////////////////////////////////////////
		// 
	static COLORREF PixelAlpha(COLORREF src, COLORREF dst, int percent)
	{	int ipercent = 100 - percent;
		return RGB(
			(GetRValue(src) * percent + GetRValue(dst) * ipercent) / 100,
			(GetGValue(src) * percent + GetGValue(dst) * ipercent) / 100,
			(GetBValue(src) * percent + GetBValue(dst) * ipercent) / 100);
	}
	/////////////////////////////////////////////////////////////////////////////
		// 
	static void FillSolidRect(CDC *dc, CRect const *rc, COLORREF color)
	{	FillSolidRect(dc->m_hDC,rc,color);
	}
		// 
	static void FillSolidRect(HDC dc, CRect const *rc, COLORREF color)
	{	HBRUSH hBrush = ::CreateSolidBrush(color);
		::FillRect(dc,rc,hBrush);
		::DeleteObject(hBrush);
	}
	/////////////////////////////////////////////////////////////////////////////
		// 
	static bool DrawMarker(CDC *pDC, CRect const *pRect, HBITMAP hbmpMask, COLORREF color)
	{	BITMAP maskInfo;
		if(::GetObject(hbmpMask,sizeof(maskInfo),&maskInfo)==0) return false;
			// 
		CSize szDst(min(pRect->Width(),maskInfo.bmWidth),min(pRect->Height(),maskInfo.bmHeight));
			// 
		HDC dcMask=NULL, dc1=NULL, dc2=NULL;
		HBITMAP bmp1=NULL, bmp2=NULL;
			// 
		bool res = false;
			// 
		if((dcMask = ::CreateCompatibleDC(pDC->m_hDC))!=NULL &&
			(dc1 = ::CreateCompatibleDC(pDC->m_hDC))!=NULL && 
			(dc2 = ::CreateCompatibleDC(pDC->m_hDC))!=NULL &&
			(bmp1 = ::CreateCompatibleBitmap(pDC->m_hDC,szDst.cx,szDst.cy))!=NULL && 
			(bmp2 = ::CreateCompatibleBitmap(pDC->m_hDC,szDst.cx,szDst.cy))!=NULL)
		{
			HBITMAP bmpMaskOld = (HBITMAP)::SelectObject(dcMask,hbmpMask);
			HBITMAP bmpOld1 = (HBITMAP)::SelectObject(dc1,bmp1);
			HBITMAP bmpOld2 = (HBITMAP)::SelectObject(dc2,bmp2);
				// 
			::BitBlt(dc1,0,0,szDst.cx,szDst.cy,dcMask,0,0,SRCCOPY);
				// 
			::BitBlt(pDC->m_hDC,pRect->left,pRect->top,szDst.cx,szDst.cy,dc1,0,0,SRCAND);
				// 
			::BitBlt(dc1,0,0,szDst.cx,szDst.cy,NULL,0,0,DSTINVERT);
				// 
			HBRUSH br = ::CreateSolidBrush(color);
			HBRUSH brOld = (HBRUSH)::SelectObject(dc2,br);
			::BitBlt(dc2,0,0,szDst.cx,szDst.cy,dc1,0,0,MERGECOPY);
			::SelectObject(dc2,brOld);
			::DeleteObject(br);
				// 
			::BitBlt(pDC->m_hDC,pRect->left,pRect->top,szDst.cx,szDst.cy,dc2,0,0,SRCPAINT);
				// 
			::SelectObject(dcMask,bmpMaskOld);
			::SelectObject(dc1,bmpOld1);
			::SelectObject(dc2,bmpOld2);
				// 
			res = true;
		}
			// 
		if(bmp1!=NULL) ::DeleteObject(bmp1);
		if(bmp2!=NULL) ::DeleteObject(bmp2);
		if(dcMask!=NULL) ::DeleteDC(dcMask);
		if(dc1!=NULL) ::DeleteDC(dc1);
		if(dc2!=NULL) ::DeleteDC(dc2);
			// 
		return res;
	}
		// 
	static void DrawMarker(CDC *pDC, CRect const *pRect, CImageList *pImageList, int iImage, COLORREF color)
	{	bool res = false;
			// 
		HICON hIcon = pImageList->ExtractIcon(iImage);
			// 
		if(hIcon!=NULL)	// try to draw using mask.
		{	ICONINFO iconinfo;
			IMAGEINFO imginfo;
				// 
			if(pImageList->GetImageInfo(iImage,&imginfo)!=0 &&
				imginfo.hbmMask!=NULL &&
				::GetIconInfo(hIcon,&iconinfo)!=0)
			{
				res = DrawMarker(pDC,pRect,iconinfo.hbmMask,color);
					// 
				::DeleteObject(iconinfo.hbmColor);
				::DeleteObject(iconinfo.hbmMask);
			}
			::DestroyIcon(hIcon);
		}
			// 
		if(res==false)
			pImageList->Draw(pDC,iImage,CPoint(pRect->left,pRect->top),ILD_TRANSPARENT);
	}
	/////////////////////////////////////////////////////////////////////////////
		// 
	static bool CorrectFitSpaceString(CDC *pDC, CString const *strSrc, int maxLength, CString *strDst/*out*/)
	{	int strSrcLength = strSrc->GetLength();
			// 
		int count;
		SIZE sz;
		if(::GetTextExtentExPoint(pDC->m_hDC,*strSrc,strSrcLength,maxLength,&count,NULL,&sz)!=0)
			if(count < strSrcLength)
			{	if(count>0)
				{	int pointsWidth = pDC->GetTextExtent(_T("...")).cx;
					if(::GetTextExtentExPoint(pDC->m_hDC,*strSrc,strSrcLength,max(0,maxLength-pointsWidth),&count,NULL,&sz)==0) return false;
					*strDst = strSrc->Left(count);
					*strDst += _T("...");
				}
				else
					*strDst = _T("...");
				return true;
			}
			// 
		return false;
	}
};
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
struct VirtualWindow : public CDC
{	VirtualWindow(CDC *dcDst, CWnd *wndDst)
	{	_ASSERTE(dcDst!=NULL && dcDst->m_hDC!=NULL);
		_ASSERTE(wndDst!=NULL && ::IsWindow(wndDst->m_hWnd)!=0);
			// 
		wndDst->GetClientRect(&m_rcDst);
		success = Init(dcDst);
	}
	VirtualWindow(CDC *dcDst, CRect const *rcDst)
	{	_ASSERTE(dcDst!=NULL && dcDst->m_hDC!=NULL);
		_ASSERTE(rcDst!=NULL);
			// 
		m_rcDst = *rcDst;
		success = Init(dcDst);
	}
	~VirtualWindow()
	{	if(success==true)
		{	m_pDcDst->BitBlt(m_rcDst.left,m_rcDst.top,m_rcDst.Width(),m_rcDst.Height(),
				this,m_rcDst.left,m_rcDst.top, SRCCOPY);
			SelectObject(m_pOldBmp);
		}
		else
			Detach();
	}

public:
	static void DoubleBuffering(bool use)
	{	*GetDoubleBufferingFlag() = use;
	}
	static bool IsDoubleBuffering()
	{	return *GetDoubleBufferingFlag();
	}


private:
	bool Init(CDC *dcDst)
	{	m_pDcDst = dcDst;
			// 
		bool res = 
			IsDoubleBuffering()==true &&
			(CreateCompatibleDC(m_pDcDst)!=0 && 
			m_bmpSrc.CreateCompatibleBitmap(m_pDcDst,m_rcDst.Width(),m_rcDst.Height())!=0);
			// 
		if(res==true)
			m_pOldBmp = SelectObject(&m_bmpSrc);
		else
		{	if(m_hDC!=NULL) DeleteDC();
			if(m_bmpSrc.m_hObject!=NULL) m_bmpSrc.DeleteObject();
			Attach(m_pDcDst->m_hDC);
		}
		SetBkMode(TRANSPARENT);
		return res;
	}
		// 
	static bool *GetDoubleBufferingFlag()
	{	static bool bDoubleBuffering = true;
		return &bDoubleBuffering;
	}

private:
	CDC *m_pDcDst;
	CRect m_rcDst;
	CBitmap m_bmpSrc;
	CBitmap *m_pOldBmp;
	bool success;
};
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
struct ImageListRotate270
{		// 
	bool Rotate(CImageList *src, CImageList *dst/*out*/)
	{	try
		{	int count = src->GetImageCount();
				// 
			if(count>0)
			{	IMAGEINFO srcImgListInfo;
				if(src->GetImageInfo(0,&srcImgListInfo)==FALSE) throw std::exception();
					// 
				int srcWidth,srcHeight, dstWidth,dstHeight;
				srcWidth = dstHeight = srcImgListInfo.rcImage.right-srcImgListInfo.rcImage.left;
				srcHeight = dstWidth = srcImgListInfo.rcImage.bottom-srcImgListInfo.rcImage.top;
					// 
				UINT nFlags = ILC_COLOR32 | (srcImgListInfo.hbmMask!=NULL ? ILC_MASK : 0);
				if(dst->Create(dstWidth,dstHeight,nFlags,count,0)==0) throw std::exception();
					// 
				CWindowDC dc(NULL);
				bool res = false;
					// 
				for(int i=0; i<count; ++i)
				{	HICON hIcon = src->ExtractIcon(i);
						// 
					if(hIcon!=NULL)
					{	ICONINFO info;
						if(::GetIconInfo(hIcon,&info)!=0)
						{	CBitmap bmp, bmpMask;
								// 
							if(RotateBitmap(&dc,info.hbmColor,srcWidth,srcHeight,dstWidth,dstHeight,&bmp)==true &&
								(srcImgListInfo.hbmMask==NULL || 
									RotateMonochromeBitmap(&dc,info.hbmMask,srcWidth,srcHeight,dstWidth,dstHeight,&bmpMask)==true))
								res = (dst->Add(&bmp,&bmpMask)!=-1);
								// 
							::DeleteObject(info.hbmColor);
							::DeleteObject(info.hbmMask);
						}
						::DestroyIcon(hIcon);
					}
					if(res==false) break;
				}
				if(res==false) throw std::exception();
			}
		}
		catch(...)
		{	dst->DeleteImageList();
			return false;
		}
		return true;
	}
	/////////////////////////////////////////////////////////////////////////////
private:
	/////////////////////////////////////////////////////////////////////////////
		// 
	bool RotateBitmap(CDC *pDC, HBITMAP bmpSrc, 
		int srcWidth, int srcHeight, int dstWidth, int dstHeight,
		CBitmap *bmpDst/*out*/)
	{
		BITMAPINFOHEADER infohdr = {sizeof(BITMAPINFOHEADER),0,0,1,32,BI_RGB,0,0,0,0,0};
		COLORREF clr, *pBufSrc=NULL;
		try
		{	pBufSrc = BLADE_TMP_ALLOCT( COLORREF, srcWidth*srcHeight + dstWidth*dstHeight );
				// 
			COLORREF *pBufDst = pBufSrc + srcWidth*srcHeight;
			infohdr.biWidth = srcWidth;
			infohdr.biHeight = srcHeight;
			if(::GetDIBits(pDC->m_hDC,bmpSrc,0,srcHeight,pBufSrc,(BITMAPINFO *)&infohdr,DIB_RGB_COLORS)==0) throw std::bad_alloc();
				// 
			for(int y=0; y<srcHeight; ++y)
				for(int x=0; x<srcWidth; ++x)
				{	clr = pBufSrc[ y*srcWidth+x ];
					pBufDst[ (dstHeight-1-x)*dstWidth+y ] = clr;
				}
				// 
			if(bmpDst->CreateCompatibleBitmap(pDC,dstWidth,dstHeight)==FALSE) throw std::bad_alloc();
				// 
			infohdr.biWidth = dstWidth;
			infohdr.biHeight = dstHeight;
			if(::SetDIBits(pDC->m_hDC,(HBITMAP)bmpDst->m_hObject,0,dstHeight,pBufDst,(BITMAPINFO *)&infohdr,DIB_RGB_COLORS)==0)
				throw std::bad_alloc();
				// 
			BLADE_TMP_FREE(pBufSrc);
		}
		catch(std::bad_alloc &)
		{	if(pBufSrc!=NULL) BLADE_TMP_FREE(pBufSrc);;
			return false;
		}
		return true;
	}
		// 
	bool GetMonochromeBit(char const *p, int x, int y, int width) const
	{	int bitsInLine = width + ((width % 32)!=0 ? 32-(width % 32) : 0);
		int bytesInLine = bitsInLine / 8;
			// 
		p += y*bytesInLine + (x / 8);
		char bit = (char)((*p >> (7-(x % 8))) & 0x1);
		return bit!=0;
	}
		// 
	void SetMonochromeBit(char *p, int x, int y, int width) const
	{	int bitsInLine = width + ((width % 16)!=0 ? 16-(width % 16) : 0);
		int bytesInLine = bitsInLine / 8;
			// 
		p += y*bytesInLine + (x / 8);
		char val = (char)(0x1 << (7-(x % 8)));
		*p |= val;
	}
		// 
	bool RotateMonochromeBitmap(CDC *pDC, HBITMAP bmpSrc, 
		int srcWidth, int srcHeight, int dstWidth, int dstHeight,
		CBitmap *bmpDst/*out*/)
	{
		struct BITMAPINFOEX
		{	BITMAPINFOHEADER hdr;
			RGBQUAD arr[2];	// for 1bpp the color table consists 2 entry (black and white). 
		};
			// 
		BITMAPINFOEX infohdr = {sizeof(BITMAPINFOHEADER),srcWidth,srcHeight,1,1,BI_RGB,1,0,0,0,0,0};
		if(::GetDIBits(pDC->m_hDC,bmpSrc, 0,srcHeight,NULL,(BITMAPINFO *)&infohdr,DIB_RGB_COLORS)==0)	// size of buffer receive.
			throw std::bad_alloc();
			// 
		char *pBufSrc = NULL; 
		try
		{	pBufSrc = BLADE_TMP_ALLOCT(char, 2 * infohdr.hdr.biSizeImage);
				// 
			char *pBufDst = pBufSrc + infohdr.hdr.biSizeImage;
			::memset(pBufDst,0,infohdr.hdr.biSizeImage);
				// 
			if(::GetDIBits(pDC->m_hDC,bmpSrc,0,srcHeight,pBufSrc,(BITMAPINFO *)&infohdr,DIB_RGB_COLORS)==0) throw std::bad_alloc();
				// 
			for(int y=0; y<srcHeight; ++y)
				for(int x=0; x<srcWidth; ++x)
					if(GetMonochromeBit(pBufSrc,x,y,srcWidth)==true)
						SetMonochromeBit(pBufDst,y,x,dstWidth);
				// 
			if(bmpDst->CreateBitmap(dstWidth,dstHeight,1,1,pBufDst)==0) throw std::bad_alloc();
				// 
			BLADE_TMP_FREE(pBufSrc);
		}
		catch(std::bad_alloc &)
		{	if(pBufSrc!=NULL) BLADE_TMP_FREE(pBufSrc);
			return false;
		}
		return true;
	}
};
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////



