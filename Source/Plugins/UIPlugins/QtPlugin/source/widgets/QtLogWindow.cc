/********************************************************************
	created:	2016/7/26
	filename: 	QtLogWindow.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <QtPluginPCH.h>
#include <interface/ILogManager.h>
#include <widgets/QtLogWindow.h>

namespace Blade
{
	const TString QtLogWindow::WIDGET_TYPE = BTString(BLANG_LOG);

	//////////////////////////////////////////////////////////////////////////
	QtLogWindow::QtLogWindow()
		:QtWidgetBase<IUILogView>(WIDGET_TYPE)
	{
		hSelf.bind(this);
		ILogManager::getSingleton().redirectLogs(hSelf, true);
		IFramework::getSingleton().addUpdater(this);

		this->setWidget(&mContent);

		QColor colors[ILog::LL_COUNT] = 
		{
			QColor(0, 0, 0), QColor(255, 0, 0), QColor(255, 255, 0), QColor(255, 255, 255), QColor(0, 255, 0),
		};

		TString labels[ILog::LL_COUNT] = {
			BTLang(BLANG_ALL),
			BTLang(BLANG_ERROR),
			BTLang(BLANG_WARNING),
			BTLang(BLANG_INFO),
			BTLang(BLANG_DEBUG),
		};

		TString icons[ILog::LL_COUNT] =
		{
			BTString("detail"),
			BTString("error"),
			BTString("warning"),
			BTString("Info"),
			BTString("bug"),
		};

		mContent.setDockOptions(QMainWindow::ForceTabbedDocks|QMainWindow::AllowTabbedDocks);
		for (int i = ILog::LL_BEGIN; i < ILog::LL_COUNT; ++i)
		{
			mTabs[i].setAllowedAreas(Qt::LeftDockWidgetArea);
			mTabs[i].setFeatures(NoDockWidgetFeatures);
			mTabs[i].setWindowTitle(TString2QString(labels[i]));
			//hide title bar
			QWidget* emptyTitle = QT_NEW QWidget(&mTabs[i]);
			mTabs[i].setTitleBarWidget(emptyTitle);
			mTabs[i].setWidget(&mTexts[i]);
			if( i == 0 )
				mContent.addDockWidget(Qt::LeftDockWidgetArea, &mTabs[i]);
			else
			{
				mContent.tabifyDockWidget(&mTabs[i-1], &mTabs[i]);
				mTexts[i].setTextColor(colors[i]);
			}

			mTextColor[i] = colors[i];
			mTexts[i].setReadOnly(true);
			mTexts[i].setLineWrapMode(QTextEdit::NoWrap);
		}

		mContent.update();
		QTabBar* tabBar = mContent.findChild<QTabBar*>();
		if(tabBar != NULL)
		{
			for (int i = ILog::LL_BEGIN; i < ILog::LL_COUNT; ++i)
			{
				IconIndex index = IIconManager::getSingleton().loadSingleIconImage(icons[i] + BTString(".png"));
				QIcon* icon = QtIconManager::getSingleton().getIcon(index);
				if(icon != NULL)
					tabBar->setTabIcon(i, *icon);
			}
		}

		//make first one as default
		mTabs[0].raise();
		mTabs[0].show();
	}

	//////////////////////////////////////////////////////////////////////////
	QtLogWindow::~QtLogWindow()
	{
		ILogManager::getSingleton().redirectLogs(HLOGREDIRECTOR::EMPTY);
		hSelf.unbind();
		IFramework::getSingleton().removeUpdater(this);
	}

	/************************************************************************/
	/* IUILogView interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool QtLogWindow::enableLogRedirection(bool enable/* = true*/)
	{
		return ILogManager::getSingleton().redirectLogs(enable ? hSelf : HLOGREDIRECTOR::EMPTY);
	}
	
	/************************************************************************/
	/* ILogRedirector interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void QtLogWindow::outputLog(ILog::ELogLevel level, const tchar* content)
	{
#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
		::OutputDebugString(content);
#endif
		//add extra lock in case that background thread is outputting log while main thread is flushLogBuffer
		ScopedLock lock(mBufferLock);

		mBuffer.push_back(SLogItem(level));
		SLogItem& item = mBuffer.back();

		size_t len = TCharTraits::length(content);
		item.logText = BLADE_TMP_ALLOCT(tchar, len+1);
		::memcpy(item.logText, content, len*sizeof(tchar));
		item.logText[len] = TEXT('\0');
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void QtLogWindow::flushLogBuffer()
	{
		ScopedLock lock(mBufferLock);

		bool hasOutput[ILog::LL_COUNT] = { false, false, false, false, false };
		bool scrollOutput[ILog::LL_COUNT] = { false, false, false, false, false };
		index_t oldPos[ILog::LL_COUNT] = { 0 };

		for (LogBuffer::iterator i = mBuffer.begin(); i != mBuffer.end(); ++i)
		{
			const SLogItem& item = *i;

			size_t size = TCharTraits::length(item.logText);
			tchar* content = (tchar*)item.logText;
			while(size > 1 && content[size-1] == TEXT('\n') || content[size-1] == TEXT('\r') )
				content[(size--)-1] = TEXT('\0');
			ILog::ELogLevel level = item.level;

			assert(level > 0 && level < ILog::LL_COUNT);

			{
				int start, end, lastSel;
				QTextCursor cursor = mTexts[level].textCursor();
				start = cursor.selectionStart();
				end = cursor.selectionEnd();

				if (start != end)
					lastSel = 0;
				else
					lastSel = start;

				//don't scroll if user manual selecting text
				scrollOutput[level] = (lastSel == (int)mTextCursor[level]);
				oldPos[level] = mTextCursor[level];

				mTexts[level].append( QString((QChar*)content, size) );
				mTextCursor[level] += size;

				if (!scrollOutput[level])
				{
					//mTexts[level].setTextCursor( QTextCursor(start, end);
				}
				else
					mTexts[0].moveCursor(QTextCursor::End);

				hasOutput[level] = true;
			}

			{
				int start, end, lastSel;
				QTextCursor cursor = mTexts[level].textCursor();
				start = cursor.selectionStart();
				end = cursor.selectionEnd();

				if (start != end)
					lastSel = 0;
				else
					lastSel = start;

				//de-select text && set cursor
				//don't scroll if user manual selecting text
				scrollOutput[0] = (lastSel == (int)mTextCursor[0]);
				oldPos[0] = mTextCursor[0];

				mTexts[0].setTextColor(mTextColor[level]);
				mTexts[0].append( QString((QChar*)content, size) );
				mTextCursor[0] += size;

				if (!scrollOutput[0])
				{
					//mTexts[0].setTextCursor( QTextCursor(start, end);
				}
				else
					mTexts[0].moveCursor(QTextCursor::End);

				hasOutput[0] = true;
			}
		}
		mBuffer.clear();
	}

}//namespace Blade