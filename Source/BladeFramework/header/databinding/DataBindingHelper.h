/********************************************************************
	created:	2016/01/03
	filename: 	DataBindingHelper.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_DataBindingHelper_h__
#define __Blade_DataBindingHelper_h__
#include <BladeFramework.h>
#include <utility/Variant.h>
#include <utility/Delegate.h>
#include <utility/Bindable.h>
#include <interface/public/IElement.h>
#include <interface/IEntity.h>

namespace Blade
{
	class DataBinder;

	namespace DataBindingHelper
	{

		/** @brief  */
		BLADE_FRAMEWORK_API Variant	getBindingData(Bindable* target, const TString& targetClassType, const TString& dataName);

		inline Variant getElementData(IElement* element, const TString& dataName)
		{
			return getBindingData(element, element->getType(), dataName);
		}

		inline Variant getElementData(IEntity& entity, const TString& elementType, const TString& dataName)
		{
			return getElementData(entity.getElementByType(elementType), dataName);
		}


		/** @brief  */
		BLADE_FRAMEWORK_API bool		setBindingData(Bindable* target, const TString& targetClassType, const TString& dataName, const Variant& val);

		inline bool setElementData(IElement* element, const TString& dataName, const Variant& val)
		{
			return setBindingData(element, element->getType(), dataName, val);
		}

		inline Variant setElementData(IEntity& entity, const TString& elementType, const TString& dataName, const Variant& val)
		{
			return setElementData(entity.getElementByType(elementType), dataName, val);
		}


		/** @brief used for editor */
		BLADE_FRAMEWORK_API Handle<DataBinder> bindEntity(const IEntity* entity);

		/** @brief  */
		BLADE_FRAMEWORK_API	bool addDelegate(Bindable* target, const TString& targetClassType, const TString& dataName, const Delegate& d);

		inline bool addElementDelegate(IElement* element, const TString& dataName, const Delegate& d)
		{
			return addDelegate(element, element->getType(), dataName, d);
		}

		inline bool addElementDelegate(IEntity& entity, const TString& elementType, const TString& dataName, const Delegate& d)
		{
			return addElementDelegate(entity.getElementByType(elementType), dataName, d);
		}

		BLADE_FRAMEWORK_API	bool removeDelegate(Bindable* target, const TString& targetClassType, const TString& dataName, void* ptr);

		inline bool removeDelegate(IElement* element, const TString& dataName, void* ptr)
		{
			return removeDelegate(element, element->getType(), dataName, ptr);
		}

		inline bool removeDelegate(IEntity& entity, const TString& elementType, const TString& dataName, void* ptr)
		{
			return removeDelegate(entity.getElementByType(elementType), dataName, ptr);
		}

	}//namespace DatabindingHelper
	
}//namespace Blade


#endif // __Blade_DataBindingHelper_h__