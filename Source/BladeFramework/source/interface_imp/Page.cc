/********************************************************************
	created:	2016/04/23
	filename: 	Page.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <utility/Profiling.h>
#include <interface/public/CommonState.h>
#include <parallel/ParallelPriority.h>
#include "Page.h"
#include "Stage.h"
#include "../StageSerializer.h"

namespace Blade
{
	const TString IPage::DEFAULT_LAYER = BTString("Default");
	static const TString PAGE_SUBPATH = BTString("/pages/");

	namespace Impl
	{
		//////////////////////////////////////////////////////////////////////////
		AABB	getEntityWorldBounding(const IEntity* entity)
		{
			TStringParam names;
			ElementListParam elements;
			entity->getAllElements(names, elements);

			Vector3 position;
			Vector3 scale;
			Quaternion rotation;
			AABB localAAB(BT_NULL);

			int16 posPriority = PP_INVALID;
			int16 scalePriority = PP_INVALID;
			int16 rotationPriority = PP_INVALID;
			int16 aabPriority = PP_INVALID;

			//enumerate all elements to find bounding box with highest priority
			for (size_t j = 0; j < elements.size(); ++j)
			{
				IElement* elem = elements[j];
				const ParaStateSet& paraStates = elem->getParallelStates();

				const IParaState* posState = paraStates[CommonState::POSITION];
				if (posState != NULL && posState->getPriority() > posPriority)
				{
					posPriority = posState->getPriority();
					position = *(const Vector3*)posState->getStateData();
				}

				const IParaState* scaleState = paraStates[CommonState::SCALE];
				if (scaleState != NULL && scaleState->getPriority() > scalePriority)
				{
					scalePriority = scaleState->getPriority();
					scale = *(const Vector3*)scaleState->getStateData();
				}

				const IParaState* rotState = paraStates[CommonState::ROTATION];
				if (rotState != NULL && rotState->getPriority() > rotationPriority)
				{
					rotationPriority = rotState->getPriority();
					rotation = *(const Quaternion*)rotState->getStateData();
				}

				const IParaState* boundingState = paraStates[CommonState::BOUNDING];
				if (boundingState != NULL && boundingState->getPriority() > aabPriority)
				{
					aabPriority = boundingState->getPriority();
					localAAB = *(const AABB*)boundingState->getStateData();
				}
			}

			if (posPriority != PP_INVALID && scalePriority != PP_INVALID && rotationPriority != PP_INVALID && aabPriority != PP_INVALID)
			{
				//skip empty/infinite bounding. entity with empty/infinite bounding will be loaded directly in stage
				if (localAAB.isNull() || localAAB.isInfinite())
					return localAAB;

				Matrix44 transform = Matrix44::generateTransform(position, scale, rotation);
				AABB worldAAB = localAAB * transform;

				return worldAAB;
			}

			return AABB::EMPTY;
		}
		
		//////////////////////////////////////////////////////////////////////////
		TString		generatePagePath(const TString& prefix, size_t x, size_t z)
		{
			return prefix + PAGE_SUBPATH + TStringHelper::fromUInt(x) + TEXT('_') + TStringHelper::fromUInt(z) + BTString(".bpage");
		}

	}//namespace Impl

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	Page::Page(const Vector2& pos, size_t x, size_t z, size_t visibleCount)
		:mPosition(pos)
		,mStatus(PS_EMPTY)
	{
		//note: use fixed (MOD) local indices, since Graphics sub space's page indices are used for serialization and should always match its 3d position
		/*
		uint32 x = 0, z = 0;
		Impl::getPageFromPatitionMask(partitionMask, x, z);

		ISpace* space = this->getSpaceofPage(x, z);
		*/
		mX = (uint16)x;
		mZ = (uint16)z;
		mLocalX = (uint8)(x % visibleCount);
		mLocalZ = (uint8)(z % visibleCount);
		mModified = false;
		mInitial = false;
	}

	//////////////////////////////////////////////////////////////////////////
	Page::~Page()
	{
		if (mPageResource != NULL)
			IResourceManager::getSingleton().unloadResource(mPageResource);

		for (LayerMap::iterator i = mLayerMap.begin(); i != mLayerMap.end(); ++i)
			BLADE_DELETE i->second;

		mOrderedLayers.clear();
	}

	//////////////////////////////////////////////////////////////////////////
	bool	Page::addEntity(IEntity* entity, const AABB& aab, bool inLoading/* = false*/)
	{
		Entity* entityImpl = static_cast<Entity*>(entity);
		if (entity == NULL || !mEntities.insert(entityImpl).second)
		{
			assert(false);
			return false;
		}

		entityImpl->incrementPage();

		for (LayerMap::iterator i = mLayerMap.begin(); i != mLayerMap.end(); ++i)
		{
			PageLayer* layer = i->second;
			layer->addEntity(entityImpl, aab);
		}

		if (!inLoading)
		{
			if (mStatus == PS_EMPTY)
				mStatus = PS_CACHED;
			mModified = true;
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	Page::addEntity(IEntity* entity, bool inLoading/* = false*/)
	{
		AABB aab = Impl::getEntityWorldBounding(entity);
		return this->addEntity(entity, aab, inLoading);
	}

	//////////////////////////////////////////////////////////////////////////
	bool	Page::removeEntity(IEntity* entity)
	{
		if (entity == NULL)
		{
			assert(false);
			return false;
		}

		//note: removing entity only happens on editing mode
		Entity* entityImpl = static_cast<Entity*>(entity);
		EntitySet::iterator iter = mEntities.find(entityImpl);

		if (iter == mEntities.end())
		{
			//assert(false && "failed removing entity.");
			return false;
		}

		entityImpl->decrementPage();

		for (LayerMap::iterator i = mLayerMap.begin(); i != mLayerMap.end(); ++i)
		{
			PageLayer* layer = i->second;
			layer->removeEntity(entityImpl);
		}

		mEntities.erase(iter);
		mModified = true;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	Page::addLayer(const TString& name, const TStringParam& elementTypes, size_t subPageSize, size_t pageSize, scalar range, int priority)
	{
		assert(mLayerMap.size() == mOrderedLayers.size());

		PageLayer*& layer = mLayerMap[name];
		if (layer != NULL)
		{
			assert(false && "layer with the same name already exist.");
			return false;
		}

		if (subPageSize < MIN_SUBPAGE_SIZE)
			subPageSize = MIN_SUBPAGE_SIZE;
		size_t count = pageSize / subPageSize;
		fp32 size = (fp32)(pageSize / count);

		Vector2 pos = mPosition - (scalar)pageSize / 2;
		layer = BLADE_NEW PageLayer(pos, size, count, range, elementTypes, priority);

		PrioriyLayers::iterator i = mOrderedLayers.begin();
		while (i != mOrderedLayers.end())
		{
			if ((*i)->getPriority() >= priority)
				++i;
			else
				break;
		}
		mOrderedLayers.insert(i, layer);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	Page::cachePage(const TString& path, IStage* stage, bool createOnFail, const ListenerSet& listeners)
	{
		if (mPageResource != NULL)
		{
			assert(mStatus == PS_CACHED);
			return true;
		}
		if (mStatus == PS_LOADING)
		{
			assert(mPageResource == NULL);
			return true;
		}
		assert(this->isUnloaded());

		std::for_each(listeners.begin(), listeners.end(), std::bind2nd(std::mem_fun(&IPagingManager::IListener::onPageLoad), this));

		ParamList params;
		params[BTString("STAGE")] = stage;
		params[BTString("PAGE")] = static_cast<IPage*>(this);
		params[BTString("PROGRESS")] = (void*)&ProgressNotifier::EMPTY;

		bool ret = false;
		mStatus = PS_LOADING;
		if (createOnFail && IResourceManager::getSingleton().loadStream(path) == NULL)
		{
			assert(mPageResource == NULL);
			mPageResource = IResourceManager::getSingleton().createResource(STAGE_RESOURCE_TYPE, path, params);
			mStatus = PS_CACHED;
			mInitial = true;
			ret = mPageResource != NULL;
		}
		else
			ret = IResourceManager::getSingleton().loadResource(STAGE_RESOURCE_TYPE, path, this, PAGE_SERIALIZER_TYPE, &params);
		assert(ret);
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	Page::loadPage(const TString& path, IStage* stage, bool createOnFail, const ListenerSet& listeners, const ProgressNotifier& notifier)
	{
		BLADE_TS_VERIFY(TS_MAIN_SYNC);
		//page caching in progress
		if(this->isLoaded() || this->isLoading())
			return true;

		notifier.beginStep(1);	//loadResourceSync/createResource will finish it

		ParamList params;
		params[BTString("STAGE")] = stage;
		params[BTString("PAGE")] = static_cast<IPage*>(this); 
		params[BTString("PROGRESS")] = (void*)&notifier;
		mStatus = PS_LOADING;
		mPageResource = IResourceManager::getSingleton().loadResourceSync(path, PAGE_SERIALIZER_TYPE, &params);
		if (mPageResource == NULL && createOnFail)
		{
			mPageResource = IResourceManager::getSingleton().createResource(STAGE_RESOURCE_TYPE, path, params);
			mInitial = true;
		}
		mStatus = uint8((mPageResource != NULL || mModified) ? PS_CACHED : PS_EMPTY);

		if(mPageResource != NULL)
			std::for_each(listeners.begin(), listeners.end(), std::bind2nd(std::mem_fun(&IPagingManager::IListener::onPageLoad), this));

		//if page contains new entity, this will add saved entity to it, but the page is still modified
		//onPageLoad must be notified for all pages and then load entities, otherwise shared entity by pages will get no-empty page move
		//mModified = false;
		return mPageResource != NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	Page::savePage(const TString& path, IStage* stage, bool textFormat)
	{
		BLADE_TS_VERIFY(TS_MAIN_SYNC);
		if (mPageResource != NULL && mPageResource->getSource() != path)
		{
			IResourceManager::getSingleton().unloadResource(mPageResource);
			mModified = true;
		}
		if (!mModified)
			return true;

		//page caching in progress
		if (mStatus == PS_LOADING || mStatus == PS_EMPTY)
			return true;
		assert(mStatus == PS_CACHED);

		bool needUnload = false;
		if (mPageResource == NULL) //unloaded page, entity added, so now is cached(modified), but resource is not actually loaded
		{
			//we need to load original resource to keep sync, otherwise old data will be overwritten and original entities will be lost.

			ParamList params;
			params[BTString("STAGE")] = stage;
			params[BTString("PAGE")] = static_cast<IPage*>(this);
			//page not cached, but new entity added, try load original page file first
			mStatus = PS_LOADING;
			mPageResource = IResourceManager::getSingleton().loadResourceSync(path, PAGE_SERIALIZER_TYPE, &params);

			//keep to old state
			needUnload = (mPageResource != NULL);	//creation of world: first time init?

			if(mPageResource == NULL)
				mPageResource = IResourceManager::getSingleton().createResource(STAGE_RESOURCE_TYPE, path, params);
			mStatus = PS_CACHED;
		}

		bool ret = IResourceManager::getSingleton().saveResourceSync(mPageResource, true, textFormat ? PAGE_SERIALIZER_TYPE_XML : PAGE_SERIALIZER_TYPE_BINARY);
		mModified = false;
		mInitial = false;

		if (needUnload)	//keep to old state
			this->unloadImpl(static_cast<Stage*>(stage));
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	Page::unloadPage(Stage* stage, const ListenerSet& listeners, bool saveCurrent)
	{
		if (this->isUnloaded())
		{
			assert(mPageResource == NULL);
			return false;
		}
		
		{
			//BLADE_LW_PROFILING(DISPATCH);
			std::for_each(listeners.begin(), listeners.end(), std::bind2nd(std::mem_fun(&IPagingManager::IListener::onPageUnload), this));
		}
		
		if (mStatus == PS_LOADING)
		{
			assert(mPageResource == NULL);
			assert(this->isListening());
			this->cancel();
			mStatus = PS_EMPTY;
			return true;
		}

		assert(mPageResource != NULL);
		assert(mStatus == PS_CACHED);

		if (mModified && saveCurrent && !mInitial)
			IResourceManager::getSingleton().saveResourceSync(mPageResource, true, PAGE_SERIALIZER_TYPE_XML);

		return this->unloadImpl(stage, !mInitial);
	}

	//////////////////////////////////////////////////////////////////////////
	bool	Page::unloadImpl(Stage* stage, bool destroyEntities/* = true*/)
	{
		mStatus = PS_EMPTY;
		{
			//BLADE_LW_PROFILING(UNLOAD);
			IResourceManager::getSingleton().unloadResource(mPageResource);

			for (LayerMap::const_iterator i = mLayerMap.begin(); i != mLayerMap.end(); ++i)
			{
				PageLayer* layer = i->second;
				layer->unload(destroyEntities);
			}
		}

		if(destroyEntities)
		{
			//BLADE_LW_PROFILING(DESTROY);
			stage->destroyPagingEntities(mEntities, false);
			mEntities.clear();
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void	Page::update(const Vector2& position, bool forceAsync)
	{
		if (mStatus == PS_EMPTY || mStatus == PS_LOADING)
		{
			assert(mPageResource == NULL);
			return;
		}
		assert(mStatus == PS_CACHED);

		static const size_t LOADING_LIMIT = 5;

		for (PrioriyLayers::const_iterator i = mOrderedLayers.begin(); i != mOrderedLayers.end(); ++i)
		{
			PageLayer* layer = *i;
			layer->update(position, forceAsync, LOADING_LIMIT);
		}
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	PagingManager::PagingManager(Stage* stage)
		:mStage(stage)
		,mRange(Box2::EMPTY)
		,mStartX(0)
		,mStartZ(0)
		,mEditMode(false)
	{
		this->preInit(512, 1, 1);
	}

	//////////////////////////////////////////////////////////////////////////
	PagingManager::~PagingManager()
	{
		if (!mPages.empty())
		{
			size_t totalVisible = mDesc.mVisiblePages * mDesc.mVisiblePages;
			for (size_t i = 0; i < totalVisible; ++i)
			{
				size_t x = i % mDesc.mVisiblePages + mStartX;
				size_t z = i / mDesc.mVisiblePages + mStartZ;

				size_t index = z * mDesc.mPageCount + x;

				//std::for_each(mListeners.begin(), mListeners.end(), std::bind2nd(std::mem_fun(&IPagingManager::IListener::onPageUnload), mPages[index]));
				mPages[index]->unloadPage(mStage, mListeners, mEditMode);
			}

			for (size_t i = 0; i < mPages.size(); ++i)
				BLADE_DELETE mPages[i];
		}
	}

	//////////////////////////////////////////////////////////////////////////
	IPage*		PagingManager::getPage(size_t x, size_t z) const
	{
		size_t index = z * (size_t)mDesc.mPageCount + x;
		if(x >= mDesc.mPageCount || z >= mDesc.mPageCount || index >= mPages.size())
		{
			assert(false);
			return NULL;
		}
		return mPages[index];
	}

	//////////////////////////////////////////////////////////////////////////
	IPage*		PagingManager::getPageByPosition(scalar x, scalar z) const
	{
		Vector2 pos(x, z);
		pos = Vector2::getMaxVector(pos, Vector2::ZERO);
		pos = Vector2::getMinVector(pos, Vector2::UNIT_ALL*(scalar)(mDesc.mPageSize*(mDesc.mPageCount - 1)));

		size_t ix = (size_t)(pos.x / (scalar)mDesc.mPageSize);
		size_t iz = (size_t)(pos.y / (scalar)mDesc.mPageSize);
		return this->PagingManager::getPage(ix, iz);
	}

	//////////////////////////////////////////////////////////////////////////
	size_t		PagingManager::getEntities(const AABB& aab, const TStringParam& elementTypes, TPointerParam<IEntity>& outEntities, size_t limit/* = -1*/) const
	{
		if (aab.isNull() || elementTypes.size() == 0)
			return 0;
		assert(!aab.isInfinite());
		Box2 aab2(aab.getMinPoint().x, aab.getMinPoint().z, aab.getMaxPoint().x, aab.getMaxPoint().z);

		size_t totalVisible = (mDesc.mVisiblePages + 2) * (mDesc.mVisiblePages + 2);
		for (size_t i = 0; i < totalVisible; ++i)
		{
			size_t x = i % mDesc.mVisiblePages + std::max(0, (int)(mStartX - 1));
			size_t z = i / mDesc.mVisiblePages + std::max(0, (int)(mStartZ - 1));

			size_t index = z * mDesc.mPageCount + x;
			if (index >= mPages.size())
				continue;

			Box2 bounds2;
			Page* p = mPages[index];
			Vector2 pageSize = Vector2::UNIT_ALL*(scalar)mDesc.mPageSize;
			bounds2.set(p->getPosition() - pageSize, p->getPosition() + pageSize);

			if(!aab2.intersect(bounds2))
				continue;

			if (p->getEntities(aab2, elementTypes, outEntities, limit) >= limit)
				break;
		}
		return outEntities.size();
	}

	//////////////////////////////////////////////////////////////////////////
	bool		PagingManager::addLayer(const TString& name, const TStringParam& elementTypes, size_t subPageSize, scalar range, int priority/* = IPage::DEFAULT_PRIORITY*/) const
	{
		if (name.empty() || elementTypes.size() == 0 || name == IPage::DEFAULT_LAYER)
		{
			assert(false);
			return false;
		}
		if (subPageSize > mDesc.mPageSize)
			subPageSize = mDesc.mPageSize;

		bool ret = true;
		for (size_t i = 0; i < mPages.size(); ++i)
		{
			Page* page = mPages[i];
			ret = page->addLayer(name, elementTypes, subPageSize, mDesc.mPageSize, range, priority) && ret;
		}
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		PagingManager::resetPaging()
	{
		//clear old data
		for (size_t i = 0; i < mPages.size(); ++i)
			mPages[i]->clear();

		//add re-fill data
		const EntitySet& entities = mStage->getEntities();

		for(EntitySet::const_iterator i = entities.begin(); i != entities.end(); ++i)
		{
			IEntity* entity = *i;
			this->notifyEntity(entity, true);
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		PagingManager::loadPages(const TString& path, const Vector3& position, const ProgressNotifier& notifier/* = ProgressNotifier::EMPTY*/)
	{
		size_t totalVisible = mDesc.mVisiblePages * mDesc.mVisiblePages;

		Vector2 initialPos(position.x, position.z);
		initialPos = Vector2::getMaxVector(initialPos - Vector2::UNIT_ALL*(scalar)(mDesc.mPageSize), Vector2::ZERO);
		initialPos = Vector2::getMinVector(initialPos, Vector2::UNIT_ALL*(scalar)(mDesc.mPageSize*(mDesc.mPageCount-1)));

		size_t startX = (size_t)(initialPos.x / mDesc.mPageSize);
		size_t startZ = (size_t)(initialPos.y / mDesc.mPageSize);

		bool newPos = (startX != mStartX) || (startZ != mStartZ);
		//unload previous pages & notify
		for (size_t i = 0; i < totalVisible; ++i)
		{
			size_t x = i % mDesc.mVisiblePages + mStartX;
			size_t z = i / mDesc.mVisiblePages + mStartZ;
			size_t index = z * mDesc.mPageCount + x;

			if (mPages[index]->isUnloaded())	//initial unload dispatch
				std::for_each(mListeners.begin(), mListeners.end(), std::bind2nd(std::mem_fun(&IPagingManager::IListener::onPageUnload), mPages[index]));

			if (newPos)
			{
				if (x < startX || x >= startX + mDesc.mVisiblePages || z < startZ || z >= startZ + mDesc.mVisiblePages)
					mPages[index]->unloadPage(mStage, mListeners, mEditMode);
			}
		}
		this->setStartIndex(startX, startZ);

		bool ret = true;

		for (size_t i = 0; i < totalVisible; ++i)
		{
			size_t x = i % mDesc.mVisiblePages + mStartX;
			size_t z = i / mDesc.mVisiblePages + mStartZ;

			size_t index = z * mDesc.mPageCount + x;
			notifier.beginStep(0.2f / scalar(totalVisible));

			const TString pageFile = Impl::generatePagePath(path, x, z);
			//create resource for empty pages in edit mode: to distinguish loaded pages from unloaded neighbors (both have no initial resource but may be modified) 
			//so that Page::unloadPage will dispatch events only for loaded pages, not for unloaded neighbors 
			ret = mPages[index]->loadPage(pageFile, mStage, mEditMode, mListeners, notifier) && ret;
			//assert(ret);	//there maybe no page resource, i.e. model viewer scene
		}

		//note: onLoadPage events must be notified for all pages before doing actual entity load/page update, otherwise entity shared by multiple pages maybe loaded to other pages/spaces in advance,
		//and later when other page's onLoadPage event occur, space may be moved but it is not empty.
		
		for (size_t i = 0; i < totalVisible; ++i)
		{
			size_t x = i % mDesc.mVisiblePages + mStartX;
			size_t z = i / mDesc.mVisiblePages + mStartZ;

			size_t index = z * mDesc.mPageCount + x;

			const TString pageFile = Impl::generatePagePath(path, x, z);
			notifier.beginStep(0.8f / scalar(totalVisible));
			//create resource for empty pages in edit mode: to distinguish loaded pages from unloaded neighbors (both have no initial resource but may be modified) 
			//so that Page::unloadPage will dispatch events only for loaded pages, not for unloaded neighbors 
			mPages[index]->update(initialPos, false);
			notifier.onNotify(1.0f);
		}
		mPath = path;
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		PagingManager::savePages(const TString& path, bool textFormat) const
	{
		bool ret = mEditMode;
		if (mEditMode)
		{
			for (size_t i = 0; i < mPages.size(); ++i)
			{
				size_t x = i % mDesc.mPageCount;
				size_t z = i / mDesc.mPageCount;
				const TString pageFile = Impl::generatePagePath(path, x, z);
				ret = mPages[i]->savePage(pageFile, mStage, textFormat) && ret;
				assert(ret);
			}
		}
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	void		PagingManager::update(const Vector3& position)
	{
		Vector2 pos(position.x, position.z);
		pos = Vector2::getMaxVector(pos, Vector2::ZERO);
		pos = Vector2::getMinVector(pos, Vector2::UNIT_ALL*(scalar)(mDesc.mPageSize*(mDesc.mPageCount-1)));

		int switched = 0;

		if (mDesc.mVisiblePages > 1)
		{
			Vector2 bmin = mRange.getMinPoint();
			Vector2 bmax = mRange.getMaxPoint();

			Vector2 outer = (bmax - bmin) * 0.25f;
			if (mDesc.mVisiblePages == 2)
				outer -= Vector2::UNIT_ALL * scalar(mDesc.mPageSize) * 0.25f;

			bmin += outer;
			bmax -= outer;

			if (mDesc.mVisiblePages > 1 && !(pos >= bmin && pos <= bmax))
			{
				//switch page
				size_t maxX = mDesc.mPageCount - mDesc.mVisiblePages;
				size_t maxZ = maxX;

				//unloadPage may use mStartX, mStartZ. change those later.
				size_t startX = mStartX;
				size_t startZ = mStartZ;

				//TODO: handle teleporting
				//unload left
				if (pos.x > bmax.x && mStartX < maxX)
				{
					BLADE_LW_PROFILING(UNLOAD1);
					for (size_t z = 0; z < mDesc.mVisiblePages; ++z)
					{
						size_t index = (mStartZ + z) * mDesc.mPageCount + mStartX;
						mPages[index]->unloadPage(mStage, mListeners, mEditMode);
					}
					++startX;
					++switched;
				}
				//unload right
				else if (pos.x < bmin.x && mStartX > 0)
				{
					BLADE_LW_PROFILING(UNLOAD2);
					for (size_t z = 0; z < mDesc.mVisiblePages; ++z)
					{
						size_t index = (mStartZ + z) * mDesc.mPageCount + (mStartX + mDesc.mVisiblePages - 1);
						mPages[index]->unloadPage(mStage, mListeners, mEditMode);
					}
					--startX;
					++switched;
				}

				//unload top
				if (pos.y > bmax.y && mStartZ < maxZ)
				{
					BLADE_LW_PROFILING(UNLOAD3);
					for (size_t x = 0; x < mDesc.mVisiblePages; ++x)
					{
						size_t index = mStartZ * mDesc.mPageCount + mStartX + x;
						mPages[index]->unloadPage(mStage, mListeners, mEditMode);
					}
					++startZ;
					++switched;
				}
				//unload bottom
				else if (pos.y < bmin.y && mStartZ > 0)
				{
					BLADE_LW_PROFILING(UNLOAD4);
					for (size_t x = 0; x < mDesc.mVisiblePages; ++x)
					{
						size_t index = (mStartZ + mDesc.mVisiblePages - 1) * mDesc.mPageCount + mStartX + x;
						mPages[index]->unloadPage(mStage, mListeners, mEditMode);
					}
					--startZ;
					++switched;
				}

				if (switched)
				{
					BLADE_LW_PROFILING(LOAD);
					size_t totalVisible = mDesc.mVisiblePages * mDesc.mVisiblePages;
					for (size_t i = 0; i < totalVisible; ++i)
					{
						size_t x = i % mDesc.mVisiblePages + startX;
						size_t z = i / mDesc.mVisiblePages + startZ;

						size_t index = z * mDesc.mPageCount + x;
						const TString pageFile = Impl::generatePagePath(mPath, x, z);
						mPages[index]->cachePage(pageFile, mStage, mEditMode, mListeners);
					}
				}
				this->setStartIndex(startX, startZ);
			}
		}

		{
			BLADE_LW_PROFILING_SWITCH(UPDATE, switched != 0);
			size_t totalVisible = mDesc.mVisiblePages * mDesc.mVisiblePages;
			for (size_t i = 0; i < totalVisible; ++i)
			{
				size_t x = i % mDesc.mVisiblePages + mStartX;
				size_t z = i / mDesc.mVisiblePages + mStartZ;

				size_t index = z * mDesc.mPageCount + x;

				mPages[index]->update(pos, true);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void		PagingManager::preInit(uint32 pageSize, uint32 pageCount/* = INVALID_PARAM*/, uint32 visiblePage/* = INVALID_PARAM*/)
	{
		if (pageSize != INVALID_PARAM)
			mDesc.mPageSize = pageSize;
		if (pageCount != INVALID_PARAM)
			mDesc.mPageCount = pageCount;
		if (visiblePage != INVALID_PARAM)
			mDesc.mVisiblePages = visiblePage;

		if (mDesc.mPageCount < mDesc.mVisiblePages)
			mDesc.mPageCount = mDesc.mVisiblePages;

		this->setStartIndex(mStartX, mStartZ);	//init range
	}

	//////////////////////////////////////////////////////////////////////////
	void		PagingManager::initialize(const TStringParam& defaultElements)
	{
		uint32 pageSize = mDesc.mPageSize;
		uint32 pageCount = mDesc.mPageCount;

		mPages.resize(pageCount*pageCount);
		for (size_t i = 0; i < mPages.size(); ++i)
		{
			size_t x = i % pageCount;
			size_t z = i / pageCount;
			Vector2 pos = Vector2(scalar(x*pageSize + pageSize / 2), scalar(z*pageSize + pageSize / 2));

			mPages[i] = BLADE_NEW Page(pos, x, z, mDesc.mVisiblePages);

			mPages[i]->addLayer(IPage::DEFAULT_LAYER, defaultElements, pageSize, pageSize, (scalar)(pageSize*mDesc.mVisiblePages), IPage::DEFAULT_PRIORITY);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool		PagingManager::removeEntity(IEntity* entity)
	{
		bool removed = false;
		if (static_cast<Entity*>(entity)->isPagingEnabled())
		{
			//extend removing page range: some entity may be in unloaded pages (especially in editor mode, entities may be placed to empty pages)
			size_t totalVisible = (mDesc.mVisiblePages+2) * (mDesc.mVisiblePages+2);
			for (size_t i = 0; i < totalVisible; ++i)
			{
				size_t x = i % mDesc.mVisiblePages + std::max(0, (int)(mStartX-1));
				size_t z = i / mDesc.mVisiblePages + std::max(0, (int)(mStartZ-1));

				size_t index = z * mDesc.mPageCount + x;
				if(index >= mPages.size())
					continue;

				if (mPages[index]->removeEntity(entity))
					removed = true;	//continue removing: entity may be in multiple pages
			}
		}
		return removed;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		PagingManager::notifyEntity(IEntity* entity, bool newCreated/* = false*/, bool performLoad/* = false*/)
	{
		Entity* entityImpl = static_cast<Entity*>(entity);
		assert(entityImpl != NULL);
		if (!newCreated)
			this->removeEntity(entity);
		else
			entityImpl->postLoad(true);

		TStringParam names;
		ElementListParam elements;
		entityImpl->getAllElements(names, elements);

		AABB aab = Impl::getEntityWorldBounding(entityImpl);
		
		entityImpl->enablePaging(0);

		//skip empty/infinite bounding. entity with empty/infinite bounding will be loaded directly in stage
		bool ret = !(aab.isNull() || aab.isInfinite());

		if (ret)
		{
			//find page with aab
			TPointerParam<Page> pages;
			this->getPages(aab, pages);

			size_t count = pages.size();
			static_cast<Entity*>(entity)->enablePaging((uint16)count);

			ret = (count != 0);
			if (ret)
			{
				for (size_t k = 0; k < count; ++k)
					pages[k]->addEntity(entity, aab);
			}
		}

		if (performLoad)
			mStage->loadEntity(entity);

		return ret;
	}

}//namespace Blade