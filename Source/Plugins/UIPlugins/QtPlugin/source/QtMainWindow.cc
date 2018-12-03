/********************************************************************
	created:	2016/07/23
	filename: 	QtMainWindow.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <QtPluginPCH.h>
#include <QtMainWindow.h>
#include <QtToolbox.h>
#include <QtMenuManager.h>
#include <QtIconManager.h>
#include <QtLayoutManager.h>
#include <QtNewFileDialog.h>
#include <QtConfigDialog.h>
#include <QtProgressWindow.h>

namespace Blade
{
	const TString QtMainWindow::QT_MAIN_WINDOW_TYPE = BTString("QtMainWindow");

	//////////////////////////////////////////////////////////////////////////
	QtMainWindow::QtMainWindow()
	{
		mTitle = BTString("QtMainWindow");
		this->setObjectName("QtMainWindow");
		mLayoutMananger = BLADE_NEW QtLayoutManager(this);
		mFrameworkData = NULL;
		mUserData = NULL;

		//ui module need Qt keyboard & mouse type explicit
		//TODO: explicit order control over HotkeyManager. 
		//HotKeyManager will listen global keyboard. for now QtMainWindow construct before HotKeyManager so it OK,
		//but if it construct after HokeyManager, HokeyManager may listen to previous implementation of keyboard type(ie.e Win32GlobalKeyboard)
		//or transfer listeners on MessagePump/GlobalKeyboard interchange.
		IWindowService::getSingleton().setWindowTypeOverride( BTString("QtWindow") );
		const TString QT_KEYBOARD_TYPE = BTString("QtKeyboard");
		const TString QT_MOUSE_TYPE = BTString("QtMouse");
		IUIService::getSingleton().setInputTypeOverride(&QT_KEYBOARD_TYPE, &QT_MOUSE_TYPE);

		mProgress = BLADE_NEW QtProgress();
	}

	//////////////////////////////////////////////////////////////////////////
	QtMainWindow::~QtMainWindow()
	{
		HWINDOW hWindow = IWindowService::getSingleton().getAppWindow();
		IWindowService::getSingleton().destroyWindow(hWindow->getUniqueName());

		IFramework::getSingleton().removeUpdater(this);
		BLADE_DELETE mLayoutMananger;

		////note: window will delete immediately, @see QtWindowDevice::eventFilter
		////if close is invoked through menu, the menu will deleted immediate, and mouse event will still go to it
		////so detach the menu
		//QMenuBar* menuBar = this->menuBar();
		//this->setMenuBar(NULL);
		//menuBar->setParent(NULL);

		BLADE_DELETE mProgress;
	}
	
	/************************************************************************/
	/* IUIWindow interface                                                                    */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool			QtMainWindow::initWindow(uintptr_t parentWindowImpl, IconIndex iconIndex, const TString& caption/* = TString::EMPTY*/)
	{
		mTitle = caption;
		mIcon = iconIndex;

		this->setDockOptions(AllowNestedDocks|GroupedDragging|AnimatedDocks|AllowTabbedDocks);
		//this->setAnimated(false);
		this->setWindowTitle( TString2QString(caption) );
		QMenuBar* menubar = QtMenuManager::getSingleton().getAppMenuBar();
		menubar->setVisible(true);
		this->setMenuBar(menubar);
		this->setStatusBar(QT_NEW QStatusBar());
		this->statusBar()->setVisible(true);
		QMdiArea* mdi = QT_NEW QMdiArea();
		mdi->setViewMode(QMdiArea::TabbedView);
		mdi->setTabsMovable(true);
		mdi->setTabsClosable(true);
		this->setCentralWidget(mdi);
		
		this->showMaximized();
		this->setAttribute(Qt::WA_DeleteOnClose, true);
		QIcon* icon = QtIconManager::getSingleton().getIcon(iconIndex);
		if(icon != NULL)
			this->setWindowIcon(*icon);

		mWindow = IWindowService::getSingleton().attachWindow((uintptr_t)static_cast<QWidget*>(this));
		IWindowService::getSingleton().setAppWindow(mWindow);

		IFramework::getSingleton().addUpdater(this);
		return true;
	}

	/************************************************************************/
	/* IUIQtMainWindow interface                                                                    */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool			QtMainWindow::showCreateFileUI(TString& outExt, TString& outDir, TString& outName, const NEW_FILE_DESC* desc, size_t count)
	{
		QtNewFileDialog dialog(this);
		dialog.initialize(desc, count);

		if( dialog.exec() )
		{
			index_t index = dialog.getSelected();
			assert(index < count);

			outExt = desc[index].mExtension;
			outDir = dialog.getFolder();
			outName = dialog.getName();
			return true;
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			QtMainWindow::showOpenFileUI(TString& outExt, TString& outDir, TString& outName, const TStringParam& extensions, const TStringParam& descs)
	{
		if(extensions.size() != descs.size())
		{
			assert(false);
			return false;
		}

		QStringList filters;
		filters.reserve( extensions.size()+2 );

		//all supported
		TStringConcat cat(BTLang(BLANG_ALL_SUPPORTED_FILE) + BTString(" ("));
		for (size_t i = 0; i < extensions.size(); ++i)
			cat += TEXT("*.") + extensions[i] + TEXT(' ');
		cat += TEXT(')');
		filters.append( TString2QString(cat) );
		//all files
		filters.append( TString2QString( BTLang(BLANG_ALL_FILE) + BTString(" (*)") ));
		//each file types
		for(size_t i = 0; i < extensions.size(); ++i)
		{
			TStringConcat cat(TEXT(""));
			cat += descs[i] + BTString(" (*.") + extensions[i] + BTString(")");
			filters.append( TString2QString(cat) );
		}

		//use QSettings to record last dir. TODO: use blade config?
		QSettings settings;
		const QString LAST_DIR("LastDir");
		QString lastDir = settings.value(LAST_DIR).toString();
		if(lastDir.isEmpty() || IResourceManager::getSingleton().loadFolder(QString2TString(lastDir)) == NULL)
			lastDir = "../Data";

		QFileDialog dialog;
		dialog.setOption(QFileDialog::DontUseNativeDialog, true);
		dialog.setAcceptMode(QFileDialog::AcceptOpen);
		dialog.setFileMode(QFileDialog::ExistingFile);
		dialog.setNameFilters(filters);
		dialog.setDirectory(lastDir);

		if( dialog.exec() )
		{
			settings.setValue(LAST_DIR, dialog.directory().absolutePath());

			QStringList files = dialog.selectedFiles();
			if(files.size() == 1)
			{
				const QString& file = files[0];
				TString tmp( (const tchar*)file.constData(), (size_t)file.size(), TString::CONST_HINT);
				TString base;
				TStringHelper::getFilePathInfo(tmp, outDir, outName, base, outExt);
				return true;
			}
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			QtMainWindow::showSaveFileUI(TString& outDir, TString& outName, const TString& extension/* = TString::EMPTY*/)
	{
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			QtMainWindow::showMultipleGroupConfig(const TString& diallogName, IConfig** configs, size_t count,
		const IConfigDialog::INFO& info/* =  IConfigDialog::INFO() */)
	{
		QtConfigDialog* dialog =  static_cast<QtConfigDialog*>(this->findChild<QDialog*>(TString2QString(diallogName)));
		if (dialog != NULL)
		{
			dialog->activateWindow();
			dialog->setFocus();
			return true;
		}

		dialog = BLADE_NEW QtConfigDialog(this);
		dialog->setAttribute(Qt::WA_DeleteOnClose, true);

		for(size_t i = 0; i < count; ++i)
			dialog->addConfig(configs[i]);
		bool ret = dialog->show(info, diallogName);

		if (info.mModal && ret)
			info.mNoneModalDelegate.call(info.mUserData);
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	IProgressCallback*	QtMainWindow::showProgress(const TString& title)
	{
		assert(mProgress != NULL);
		mProgress->reset(this, title);
		return mProgress;
	}

	//////////////////////////////////////////////////////////////////////////
	void		QtMainWindow::update()
	{
		for(WindowList::iterator i = msWindows.begin(); i != msWindows.end(); ++i)
		{
			QtWindow* window = *i;
			if(window->isVisible())
				window->updateUI();
		}
	}

}//namespace Blade