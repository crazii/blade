/********************************************************************
	created:	2010/08/11
	filename: 	IEntity.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_IEntity_h__
#define __Blade_IEntity_h__
#include <BladeFramework.h>
#include <utility/StringList.h>
#include <interface/public/IElement.h>

namespace Blade
{
	class ElementList;
	class IStage;

	typedef TPointerParam<IElement> ElementListParam;
	typedef TPointerParam<Interface> InterfaceListParam;

	class IEntity
	{
	public:
		///resource type & serializer type
		static BLADE_FRAMEWORK_API const TString ENTITY_RESOURCE_TYPE;
		///file name extension/suffix
		static BLADE_FRAMEWORK_API const TString ENTITY_RESOURCE_EXTENSION;

	public:
		virtual ~IEntity()	{}

		/**
		@describe 
		@param
		@return
		*/
		virtual const TString&	getName() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual IStage*		getStage() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void		removeElement(const TString& name) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual IElement*	getElement(const TString& name) const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual size_t		getAllElements(TStringParam& nameList, ElementListParam& elemList) const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual size_t		getElementsByType(ElementListParam& elemList, const TString& type, TStringParam* nameList = NULL) const = 0;
		
		/** @brief  */
		inline IElement*	getElementByType(const TString& type)
		{
			ElementListParam list;
			this->getElementsByType(list, type);
			return list.size() > 0 ? list[0] : NULL;
		}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual size_t		getElementsBySystemType(ElementListParam& elemList, const TString& sytemType) const = 0;
		inline IElement*	getElementBySystemType(const TString& systemType, const size_t* verityCount = NULL)
		{
			ElementListParam list;
			if (this->getElementsBySystemType(list, systemType))
			{
				if (verityCount != NULL)
					assert(*verityCount == list.size());
				return list[0];
			}
			return NULL;
		}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void		addElement(const TString& name,const HELEMENT& elem) = 0;

		/**
		@describe get interface supported by elements
		@param
		@return
		*/
		virtual bool		getInterfaces(InterfaceName type, InterfaceListParam& interfaceList) const = 0;
		
		inline Interface*	getInterface(InterfaceName type)
		{
			InterfaceListParam list;
			this->getInterfaces(type, list);
			return list.size() > 0 ? list[0] : NULL;
		}

		template<typename T>
		inline T*			getInterface(InterfaceID<T>)
		{
			return static_cast<T*>(this->getInterface(InterfaceID<T>::getName()));
		}

	protected:
		/** @brief  */
		static inline void	takeOwnership(IElement* element, IEntity* entity)
		{
			assert(element->getEntity() == NULL || element->getEntity() == entity);
			element->setEntity(entity);
		}

		/** @brief  */
		static inline void initParallelStates(IElement* element)
		{
			assert(element->getEntity() != NULL);
			element->initParallelStates();
		}
	};//class IEntity

	typedef Handle<IEntity> HENTITY;
	
}//namespace Blade


#endif //__Blade_IEntity_h__