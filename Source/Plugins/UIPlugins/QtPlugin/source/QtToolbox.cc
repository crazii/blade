/********************************************************************
	created:	2016/07/23
	filename: 	QtToolbox.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <QtPluginPCH.h>
#include <interface/public/ui/IMenu.h>
#include <QtToolbox.h>
#include <QtIconManager.h>
#include <QtMenu.h>
#include <QtWindow.h>
#include <QtConfigDialog.h>

namespace Blade
{
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class QtToolButton : public ICommandUI, public Allocatable
	{
	public:
		QtToolButton(const TString& name, QToolButton* button, IUICommand* cmd, IMenu* menu = NULL)
			:mName(name)
			,mButton(button)
			,mCmd(cmd)
			,mMenu(menu)
			,mIndex(INVALID_INDEX)
		{
		}
		virtual ~QtToolButton() {}

		/** @brief  */
		virtual const TString&	getName() const { return mName; }
		/** @brief  */
		virtual void setEnabled(bool enable) { mButton->setEnabled(enable); }
		/** @brief  */
		virtual void setChecked(bool checked) { if (mButton->isCheckable()) mButton->setChecked(checked); }

	public:
		/** @brief  */
		virtual void updateUI(index_t index)
		{
			if (mCmd != NULL)
				mCmd->update(this, index);
			if (mMenu != NULL)
			{
				if(mMenu->getBindingCommand() != NULL)
					mMenu->getBindingCommand()->update(this, index);

				//update all drop down items
				for (size_t i = 0; i < mMenu->getSubMenuCount(); ++i)
				{
					IMenu* subMenu = mMenu->getSubMenu(i);
					IUICommand* subCmd = subMenu->getBindingCommand();
					if (subCmd != NULL)
						subCmd->update(subMenu, index);
				}

				IMenu* subMenu = mMenu->getCheckedSubMenu();
				index_t subIndex;;
				if (mMenu->isGrouped() && subMenu != NULL && (subIndex=subMenu->getMenuData()->mIndex) != mIndex)
				{
					mIndex = subIndex;
					if(mButton->toolButtonStyle() != Qt::ToolButtonIconOnly )
						mButton->setText(TString2QString(BTString2Lang(subMenu->getName())));
					if(mButton->toolButtonStyle() != Qt::ToolButtonTextOnly)
					{
						QIcon* icon = QtIconManager::getSingleton().getIcon(subMenu->getIconIndex());
						if(icon != NULL)
							mButton->setIcon(*icon);
					}
				}
			}
		}

		/** @brief  */
		virtual void execute()
		{
			if (mCmd != NULL)
				mCmd->execute(this);
		}

		TString	mName;
		QToolButton* mButton;
		IUICommand* mCmd;
		IMenu* mMenu;
		index_t mIndex;
	};

	class QtConfigSetButton : public QtToolButton
	{
	public:
		QtConfigSetButton(const TString& name, QToolButton* button, IConfig* config)
			:QtToolButton(name, button, NULL)
			,mConfig(config)	{}

		/** @brief  */
		virtual void updateUI(index_t /*index*/)
		{
			mButton->setEnabled((mConfig->getAccess()&CAF_WRITE)!=0);
		}

		/** @brief  */
		virtual void execute()
		{
			//drop down config panel
			QtConfigDialog* dialog = BLADE_NEW QtConfigDialog();
			dialog->addConfig(mConfig);
			QPoint pos = static_cast<QWidget*>(mButton->parent())->mapToGlobal(mButton->pos());
			pos.setY(pos.y() + mButton->size().height());
			dialog->move(pos);
			dialog->setMode(QtConfigDialog::POPUP);
			dialog->show(IConfigDialog::INFO(), TString::EMPTY);
		}

	protected:
		IConfig* mConfig;
	};

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	QtToolbox::QtToolbox(const TString& name, ICONSIZE is, QtWindow* parent)
		:mIconSize(is)
		,mParent(parent)
		,mName(name)
		,mConfigToolHelper(this)
	{
		this->setWindowTitle(TString2QString(BTString2Lang(name)));
		this->setObjectName(TString2QString(name));
		this->setIconSize(is);
		this->setContextMenuPolicy(Qt::PreventContextMenu);
		parent->addToolBar(this);
		mConfigToolHelper.initialize();
	}

	//////////////////////////////////////////////////////////////////////////
	QtToolbox::~QtToolbox()
	{
		for(ButtonList::iterator i = mButtons.begin(); i != mButtons.end(); ++i)
			BLADE_DELETE *i;
	}

	/************************************************************************/
	/* IUIToolBox interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool			QtToolbox::addButton(IUICommand* cmd, const TString& name, IconIndex icon, int style/* = TBS_NONE*/)
	{
		QIcon* qicon = QtIconManager::getSingleton().getIcon(icon);

		QToolButton* button = QT_NEW QToolButton();
		button->setObjectName( TString2QString(name) );
		if(mParent->needFilterToolButton())
			button->installEventFilter(mParent);	//@see QtViewPort::eventFilter
		if(qicon != NULL)
			button->setIcon(*qicon);
		button->setCheckable(style&TBS_CHECK);
		button->setIconSize(this->iconSize());
		button->setToolTip(TString2QString(BTString2Lang(name)));
		button->installEventFilter(this);	//update popup menu & right click config
		this->QToolBar::addWidget(button);

		QtToolButton* uibutton = BLADE_NEW QtToolButton(name, button, cmd);
		mButtons.push_back(uibutton);
		QObject::connect(button, &QToolButton::clicked, makeMemFn(uibutton, &QtToolButton::execute));
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			QtToolbox::addButton(IMenu* menu, int style/* = TBS_CHECKGROUP*/)
	{
		QIcon* qicon = QtIconManager::getSingleton().getIcon(menu->getIconIndex());
		QString qstr = TString2QString(BTString2Lang(menu->getName()));
		QToolButton* button = QT_NEW QToolButton();
		button->setToolTip(qstr);
		button->setObjectName(TString2QString(menu->getName()));
		if (mParent->needFilterToolButton())
			button->installEventFilter(mParent);	//@see QtViewPort::eventFilter
		if(qicon != NULL)
			button->setIcon(*qicon);
		button->setCheckable(menu->isCheckable());
		button->setIconSize(this->iconSize());
		button->installEventFilter(this);	//update popup menu & right click config
		if(style&TBS_DROPDOWN)
		{
			if(!menu->isEmbedded() /*&& menu->isGrouped()*/ )
			{
				button->setMenu( static_cast<QtMenu*>(menu)->getQMenu() );
				button->setPopupMode(QToolButton::InstantPopup);
			}
		}
		if(style&TBS_TEXT)
		{
			button->setText(qstr);
			button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
		}
		this->QToolBar::addWidget(button);

		QtToolButton* uibutton = BLADE_NEW QtToolButton(menu->getName(), button, menu->getBindingCommand(), menu);
		mButtons.push_back( uibutton );
		QObject::connect(button, &QToolButton::clicked, makeMemFn(uibutton, &QtToolButton::execute));
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			QtToolbox::addTool(IConfig* config, int width/* = -1*/)
	{
		QWidget* widget = mConfigToolHelper.createEditor(config, NULL);
		widget->setObjectName(TString2QString(config->getName()));
		this->addSeparator();
		this->addWidget(widget);
		widget->setMaximumWidth(width);
		widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			QtToolbox::addToolSet(IConfig* config, IconIndex icon)
	{
		const TString& name = config->getName();
		QIcon* qicon = QtIconManager::getSingleton().getIcon(icon);

		QToolButton* button = QT_NEW QToolButton();
		if (mParent->needFilterToolButton())
			button->installEventFilter(mParent);	//@see QtViewPort::eventFilter
		if (qicon != NULL)
			button->setIcon(*qicon);
		button->setIconSize(this->iconSize());
		button->setToolTip(TString2QString(BTString2Lang(name)));
		this->QToolBar::addWidget(button);

		QtConfigSetButton* uibutton = BLADE_NEW QtConfigSetButton(name, button, config);
		mButtons.push_back(uibutton);
		QObject::connect(button, &QToolButton::clicked, makeMemFn(uibutton, &QtConfigSetButton::execute));
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void			QtToolbox::updateTools()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	size_t			QtToolbox::getButtonCount() const
	{
		return (size_t)this->actions().count();
	}

	//////////////////////////////////////////////////////////////////////////
	bool			QtToolbox::setIconSize(ICONSIZE is)
	{
		mIconSize = is;
		size_t _iconSize = QtIconManager::getSingleton().getIconSize(is);
		this->QToolBar::setIconSize( QSize((int)_iconSize, (int)_iconSize));
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void			QtToolbox::updateUI()
	{
		for(ButtonList::iterator i = mButtons.begin(); i != mButtons.end(); ++i)
			(*i)->updateUI( mParent->getIndex() );
	}

	//////////////////////////////////////////////////////////////////////////
	bool			QtToolbox::eventFilter(QObject *watched, QEvent *evt)
	{
		if (evt->type() == QEvent::MouseButtonPress)
		{
			for (ButtonList::const_iterator i = mButtons.begin(); i != mButtons.end(); ++i)
			{
				QtToolButton* button = *i;
				if (button->mButton == watched)
				{
					if(button->mMenu != NULL)
						static_cast<QtMenu*>(button->mMenu)->updateCmdUI(mParent->getIndex());

					QMouseEvent* mse = static_cast<QMouseEvent*>(evt);
					if (mse->button() == Qt::RightButton && button->mCmd != NULL)
					{
						button->mCmd->config(button);
						evt->ignore();
						return true;
					}
				}
			}

		}
		return QToolBar::eventFilter(watched, evt);
	}
	
}//namespace Blade