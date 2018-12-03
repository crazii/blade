/********************************************************************
	created:	2016/7/26
	filename: 	QtToolPanel.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <QtPluginPCH.h>
#include <widgets/QtToolPanel.h>
#include <QtIconManager.h>
#include <QtToolbox.h>
#include <QtMainWindow.h>

namespace Blade
{
	const TString QtToolPanel::WIDGET_TYPE = BTString(BLANG_TOOL);

	//////////////////////////////////////////////////////////////////////////
	QtToolPanel::QtToolPanel()
		:QtWidgetBase<IUIToolPanel>(WIDGET_TYPE)
		,mCateIndex(INVALID_INDEX)
		,mTimer(this)
		,mButtonSize(-1)
		,mVisibleUpdate(0)
	{
		this->setWidget(&mContent);
		mTimer.setSingleShot(true);
		QObject::connect(&mTimer, &QTimer::timeout, this, &QtToolPanel::visibilityUpdate);
	}

	//////////////////////////////////////////////////////////////////////////
	QtToolPanel::~QtToolPanel()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	bool	QtToolPanel::setupCategory(const TString& defCategory, IconIndex defIcon, const TString* categories, IconIndex* icons, size_t count)
	{
		mDefCategory = defCategory;

		Category defCat = {defCategory, defIcon };
		mCategoryList.push_back(defCat);

		for(size_t i = 0; i < count; ++i)
		{
			assert(categories[i] != mDefCategory);

			Category cat = {categories[i] , icons[i] };
			mCategoryList.push_back(cat);
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	QtToolPanel::addToolBox(const TStringParam& categories, IUIToolBox* toolbox, bool allowMove/* = true*/)
	{
		if(toolbox == NULL)
		{
			assert(false);
			return false;
		}

		TempVector<TString> tmpCates;
		tmpCates.reserve(categories.size());
		for (size_t i = 0; i < categories.size(); ++i)
			tmpCates.push_back(categories[i]);

		if(tmpCates.size() == 0)
			tmpCates.resize(1, mDefCategory);

		bool ret = false;

		QtToolbox* qToolBox = static_cast<QtToolbox*>(toolbox);
		QList<QAction*> act = qToolBox->actions();

		for (int j = 0; j < act.count(); ++j)
		{		
			//steal button from tool bar...
			QToolButton* button = static_cast<QToolButton*>(qToolBox->widgetForAction(act[j]));
			if (mButtonSize == -1)
				mButtonSize = button->sizeHint().width();

			for(size_t i = 0; i < tmpCates.size(); ++i)
			{
				Category finder = {tmpCates[i], INVALID_ICON_INDEX};
				CategoryList::iterator iter = std::find(mCategoryList.begin(), mCategoryList.end(), finder);

				if(iter != mCategoryList.end())
				{
					ret = true;
					//don't steal buttons on init
					//QtMainWindow::getSingleton().removeToolBar(qToolBox);
					//qToolBox->hide();
					////steal tool bar from main window
					//qToolBox->setParent(&mContent);
					//button->setParent(&mContent);

					ToolInfo info = { toolbox, ToolButtonList(), allowMove };
					iter->tools.push_back(info);
					iter->tools.back().buttons.push_back(button);
				}
			}
		}
		//if(ret)
		//	qToolBox->clear();

		if(mButtonSize != -1)
			mContent.setMinimumSize(mButtonSize, mButtonSize);
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	QtToolPanel::switchCategory(const TString& category)
	{
		Category finder = {category, INVALID_ICON_INDEX};
		CategoryList::const_iterator i = std::find(mCategoryList.begin(), mCategoryList.end(), finder);
		if( i == mCategoryList.end() )
		{
			assert(false);
			return false;
		}

		index_t index = 0;
		CategoryList::const_iterator iter = mCategoryList.begin();
		while( iter++ != i)
			++index;

		this->onSwitchCategory((int)index);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void	QtToolPanel::onSwitchCategory(int index)
	{
		if( (index_t)index >= mCategoryList.size() )
		{
			assert(false);
			return;
		}

		if(mCateIndex == index)
			return;
		mCateIndex = index;

		if (!this->QDockWidget::isVisible())
			return;

		CategoryList::const_iterator i = mCategoryList.begin();
		std::advance(i, (indexdiff_t)index);

		QSize sz = mContent.size();
		int offy = 0;

		int colCount = std::max(1, sz.width() / mButtonSize);
		index_t buttonIndex = 0;

		typedef TempSet<QToolButton*> VisiblitySet;
		VisiblitySet buttonVisibilities;

		for(CategoryList::const_iterator iter = mCategoryList.begin(); iter != mCategoryList.end(); ++iter)
		{
			bool show = (iter == i) || (iter->name == mDefCategory);
			const ToolList& list = iter->tools;
			for(ToolList::const_iterator j = list.begin(); j != list.end(); ++j)
			{
				const ToolButtonList& list2 = j->buttons;
				for(ToolButtonList::const_iterator i = list2.begin(); i != list2.end(); ++i)
				{
					QToolButton* bt = *i;
					if(bt->parent() != &mContent)
						continue;

					if(buttonVisibilities.insert(bt).second)		
						bt->setVisible(false);
					if (show)
					{
						int x = buttonIndex % colCount;
						int y = buttonIndex / colCount;
						bt->move(x*mButtonSize, y*mButtonSize);
						++buttonIndex;
						bt->setVisible(show);
					}
				}
			}
			//align to next row
			int x = buttonIndex%colCount;
			if (x != 0)
				buttonIndex += colCount - x;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void QtToolPanel::resizeEvent(QResizeEvent* evt)
	{
		QtWidgetBase<IUIToolPanel>::resizeEvent(evt);

		index_t index = mCateIndex;
		mCateIndex = INVALID_INDEX;
		//reset button layout
		if(index != INVALID_INDEX)
		{
			this->onSwitchCategory(index);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void QtToolPanel::showEvent(QShowEvent* evt)
	{
		QtWidgetBase<IUIToolPanel>::showEvent(evt);
		
		if (!mVisibleUpdate)
		{
			mVisibleUpdate = 1;
			mTimer.start(30);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void QtToolPanel::hideEvent(QHideEvent* evt)
	{
		QtWidgetBase<IUIToolPanel>::hideEvent(evt);

		if(!mVisibleUpdate)
		{
			mVisibleUpdate = 1;
			mTimer.start(30);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void QtToolPanel::visibilityUpdate()
	{
		if (!mVisibleUpdate)
			return;
		mVisibleUpdate = 0;

		if (this->QDockWidget::isVisible())
		{
			for (CategoryList::iterator iter = mCategoryList.begin(); iter != mCategoryList.end(); ++iter)
			{
				ToolList& tools = iter->tools;
				for (ToolList::iterator j = tools.begin(); j != tools.end(); ++j)
				{
					ToolButtonList& buttons = j->buttons;
					QtToolbox* toolBox = static_cast<QtToolbox*>(j->toolBox);
					QtMainWindow::getSingleton().removeToolBar(toolBox);
					toolBox->hide();
					toolBox->setParent(&mContent);

					//steal button from tool bar
					for (ToolButtonList::iterator i = buttons.begin(); i != buttons.end(); ++i)
					{
						QToolButton* button = *i;
						if (button != NULL)
							button->setParent(&mContent);
					}
				}
			}

			//tool box may be shared between categories, clear tool box button at last
			for (CategoryList::iterator iter = mCategoryList.begin(); iter != mCategoryList.end(); ++iter)
			{
				ToolList& tools = iter->tools;
				for (ToolList::iterator j = tools.begin(); j != tools.end(); ++j)
				{
					QtToolbox* toolBox = static_cast<QtToolbox*>(j->toolBox);
					toolBox->clear();
				}
			}

			index_t index = mCateIndex;
			mCateIndex = INVALID_INDEX;
			//reset button layout
			if (index != INVALID_INDEX)
			{
				this->onSwitchCategory(index);
			}
		}
		else
		{
			//return the stolen tool buttons to their original parents
			for (CategoryList::iterator iter = mCategoryList.begin(); iter != mCategoryList.end(); ++iter)
			{
				ToolList& tools = iter->tools;
				for (ToolList::iterator j = tools.begin(); j != tools.end(); ++j)
				{
					ToolButtonList& buttons = j->buttons;
					QtToolbox* toolbox = static_cast<QtToolbox*>(j->toolBox);
					toolbox->setVisible(false);
					for (ToolButtonList::const_iterator i = buttons.begin(); i != buttons.end(); ++i)
					{
						(*i)->setParent(NULL);
						(*i)->setVisible(true);
						toolbox->addWidget(*i);
					}
					//buttons.clear();
					toolbox->setParent(NULL);
					QtMainWindow::getSingleton().addToolBar(toolbox);
					toolbox->setVisible(true);
				}
			}
		}
	}
	
}//namespace Blade