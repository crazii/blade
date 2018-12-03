/********************************************************************
	created:	2016/7/27
	filename: 	QtLogWindow.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_QtLogWindow_h__
#define __Blade_QtLogWindow_h__
#include <interface/IFramework.h>
#include <interface/public/ui/widgets/IUILogView.h>
#include <widgets/QtWidgetBase.h>

namespace Blade
{
	class QtLogWindow : public QtWidgetBase<IUILogView>, public ILogRedirector, public IFramework::IMainLoopUpdater, public Allocatable
	{
	public:
		static const TString WIDGET_TYPE;
	public:
		QtLogWindow();
		~QtLogWindow();

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
			this->flushLogBuffer();
		}

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		void		flushLogBuffer();

	protected:
		QMainWindow mContent;
		QDockWidget	mTabs[ILog::LL_COUNT];
		QTextEdit	mTexts[ILog::LL_COUNT];
		QColor		mTextColor[ILog::LL_COUNT];
		size_t		mTextCursor[ILog::LL_COUNT];

		HLOGREDIRECTOR	hSelf;
		struct SLogItem
		{
			SLogItem(ILog::ELogLevel l) :level(l) {logText = NULL;}
			~SLogItem() {BLADE_TMP_FREE(logText);}

			ILog::ELogLevel	level;
			tchar*			logText;
		};
		typedef	TempList<SLogItem>	LogBuffer;
		LogBuffer	mBuffer;
		Lock		mBufferLock;
	};
	
}//namespace Blade

#endif//__Blade_QtLogWindow_h__