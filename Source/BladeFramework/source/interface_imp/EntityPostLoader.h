/********************************************************************
	created:	2013/02/26
	filename: 	EntityPostLoader.h
	author:		Crazii
	purpose:	stage - related post processing after entity resoruce is serialized
*********************************************************************/
#ifndef __Blade_EntityPostLoader_h__
#define __Blade_EntityPostLoader_h__
#include <utility/BladeContainer.h>
#include <interface/public/IResource.h>
#include <interface/IEntity.h>
#include <ResourceState.h>
#include <EntityResourceDesc.h>

namespace Blade
{
	class Stage;

	class EntityPostLoader : public ResourceStateGroup, public TempAllocatable
	{
	public:
		EntityPostLoader(Stage* stage, const EntityResourceDesc* desc);
		~EntityPostLoader();

		/************************************************************************/
		/* IResource::IListener interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param resource maybe NULL when loading failed
		@return 
		*/
		virtual void	postLoad(const HRESOURCE& resource);

		/*
		@describe when loading succeed
		@param
		@return
		*/
		virtual bool	onReady();

		/*
		@describe when loading failed
		@param
		@return 
		*/
		virtual bool	onFailed();

		/*
		@describe if loadAll is true, load all elements for entity, use desc as optional. Otherwise load elements only present in desc
		@param 
		@return 
		*/
		static void	loadElementsSync(IEntity* entity, const EntityResourceDesc* desc, const ProgressNotifier& notifier = ProgressNotifier::EMPTY, bool loadAll = true);

	protected:
		Stage*		mStage;
		HRESOURCE	mResource;
		size_t		mEntityLoaded;
		EntityResourceDesc	mDesc;
	};


	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class ElementPostLoader : public ResourceStateGroup, public TempAllocatable
	{
	public:
		ElementPostLoader(Stage* stage, IEntity* entity, const EntityResourceDesc* desc);
		ElementPostLoader(Stage* stage, IEntity* entity, IElement* element);
		~ElementPostLoader();

		/************************************************************************/
		/* IResource::IListener interface                                                                     */
		/************************************************************************/
		/*
		@describe when loading succeed
		@param
		@return
		*/
		virtual bool	onReady();

		/*
		@describe when loading failed
		@param
		@return
		*/
		virtual bool	onFailed();

		/*
		@describe only load elements present in desc.
		@param
		@return
		*/
		static bool	loadElements(Stage* stage, IEntity* entity, const EntityResourceDesc& desc, bool forceAsync);

		/**
		@describe 
		@param 
		@return 
		*/
		static bool loadElement(IElement* element, bool forceAsync);

	protected:
		/**
		@describe 
		@param
		@return
		*/
		bool loadElements();

		/**
		@describe 
		@param 
		@return 
		*/
		bool loadElement();

		Stage*		mStage;
		IEntity*	mEntity;
		IElement*	mSingleElement;
		EntityResourceDesc	mDesc;
	};
	
}//namespace Blade

#endif //  __Blade_EntityPostLoader_h__