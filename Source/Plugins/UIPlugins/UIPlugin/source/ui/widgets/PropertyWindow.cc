/********************************************************************
	created:	2011/05/12
	filename: 	PropertyWindow.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include "stdafx.h"
#include "resource.h"
#include "PropertyWindow.h"
#include <interface/IEventManager.h>

#include <interface/public/ui/UIEvents.h>
#include <ConfigTypes.h>
#include <interface/IEntity.h>

namespace Blade
{

	//
	BEGIN_MESSAGE_MAP(PropertyWindow, DockWindowBase<IUIPropertyGrid>)
		ON_WM_WINDOWPOSCHANGING()
		ON_WM_SHOWWINDOW()
	END_MESSAGE_MAP()

	namespace PropertyString
	{
		static const TString ENTITY_CONFIG_NAME = BTString("IEntity");
		static const TString NAME = BTString(BLANG_NAME);

	}//namespace PropertyString

	const TString PropertyWindow::WIDGET_TYPE = BXLang(BLANG_PROPERTY);

	//////////////////////////////////////////////////////////////////////////
	PropertyWindow::PropertyWindow()
		:DockWindowBase(WIDGET_TYPE)
	{
		IEventManager::getSingleton().addEventHandler(EntitySlectedEvent::NAME, EventDelegate(this, &PropertyWindow::onBindProperty));
		IEventManager::getSingleton().addEventHandler(ConfigurableSlectedEvent::NAME, EventDelegate(this, &PropertyWindow::onBindProperty));

		ConfigGroup* group = BLADE_NEW ConfigGroup(PropertyString::ENTITY_CONFIG_NAME);
		mRootEntityConfig.bind(group);

		mTargetEntity = NULL;
		mTimer = ITimeDevice::create();

		IFramework::getSingleton().addUpdater(this);
	}

	//////////////////////////////////////////////////////////////////////////
	PropertyWindow::~PropertyWindow()
	{
		IEventManager::getSingleton().removeEventHandlers(EntitySlectedEvent::NAME, this);
		IEventManager::getSingleton().removeEventHandlers(ConfigurableSlectedEvent::NAME, this);

		IFramework::getSingleton().removeUpdater(this);
		this->clearBinding();
		BLADE_DELETE mTimer;
	}

	//////////////////////////////////////////////////////////////////////////
	void	PropertyWindow::update()
	{
		AFX_MANAGE_STATE(::AfxGetStaticModuleState());
		if (this->GetSafeHwnd() == NULL || !this->IsWindowVisible())
			return;

		if (mTargetEntity == NULL)
			mPropertyGrid.DeleteAllItems();
		else
		{
			mTimer->update();
			if (mTimer->getMilliseconds() > 200)
			{
				mTimer->reset();
				mPropertyGrid.refresh();
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	BOOL PropertyWindow::Create(LPCTSTR lpszText, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID/* = 0xffff*/)
	{
		if (!CStatic::Create(lpszText, dwStyle, rect, pParentWnd, nID))
			return FALSE;

		CRect rcTitle(0, 0, rect.right, 20);
		mTitle.Create(TEXT(""), WS_CHILD | WS_VISIBLE, rcTitle, this);

		CRect rcOption(0, 20, rect.right, rect.bottom);
		mPropertyGrid.Create(this, rcOption, IDC_OPTION_LIST);
		//expand all by default
		mPropertyGrid.setDefaultExpand(true);

		mPropertyGrid.ShowWindow(SW_SHOW);

		return TRUE;
	}

	//////////////////////////////////////////////////////////////////////////
	void		PropertyWindow::ShowProperty(Blade::IConfig* config/* = NULL*/)
	{
		mPropertyGrid.SetRedraw(FALSE);
		mPropertyGrid.DeleteAllItems();
		if (config != NULL)
			mPropertyGrid.setSubConfigs(config);

		mPropertyGrid.SetRedraw(TRUE);
	}


	//////////////////////////////////////////////////////////////////////////
	void		PropertyWindow::OnWindowPosChanging(WINDOWPOS* lpwndpos)
	{
		if (!(lpwndpos->flags & SWP_NOSIZE) && mPropertyGrid.GetSafeHwnd() != NULL)
		{
			mTitle.MoveWindow(0, 0, lpwndpos->cx, 20);
			CRect rect(0, 20, lpwndpos->cx, lpwndpos->cy);
			mPropertyGrid.MoveWindow(&rect);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void		PropertyWindow::OnShowWindow(BOOL bShow, UINT nStatus)
	{
		DockWindowBase::OnShowWindow(bShow, nStatus);
		if (bShow && mTargetEntity != NULL)
		{
			mRootEntityConfig->updateData(true);
			mPropertyGrid.DeleteAllItems();
			mPropertyGrid.setSubConfigs(mRootEntityConfig);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void		PropertyWindow::onBindProperty(const Blade::Event& data)
	{
		AFX_MANAGE_STATE(::AfxGetStaticModuleState());

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

	//////////////////////////////////////////////////////////////////////////
	void		PropertyWindow::clearBinding()
	{
		for (BinderList::iterator i = mConfigBinder.begin(); i != mConfigBinder.end(); ++i)
		{
			(*i)->unbindTarget();
			BLADE_DELETE *i;
		}
		mConfigBinder.clear();
	}

	//////////////////////////////////////////////////////////////////////////
	bool		PropertyWindow::bindEntity(const IEntity* entity)
	{
		mPropertyGrid.DeleteAllItems();
		mRootEntityConfig->clearSubConfigs();
		this->clearBinding();
		mTargetEntity = entity;

		if (mTargetEntity != NULL)
		{
			//entity config
			{
				ConfigAtom* nameConfig = BLADE_NEW ConfigAtom(PropertyString::NAME, mTargetEntity->getName(), CAF_READ);
				nameConfig->setUIHint(CUIH_NONE);
				HCONFIG hConfig;
				hConfig.bind(nameConfig);
				mRootEntityConfig->addSubConfig(hConfig);
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
					mRootEntityConfig->addSubConfig(config);
					mConfigBinder.push_back(binder);
				}
				else
				{
					assert(false);
					BLADE_DELETE binder;
					return false;
				}
			}

			if (this->IsWindowVisible())
			{
				mRootEntityConfig->updateData(true);
				mPropertyGrid.setSubConfigs(mRootEntityConfig);
				mPropertyGrid.refresh();
			}
		}
		return true;
	}
	
}//namespace Blade