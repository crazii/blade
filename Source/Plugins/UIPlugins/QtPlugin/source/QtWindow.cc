/********************************************************************
	created:	2016/07/25
	filename: 	QtWindow.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <QtPluginPCH.h>
#include <QtWindow.h>
#include <QtMenu.h>
#include <QtToolbox.h>

namespace Blade
{
	QtWindow::WindowList QtWindow::msWindows;

	//////////////////////////////////////////////////////////////////////////
	QtWindow::QtWindow()
		:mFilterToolboxButton(false)
	{
		msWindows.push_back(this);
	}

	//////////////////////////////////////////////////////////////////////////
	QtWindow::~QtWindow()
	{
		WindowList::iterator i = std::find(msWindows.begin(), msWindows.end(), this);
		assert( i != msWindows.end() );
		msWindows.erase(i);
	}

	//////////////////////////////////////////////////////////////////////////
	void	QtWindow::updateUI()
	{
		index_t index = this->getIndex();

		for(ToolBoxList::iterator i = mToolBoxes.begin(); i != mToolBoxes.end(); ++i)
			(*i)->updateUI();
	}

	//////////////////////////////////////////////////////////////////////////
	QtToolbox*		QtWindow::createQtToolBox(const TString& name, ICONSIZE is, Qt::ToolBarAreas areas/* = Qt::TopToolBarArea|Qt::LeftToolBarArea*/, bool allowMove/* = true*/)
	{
		if (this->getQtToolBox(name) != NULL)
		{
			assert(false);
			return NULL;
		}
		QtToolbox* toolbox = BLADE_NEW QtToolbox(name, is, this);
		toolbox->setMovable(allowMove);
		toolbox->setAllowedAreas(areas);
		mToolBoxes.push_back(toolbox);
		return toolbox;
	}

	//////////////////////////////////////////////////////////////////////////
	QtToolbox*		QtWindow::getQtToolBox(const TString& name) const
	{
		for (ToolBoxList::const_iterator i = mToolBoxes.begin(); i != mToolBoxes.end(); ++i)
		{
			QtToolbox* tb = *i;
			if (tb->getName() == name)
				return tb;
		}
		return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	void	QtWindow::setMenuBar(QMenuBar *menubar)
	{
		QMainWindow::setMenuBar(menubar);
		if (menubar != NULL)
			menubar->installEventFilter(this);
	}

	//////////////////////////////////////////////////////////////////////////
	bool QtWindow::eventFilter(QObject *watched, QEvent *evt)
	{
		if (watched == this->menuBar() && evt->type() == QEvent::MouseButtonPress )
		{
			QList<QAction*> act = this->menuBar()->actions();
			for (int i = 0; i < act.count(); ++i)
			{
				QMenu* qmenu = act[i]->menu();
				//see QtMenuManager::addRootMenu or QtMenu::addSubMenu
				//which calling QMenu::setUserData
				QObjectUserData* data = qmenu->userData(0);
				if (data != NULL)
				{
					QMenuData* menuData = static_cast<QMenuData*>(data);
					menuData->qtmenu->updateCmdUI(this->getIndex());
				}
			}
		}
		return QMainWindow::eventFilter(watched, evt);
	}
	
}//namespace Blade