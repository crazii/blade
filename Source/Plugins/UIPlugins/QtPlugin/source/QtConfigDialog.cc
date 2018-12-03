/********************************************************************
	created:	2016/07/25
	filename: 	QtConfigDialog.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <QtPluginPCH.h>
#include <QtConfigDialog.h>
#include <interface/IConfigManager.h>
#include <interface/IEnvironmentManager.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	QtConfigDialog::QtConfigDialog(QWidget* parent/* = NULL*/, Qt::WindowFlags f/* = Qt::WindowFlags()*/)
		:QDialog(parent, f)
		,mBanner(this)
		,mTab(this)
		,mPropGrid(this)
		,mOK(this)
		,mCancel(this)
		,mCheckAutoShow(this)
		,mMode(NORMAL)
	{
		mOK.setAutoDefault(true);

		QObject::connect(&mOK, SIGNAL(clicked()), this, SLOT(accept()));
		QObject::connect(&mCancel, SIGNAL(clicked()), this, SLOT(reject()));

		//note: we don't use signal because don't want Qt MOC on this to generate meta data,
		//so we use pure functors
		QObject::connect(&mTab, &QTabBar::currentChanged, this, &QtConfigDialog::onTabChange);

		QVBoxLayout* layout = QT_NEW QVBoxLayout();
		this->setLayout(layout);

		mBanner.setVisible(false);
		mCheckAutoShow.setVisible(false);

		layout->addWidget(&mBanner);
		layout->addWidget(&mTab);
		layout->addWidget(mPropGrid.getWidget());

		QHBoxLayout* buttonLayout = QT_NEW QHBoxLayout();
		layout->addLayout(buttonLayout);
		buttonLayout->addWidget(&mCheckAutoShow, Qt::AlignLeft);
		buttonLayout->addStretch(1080);
		buttonLayout->addWidget(&mCancel, Qt::AlignRight);
		buttonLayout->addWidget(&mOK, Qt::AlignRight);
	}

	//////////////////////////////////////////////////////////////////////////
	QtConfigDialog::~QtConfigDialog()
	{
		
	}

	//////////////////////////////////////////////////////////////////////////
	bool QtConfigDialog::show(const INFO& info, const HCONFIG* configs, size_t count,
		const TString& caption/* = BTString("Blade Config") */)
	{
		mConfigList.clear();
		for(size_t i = 0; i < count; ++i)
		{
			IConfig* config = configs[i];
			config->updateData(true, true);
			config->backupValue();
			mTab.addTab(TString2QString(BTString2Lang(config->getName())));
			mConfigList.push_back(config);
		}
		mPropGrid.bindConfigs(mConfigList.size() > 0 ? *mConfigList.begin() : NULL);

		return this->showImpl(info, caption);
	}

	//////////////////////////////////////////////////////////////////////////
	bool QtConfigDialog::addConfig(IConfig* configItem)
	{
		bool devMode = IEnvironmentManager::getSingleton().getVariable(ConstDef::EnvBool::DEVELOPER_MODE);
		CONFIG_ACCESS access = configItem->getAccess();
		if( (access&CAF_READWRITE) == 0 || (!devMode && (access&CAF_DEVMODE)) )
			return true;

		for (ConfigList::iterator i = mConfigList.begin(); i != mConfigList.end(); ++i)
		{
			if ((*i) == configItem || (*i)->getName() == configItem->getName())
				return false;
		}
		configItem->updateData(true);
		mTab.addTab(TString2QString(BTString2Lang(configItem->getName())));
		mConfigList.push_back(configItem);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	QtConfigDialog::show(const INFO& info, const TString& caption)
	{
		for (ConfigList::iterator i = mConfigList.begin(); i != mConfigList.end(); ++i)
			(*i)->backupValue();

		mPropGrid.bindConfigs(mConfigList.size() > 0 ? *mConfigList.begin() : NULL);
		return this->showImpl(info, caption);
	}

	//////////////////////////////////////////////////////////////////////////
	void QtConfigDialog::setMode(EMode mode)
	{
		mMode = mode;
		bool toplevel = (mode == TOPLEVEL);
		mBanner.setVisible(toplevel);
		mCheckAutoShow.setVisible(toplevel);

		if (toplevel)
		{
			const TString& image = IEnvironmentManager::getSingleton().getVariable(ConstDef::EnvString::STARTUP_IMAGE);
			HSTREAM stream = IResourceManager::getSingleton().loadStream(image);
			if (stream == NULL || !stream->isValid())
				return;
			HIMAGE img = IImageManager::getSingleton().loadImage(stream, IP_TEMPORARY, PO_NONE, IMGO_TOP_DOWN);
			if (img == NULL)
				return;

			const int MAX_HEIGHT = 169;
			int height = std::min(MAX_HEIGHT, (int)img->getHeight());
			img = IImageManager::getSingleton().scaleImage(img, SIZE2I(img->getWidth(), height), IP_TEMPORARY);

			Handle<QImage> qimg = toQImage(img);
			QPixmap pm;
			pm.convertFromImage(*qimg);
			mBanner.setPixmap(pm);
			this->setFixedWidth(pm.width());
		}
		else if (mode == POPUP)
		{
			this->layout()->setMargin(0);
			this->layout()->setSpacing(0);
			//this->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
			this->setWindowFlags(Qt::Popup);
			mCancel.setVisible(false);
			mOK.setVisible(false);
			mBanner.setVisible(false);
			mTab.setVisible(false);
			this->setAttribute(Qt::WA_DeleteOnClose, true);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void QtConfigDialog::accept()
	{
		QDialog::accept();

		if (mCheckAutoShow.isVisible())
		{
			bool checked = mCheckAutoShow.isChecked();
			TString ShowStringBOOL = TStringHelper::fromBool(checked);
			IConfigManager::getSingleton().setEntry(BTString("ShowDialog"), ShowStringBOOL, CONFIG_DIALOG_SECTION);
		}

		if (!mInfo.mModal)
			mInfo.mNoneModalDelegate.call(mInfo.mUserData);
	}

	//////////////////////////////////////////////////////////////////////////
	void QtConfigDialog::reject()
	{
		if(mMode != POPUP)
		{
			for (ConfigList::iterator i = mConfigList.begin(); i != mConfigList.end(); ++i)
				(*i)->restoreToBackup();
		}

		QDialog::reject();
	}

	//////////////////////////////////////////////////////////////////////////
	bool		QtConfigDialog::showImpl(const INFO& info, const TString& caption)
	{
		QString cap = TString2QString(caption);
		this->setObjectName(cap);

		if (mMode == POPUP)
		{
			this->QDialog::show();
			this->activateWindow();
			return true;
		}

		mOK.setText(TString2QString(BTLang(BLANG_OK)));
		mCancel.setText( TString2QString(BTLang(BLANG_CANCEL)) );
		mCheckAutoShow.setText(TString2QString(BTLang("Show this dialog on startup")));
		bool autoShow = TStringHelper::toBool( IConfigManager::getSingleton().getEntry(BTString("ShowDialog"), CONFIG_DIALOG_SECTION) );
		mCheckAutoShow.setChecked(autoShow);
		this->setModal(info.mModal);
		this->setWindowTitle(cap);
		this->resize(480, 360);

		mInfo = info;
		if(info.mModal)
			return this->QDialog::exec() != 0;
		else
		{
			this->QDialog::show();
			return true;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void QtConfigDialog::hideEvent(QHideEvent* hevent)
	{
		QDialog::hideEvent(hevent);

		while (mTab.count() > 0)
			mTab.removeTab(0);

		mConfigList.clear();
		mPropGrid.bindConfigs(NULL);
	}

	//////////////////////////////////////////////////////////////////////////
	void QtConfigDialog::showEvent(QShowEvent* evt)
	{
		QDialog::showEvent(evt);
		int l,t,r,b;
		this->layout()->getContentsMargins(&l, &t, &r, &b);

		int maxHeight = QApplication::desktop()->size().height() * 2 / 3;
		int w = this->size().width();
		int h = mPropGrid.getHeight() + t + b;
		int sps = this->layout()->spacing();

		if(mMode == POPUP)
		{
			w = 360;
		}
		else if(mMode == TOPLEVEL || mMode == NORMAL)
		{
			for(ConfigList::const_reverse_iterator i = mConfigList.rbegin(); i != mConfigList.rend(); ++i)
			{
				mPropGrid.bindConfigs(*i);
				h = std::max<int>(h, (int)mPropGrid.getHeight() + t + b);
			}
			h += mTab.size().height() + mOK.size().height() + mBanner.size().height() + sps*3;
			//h = std::max(h, this->size().height());
		}
		//else if(mMode == NORMAL)
		//{
		//	h += mTab.size().height() + mOK.size().height() + sps*2;
		//	//h = std::max(h, this->size().height());
		//}
		this->resize(w, std::min(maxHeight, h));
		if(mMode != POPUP)
			this->move(this->pos().x(), (QApplication::desktop()->size().height() - this->height()) / 2);
	}

	//////////////////////////////////////////////////////////////////////////
	void QtConfigDialog::onTabChange(int index)
	{
		if(index >= mConfigList.size() )
			return;
		ConfigList::iterator i = mConfigList.begin();
		std::advance(i, (indexdiff_t)index);
		mPropGrid.bindConfigs(*i);
	}
	
}//namespace Blade