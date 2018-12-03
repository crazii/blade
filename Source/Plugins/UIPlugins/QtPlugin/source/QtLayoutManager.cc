/********************************************************************
	created:	2016/07/26
	filename: 	QtLayoutManager.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <QtPluginPCH.h>
#include <QtLayoutManager.h>
#include <widgets/QtWidgetBase.h>
#include <interface/public/ui/IUIWindow.h>

namespace Blade
{
	class UIWindowData : public QObjectUserData, public Allocatable
	{
	public:
		virtual ~UIWindowData() {}
		UIWindowData(IUIWindow* window) :mWindow(window)	{}
		IUIWindow* mWindow;
	};

	static const TString QT_LAYOUT_FILE = BTString("cwd:last.qtlayout");

	//////////////////////////////////////////////////////////////////////////
	QtLayoutManager::QtLayoutManager(QMainWindow* mainWindow)
		:mOwner(mainWindow)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	QtLayoutManager::~QtLayoutManager()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	IUIWindow*		QtLayoutManager::createWindow(IconIndex icon, const TString& wtype, const TString& caption/* = TString::EMPTY*/)
	{
		if (!UIWindowFactory::getSingleton().isClassRegistered(wtype))
		{
			assert(false);
			return NULL;
		}
		IUIWindow* window = BLADE_FACTORY_CREATE(IUIWindow, wtype);
		QMdiArea* mdi = static_cast<QMdiArea*>(mOwner->centralWidget());
		bool ret = window->initWindow((uintptr_t)static_cast<QWidget*>(mdi), icon, caption);
		assert(ret); BLADE_UNREFERENCED(ret);
		QWidget* widget = (QWidget*)window->getWindow()->getImpl();
		widget->setUserData(0, BLADE_NEW UIWindowData(window));
		QMdiSubWindow* subWindow = mdi->addSubWindow(widget, Qt::SubWindow);
		subWindow->setWindowIcon(widget->windowIcon());
		widget->setVisible(true);
		widget->activateWindow();
		widget->resize(mdi->size());
		return window;
	}

	//////////////////////////////////////////////////////////////////////////
	IUIWindow*		QtLayoutManager::getActiveWindow() const
	{
		QMdiArea* mdi = static_cast<QMdiArea*>(mOwner->centralWidget());
		QWidget* widget = NULL;
		do
		{
			QMdiSubWindow* sub = mdi->currentSubWindow();
			if (sub == NULL)
				return NULL;
			widget = sub->widget();
			if (widget == NULL)
				sub->close();
		} while (widget == NULL);

		UIWindowData* userData = static_cast<UIWindowData*>( widget->userData(0) );
		if(userData == NULL)
			return NULL;
		return userData->mWindow;
	}

	//////////////////////////////////////////////////////////////////////////
	IUIWidget*	QtLayoutManager::createWidget(const WIDGET_DATA& widgetData)
	{
		UIWidgetFinder finder(widgetData.mName);
		if( mWidgets.find(&finder) != mWidgets.end() )
		{
			assert(false);
			return false;
		}

		if( !UIWidgetFactory::getSingleton().isClassRegistered(widgetData.mType) )
		{
			assert(false);
			return NULL;
		}

		IUIWidget* widget = BLADE_FACTORY_CREATE(IUIWidget, widgetData.mType);
		widget->initWidget(widgetData);
		QtWidgetBase<IUIWidget>* qwidget = static_cast<QtWidgetBase<IUIWidget>*>(widget);
		qwidget->setObjectName(TString2QString(widgetData.mName));
		mOwner->addDockWidget(Qt::LeftDockWidgetArea, qwidget);

		mWidgets.insert(widget);
		return widget;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		QtLayoutManager::applyLayout(const UIWidgetLayout& layout)
	{
		typedef TempSet<IUIWidget*> ShownWidget;
		ShownWidget shownList;

		size_t count = this->getWidgetCount();
		for(size_t i = 0; i < count; ++i)
		{
			IUIWidget* widget = this->getWidget(i);
			if(shownList.find(widget) != shownList.end())
				continue;

			widget->setVisible(false);

			QtWidgetBase<IUIWidget>* qwidget = static_cast<QtWidgetBase<IUIWidget>*>(widget);
			mOwner->removeDockWidget(qwidget);

			UIWidgetLayout::OutputBranch branch;
			bool ret = layout.findWidget(widget->getName(), branch);
			if(!ret || branch.size() < 1)
				continue;

			UILayoutNode* node = branch[branch.size()-1];
			UILayoutNode* subRoot = branch.size() == 1 ? node : branch[1];

			Qt::DockWidgetArea area;
			if(node->getLayoutType() == UILayoutNode::LT_HORIZONTAL)
				area = layout.getRoot().findChild(subRoot) == 0 ? Qt::LeftDockWidgetArea : Qt::RightDockWidgetArea;
			else
				area = Qt::BottomDockWidgetArea;

			QDockWidget* last = NULL;
			QDockWidget* active = NULL;
			for (index_t i = 0; i < node->getWidgetCount(); ++i)
			{
				const UILayoutNode::SWidgetInfo* widgetInfo = node->getWidgetInfo(i);
				const TString& name = widgetInfo->name;
				IUIWidget* tabbedWidget = this->getWidget(name);
				if (tabbedWidget == NULL)	//plugin maybe not installed
					continue;
				QtWidgetBase<IUIWidget>* tabbedQwidget = static_cast<QtWidgetBase<IUIWidget>*>(tabbedWidget);

				if (node->getActiveWidget() == i)
					active = tabbedQwidget;

				if(last == NULL)
					mOwner->addDockWidget(area, tabbedQwidget/*, orient*/);
				else
					mOwner->tabifyDockWidget(last, tabbedQwidget);
				last = tabbedQwidget;

				tabbedWidget->setVisible((widgetInfo->flags&WMF_VISIBLE));
				shownList.insert(tabbedQwidget);
			}
			if (active == NULL)
				active = qwidget;
			active->raise();
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		QtLayoutManager::loadLastLayout()
	{
		HSTREAM stream = IResourceManager::getSingleton().loadStream(QT_LAYOUT_FILE);
		if(stream == NULL || !stream->isValid())
			return false;
		
		IOBuffer buffer(stream);
		QByteArray bytes((const char*)buffer.getData(), buffer.getSize());
		return mOwner->restoreState(bytes);
	}

	//////////////////////////////////////////////////////////////////////////
	bool		QtLayoutManager::saveLayout() const
	{
		QByteArray bytes = mOwner->saveState();
		HSTREAM stream = IResourceManager::getSingleton().openStream(QT_LAYOUT_FILE, false);
		if(stream == NULL || !stream->isValid())
			return false;
		return stream->writeData(bytes.data(), bytes.size()) == bytes.size();
	}
	
}//namespace Blade