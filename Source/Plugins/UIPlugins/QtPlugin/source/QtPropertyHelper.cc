/********************************************************************
	created:	2016/07/25
	filename: 	QtPropertyHelper.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <QtPluginPCH.h>
#include <interface/public/ui/IUIMediaLibrary.h>
#include <interface/IEnvironmentManager.h>
#include <interface/public/ui/UIEvents.h>
#include <interface/IEventManager.h>
#include <QtPropertyHelper.h>
#include <QtIconManager.h>
#include "propertybrowser/qttreepropertybrowser.h"
#include "propertybrowser/qtpropertymanager.h"
#include "propertybrowser/qteditorfactory.h"
#include "propertybrowser/extension/fileeditfactory.h"
#include "propertybrowser/extension/BladeExtension.h"

namespace Blade
{
	namespace Impl
	{
		//////////////////////////////////////////////////////////////////////////
		static QColorHDR getColor(Color c)
		{
			scalar max = 1;
			if (c[0] > 1 || c[1] > 1 || c[2] > 1)
			{
				max = std::max(std::max(c[0], c[1]), c[2]);
				c /= max;
			}

			QColorHDR qc;
			qc.setRedF(c[0]);
			qc.setGreenF(c[1]);
			qc.setBlueF(c[2]);
			qc.setAlphaF(c[3]);
			qc.m_HDRScale = max;
			return qc;
		}
	}

	struct ImageReceiver : public IMediaLibrary::ISelectionReceiver
	{
		TString mPath;
		HIMAGE	mPreview;
		index_t	mMediaIndex;
		bool mDone;
		ImageReceiver() :mDone(false)	{}
		virtual void	setSelectedTarget(index_t mediaIndex, const IMediaLibrary::MediaFile& media)
		{
			mPath = media.mSchemePath;
			mPreview = media.mPreview;
			mMediaIndex = mediaIndex;
			mDone = true;
		}
	};

	struct PropertyManagerData : public QObjectUserData, public Allocatable
	{
		PropertyManagerData(int id) :mID(id)	{}
		int mID;
	};

	static const int PROPERTY_UPDATE_INTERVAL = 300;	//ms
	enum EPropertyManagerID
	{
		PMID_DOUBLE,
		PMID_STRING,
		PMID_COLOR,
		PMID_BOOL,
		PMID_ENUM,
		PMID_FILE,
		PMID_RANGEENUM,
	};

	//!IMPORTANT: match EPropertyManagerID order
	#define MANAGER_ARARY(var) QtAbstractPropertyManager* var[] =\
	{\
		mDoubleManager,\
		mStringManager,\
		mColorManager,\
		mBoolManager,\
		mEnumManager,\
		mFilePathManager,\
		mRangedEnumManager,\
	}

	#define EDITFORFACTORY_ARRAY(var) QtAbstractEditorFactoryBase* var[]=\
	{\
		mDoubleEditSliderFactory,\
		mLineEditFactory,\
		mColorFactory,\
		mCheckBoxFactory,\
		mComboBoxFactory,\
		mFileEditFactory,\
		mSliderFactory,\
	}

	//////////////////////////////////////////////////////////////////////////
	QtPropertyHelper::QtPropertyHelper(QWidget* parent)
		:mParent(parent)
		,mBrowser(NULL)
		,mTimerID(0)
		,mPropCreating(false)
		,mPropUpdating(false)
		,mInited(false)
	{
		mDoubleManager = QT_NEW QtDoublePropertyManager(mParent);
		mStringManager = QT_NEW QtStringPropertyManager(mParent);
		mColorManager = QT_NEW QtColorPropertyManager(mParent);
		mBoolManager = QT_NEW QtBoolPropertyManager(mParent);
		mEnumManager = QT_NEW QtEnumPropertyManager(mParent);
		mFilePathManager = QT_NEW FilePathManager(mParent);
		mRangedEnumManager = QT_NEW QtEnumPropertyManager(mParent);

		QObject::connect(mDoubleManager, &QtDoublePropertyManager::valueChanged, this, &QtPropertyHelper::rangeChanged);
		QObject::connect(mStringManager, &QtStringPropertyManager::valueChanged, this, &QtPropertyHelper::editChanged);
		QObject::connect(mColorManager, &QtColorPropertyManager::valueChanged, this, &QtPropertyHelper::colorChanged);
		QObject::connect(mBoolManager, &QtBoolPropertyManager::valueChanged, this, &QtPropertyHelper::checkChanged);
		QObject::connect(mEnumManager, &QtEnumPropertyManager::valueChanged, this, &QtPropertyHelper::listChanged);
		QObject::connect(mFilePathManager, &FilePathManager::valueChanged, this, &QtPropertyHelper::fileChanged);
		QObject::connect(mRangedEnumManager, &QtEnumPropertyManager::valueChanged, this, &QtPropertyHelper::listChanged);

		MANAGER_ARARY(managers);
		for(size_t i = 0; i < countOf(managers); ++i)
		{
			QObject::connect(managers[i], &QtAbstractPropertyManager::propertyRemoved, this, &QtPropertyHelper::propertyRemoved);
			managers[i]->setUserData(0, BLADE_NEW PropertyManagerData((int)i));
		}

		mTimerID = this->startTimer(PROPERTY_UPDATE_INTERVAL, Qt::CoarseTimer);

		mDoubleEditSliderFactory = QT_NEW QtDoubleEditSliderFactory(mParent);
		mLineEditFactory = QT_NEW QtLineEditFactory(mParent);
		mColorFactory = QT_NEW QtColorEditorFactory(mParent);
		mCheckBoxFactory = QT_NEW QtCheckBoxFactory(mParent);
		mComboBoxFactory = QT_NEW QtEnumEditorFactory(mParent);
		mFileEditFactory = QT_NEW FileEditFactory(mParent);
		mSliderFactory = QT_NEW QtEnumSliderFactory(mParent);
		mIntEditSliderFactory = QT_NEW QtIntEditSliderFactory(mParent);
		QObject::connect(mFileEditFactory, &FileEditFactory::propertyCallbackInvoked, this, &QtPropertyHelper::imageCallback);
	}

	//////////////////////////////////////////////////////////////////////////
	QtPropertyHelper::~QtPropertyHelper()
	{
		//although managers attached to parent, delete them before parent destruction is OK.
		delete mDoubleManager;
		delete mStringManager;
		delete mColorManager;
		delete mBoolManager;
		delete mEnumManager;
		delete mFilePathManager;
		delete mRangedEnumManager;

		this->clear();
		this->killTimer(mTimerID);
	}
		
	//////////////////////////////////////////////////////////////////////////
	void QtPropertyHelper::initialize(QtAbstractPropertyBrowser* browser)
	{
		assert(!mInited);
		mInited = true;
		browser->setFactoryForManager(mDoubleManager, mDoubleEditSliderFactory);
		browser->setFactoryForManager(mStringManager, mLineEditFactory);
		browser->setFactoryForManager(mColorManager, mColorFactory);
		browser->setFactoryForManager(mColorManager->subIntPropertyManager(), mIntEditSliderFactory);
		browser->setFactoryForManager(mEnumManager, mComboBoxFactory);
		browser->setFactoryForManager(mBoolManager, mCheckBoxFactory);
		browser->setFactoryForManager(mFilePathManager, mFileEditFactory);
		browser->setFactoryForManager(mRangedEnumManager, mSliderFactory);

		QObject::connect(browser, &QtAbstractPropertyBrowser::currentItemChanged, this, &QtPropertyHelper::currentItemChanged);
		mBrowser = browser;
		mBrowser->installEventFilter(this);
	}

	//////////////////////////////////////////////////////////////////////////
	QtProperty*	QtPropertyHelper::createProperty(IConfig* config, IConfig* parentConfig)
	{
		bool devMode = IEnvironmentManager::getSingleton().getVariable(ConstDef::EnvBool::DEVELOPER_MODE);
		CONFIG_ACCESS access = config->getAccess();
		if( (access&CAF_READWRITE) == 0 || (!devMode && (access&CAF_DEVMODE)) )
			return NULL;

		if(parentConfig != NULL && (parentConfig->getAccess()&CAF_WRITE) == 0 && (access&CAF_NOCASCADE) == 0)
			(int&)access &= ~CAF_WRITE;

		mPropCreating = true;

		QtProperty* prop = NULL;
		QString name = TString2QString(BTString2Lang(config->getName()));

		QtEnumPropertyManager* enumManager = NULL;
		
		EConfigUIHint hint = config->getUIHint();

		if (config->getBinaryValue().isCollection())
		{
			hint = CUIH_INPUT;
			if((access&CAF_COLLECTION_WRITE) != CAF_COLLECTION_WRITE)
				(int&)access &= ~CAF_WRITE;	//don't change original config's access.
		}

		if(!(access&CAF_WRITE))
			hint = CUIH_INPUT;
		switch(hint)
		{
		case CUIH_CHECK:
			prop = mBoolManager->addProperty(name);
			mBoolManager->setValue(prop, TStringHelper::toBool(config->getValue() ) );
			break;
		case CUIH_RANGE:
			enumManager = mRangedEnumManager;	//no break
		case CUIH_INDEX:
		case CUIH_LIST:
			{
				if(enumManager == NULL)
					enumManager = mEnumManager;
				prop = enumManager->addProperty(name);
				QStringList enumNames;
				enumNames.reserve( config->getOptions().size() );
				size_t index = INVALID_INDEX;
				for(size_t i = 0; i < config->getOptions().size(); ++i)
				{
					if(config->getOptions()[i] == config->getValue() )
						index = i;

					enumNames.append(TString2QString(BTString2Lang(config->getOptions()[i])) );
				}
				enumManager->setEnumNames(prop, enumNames);
				if(index != INVALID_INDEX)
					enumManager->setValue(prop, index);
			}
			break;
		case CUIH_COLOR:
			{
				prop = mColorManager->addProperty(name);
				prop->setDefaultExpanded(false);
				Color c = config->getBinaryValue();
				mColorManager->setValue(prop, Impl::getColor(c));
			}
			break;
		case CUIH_INPUT_RANGE:
			{
				scalar min = TStringHelper::toScalar( config->getOptions()[0] );
				scalar max = TStringHelper::toScalar( config->getOptions()[1] );
				scalar step = TStringHelper::toScalar( config->getOptions()[2] );
				prop = mDoubleManager->addProperty(name);
				mDoubleManager->setMinimum(prop, min);
				mDoubleManager->setMaximum(prop, max);
				mDoubleManager->setSingleStep(prop, step);
				mDoubleManager->setValue(prop, TStringHelper::toF64(config->getValue()));
			}
			break;
		case CUIH_FILE:
		case CUIH_PATH:
		case CUIH_IMAGE:
			{
				prop = mFilePathManager->addProperty(name);
				mFilePathManager->setReadOnly(prop, true);
				mFilePathManager->setType(prop, (hint == CUIH_FILE ? FT_FILE : (hint == CUIH_PATH ? FT_FOLDER : FT_CALLBACK)) );
				if(hint == CUIH_IMAGE)
				{
					ImageReceiver receiver;
					IMediaLibrary::getSingleton().getMediaFileInfo(config->getUIHint().getUserData(), config->getValue(), receiver);
					if(receiver.mDone)
					{
						QIcon icon;
						QtIconManager::getSingleton().fillIcon(&icon, receiver.mPreview);
						mFilePathManager->setIcon(prop, icon);

						//Handle<QImage> image = toQImage(receiver.mPreview);
						//QByteArray data;
						//QBuffer buffer(&data);
						//image->save(&buffer, "PNG", 100);
						//QString html = QString("<img src='data:image/png;base64, %0'/>").arg(QString(data.toBase64()));
						//prop->setToolTip(html);

						if(config->getValue() != receiver.mPath)
							config->setValue(receiver.mPath);
					}
				}
				mFilePathManager->setValue(prop, TString2QString(config->getValue()));
			}
			break;
		case CUIH_INPUT:
		default:
			prop = mStringManager->addProperty(name);
			mStringManager->setValue(prop, TString2QString(BTString2Lang(config->getValue())));
			bool readOnly = config->getUIHint() == CUIH_NONE || ((access&CAF_WRITE) == 0)
				|| (config->getSubConfigCount() != 0 && !config->getBinaryValue().isCollection())
				|| ((access&CAF_COLLECTION_WRITE)== 0 && config->getBinaryValue().isCollection());
			bool disabled = ((access&CAF_WRITE) == 0) && (config->getSubConfigCount() == 0 || !config->getBinaryValue().isCollection());
			mStringManager->setReadOnly(prop, readOnly);
			prop->setEnabled(!disabled);
		}

		if(prop != NULL)
			mMap[prop] = config;
		mPropCreating = false;
		return prop;
	}

	//////////////////////////////////////////////////////////////////////////
	void QtPropertyHelper::clear()
	{
		mMap.clear();
		MediaViewEvent state(TString::EMPTY, INVALID_INDEX);
		IEventManager::getSingleton().dispatchEvent(state);
	}

	//////////////////////////////////////////////////////////////////////////
	void QtPropertyHelper::initialize()
	{
		assert(!mInited);
		EDITFORFACTORY_ARRAY(factories);
		MANAGER_ARARY(managers);
		static_assert(sizeof(factories) == sizeof(managers), "invalid array");
		mLineEditFactory->setFocusCommit(false);
		for (size_t i = 0; i < countOf(managers); ++i)
		{
			switch(i)
			{
			case PMID_DOUBLE:
				static_cast<QtAbstractEditorFactory<QtDoublePropertyManager>*>(factories[i])->addPropertyManager(static_cast<QtDoublePropertyManager*>(managers[i]));
				break;
			case PMID_STRING:
				static_cast<QtAbstractEditorFactory<QtStringPropertyManager>*>(factories[i])->addPropertyManager(static_cast<QtStringPropertyManager*>(managers[i]));
				break;
			case PMID_COLOR:
				static_cast<QtAbstractEditorFactory<QtColorPropertyManager>*>(factories[i])->addPropertyManager(static_cast<QtColorPropertyManager*>(managers[i]));
				break;
			case PMID_BOOL:
				static_cast<QtAbstractEditorFactory<QtBoolPropertyManager>*>(factories[i])->addPropertyManager(static_cast<QtBoolPropertyManager*>(managers[i]));
				break;
			case PMID_RANGEENUM:	//no break
			case PMID_ENUM:
				static_cast<QtAbstractEditorFactory<QtEnumPropertyManager>*>(factories[i])->addPropertyManager(static_cast<QtEnumPropertyManager*>(managers[i]));
				break;
			case PMID_FILE:
				static_cast<QtAbstractEditorFactory<FilePathManager>*>(factories[i])->addPropertyManager(static_cast<FilePathManager*>(managers[i]));
				break;
			default:
				assert(false);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	QWidget* QtPropertyHelper::createEditor(IConfig* config, QWidget* parent)
	{
		//this is a total different mode
		assert(mBrowser == NULL);
		QtProperty* prop = this->createProperty(config, NULL);

		size_t index = INVALID_INDEX;
		EDITFORFACTORY_ARRAY(factories);
		MANAGER_ARARY(managers);
		static_assert(sizeof(factories) == sizeof(managers), "invalid array");
		for (size_t i = 0; i < countOf(managers); ++i)
		{
			if (prop->propertyManager() == managers[i])
			{
				if (managers[i] == mStringManager)
					prop->setBackupMode(QtProperty::BackupToCurrent);
				return factories[i]->createEditor(prop, parent);
			}
		}
		assert(false);
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	void QtPropertyHelper::timerEvent(QTimerEvent *evt)
	{
		mPropUpdating = true;

		//note: use ordered property list, because collection/group may delete its children on update
		//child properties always located after parents
		QList<QtProperty *> properties;
		properties.reserve(mMap.size());
		if (mBrowser != NULL)
		{
			QList<QtProperty *> topLevels = mBrowser->properties();
			TempQueue<QtProperty*> props;

			for (int i = 0; i < topLevels.size(); ++i)
				props.push_back(topLevels[i]);
			while (!props.empty())
			{
				QtProperty* prop = props.front();
				props.pop_front();
				properties.push_back(prop);

				QList<QtProperty *> subs = prop->subProperties();
				for (int i = 0; i < subs.size(); ++i)
					props.push_back(subs[i]);
			}
		}
		else
		{
			//stand alone mode: usually single property/config without collection/group, its save to update without order
			properties.reserve(mMap.size());
			for (PropertyMap::iterator i = mMap.begin(); i != mMap.end(); ++i)
				properties.push_back(i->first);
		}

		for(int i = 0; i < properties.size(); ++i)
		{
			PropertyMap::iterator iter = mMap.find(properties[i]);
			if(iter == mMap.end())	//deleted on parent update, skip
				continue;

			QtProperty* prop = iter->first;
			IConfig* config = iter->second;
			PropertyMap::iterator groupIter = mGroupMap.find(prop);
			if(!config->updateData(false, false) && groupIter == mGroupMap.end() && !config->getBinaryValue().isCollection())
				continue;
			//if(!prop->isEnabled() || config->isReadOnly() )	//read only data may also be changed, so update it.
			//	continue;
			if (config->getBinaryValue().isCollection())
				this->updateCollection(config, prop);
			if (groupIter != mGroupMap.end())
				this->updateGroup(config, prop);

			QString oldVal = prop->valueText();
			const TString& val = config->getValue();
			if(config->getUIHint() == CUIH_CHECK && val.compare_no_case(QString2TString(oldVal)) == 0)
				continue;
			if(val == (const tchar*)oldVal.constData())
				continue;
			QtAbstractPropertyManager* manager = prop->propertyManager();
			QObjectUserData* userData = manager->userData(0);
			if(userData == NULL)
				continue;

			int id = static_cast<PropertyManagerData*>(userData)->mID;
			QtEnumPropertyManager* enumManager = NULL;
			switch (id)
			{
			case PMID_DOUBLE:
				mDoubleManager->setValue(prop, TStringHelper::toF64(config->getValue()));
				break;
			case PMID_COLOR:
				{
					Color c = config->getBinaryValue();
					mColorManager->setValue(prop, Impl::getColor(c));
				}
				break;
			case PMID_BOOL:
				mBoolManager->setValue(prop, TStringHelper::toBool(config->getValue() ) );
				break;
			case PMID_ENUM:
				enumManager = mEnumManager;	//no break
			case PMID_RANGEENUM:
				if(enumManager == NULL)
					enumManager = mRangedEnumManager;
				{
					QStringList enumNames;
					enumNames.reserve( config->getOptions().size() );
					size_t index = INVALID_INDEX;
					for(size_t i = 0; i < config->getOptions().size(); ++i)
					{
						if(config->getOptions()[i] == config->getValue() )
							index = i;

						enumNames.append(TString2QString(BTString2Lang(config->getOptions()[i])) );
					}
					enumManager->setEnumNames(prop, enumNames);
					if(index != INVALID_INDEX)
						enumManager->setValue(prop, index);
				}
				break;
			case PMID_FILE:
				mFilePathManager->setValue(prop, TString2QString(config->getValue()));
				break;
			case PMID_STRING:
				mStringManager->setValue(prop, TString2QString(BTString2Lang(config->getValue())));
				break;
			default:
				assert(false);
			}

		}//for each property config pair
		mGroupMap.clear();
		mPropUpdating = false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool QtPropertyHelper::eventFilter(QObject *watched, QEvent *evt)
	{
		if (watched == mBrowser && evt->type() == QEvent::Show)
		{ 
			QtTreePropertyBrowser* b = qobject_cast<QtTreePropertyBrowser*>(mBrowser);
			if(b)
				b->adjustColumns(true);
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	void QtPropertyHelper::currentItemChanged(QtBrowserItem* qitem)
	{
		if(qitem == NULL)
			return;

		QtProperty* prop = qitem->property();
		IConfig* cfg = this->getConfig(prop);
		if(cfg == NULL)
			return;

		if(cfg->getUIHint() == CUIH_IMAGE && !cfg->getBinaryValue().isCollection())
		{
			ImageReceiver receiver;
			IMediaLibrary::getSingleton().getMediaFileInfo(cfg->getUIHint().getUserData(), cfg->getValue(), receiver);
			if(receiver.mDone)
			{
				MediaViewEvent state(receiver.mPath, receiver.mMediaIndex);
				IEventManager::getSingleton().dispatchEvent(state);
			}
		}
		else
		{
			MediaViewEvent state(TString::EMPTY, INVALID_INDEX);
			IEventManager::getSingleton().dispatchEvent(state);
		}

		if( cfg->getDataHandler() != NULL && cfg->getDataHandler()->getTarget() != NULL )
		{
			const TString& value = cfg->getValue();
			const Bindable* bindable = cfg->getDataHandler()->getTarget();
			OptionHighLightEvent state(bindable, value, cfg->getUIHint().getUserData());
			IEventManager::getSingleton().dispatchEvent(state);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void QtPropertyHelper::propertyRemoved(QtProperty* prop, QtProperty* /*parent*/)
	{
		mMap.erase(prop);
	}

	//////////////////////////////////////////////////////////////////////////
	void QtPropertyHelper::imageCallback(QtProperty* prop, QString& path, bool& done)
	{
		IConfig* cfg = this->getConfig(prop);
		if(cfg == NULL || cfg->getUIHint() != CUIH_IMAGE)
		{
			done = false;
			return;
		}
		struct ImageReceiverNoModal : public ImageReceiver, public Allocatable
		{
			FilePathManager* fileManager;
			QtProperty*		prop;
			Lock			reentrantLock;
			ImageReceiverNoModal(FilePathManager* mgr, QtProperty* p) :fileManager(mgr),prop(p)	{}
			virtual void	setSelectedTarget(index_t mediaIndex, const IMediaLibrary::MediaFile& media)
			{
				ScopedLock ls(reentrantLock);
				ImageReceiver::setSelectedTarget(mediaIndex, media);
				if(mDone)
				{
					QIcon icon;
					QtIconManager::getSingleton().fillIcon(&icon, mPreview);
					fileManager->setValue(prop, TString2QString(mPath));
					fileManager->setIcon(prop, icon);

					MediaViewEvent state(mPath, mMediaIndex);
					IEventManager::getSingleton().dispatchEvent(state);
				}
				BLADE_DELETE this;
			}
			virtual void canceled()
			{
				if(!reentrantLock.isLocked())	//assertion dialog/focus lost causing setSelectedTarget calling into canceled
					BLADE_DELETE this;
			}
		};

		ImageReceiverNoModal* receiver = BLADE_NEW ImageReceiverNoModal(mFilePathManager, prop);
		IUIMediaLibrary::getSingleton().showMediaLibrary(cfg->getUIHint().getUserData(), cfg->getValue(), *receiver);
		done = false;
	}

	//////////////////////////////////////////////////////////////////////////
	void QtPropertyHelper::listChanged(QtProperty *prop, int val)
	{
		if(mPropUpdating)
			return;

		IConfig* cfg = this->getConfig(prop);
		if(cfg == NULL)
			return;
		assert(cfg->getUIHint() == CUIH_LIST || cfg->getUIHint() == CUIH_INDEX || cfg->getUIHint() == CUIH_RANGE);
		assert(val < (int)cfg->getOptions().size());
		cfg->setValue( cfg->getOptions()[(index_t)val] );
		this->checkGroupChange(cfg, prop);
	}

	//////////////////////////////////////////////////////////////////////////
	void QtPropertyHelper::checkChanged(QtProperty *prop, bool val)
	{
		if(mPropUpdating)
			return;

		IConfig* cfg = this->getConfig(prop);
		if(cfg == NULL)
			return;
		TString b = TStringHelper::fromBool(val);
		cfg->setValue(b);
		this->checkGroupChange(cfg, prop);
	}

	//////////////////////////////////////////////////////////////////////////
	void QtPropertyHelper::colorChanged(QtProperty* prop, const QColorHDR &val)
	{
		if(mPropUpdating)
			return;

		IConfig* cfg = this->getConfig(prop);
		if(cfg == NULL)
			return;
		Color c(val.redF(), val.greenF(), val.blueF(), 0);
		c *= val.m_HDRScale;
		c[3] = val.alphaF();
		cfg->setBinaryValue(c);
	}

	//////////////////////////////////////////////////////////////////////////
	void QtPropertyHelper::editChanged(QtProperty* prop, const QString& val)
	{
		if(mPropUpdating)
			return;

		IConfig* cfg = this->getConfig(prop);
		if(cfg == NULL)
			return;
		cfg->setValue(QString2TString(val));
		//note: deleting row in edit state will f**k up browser (QTreeWidget,QAbstractItemView or somthing else)'s editing state
		//and Return Key routed to parent dialog, causing dialog ended.
		//so handle collection elements count changes in timerEvent() update.
		//cfg->updateData(true, false);
		//this->updateCollection(cfg, prop);
	}

	//////////////////////////////////////////////////////////////////////////
	void QtPropertyHelper::rangeChanged(QtProperty* prop, double value)
	{
		if(mPropUpdating)
			return;

		IConfig* cfg = this->getConfig(prop);
		if(cfg == NULL)
			return;
		cfg->setValue(TStringHelper::fromF64(value));
	}
	
	//////////////////////////////////////////////////////////////////////////
	void QtPropertyHelper::fileChanged(QtProperty* prop, const QString& file)
	{
		if(mPropUpdating)
			return;

		IConfig* cfg = this->getConfig(prop);
		if(cfg == NULL)
			return;
		cfg->setValue(QString2TString(file));
	}

	//////////////////////////////////////////////////////////////////////////
	void QtPropertyHelper::updateCollection(IConfig* config, QtProperty* prop)
	{
		if (config->getBinaryValue().isCollection())
		{
			QList<QtProperty *> subProps = prop->subProperties();

			if (subProps.size() == config->getSubConfigCount())
				return;

			//delete original
			for (int i = 0; i < subProps.size(); ++i)
				QtPropertyHelper::removeProperty(subProps[i], prop);	//this will call QtPropertyHelper::propertyRemoved. 
																		//its OK to remove element(not the current iterator element)  during iteration

			//create new
			for (size_t i = 0; i < config->getSubConfigCount(); ++i)
			{
				IConfig* subCfg = config->getSubConfig(i);
				QtProperty* subProp = this->createProperty(subCfg, config);
				prop->addSubProperty(subProp);

				//recursive update
				if (subCfg->getBinaryValue().isCollection())
					this->updateCollection(subCfg, subProp);
				else if (subCfg->getSubConfigCount() > 0)
					this->updateGroup(subCfg, subProp);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void QtPropertyHelper::updateGroup(IConfig* config, QtProperty* prop)
	{
		QList<QtProperty *> subProps = prop->subProperties();
		for (int i = 0; i < subProps.size(); ++i)
			QtPropertyHelper::removeProperty(subProps[i], prop);	//this will call QtPropertyHelper::propertyRemoved. 
																	//its OK to remove element(not the current iterator element)  during iteration

		for (size_t i = 0; i < config->getSubConfigCount(); ++i)
		{
			IConfig* subCfg = config->getSubConfig(i);
			//rebind switched sub config to target if they are not bound
			if(config->getDataHandler() != NULL)
				subCfg->bindTarget(config->getDataHandler()->getTarget());
			QtProperty* subProp = this->createProperty(subCfg, config);
			prop->addSubProperty(subProp);

			//recursive update
			if (subCfg->getBinaryValue().isCollection())
				this->updateCollection(subCfg, subProp);
			else if (subCfg->getSubConfigCount() > 0)
				this->updateGroup(subCfg, subProp);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void QtPropertyHelper::checkGroupChange(IConfig* config, QtProperty* prop)
	{
		if (prop->subProperties().size() >= 1 /* multiple children existing, count may change */||  config->getSubConfigCount() >= 1 /* multiple children after changed */)
			mGroupMap[prop] = config;
	}

	//////////////////////////////////////////////////////////////////////////
	void QtPropertyHelper::removeProperty(QtProperty* prop, QtProperty* parent)
	{
		QList<QtProperty *> subProps = prop->subProperties();
		//deep-first
		for (int i = 0; i < subProps.size(); ++i)
			QtPropertyHelper::removeProperty(subProps[i], prop);

		parent->removeSubProperty(prop);
		BLADE_DELETE prop;
	}

}//namespace Blade