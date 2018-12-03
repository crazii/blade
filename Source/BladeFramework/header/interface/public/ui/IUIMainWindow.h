/********************************************************************
	created:	2016/6/2
	filename: 	IUIMainWindow.h
	author:		Crazii
	purpose:	root window of UI system
*********************************************************************/
#ifndef __Blade_IUIMainWindow_h__
#define __Blade_IUIMainWindow_h__
#include <BladeFramework.h>
#include <utility/String.h>
#include <utility/Version.h>
#include "UITypes.h"
#include "IUIWindow.h"
#include "IConfigDialog.h"
#include "../IProgressCallback.h"

namespace Blade
{
	class IUIToolBox;
	class IUIWidget;
	class IUILayoutManager;

	class BLADE_FRAMEWORK_API IUIMainWindow : public IUIWindow
	{
	public:
		typedef struct SNewFileDesc
		{
			TString		mDesc;
			TString		mExtension;
			TString		mDefaultPath;
			IconIndex	mIcon;
			IConfig*	mNewFileConfig;
			bool		mEnable;
			static inline bool compare(const SNewFileDesc& lhs, const SNewFileDesc& rhs) { return lhs.mDesc < rhs.mDesc; }
			static inline bool comparePtr(const SNewFileDesc* lhs, const SNewFileDesc* rhs) { return compare(*lhs, *rhs); }
			inline bool operator<(const SNewFileDesc& rhs) const { return compare(*this, rhs); }
		}NEW_FILE_DESC;
	public:
		virtual ~IUIMainWindow() {}

		/** @brief  */
		virtual bool			setUpdater(IUIWindow::IUpdater* /*updater*/) { return false; }

		/** @brief  */
		virtual IUIWindow::IUpdater*getUpdater() const { return NULL; }

		/** @brief  */
		virtual bool			showCreateFileUI(TString& outExt, TString& outDir, TString& outName, const NEW_FILE_DESC* desc, size_t count) = 0;

		/** @brief  */
		virtual bool			showOpenFileUI(TString& outExt, TString& outDir, TString& outName, const TStringParam& extensions, const TStringParam& descs) = 0;

		/** @brief  */
		virtual bool			showSaveFileUI(TString& outDir, TString& outName, const TString& extension = TString::EMPTY) = 0;

		/** @brief multiple groups separated in tabs */
		virtual bool			showMultipleGroupConfig(const TString& dialogName, IConfig** configs, size_t count,
			const IConfigDialog::INFO& info = IConfigDialog::INFO()) = 0;

		inline bool				showSingleGroupConfig(const TString& diallogName, IConfig* config,
			const IConfigDialog::INFO& info = IConfigDialog::INFO())
		{
			return this->showMultipleGroupConfig(diallogName, &config, 1, info);
		}

		/** @brief  */
		virtual IProgressCallback*	showProgress(const TString& title) = 0;

		/** @brief  */
		virtual IUILayoutManager*	getLayoutManager() const = 0;

	};//class IUIMainWindow

	extern template class BLADE_FRAMEWORK_API Factory<IUIMainWindow>;
	typedef Factory<IUIMainWindow> UIMainWindowFactory;
	
}//namespace Blade


#endif//__Blade_IUIMainWindow_h__