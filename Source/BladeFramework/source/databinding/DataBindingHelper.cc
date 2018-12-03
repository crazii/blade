/********************************************************************
	created:	2016/01/03
	filename: 	DataBindingHelper.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <databinding/DataBindingHelper.h>
#include <databinding/IDataSourceDataBase.h>
#include "DataBindingHelper_Private.h"
#include "../interface_imp/Entity.h"

namespace Blade
{

	//////////////////////////////////////////////////////////////////////////
	Variant	DataBindingHelper::getBindingData(Bindable* target, const TString& targetClassType, const TString& dataName)
	{
		if( target == NULL )
		{
			assert(false);
			return Variant();
		}

		DataBinder binder;
		binder.bindTarget(targetClassType, target);
		if( !binder.isBound() )
		{
			assert(false);
			return Variant();
		}

		const HCONFIG& config = binder.getConfig();
		const HCONFIG& targetConfig = config->getSubConfigByName(dataName);
		if( targetConfig == NULL )
		{
			assert(false);
			return Variant();
		}

		return targetConfig->getBinaryValue();
	}

	//////////////////////////////////////////////////////////////////////////
	bool		DataBindingHelper::setBindingData(Bindable* target, const TString& targetClassType, const TString& dataName, const Variant& val)
	{
		if( target == NULL )
		{
			assert(false);
			return false;
		}

		DataBinder binder;
		binder.bindTarget(targetClassType, target);
		if( !binder.isBound() )
		{
			assert(false);
			return false;
		}

		const HCONFIG& config = binder.getConfig();
		const HCONFIG& targetConfig = config->getSubConfigByName(dataName);
		if( targetConfig == NULL )
		{
			assert(false);
			return false;
		}
		return targetConfig->setBinaryValue(val);
	}

	//////////////////////////////////////////////////////////////////////////
	Handle<DataBinder> DataBindingHelper::bindEntity(const IEntity* entity)
	{
		Entity* entityImpl = static_cast<Entity*>(const_cast<IEntity*>(entity));	//dirty cast
		if (entityImpl == NULL || entityImpl->getStatus() == ES_DELETE_LOADING)
			return Handle<DataBinder>::EMPTY;

		return Handle<DataBinder>(BLADE_NEW DataBinder(ENTITY_BNDING_CONFIG, entityImpl));
	}

	//////////////////////////////////////////////////////////////////////////
	bool DataBindingHelper::addDelegate(Bindable* target, const TString& targetClassType, const TString& dataName, const Delegate& d)
	{
		if (target == NULL)
		{
			assert(false);
			return false;
		}

		DataBinder binder;
		binder.bindTarget(targetClassType, target);
		if (!binder.isBound())
		{
			assert(false);
			return false;
		}

		const HCONFIG& config = binder.getConfig();
		const HCONFIG& targetConfig = config->findConfig(dataName);
		if (targetConfig == NULL)
		{
			assert(false);
			return false;
		}

		return DatabindingDelegateRouter::getSingleton().add(target, dataName, d);
	}

	//////////////////////////////////////////////////////////////////////////
	bool DataBindingHelper::removeDelegate(Bindable* target, const TString& targetClassType, const TString& dataName, void* ptr)
	{
		if (target == NULL)
		{
			assert(false);
			return false;
		}

		DataBinder binder;
		binder.bindTarget(targetClassType, target);
		if (!binder.isBound())
		{
			assert(false);
			return false;
		}

		const HCONFIG& config = binder.getConfig();
		const HCONFIG& targetConfig = config->findConfig(dataName);
		if (targetConfig == NULL)
		{
			assert(false);
			return false;
		}

		return DatabindingDelegateRouter::getSingleton().remove(target, dataName, ptr);
	}
	
}//namespace Blade