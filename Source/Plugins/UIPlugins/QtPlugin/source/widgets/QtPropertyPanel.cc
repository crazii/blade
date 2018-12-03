/********************************************************************
	created:	2016/7/26
	filename: 	QtPropertyPanel.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <QtPluginPCH.h>
#include <interface/IEntity.h>
#include <ConfigTypes.h>
#include <widgets/QtPropertyPanel.h>
#include <interface/IEventManager.h>
#include <interface/public/ui/UIEvents.h>
#include <databinding/DataBindingHelper.h>

namespace Blade
{
	namespace PropertyString
	{
		static const TString ENTITY_CONFIG_NAME = BTString("IEntity");
		static const TString NAME = BTString(BLANG_NAME);
	}//namespace PropertyString

	const TString QtPropertyPanel::WIDGET_TYPE = BTString(BLANG_PROPERTY);
	
	//////////////////////////////////////////////////////////////////////////
	QtPropertyPanel::QtPropertyPanel()
		:QtWidgetBase<IUIPropertyGrid>(WIDGET_TYPE)
		,mPropGrid(this)
		,mTargetEntity(NULL)
	{
		this->setWidget(mPropGrid.getWidget());

		IEventManager::getSingleton().addEventHandler(EntitySlectedEvent::NAME, EventDelegate(this, &QtPropertyPanel::onBindProperty));
		IEventManager::getSingleton().addEventHandler(ConfigurableSlectedEvent::NAME, EventDelegate(this, &QtPropertyPanel::onBindProperty));

		ConfigGroup* group = BLADE_NEW ConfigGroup(PropertyString::ENTITY_CONFIG_NAME);
		mRootConfig.bind(group);
	}

	//////////////////////////////////////////////////////////////////////////
	QtPropertyPanel::~QtPropertyPanel()
	{
		IEventManager::getSingleton().removeEventHandlers(this);
		this->clear();
	}

	/************************************************************************/
	/* IUIPropertyGrid interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool	QtPropertyPanel::bindEntity(const IEntity* entity)
	{
		this->clear();
		mTargetEntity = entity;

		if (mTargetEntity != NULL)
		{
			//entity config
			{
				Handle<DataBinder> binder = DataBindingHelper::bindEntity(mTargetEntity);
				if (binder != NULL && binder->isBound())
				{
					const HCONFIG& config = binder->getConfig();
					mRootConfig->addSubConfig(config);
					mConfigBinder.push_back(binder.unbind());
				}
			}

			ElementListParam list;
			TStringParam param;
			mTargetEntity->getAllElements(param, list);

			for (size_t i = 0; i < list.size(); ++i)
			{
				IElement* element = list[i];

				DataBinder* binder = BLADE_NEW DataBinder();
				binder->bindTarget(element->getType(), element);
				if (binder->isBound())
				{
					const HCONFIG& config = binder->getConfig();
					mRootConfig->addSubConfig(config);
					mConfigBinder.push_back(binder);
				}
				else
				{
					assert(false);
					BLADE_DELETE binder;
					return false;
				}
			}

			if (this->QWidget::isVisible())
			{
				mRootConfig->updateData(true);
				mPropGrid.bindConfigs(mRootConfig);
			}
		}
		return true;
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void	QtPropertyPanel::clear()
	{
		for (BinderList::iterator i = mConfigBinder.begin(); i != mConfigBinder.end(); ++i)
		{
			(*i)->unbindTarget();
			BLADE_DELETE *i;
		}
		mConfigBinder.clear();
		mPropGrid.bindConfigs(NULL);
		mRootConfig->clearSubConfigs();
	}

	//////////////////////////////////////////////////////////////////////////
	void	QtPropertyPanel::onBindProperty(const Event& data)
	{
		if (data.mName == EntitySlectedEvent::NAME)
		{
			const EntitySlectedEvent& state = static_cast<const EntitySlectedEvent&>(data);
			if (mTargetEntity == state.mEntity)
				return;
			this->bindEntity(state.mEntity);
		}
		else if (data.mName == ConfigurableSlectedEvent::NAME)
		{
			assert(false && "not implemented yet");
		}
	}
	
}//namespace Blade