/********************************************************************
	created:	2013/02/26
	filename: 	EntityPostLoader.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <interface/IResourceManager.h>
#include "EntityPostLoader.h"
#include "EntityResource.h"
#include "Stage.h"
#include "Entity.h"

namespace Blade
{
	class ElementResourceState : public SubResourceState, public TempAllocatable
	{
	public:
		inline ElementResourceState(Entity* entity, const TString& elementName, const ParamList& params)
			:SubResourceState()
			,mElementName(elementName)
			,mEntity(entity)
			,mParams(params)
		{}

		/** @brief  */
		virtual void generateParams(ParamList& params) const
		{
			params = mParams;
		}

		/** @brief  */
		inline IElement*	getElement() const
		{
			return mEntity->getElement(mElementName);
		}
		/** @brief  */
		inline const HRESOURCE&	getElementResource() const {return mResource;}
		/** @brief  */
		inline void			setResource(const HRESOURCE& resource)
		{
			assert(resource != NULL);
			mLoadState = LS_LOADED;
			if (mResource != resource && mResource != NULL)
				IResourceManager::getSingleton().unloadResource(mResource);
			mResource = resource;
		}
		virtual bool		onReady()
		{
			IElement* element = this->getElement();
			if (element != NULL)	//deleted ?
				element->setBoundResource(mResource);
			return ResourceState::onReady();
		}
		virtual bool		onFailed()
		{
			IElement* element = this->getElement();
			if (element != NULL)	//deleted ?
				element->setBoundResource( HRESOURCE::EMPTY );
			return ResourceState::onFailed();
		}
	protected:
		TString	mElementName;	//use element name in case element deleted
		Entity* mEntity;
		ParamList mParams;
	};

	struct ElementResource
	{
		IElement*	element;
		TString		resource;

		inline bool operator<(const ElementResource& rhs) const
		{
			return FnSerializableSorter::compare(element, rhs.element);
		}
	};
	typedef TempVector<ElementResource>  ElementResourceList;

	//////////////////////////////////////////////////////////////////////////
	EntityPostLoader::EntityPostLoader(Stage* stage, const EntityResourceDesc* desc)
		:mStage(stage)
		,mEntityLoaded(0)
	{
		if(desc != NULL)
			mDesc = *desc;
	}

	//////////////////////////////////////////////////////////////////////////
	EntityPostLoader::~EntityPostLoader()
	{
		this->clearAndDeleteSubStates();
	}

	/************************************************************************/
	/* IResource::IListener interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void	EntityPostLoader::postLoad(const HRESOURCE& resource)
	{
		mResource = resource;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	EntityPostLoader::onReady()
	{
		ResourceStateGroup::onReady();
		if( !this->isLoaded() )
			return true;

		EntityResource* res = static_cast<EntityResource*>(mResource);
		assert( res->getEntity() != NULL );
		Entity* entity = static_cast<Entity*>(res->getEntity());

		if (mEntityLoaded == 0)
		{
			Lock::set(&mEntityLoaded, 1);
			assert(mResource != NULL);
			assert(mStage != NULL);
			assert(this->getSubStateCount() == 0);
			const Entity::ElementMap& elements = entity->getElements();

			size_t count = 0;
			for (Entity::ElementMap::const_iterator i = elements.begin(); i != elements.end(); ++i)
			{
				IElement* element = i->second;
				const TString& elementName = i->first;

				TString elementResource = element->getResourcePath();

				TString descResource;
				bool bOverride = false;
				if (mDesc.getElementResource(elementName, descResource, bOverride) && (bOverride || elementResource == TString::EMPTY))
					elementResource = descResource;

				if (elementResource == TString::EMPTY)
					continue;

				ParamList params;
				TString type;
				TString loaderType;
				element->getResourceParams(params, type, loaderType);
				ElementResourceState* state = BLADE_NEW ElementResourceState(entity, elementName, params);
				state->setResourceInfo(type, elementResource, loaderType);
				if (element->getBoundResource() != NULL)
					state->setResource(element->getBoundResource());

				this->addSubState(state);
				++count;
			}

			entity->postLoad(true);
			if (count > 0)
			{
				//load element resources
				this->reload();
				return true;
			}
		}
		else
			entity->postProcess();

		//entity loaded & element resource loaded, final dispatch
		assert( mEntityLoaded == 1 );

		mStage->setEntityState(res->getEntity(), ES_READY, 0, &mResource);

		BLADE_DELETE this;
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	EntityPostLoader::onFailed()
	{
		ResourceStateGroup::onFailed();
		EntityResource* res = static_cast<EntityResource*>(mResource);
		assert(res->getEntity() != NULL);
		Entity* entity = static_cast<Entity*>(res->getEntity());
		entity->postLoad(false);

		BLADE_DELETE this;
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	void	EntityPostLoader::loadElementsSync(IEntity* entity, const EntityResourceDesc* desc, const ProgressNotifier& notifier/* = ProgressNotifier::EMPTY*/, bool loadAll/* = false*/)
	{
		if (desc == NULL && !loadAll)
		{
			notifier.onNotify(1.0f);
			return;
		}
		const Entity::ElementMap& elements = static_cast<Entity*>(entity)->getElements();

		ElementResourceList list;

		size_t count = elements.size();
		list.reserve(count);
		size_t n = 0;
		for(Entity::ElementMap::const_iterator i = elements.begin(); i != elements.end(); ++i)
		{
			IElement* element = i->second;
			const TString& elementName = i->first;

			TString elementResource = element->getResourcePath();
			TString descResource;
			bool bOverride = false;
			if ((desc == NULL || !desc->getElementResource(elementName, descResource, bOverride)) && !loadAll)
			{
				notifier.onStep(n++, count);
				continue;
			}
			else if((bOverride || elementResource == TString::EMPTY) )
				elementResource = descResource;

			ElementResource r = { element, elementResource };
			list.push_back(r);
		}

		//sort by priority
		std::sort(list.begin(), list.end());

		for (size_t i = 0; i < list.size(); ++i)
		{
			const ElementResource& r = list[i];
			IElement* element = r.element;
			const TString& elementResource = r.resource;
			//HRESOURCE hResource = IResourceManager::getSingleton().loadResourceSync( elementResource );
			//element->setBoundResource(hResource);
			if (elementResource != element->getResourcePath())
			{
				if(element->getBoundResource() != NULL)
					element->unloadResource(true);
				element->setResourcePath(elementResource);
			}
			element->loadResource(false);

			notifier.onStep(n++, count);
		}
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	ElementPostLoader::ElementPostLoader(Stage* stage, IEntity* entity, const EntityResourceDesc* desc)
		:mStage(stage)
		,mEntity(entity)
		,mSingleElement(NULL)
	{
		mDesc = *desc;
	}

	//////////////////////////////////////////////////////////////////////////
	ElementPostLoader::ElementPostLoader(Stage* stage, IEntity* entity, IElement* element)
		:mStage(stage)
		,mEntity(entity)
		,mSingleElement(element)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	ElementPostLoader::~ElementPostLoader()
	{
		this->clearAndDeleteSubStates();
	}

	/************************************************************************/
	/* IResource::IListener interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool	ElementPostLoader::onReady()
	{
		ResourceStateGroup::onReady();
		if (!this->isLoaded())
			return true;

		size_t count = this->getSubStateCount();

		//entity is deleted after trigger loading
		Entity* entity = static_cast<Entity*>(mEntity);
		if (entity->isPendingDeleted())
		{
			//delete
#if BLADE_DEBUG
			mStage->setEntityState(mEntity, ES_INVALID);
#else
			BLADE_DELETE mEntity;
#endif

			//unload resource
			for (size_t i = 0; i < count; ++i)
			{
				ElementResourceState* state = static_cast<ElementResourceState*>(this->getSubState(i));
				state->unload();
			}

			BLADE_DELETE this;
			return false;
		}

		ElementResourceList list;
		list.reserve(count);

		for (size_t i = 0; i < count; ++i)
		{
			ElementResourceState* state = static_cast<ElementResourceState*>(this->getSubState(i));
			IElement* element = state->getElement();

			if(element != NULL)	//element not deleted?
			{
				ScopedLock lock(element->getDesc().mSyncLock);
				if (element->getDesc().mLoadedCount == 0)	//unloaded after async loading
				{
					element->setBoundResource(HRESOURCE::EMPTY);
					element = NULL;
				}
			}
			if (element != NULL)
			{
				ElementResource er = { element, TString::EMPTY };
				list.push_back(er);
			}
		}

		for (size_t i = 0; i < list.size(); ++i)
			list[i].element->postProcess();

		bool ret = mStage->setEntityState(mEntity, ES_INVALID, -(int16)count);
		assert(ret); BLADE_UNREFERENCED(ret);

		BLADE_DELETE this;
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	ElementPostLoader::onFailed()
	{
		return ResourceStateGroup::onFailed();
		//BLADE_DELETE this;
	}

	//////////////////////////////////////////////////////////////////////////
	/*static */bool	ElementPostLoader::loadElements(Stage* stage, IEntity* entity, const EntityResourceDesc& desc, bool forceAsync)
	{
		if (BLADE_TS_CHECK(TS_MAIN_SYNC) && !forceAsync)
		{
			EntityPostLoader::loadElementsSync(entity, &desc, ProgressNotifier::EMPTY, false);
			return true;
		}

		ElementPostLoader* loader = BLADE_NEW ElementPostLoader(stage, entity, &desc);
		loader->loadElements();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	/*static */bool ElementPostLoader::loadElement(IElement* element, bool forceAsync)
	{
		if (element->isLoaded())
			return true;

		IEntity* entity = element->getEntity();
		Stage* stage = static_cast<Stage*>(entity->getStage());

		if (BLADE_TS_CHECK(TS_MAIN_SYNC) && !forceAsync)
		{
			if (!element->getResourcePath().empty())
			{
				ParamList params;
				const TString& path = element->getResourcePath();
				TString type;
				TString loaderType;
				element->getResourceParams(params, type, loaderType);
				HRESOURCE hResource = IResourceManager::getSingleton().loadResourceSync(type, path, loaderType, &params);
				element->setBoundResource(hResource);
			}
			element->postProcess();
			return true;
		}

		ElementPostLoader* loader = BLADE_NEW ElementPostLoader(stage, entity, element);
		loader->loadElement();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool ElementPostLoader::loadElements()
	{
		assert(mSingleElement == NULL);
		size_t count = 0;

		Entity* entity = static_cast<Entity*>(mEntity);

		for (size_t i = 0; i < mDesc.getElementResourceCount(); ++i)
		{
			TString name, path;
			bool force = false;
			bool ret = mDesc.getElementResourceInfo(i, name, path, force);
			assert(ret); BLADE_UNREFERENCED(ret);

			IElement* element = entity->getElement(name);
			if (element == NULL)
			{
				assert(false);	//element deleted?
				continue;
			}
			if (element->isLoaded() && !force)
			{
				//assert(false);
				continue;
			}

			if(!force && path.empty())
				path = element->getResourcePath();
			ParamList params;
			TString type;
			TString loaderType;
			element->getResourceParams(params, type, loaderType);
			ElementResourceState* state = BLADE_NEW ElementResourceState(entity, name, params);
			state->setResourceInfo(type, path, loaderType);
			state->setEnable(!path.empty());

			if (element->getBoundResource() != NULL)
				state->setResource(element->getBoundResource());

			this->addSubState(state);
			++count;
		}

		if (count > 0)
		{
			//note: entity set already locked in stage.
			entity->changeElementLoadingCount((int16)count);
			this->load();
		}
		else
			BLADE_DELETE this;
		return count > 0;
	}

	//////////////////////////////////////////////////////////////////////////
	bool ElementPostLoader::loadElement()
	{
		assert(mSingleElement != NULL);
		TString path = mSingleElement->getResourcePath();
		if (mSingleElement->isLoaded())
		{
			BLADE_DELETE this;
			return true;
		}

		Entity* entity = static_cast<Entity*>(mEntity);

		TStringParam elementNames;
		ElementListParam elements;
		entity->getAllElements(elementNames, elements);
		assert(elements.size() == elementNames.size());

		size_t index = 0;
		for (; index < elementNames.size(); ++index)
		{
			if (elements[index] == mSingleElement)
				break;
		}
		if (index >= elementNames.size() || !mStage->setEntityState(mEntity, ES_INVALID, 1))
		{
			BLADE_DELETE this;
			return false;
		}

		ParamList params;
		TString type;
		TString loaderType;
		mSingleElement->getResourceParams(params, type, loaderType);

		ElementResourceState* state = BLADE_NEW ElementResourceState(entity, elementNames[index], params);
		state->setResourceInfo(type, path, loaderType);
		state->setEnable(!path.empty());

		if (mSingleElement->getBoundResource() != NULL)
			state->setResource(mSingleElement->getBoundResource());

		this->addSubState(state);
		this->load();
		return true;
	}

}//namespace Blade