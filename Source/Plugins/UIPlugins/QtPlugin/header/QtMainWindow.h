/********************************************************************
	created:	2016/07/23
	filename: 	QtMainWindow.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_QtMainWindow_h__
#define __Blade_QtMainWindow_h__
#include <interface/IFramework.h>
#include <interface/public/ui/IUIMainWindow.h>
#include <QtWindow.h>
#include <QtToolbox.h>

namespace Blade
{
	class QtProgress;

	class QtMainWindow : public QtWindow, public IUIMainWindow, public IFramework::IMainLoopUpdater, public Singleton<QtMainWindow>
	{
	public:
		static const TString QT_MAIN_WINDOW_TYPE;
	public:
		QtMainWindow();
		~QtMainWindow();

		/************************************************************************/
		/* IUIWindow interface                                                                    */
		/************************************************************************/
		/** @brief  */
		virtual const TString&	getWindowType() const { return QT_MAIN_WINDOW_TYPE; }

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
		virtual IUIToolBox*		createToolBox(const TString& name, ICONSIZE is) { return QtWindow::createQtToolBox(name, is); }

		/** @brief  */
		virtual IUIToolBox*		getToolBox(const TString& name) const { return QtWindow::getQtToolBox(name); }

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
		virtual IUILayoutManager*	getLayoutManager() const { return mLayoutMananger; }

		/************************************************************************/
		/* IMainLoopUpdater                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual void	update();

	protected:

		HWINDOW			mWindow;
		TString			mTitle;
		Version			mVersion;
		IconIndex		mIcon;
		IUILayoutManager* mLayoutMananger;
		QtProgress*		mProgress;
		void*			mFrameworkData;
		void*			mUserData;
	};
	

}//namespace Blade


#endif // __Blade_QtMainWindow_h__