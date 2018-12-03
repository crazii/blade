/********************************************************************
	created:	2011/05/05
	filename: 	LogWindow.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_LogWindow_h__
#define __Blade_LogWindow_h__
#include <interface/ILog.h>
#include <interface/IFramework.h>
#include <ui/DockWindow.h>
#include <ui/private/UITabCtrl.h>
#include <utility/BladeContainer.h>
#include <interface/public/ui/widgets/IUILogView.h>

namespace Blade
{

	class LogWindow : public DockWindowBase<IUILogView>, public ILogRedirector, public IFramework::IMainLoopUpdater, public Singleton<LogWindow>
	{
	public:
		static const TString WIDGET_TYPE;
	public:
		using Singleton<LogWindow>::operator new;
		using Singleton<LogWindow>::operator delete;
		using Singleton<LogWindow>::operator new[];
		using Singleton<LogWindow>::operator delete[];
	public:
		LogWindow();
		~LogWindow();

		/************************************************************************/
		/* IUILogView interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual bool enableLogRedirection(bool enable = true);

		/************************************************************************/
		/* ILogRedirector interface                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		virtual void outputLog(ILog::ELogLevel level, const tchar* content);

		/************************************************************************/
		/* IFramework::IMainLoopUpdater interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual void update()
		{
			if (this->GetSafeHwnd())
				this->flushLogBuffer();
		}

		/************************************************************************/
		/* MFC related                                                                     */
		/************************************************************************/
		/** @brief  */
		afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
		/** @brief  */
		afx_msg void OnSize(UINT nType, int cx, int cy);
		/** @brief  */
		afx_msg void OnTabSelChange(NMHDR*pNotifyStruct, LRESULT*result);

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		void		flushLogBuffer();

	protected:
		/** @brief  */
		void		initialize();

		DECLARE_MESSAGE_MAP()

	protected:
		HLOGREDIRECTOR	hSelf;
		struct SLogItem
		{
			ILog::ELogLevel	level;
			TString			logText;
		};

		typedef	TempList<SLogItem>	LogBuffer;

		CUITabCtrl		mLogTypeTab;
		CRichEditCtrl	mLogContent[ILog::LL_COUNT];
		COLORREF		mTextColor[ILog::LL_COUNT];
		size_t			mTextCursor[ILog::LL_COUNT];

		LogBuffer	mBuffer;
		Lock		mBufferLock;
	};

	
}//namespace Blade


#endif // __Blade_LogWindow_h__