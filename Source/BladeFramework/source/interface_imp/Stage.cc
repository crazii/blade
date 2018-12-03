/********************************************************************
	created:	2010/04/08
	filename: 	Stage.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "Stage.h"
#include <utility/Profiling.h>
#include <interface/public/IScene.h>
#include <interface/IResourceManager.h>
#include <EntityResourceDesc.h>
#include "EntityResource.h"
#include "EntityPostLoader.h"
#include "Page.h"
#include "StageConfig.h"

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	Stage::Stage(const TString&name, IWindow* window, IStageManager* manager, bool autoScene, bool autoDelete, bool showWindow)
		:mManager(manager)
	{
		mDesc.mWindow = window;
		mDesc.mName = name;
		mDesc.mAutoSceneBinding = autoScene;
		mDesc.mAutoDelete = autoDelete;

		assert( mManager != NULL );
		assert(window != NULL );
		window->setVisibility(showWindow);

		window->addEventHandler(this);
		mWinCaption = window->getCaption();

		mPageManager = BLADE_NEW PagingManager(this);
	}

	//////////////////////////////////////////////////////////////////////////
	Stage::~Stage()
	{
		BLADE_DELETE mPageManager;

		if( mStageResource != NULL )
			IResourceManager::getSingleton().unloadResource(mStageResource);

		if( mDesc.mWindow != NULL )
		{
			mDesc.mWindow->removeEventHandler(this);
			//restore original caption
			if( !mDesc.mAutoDelete )
				mDesc.mWindow->setCaption(mWinCaption);
		}

		for(SceneMap::iterator i = mSceneMap.begin(); i != mSceneMap.end(); ++i)
		{
			IScene* pscene = i->second;
			pscene->onDetachFromStage(this);
		}
		this->clearStage();
	}

	/************************************************************************/
	/* IStage interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	const IStage::DESC&		Stage::getDesc() const
	{
		return mDesc;
	}

	//////////////////////////////////////////////////////////////////////////
	void			Stage::setTitle(const TString& tittle)
	{
		mDesc.mTitle = tittle;
		mDesc.mWindow->setCaption(tittle + TEXT(" - ") + mWinCaption );
	}

	//////////////////////////////////////////////////////////////////////////
	bool			Stage::addScene(IScene* pscene)
	{
		assert(pscene != NULL);

		const TString& type = pscene->getSceneType();
		IScene*& empty = mSceneMap[type];

		if (empty != NULL)
		{
			assert(false);
			return false;
		}
		empty = pscene;

		//register scene for element creation
		ElementTypeList list;
		pscene->getAvailableElementTypes(list);
		for( size_t i = 0; i < list.size(); ++i )
		{
			mElementCreatorMap[ list[i] ] = pscene;
		}

		pscene->onAttachToStage(this);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			Stage::removeScene(const TString& sceneType)
	{
		SceneMap::iterator it = mSceneMap.find(sceneType);

		bool success =  it != mSceneMap.end();

		IScene* pscene = it->second;

		//clear the element creator if there is registered entry
		if( success )
		{
			for( SceneMap::iterator i = mElementCreatorMap.begin(); i != mElementCreatorMap.end();)
			{
				if( i->second == pscene )
					mElementCreatorMap.erase(i++);
				else
					++i;
			}
		}

		pscene->onDetachFromStage(this);

		mSceneMap.erase(it);
		return success;
	}

	//////////////////////////////////////////////////////////////////////////
	IScene*			Stage::getScene(const TString& sceneType) const
	{
		SceneMap::const_iterator i = mSceneMap.find(sceneType);
		if( i == mSceneMap.end() )
			return NULL;
		else
			return i->second;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t			Stage::getSceneCount() const
	{
		return mSceneMap.size();
	}

	//////////////////////////////////////////////////////////////////////////
	IScene*			Stage::getScene(index_t index) const
	{
		if( index >= mSceneMap.size() )
			return NULL;

		SceneMap::const_iterator i = mSceneMap.begin();
		std::advance(i, index);
		return i->second;
	}

	//////////////////////////////////////////////////////////////////////////
	IEntity*		Stage::createEntity(const TString& entityName)
	{
		return this->createEntityEntry(entityName);
	}

	//////////////////////////////////////////////////////////////////////////
	bool			Stage::loadEntity(IEntity* entity, const EntityResourceDesc* desc/* = NULL*/, bool forceAsync/* = false*/)
	{
		if( entity == NULL )
			return false;

		if (!forceAsync && BLADE_TS_CHECK(TS_MAIN_SYNC))
		{
			bool ret = this->loadEntitySyncImpl(entity, desc);
			return ret;
		}

		const TString& resource = desc != NULL ? desc->getPath() : TString::EMPTY;
		Entity* entityImpl = static_cast<Entity*>(entity);

		EntityResourceDesc fullDesc;
		{
			TStringParam names;
			ElementListParam elements;
			entity->getAllElements(names, elements);
			size_t count = names.size();
			for (size_t i = 0; i < count; ++i)
				fullDesc.addElementResource(names[i], elements[i]->getResourcePath());
		}

		{
			EntityFinder finder(entity->getName());

			//check if entity deleted by other threads
			ScopedLock sl(mEntitySet.getLock());
			EntitySet::iterator iter = mEntitySet.find(&finder);
			if (iter == mEntitySet.end())
				return false;

			assert(entity == *iter);

			//load entity from file
			if (!entityImpl->hasResource() && !resource.empty())
			{
				{
					ParamList params;
					params[BTString("ENTITY")] = (void*)entity;
					entityImpl->setResource(IResourceManager::getSingleton().createResource(IEntity::ENTITY_RESOURCE_TYPE, resource, params));
				}

				{
					EntityPostLoader* postLoader = BLADE_NEW EntityPostLoader(this, desc);
					ParamList params;
					params[BTString("STAGE")] = (void*)this;
					params[BTString("ENTITY")] = (void*)entity;

					entityImpl->setStatus(ES_LOADING);
					return IResourceManager::getSingleton().reloadResource(entityImpl->getResource(), resource, postLoader, &params);
				}
			}
			//load all elements
			else
			{
				//load elements only
				return ElementPostLoader::loadElements(this, entity, fullDesc, forceAsync);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool			Stage::loadEntitySync(IEntity* entity, const EntityResourceDesc* desc/* = NULL*/, const ProgressNotifier& notifier/* = ProgressNotifier::EMPTY*/)
	{
		BLADE_TS_VERIFY(TS_MAIN_SYNC);
		return this->loadEntitySyncImpl(entity, desc, notifier);
	}

	//////////////////////////////////////////////////////////////////////////
	bool			Stage::saveEntity(IEntity* entity, const TString& resource, 
		bool textFormat/* = false*/, bool saveAllElements/* = false*/, const TStringParam* elements/* = NULL*/)
	{
		BLADE_TS_VERIFY( TS_MAIN_SYNC );

		if( entity == NULL)
		{
			assert(false);
			return false;
		}

		EntityResource* res;
		Entity* entityImpl = static_cast<Entity*>(entity);
		{
			EntityFinder finder(entity->getName());
			//check if entity deleted by other thread
			ScopedLock sl(mEntitySet.getLock());
			EntitySet::iterator i = mEntitySet.find(&finder);
			if (i == mEntitySet.end() )
				return false;

			assert(entity == *i);
			if (!entityImpl->isReady())
			{
				assert(false && "entity not ready");
				return false;
			}

			if (!entityImpl->hasResource())
			{
				ParamList params;
				params[BTString("ENTITY")] = (void*)entity;
				entityImpl->setResource(IResourceManager::getSingleton().createResource(IEntity::ENTITY_RESOURCE_TYPE, resource, params));
			}
			res = static_cast<EntityResource*>(entityImpl->getResource());
		}
		assert(res != NULL);
		EntitySaveDesc desc;
		desc.saveAll = saveAllElements;
		desc.elements = elements;

		res->setSaveDesc(&desc);
		bool ret = IResourceManager::getSingleton().saveResourceSync(entityImpl->getResource(), true,
			textFormat? EntityResource::ENTITY_SERIALIZER_TYPE_XML : EntityResource::ENTITY_SERIALIZER_TYPE_BINARY );
		res->setSaveDesc(NULL);

		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	IEntity*		Stage::getEntity(const TString& entityName) const
	{
		EntityFinder finder(entityName);

		ScopedLock sl( mEntitySet.getLock() );
		EntitySet::const_iterator i = mEntitySet.find(&finder);
		if( i == mEntitySet.end() )
			return NULL;

		Entity* entity = *i;
		if (entity->isReady())
			return entity;
		else
		{
			assert(false && "entity not ready.");
			return NULL;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool			Stage::renameEntity(const TString& entityName, const TString& newName)
	{
		EntityFinder finder(entityName);
		EntityFinder tester(newName);
		{
			ScopedLock ls(mEntitySet.getLock());

			EntitySet::const_iterator i = mEntitySet.find(&finder);
			EntitySet::const_iterator i2 = mEntitySet.find(&tester);
			if (i == mEntitySet.end() || i2 != mEntitySet.end() )
			{
				//assert(false);
				return false;
			}
			Entity* entity = *i;
			mEntitySet.erase(i);

			entity->setName(newName);
			mEntitySet.insert(entity);
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			Stage::loadStageFile(const TString& pathName, const ProgressNotifier& notifier/* = ProgressNotifier::EMPTY*/)
	{
		BLADE_TS_VERIFY( TS_MAIN_SYNC );

		this->clearStage();

		ParamList params;
		params[BTString("STAGE")] = this;
		params[BTString("PROGRESS")] = (void*)&notifier;

		bool ret = true;
		BLADE_LW_PROFILING(LOADING);
		if (mStageResource != NULL && mStageResource->getSource() != pathName)
			IResourceManager::getSingleton().unloadResource(mStageResource);

		if(mStageResource == NULL)
			mStageResource = IResourceManager::getSingleton().loadResourceSync(pathName, TString::EMPTY, &params);
		else
		{
			ret = IResourceManager::getSingleton().reloadResourceSync(mStageResource, pathName, &params);
			assert(ret);
		}
		return ret && mStageResource != NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			Stage::saveStageFile(const TString& pathName, bool textFormat/* = false*/, bool savePage/* = true*/)
	{
		BLADE_TS_VERIFY( TS_MAIN_SYNC );

		if( mStageResource != NULL && mStageResource->getSource() != pathName )
			IResourceManager::getSingleton().unloadResource(mStageResource);

		if( mStageResource == NULL )
		{
			ParamList params;
			params[BTString("STAGE")] = this;
			mStageResource = IResourceManager::getSingleton().createResource(STAGE_RESOURCE_TYPE,pathName,params);
		}

		bool result = IResourceManager::getSingleton().saveResourceSync(mStageResource, true, textFormat ? STAGE_SERIALIZER_TYPE_XML : STAGE_SERIALIZER_TYPE_BINARY);
		if (result && savePage)
		{
			TString dir, filename;
			TStringHelper::getFilePathInfo(pathName, dir, filename);
			result = this->getPagingManager()->savePages(dir, textFormat);
		}
		assert(result);
		return result;
	}

	/************************************************************************/
	/* IWindowEventHandler interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void			Stage::onWindowDestroy(IWindow* pwin)
	{
		assert( pwin == mDesc.mWindow );
		BLADE_UNREFERENCED(pwin);
		pwin->removeEventHandler(this);
		mDesc.mWindow = NULL;
		if(mDesc.mAutoDelete)
			mManager->destroyStage(this);
	}

	/************************************************************************/
	/* ISerializable interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void			Stage::postProcess(const ProgressNotifier& notifier)
	{
		notifier.onNotify(1.0f);
	}

	//////////////////////////////////////////////////////////////////////////
	void			Stage::instantProcess(const ProgressNotifier& callback)
	{
		//setup page layer
		const StageConfig::LayerConfigMap& layers = StageConfig::getSingleton().getLayerConfig();
		StageConfig::LayerConfigMap::const_iterator defIter = layers.find(IPage::DEFAULT_LAYER);
		assert(defIter != layers.end());
		static_cast<PagingManager*>(mPageManager)->initialize(defIter->second.elements);

		for (StageConfig::LayerConfigMap::const_iterator iter = layers.begin(); iter != layers.end(); ++iter)
		{
			if(iter == defIter)
				continue;

			const TString& layerName = iter->first;
			const StageConfig::PageLayerConfig& layerCfg = iter->second;
			mPageManager->addLayer(layerName, layerCfg.elements, layerCfg.size, layerCfg.range, layerCfg.priority);
		}

		callback.onNotify(1.0f);
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool			Stage::setEntityState(IEntity* entity, ENTITY_SATUS state, int16 elementLoadingCount/* = 0*/, const HRESOURCE* entityResource/* = NULL*/)
	{
		BLADE_TS_VERIFY(TS_MAIN_SYNC);

		if (entity == NULL /*|| (state == ES_READY && (entityResource == NULL || *entityResource == NULL))*/)
		{
			assert(false);
			return false;
		}

		Entity* entityImpl = static_cast<Entity*>(entity);
		bool doDelete = false;

		{
			EntityFinder finder(entity->getName());

			ScopedLock lock(mEntitySet.getLock());
			EntitySet::iterator i = mEntitySet.find(&finder);
			if (i == mEntitySet.end() || entityImpl->getStatus() == ES_DELETE_LOADING)
			{
				assert(entityImpl->getStatus() == ES_DELETE_LOADING && i == mEntitySet.end());
				doDelete = true;
			}

			if (!doDelete)
			{
				assert(entityImpl->getStatus() == ES_LOADING || entityImpl->getStatus() == ES_READY);
				if(entityResource != NULL)
					entityImpl->setResource(*entityResource);

				if(state != ES_INVALID)
					entityImpl->setStatus(state);
				entityImpl->changeElementLoadingCount(elementLoadingCount);
			}
		}

		if (doDelete)
		{
			assert(entityResource == NULL || entityResource->refcount() == 1);
			BLADE_DELETE entityImpl;
			return false;
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void			Stage::destroyEntityImpl(const TString& entityName, bool notifyPaging)
	{
		Entity* entity = NULL;
		EntityFinder finder(entityName);

		{
			ScopedLock ls(mEntitySet.getLock());

			EntitySet::const_iterator i = mEntitySet.find(&finder);
			if (i != mEntitySet.end())
			{
				entity = *i;

				if (entity->hasResource())
					IResourceManager::getSingleton().unloadResource(entity->getResource());

				if (!entity->isReady() || entity->getElementLoadingCount() != 0)
				{
					//delay delete the entity
					entity->setStatus(ES_DELETE_LOADING);
					entity = NULL;
				}

				if (notifyPaging && entity->isPagingEnabled())
					static_cast<PagingManager*>(mPageManager)->removeEntity(entity);

				mEntitySet.erase(i);
			}
		}

		if (entity != NULL)
		{
			BLADE_DELETE entity;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void			Stage::destroyPagingEntities(const EntitySet& entities, bool notifyPaging)
	{
		BLADE_TS_VERIFY(TS_MAIN_SYNC);

		if (entities.size() == 0)
			return;

		{
			ScopedLock ls(mEntitySet.getLock());

			for (EntitySet::const_iterator i = entities.begin(); i != entities.end(); ++i)
			{
				Entity* entity = *i;
				assert(entity->isPagingEnabled());

				if (entity->decrementPage() != 0)
					continue;

				EntitySet::const_iterator iter = mEntitySet.find(entity);
				if (iter == mEntitySet.end() || *iter != entity)
				{
					assert(false);
					continue;
				}
				if (entity->hasResource())
					IResourceManager::getSingleton().unloadResource(entity->getResource());

				if (!entity->isReady() || entity->getElementLoadingCount() != 0)
				{
					//delay delete the entity
					entity->setStatus(ES_DELETE_LOADING);
					entity = NULL;
				}

				if (notifyPaging)
					static_cast<PagingManager*>(mPageManager)->removeEntity(entity);

				mEntitySet.erase(iter);
				if (entity != NULL)
					mDeletedEntities.push_back(entity);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void			Stage::update()
	{
		BLADE_TS_VERIFY(TS_MAIN_SYNC);
		const scalar updateTimeLimit = 0.1f / 1000.0f;	//0.1ms
		StaticHandle<ITimeDevice> timer(ITimeDevice::create(ITimeDevice::TP_TEMPORARY));

		while (!mDeletedEntities.empty() && timer->getSeconds() < updateTimeLimit)
		{
			timer->update();
			Entity* entity = mDeletedEntities.back();
			mDeletedEntities.pop_back();
			BLADE_DELETE entity;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void			Stage::clearStage()
	{
		BLADE_TS_VERIFY(TS_MAIN_SYNC);

		for(EntitySet::iterator i = mEntitySet.begin(); i != mEntitySet.end(); ++i)
		{
			Entity* entity = *i;

			//unload entity resource
			if (entity->hasResource())
				IResourceManager::getSingleton().unloadResource(entity->getResource());

			if (!entity->isReady())
				//delay delete the entity
				entity->setStatus(ES_DELETE_LOADING);
			else
				BLADE_DELETE entity;
		}
		mEntitySet.clear();

		for (EntityList::iterator i = mDeletedEntities.begin(); i != mDeletedEntities.end(); ++i)
			BLADE_DELETE *i;
		mDeletedEntities.clear();
	}

	//////////////////////////////////////////////////////////////////////////
	IEntity*		Stage::createEntityEntry(const TString& name)
	{
		bool done = false;
		//alloc entity before enter lock section
		Entity* entity = BLADE_NEW Entity(name, this);
		{
			//load entity may be called cascade in loading thread, so we need locks
			ScopedLock sl( mEntitySet.getLock() );
			
			std::pair<EntitySet::iterator, bool> ret = mEntitySet.insert(entity);
			done = ret.second;
		}

		if(!done)
		{
			BLADE_DELETE entity;
			entity = NULL;
			assert(false && "entity with the same name already exist.");
		}
		return entity;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			Stage::loadEntitySyncImpl(IEntity* entity, const EntityResourceDesc* desc, const ProgressNotifier& notifier/* = ProgressNotifier::EMPTY*/)
	{
		BLADE_TS_VERIFY(TS_MAIN_SYNC);

		const TString& resPath = desc != NULL ? desc->getPath() : TString::EMPTY;

		Entity* entityImpl = static_cast<Entity*>(entity);
		{
			EntityFinder finder(entity->getName());

			EntitySet::iterator i = mEntitySet.find(&finder);
			if (i == mEntitySet.end())
			{
				notifier.onNotify(1.0f);
				return false;
			}
			assert(entity == *i);
		}

		ParamList params;
		params[BTString("STAGE")] = (void*)this;
		params[BTString("ENTITY")] = (void*)entity;

		//create resource
		if (!entityImpl->hasResource() && resPath != TString::EMPTY)
			entityImpl->setResource(IResourceManager::getSingleton().createResource(IEntity::ENTITY_RESOURCE_TYPE, resPath, params));

		bool result = true;
		if (resPath != TString::EMPTY)
			result = IResourceManager::getSingleton().reloadResourceSync(entityImpl->getResource(), resPath, &params);

		if (result)
			EntityPostLoader::loadElementsSync(entity, desc, notifier);
		else
			notifier.onNotify(1.0f);

		entityImpl->postLoad(result);
		return result;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			Stage::setPageSize(index_t, const uint32& size)
	{
		static_cast<PagingManager*>(mPageManager)->preInit(size);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			Stage::setPageCount(index_t, const uint32& count)
	{
		static_cast<PagingManager*>(mPageManager)->preInit(PagingManager::INVALID_PARAM, count);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			Stage::setVisbiblePageCount(index_t, const uint32& count)
	{
		static_cast<PagingManager*>(mPageManager)->preInit(PagingManager::INVALID_PARAM, PagingManager::INVALID_PARAM, count);
		return true;
	}
	
}//namespace Blade