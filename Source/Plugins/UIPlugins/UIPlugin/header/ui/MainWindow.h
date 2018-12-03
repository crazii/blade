/********************************************************************
	created:	2016/06/09
	filename: 	MainWindow.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_MainWindow_h__
#define __Blade_MainWindow_h__
#include <interface/public/ui/IUIMainWindow.h>
#include <interface/public/ui/IUIWidget.h>
#include <ui/LayoutManager.h>
#include <ui/private/UIFrameWnd.h>
#include <ui/private/UIMenu.h>

namespace Blade
{
	class UIToolBox;
	class LoadingProgress;

	class MainWindow : public CUIFrameWnd, public IUIMainWindow, public Singleton<MainWindow>
	{
	public:
		static const TString MAIN_WINDOW_TYPE;
		using Singleton<MainWindow>::operator new;
		using Singleton<MainWindow>::operator delete;
	public:
		MainWindow();
		virtual ~MainWindow();
		/************************************************************************/
		/* IUIWindow interface                                                                    */
		/************************************************************************/
		/** @brief  */
		virtual const TString&	getWindowType() const { return MAIN_WINDOW_TYPE; }

		/** @brief  */
		virtual bool			initWindow(uintptr_t parentWindowImpl, IconIndex iconIndex, const TString& caption = TString::EMPTY);

		/** @brief  */
		virtual IconIndex		getIcon() const { return mIcon; }

		/** @brief used by framework */
		virtual void			setFrameworkData(void* data) { mFrameworkData = data; }

		/** @brief used by framework */
		virtual void*			getFrameworkData() const { return mFrameworkData; }

		/** @brief  */
		virtual void			setUserData(void* data) { mUserData = data; }

		/** @brief  */
		virtual void*			getUserData() const { return mUserData; }

		/** @brief get platform handle if implementation is platform dependent. used by implementation only  */
		virtual IWindow*		getWindow() const { return mWindow; }

		/** @brief  */
		virtual IUIToolBox*		createToolBox(const TString& name, ICONSIZE is);

		/** @brief  */
		virtual IUIToolBox*		getToolBox(const TString& name) const;

		/************************************************************************/
		/* IUIMainWindow interface                                                                    */
		/************************************************************************/
		/** @brief  */
		virtual bool			showCreateFileUI(TString& outExt, TString& outDir, TString& outName, const NEW_FILE_DESC* desc, size_t count);

		/** @brief  */
		virtual bool			showOpenFileUI(TString& outExt, TString& outDir, TString& outName, const TStringParam& extensions, const TStringParam& descs);

		/** @brief  */
		virtual bool			showSaveFileUI(TString& outDir, TString& outName, const TString& extension = TString::EMPTY);

		/** @brief multiple groups separated in tabs */
		virtual bool			showMultipleGroupConfig(const TString& dialogName, IConfig** configs, size_t count,
			const IConfigDialog::INFO& info = IConfigDialog::INFO());

		inline bool				showSingleGroupConfig(const TString& diallogName, IConfig* config,
			const IConfigDialog::INFO& info = IConfigDialog::INFO())
		{
			return this->showMultipleGroupConfig(diallogName, &config, 1, info);
		}

		/** @brief  */
		virtual IProgressCallback*	showProgress(const TString& title);

		/** @brief  */
		virtual IUILayoutManager*	getLayoutManager() const { return mWindowManager; }

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/

	protected:
		/************************************************************************/
		/* MFC overrides                                                                     */
		/************************************************************************/
		DECLARE_MESSAGE_MAP()

		afx_msg void	OnClose();
		afx_msg int		OnCreate(LPCREATESTRUCT lpCreateStruct);
		afx_msg void	OnDestroy();
		afx_msg void	OnNcPaint();
		afx_msg void	OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
		virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
		virtual BOOL	PreTranslateMessage(MSG* pMsg);
		virtual void	PostNcDestroy();

		typedef Vector<UIToolBox*>	ToolBoxList;
		typedef Set<IUIWidget*, FnWidgetNameLess>	WidgetList;

		HWINDOW					mWindow;
		LayoutManager*			mWindowManager;
		LoadingProgress*		mLoadingProgress;
		ToolBoxList				mToolboxes;
#if BLADE_USE_MFC_FEATURE_PACK
		CUIMenuBar				mMenuBar;
#endif

		TString			mTitle;
		Version			mVersion;
		IconIndex		mIcon;
		void*			mFrameworkData;
		void*			mUserData;
	};
	
}//namespace Blade


#endif // __Blade_MainWindow_h__