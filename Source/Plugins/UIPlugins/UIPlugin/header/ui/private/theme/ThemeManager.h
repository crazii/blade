/********************************************************************
	created:	2016/07/17
	filename: 	ThemeManager.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_ThemeManager_h__
#define __Blade_ThemeManager_h__

enum EThemeColor
{
	TC_BEGIN,
	TC_FRAME = TC_BEGIN,//window frame
	TC_ACTIVEFRAME,	//active window frame
	TC_WINBK,		//window back ground
	TC_EMPTYBK,		//empty window/client
	TC_CTLBK,		//control back ground
	TC_HILIGHTBK,	//hi lighted bk
	TC_TEXT,		//text
	TC_GRAYTEXT,	//gray text
	TC_LINE,		//lines
	TC_MENULINE,	//lines, separator for menu
	TC_MENUBK,		//menu background
	TC_HILIGHTMENUBK,//hi lighted menu background
	TC_GRAYMENUTEXT,//grayed menu text
	TC_SPLITTER,	//splitter
	TC_TEXTBK,		//text background, if not transparent
	TC_TEXTBKDARK,	//dark text background 
	TC_BTN,			//button face
	TC_HILIGHTBTN,	//light button face
	TC_PUSHEDBTN,	//pushed button

	TC_COUNT,
};

struct Theme
{
	COLORREF	mColors[TC_COUNT];
	TCHAR*		mName;
};

class ThemeManager : public Blade::Singleton<ThemeManager>
{
public:
	static Theme DEFAULT_THEME;

	struct WinThemeProc
	{
		void (*PaintProc)(HWND hWnd, WNDPROC defProc);
		void (*NcPaintProc)(HWND hWNd, WNDPROC defProc);
		LRESULT (*NcCalcSizeProc)(HWND hWnd, BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp, WNDPROC defProc);
		BOOL (*EraseBkgndProc)(HWND hWNd, HDC hDC, WNDPROC defProc);
	};

	struct ThemeData
	{
		WNDPROC proc;
		bool	dialog;
	};
public:
	ThemeManager();
	~ThemeManager();

	/** @brief  */
	void	initialize();

	/** @brief  */
	void	setTheme(Theme theme);

	/** @brief  */
	const Theme& getTheme() const {return mTheme;}

	/** @brief  */
	inline COLORREF	getThemeColor(EThemeColor color)
	{
		if( color >= TC_BEGIN && color < TC_COUNT)
			return mTheme.mColors[color];
		return RGB(0xff,0,0);
	}

	/** @brief  */
	inline CBrush&	getThemeBrush(EThemeColor color)
	{
		if( color >= TC_BEGIN && color < TC_COUNT)
			return mBrushes[color];
		return mInvalidBrush;
	}

protected:
	HHOOK	mHook;
	Theme	mTheme;
	CBrush	mBrushes[TC_COUNT];
	CBrush	mInvalidBrush;

	typedef Blade::Map<HWND, ThemeData> WindProcMap;
	typedef Blade::StaticTStringMap<WinThemeProc> ThemeMap;

	WindProcMap	mMap;
	ThemeMap	mThemeMap;

	/** @brief  */
	void addWindow(HWND hWnd);
	/** @brief  */
	static LRESULT CALLBACK ThemeCBTProc(int nCode, WPARAM wParam, LPARAM lParam);
	/** @brief  */
	static LRESULT CALLBACK ThemeProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};

/** @brief  */
inline COLORREF THEMECOLOR(EThemeColor c)
{
	return ThemeManager::getSingleton().getThemeColor(c);
}

/** @brief  */
inline CBrush& THEMEBRUSH(EThemeColor c)
{
	return ThemeManager::getSingleton().getThemeBrush(c);
}

#endif // __Blade_ThemeManager_h__