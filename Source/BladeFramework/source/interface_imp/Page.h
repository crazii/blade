/********************************************************************
	created:	2016/04/23
	filename: 	Page.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_Page_h__
#define __Blade_Page_h__
#include <interface/IPage.h>
#include <interface/public/IElement.h>
#include <interface/IResourceManager.h>
#include <EntityResourceDesc.h>
#include "EntityPostLoader.h"
#include "Entity.h"

namespace Blade
{
	class IStage;
	class Stage;

	enum PageState
	{
		PS_EMPTY,
		PS_CACHED,
		PS_LOADING,
		PS_READY,
	};

	typedef Set<IPagingManager::IListener*>	ListenerSet;

	namespace Impl
	{
		AABB	getEntityWorldBounding(const IEntity* entity);
	}

	//////////////////////////////////////////////////////////////////////////
	///note: sub page info should not be serialized.
	///the idea is to serialize only pages, and sub pages can be changed at runtime.
	class SubPage : public Allocatable
	{
	public:
		SubPage(const TStringList* elementTypes) { mElements = elementTypes; mState = PS_EMPTY; }
		~SubPage()	{}

		/** @brief  */
		inline bool	addElements(Entity* entity)
		{
			if (entity == NULL || mElements->size() == 0)
			{
				assert(false);
				return false;
			}
			EntityList::iterator i = std::find(mEntities.begin(), mEntities.end(), entity);
			if (i != mEntities.end())
				return false;
			mEntities.push_back(entity);
			return true;
		}

		/** @brief  */
		inline bool	removeElements(Entity* entity)
		{
			EntityList::iterator i = std::find(mEntities.begin(), mEntities.end(), entity);
			if (i != mEntities.end())
			{
				mEntities.erase(i);
				return true;
			}
			return false;
		}

		/** @brief  */
		inline void	clear()
		{
			mState = PS_EMPTY;
			mEntities.clear();
		}

		/** @brief  */
		inline size_t loadAllElements(bool forceAsync)
		{
			if(mState == PS_LOADING || mState == PS_READY)
				return 0;
			return this->loadAllElementsImpl(forceAsync);
		}

		/** @brief  */
		inline size_t unloadAllElements()
		{
			if (mState == PS_EMPTY)
				return 0;
			return this->unloadAllElementsImpl();
		}

		/** @brief  */
		size_t loadAllElementsImpl(bool forceAsync)
		{
			mState = forceAsync ? PS_LOADING : PS_READY;
			size_t total = 0;

			for (EntityList::const_iterator iter = mEntities.begin(); iter != mEntities.end(); ++iter)
			{
				Entity* entity = *iter;
				for (size_t i = 0; i < mElements->size(); ++i)
				{
					ElementListParam list;
					size_t count = entity->getElementsByType(list, mElements->at(i));
					for (size_t j = 0; j < count; ++j)
						list[j]->loadResource(forceAsync);
					total += count;
				}
			}
			return total;
		}

		/** @brief  */
		size_t unloadAllElementsImpl()
		{
			mState = PS_EMPTY;
			size_t total = 0;

			for (EntityList::const_iterator iter = mEntities.begin(); iter != mEntities.end(); ++iter)
			{
				Entity* entity = *iter;
				for (size_t i = 0; i < mElements->size(); ++i)
				{
					ElementListParam list;
					size_t count = entity->getElementsByType(list, mElements->at(i));
					for (size_t j = 0; j < count; ++j)
						list[j]->unloadResource();
					total += count;
				}
			}
			return total;
		}

		/** @brief  */
		const EntityList& getEntityList() const { return mEntities; }

	protected:
		EntityList			mEntities;
		const TStringList*	mElements;
		PageState			mState;
	};


	//////////////////////////////////////////////////////////////////////////
	class PageLayer : public Allocatable
	{
	public:
		PageLayer(const Vector2& pos, fp32 subPageSize, size_t count, scalar range, const TStringParam& elementTypes, int priority)
			:mStart(pos)
			,mSize(subPageSize)
			,mRange(range)
			,mPriority(priority)
			,mCount((uint16)count)
			,mState(uint16(PS_EMPTY))
		{
			mLastPos = Vector2::NEGATIVE_UNIT_ALL*FLT_MAX;
			mElementTypes = elementTypes;
			mSubPages.resize(count*count);
			for (size_t i = 0; i < mSubPages.size(); ++i)
				mSubPages[i] = BLADE_NEW SubPage(&mElementTypes);
		}

		~PageLayer()
		{
			for (size_t i = 0; i < mSubPages.size(); ++i)
				BLADE_DELETE mSubPages[i];
		}

		/** @brief  */
		inline int getPriority() const { return mPriority; }

		/** @brief  */
		inline bool	addEntity(Entity* entity, const AABB& aab)
		{
			ElementListParam elements;
			for (size_t j = 0; j < mElementTypes.size(); ++j)
			{
				const TString& elemType = mElementTypes[j];
				entity->getElementsByType(elements, elemType);
			}
			//no valid elements
			if (elements.size() == 0)
				return false;

			TPointerParam<SubPage> subPages;
			const Vector2& bmin = Vector2(aab.getMinPoint().x, aab.getMinPoint().z);
			const Vector2& bmax = Vector2(aab.getMaxPoint().x, aab.getMaxPoint().z);
			this->getSubPages(Box2(bmin, bmax), subPages);
			assert(subPages.size() > 0);

			bool ret = subPages.size() > 0;
			for (size_t i = 0; i < subPages.size(); ++i)
			{
				if (!subPages[i]->addElements(entity))
					ret = false;
			}
			return  ret;
		}

		/** @brief  */
		inline bool	removeEntity(IEntity* entity)
		{
			bool ret = true;
			for (size_t i = 0; i < mSubPages.size(); ++i)
			{
				if (!mSubPages[i]->removeElements(static_cast<Entity*>(entity)))
					ret = false;
			}
			return  ret;
		}

		/** @brief  */
		inline size_t getSubPages(const Box2& aab, TPointerParam<SubPage>& outSubPages) const
		{
			Vector2 bmin = aab.getMinPoint();
			Vector2 bmax = aab.getMaxPoint();

			Box2 box(mStart + Math::LOW_EPSILON, mStart + mSize*(scalar)mCount - Math::LOW_EPSILON);
			if (!box.intersect(aab))
				return 0;

			bmin = (bmin - mStart) / mSize;
			bmax = (bmax - mStart) / mSize;

			int startZ = std::max((int)bmin.y, (int)0);
			int endZ = std::min((int)bmax.y, (int)mCount-1);
			int startX = std::max((int)bmin.x, (int)0);
			int endX = std::min((int)bmax.x, (int)mCount-1);

			for (int i = startZ; i <= endZ; ++i)
			{
				for (int j = startX; j <= endX; ++j)
				{
					SubPage* subPage = mSubPages[size_t(i*mCount + j)];
					outSubPages.push_back(subPage);
				}
			}
			return outSubPages.size();
		}

		/** @brief  */
		inline void clear()
		{
			for(size_t i = 0; i < mSubPages.size(); ++i)
				mSubPages[i]->clear();

			mLastPos = Vector2::NEGATIVE_UNIT_ALL*FLT_MAX;
			mState = PS_EMPTY;
		}

		/** @brief  */
		inline void unload(bool doClear = true)
		{
			for (size_t i = 0; i < size_t(mCount*mCount); ++i)
			{
				mSubPages[i]->unloadAllElements();
				if(doClear)
					mSubPages[i]->clear();
			}

			mLastPos = Vector2::NEGATIVE_UNIT_ALL*FLT_MAX;
			mState = PS_EMPTY;
		}

		/** @brief  */
		inline void	update(const Vector2& position, bool forceAsync, size_t limit = -1)
		{
			static const float UNLOAD_RANGE_EXP = 1.5f;

			BLADE_UNREFERENCED(limit);
			//minimal step
			Vector2 posDiff = position - mLastPos;
			if (Math::Abs(posDiff.x) < mSize && Math::Abs(posDiff.y) < mSize)
				return;

			const Vector2 lastPos = mLastPos;
			mLastPos = position;

			const Box2 box(mStart, mStart + mSize*(scalar)mCount);
			const Vector2 halfSize = Vector2::UNIT_ALL*mRange;

			//unloading range: entity outside this range will be unloaded
			//it is larger than loading range, to avoid frequent loading-unloading pingpong
			const Box2 unloadingRangeBox(position - halfSize*UNLOAD_RANGE_EXP, position + halfSize*UNLOAD_RANGE_EXP);

			//out of range
			if ( !unloadingRangeBox.intersect(box) )
			{
				if (mState == PS_CACHED)
				{
					for (size_t i = 0; i < size_t(mCount*mCount); ++i)
						mSubPages[i]->unloadAllElements();
					mState = PS_EMPTY;
				}
				return;
			}

			//single sub page
			if (mCount == 1)
			{
				if (mState != PS_CACHED)
				{
					mSubPages[0]->loadAllElements(forceAsync);
					mState = PS_CACHED;
				}
				return;
			}

			//loaded count this time
			size_t count = 0;

			//unload
			{
				/*
					     +-----------+
					     |   new     |
					     |           |
					  +---------+    |
					  |         |    |
					ez+    old  |----+
					  |         |
					sz+--+------+
					sx  ex
				*/

				const Box2 oldRangeBox(lastPos - halfSize*UNLOAD_RANGE_EXP, lastPos + halfSize*UNLOAD_RANGE_EXP);

				Vector2 bmin = (unloadingRangeBox.getMinPoint() - mStart) / mSize;
				Vector2 bmax = (unloadingRangeBox.getMaxPoint() - mStart) / mSize;

				int newStartZ = (int)bmin.y;
				int newEndZ = (int)bmax.y;
				int newStartX = (int)bmin.x;
				int newEndX = (int)bmax.x;
				newStartZ = Math::Clamp(newStartZ, (int)0, (int)mCount - 1);
				newEndZ = Math::Clamp(newEndZ, (int)0, (int)mCount - 1);
				newStartX = Math::Clamp(newStartX, (int)0, (int)mCount - 1);
				newEndX = Math::Clamp(newEndX, (int)0, (int)mCount - 1);

				bmin = (oldRangeBox.getMinPoint() - mStart) / mSize;
				bmax = (oldRangeBox.getMaxPoint() - mStart) / mSize;
				int oldStartZ = (int)bmin.y;
				int oldEndZ = (int)bmax.y;
				int oldStartX = (int)bmin.x;
				int oldEndX = (int)bmax.x;
				oldStartZ = Math::Clamp(oldStartZ, (int)0, (int)mCount - 1);
				oldEndZ = Math::Clamp(oldEndZ, (int)0, (int)mCount - 1);
				oldStartX = Math::Clamp(oldStartX, (int)0, (int)mCount - 1);
				oldEndX = Math::Clamp(oldEndX, (int)0, (int)mCount - 1);

				int startx = oldStartX;
				int endx = oldEndX;
				if (oldStartX >= newStartX)
					startx = std::max(startx, newEndX + 1);
				else
					endx = std::min(endx, newStartX - 1);

				int startz = oldStartZ;
				int endz = oldEndZ;
				if (oldStartZ >= newStartZ)
					startz = std::max(startz, newEndZ + 1);
				else
					endz = std::min(endz, newStartZ - 1);

				startx = Math::Clamp(startx, (int)0, (int)mCount);
				endx = Math::Clamp(endx, (int)0, (int)mCount - 1);
				startz = Math::Clamp(startz, (int)0, (int)mCount);
				endz = Math::Clamp(endz, (int)0, (int)mCount - 1);

				if (startx <= endx)
				{
					for (int i = oldStartZ; i <= oldEndZ; ++i)
					{
						for(int j = startx; j <= endx; ++j)
							mSubPages[size_t(i*mCount + j)]->unloadAllElements();
					}
				}

				startx = std::min(startx, mCount - 1);
				for (int i = startz; i <= endz; ++i)
				{
					for(int j = oldStartX; j <= startx; ++j)
						mSubPages[size_t(i*mCount + j)]->unloadAllElements();

					for (int j = endx; j <= oldEndX; ++j)
						mSubPages[size_t(i*mCount + j)]->unloadAllElements();
				}
			}

			//load
			{
				const Box2 newRangeBox(position - halfSize, position + halfSize);
				const Box2 oldRangeBox(lastPos - halfSize, lastPos + halfSize);

				Vector2 bmin = (newRangeBox.getMinPoint() - mStart) / mSize;
				Vector2 bmax = (newRangeBox.getMaxPoint() - mStart) / mSize;

				int newStartZ = (int)bmin.y;
				int newEndZ = (int)bmax.y;
				int newStartX = (int)bmin.x;
				int newEndX = (int)bmax.x;
				newStartZ = Math::Clamp(newStartZ, (int)0, (int)mCount - 1);
				newEndZ = Math::Clamp(newEndZ, (int)0, (int)mCount - 1);
				newStartX = Math::Clamp(newStartX, (int)0, (int)mCount - 1);
				newEndX = Math::Clamp(newEndX, (int)0, (int)mCount - 1);

				bmin = (oldRangeBox.getMinPoint() - mStart) / mSize;
				bmax = (oldRangeBox.getMaxPoint() - mStart) / mSize;
				int oldStartZ = (int)bmin.y;
				int oldEndZ = (int)bmax.y;
				int oldStartX = (int)bmin.x;
				int oldEndX = (int)bmax.x;
				oldStartZ = Math::Clamp(oldStartZ, (int)0, (int)mCount - 1);
				oldEndZ = Math::Clamp(oldEndZ, (int)0, (int)mCount - 1);
				oldStartX = Math::Clamp(oldStartX, (int)0, (int)mCount - 1);
				oldEndX = Math::Clamp(oldEndX, (int)0, (int)mCount - 1);

				int startx = newStartX;
				int endx = newEndX;
				if (newStartX >= oldStartX)
					startx = std::max(startx, oldEndX + 1);
				else
					endx = std::min(endx, oldStartX - 1);

				int startz = newStartZ;
				int endz = newEndZ;
				if (newStartZ >= oldStartZ)
					startz = std::max(startz, oldEndZ + 1);
				else
					endz = std::min(endz, oldStartZ - 1);

				startx = Math::Clamp(startx, (int)0, (int)mCount - 1);
				endx = Math::Clamp(endx, (int)0, (int)mCount - 1);
				startz = Math::Clamp(startz, (int)0, (int)mCount - 1);
				endz = Math::Clamp(endz, (int)0, (int)mCount - 1);

				if (startx <= endx)
				{
					for (int i = newStartZ; i <= newEndZ; ++i)
					{
						for (int j = startx; j <= endx; ++j)
							count += mSubPages[size_t(i*mCount + j)]->loadAllElements(forceAsync);
					}
				}

				for (int i = startz; i <= endz; ++i)
				{
					for (int j = newStartX; j <= startx; ++j)
						count += mSubPages[size_t(i*mCount + j)]->loadAllElements(forceAsync);

					for (int j = endx; j <= newEndX; ++j)
						count += mSubPages[size_t(i*mCount + j)]->loadAllElements(forceAsync);
				}

				if (count > 0 && mState != PS_CACHED)
					mState = PS_CACHED;
			}
		}

		/** @brief  */
		inline size_t getEntities(const Box2& aab2, const TStringParam& elementTypes, TPointerParam<IEntity>& outEntities, size_t limit) const
		{
			for (size_t i = 0; i < elementTypes.size(); ++i)
			{
				if(mElementTypes.find(elementTypes[i]) == INVALID_INDEX)
					continue;

				TPointerParam<SubPage> subPages;
				this->getSubPages(aab2, subPages);
				for (size_t j = 0; j < subPages.size(); ++j)
				{
					const EntityList& entities = subPages[j]->getEntityList();
					for (EntityList::const_iterator k = entities.begin(); k != entities.end(); ++k)
					{
						AABB bounds = Impl::getEntityWorldBounding(*k);
						assert(!bounds.isNull() && !bounds.isInfinite());
						Box2 bounds2(bounds.getMinPoint().x, bounds.getMinPoint().z, bounds.getMaxPoint().x, bounds.getMaxPoint().z);
						if (aab2.intersect(bounds2) && outEntities.find(*k) == INVALID_INDEX)	//entity may cross page or multiple element matches
						{
							outEntities.push_back(*k);
							if (outEntities.size() >= limit)
								return limit;
						}
					}
				}
			}
			return outEntities.size();
		}

	protected:
		typedef Vector<SubPage*>	SubPageList;

		Vector2		mLastPos;	//loading check pos
		Vector2		mStart;
		fp32		mSize;	//sub page size
		fp32		mRange;	//loading range;
		int32		mPriority;
		uint16		mCount;
		uint16		mState;	//PageState
		SubPageList	mSubPages;
		TStringList mElementTypes;
	};
	
	//////////////////////////////////////////////////////////////////////////
	class Page : public IPage, public IResource::IListener, public Allocatable
	{
	public:
		typedef TStringMap<PageLayer*> LayerMap;
		typedef List<PageLayer*> PrioriyLayers;	//layer sorted by priority
	public:
		Page(const Vector2& pos, size_t x, size_t z, size_t visibleCount);
		virtual ~Page();

		/************************************************************************/
		/* IPage interface                                                                     */
		/************************************************************************/
		/**
		@describe get page center position
		@param
		@return
		*/
		virtual const Vector2&	getPosition() const
		{
			return mPosition;
		}

		/**
		@describe get page indices (x,z)
		@param
		@return
		*/
		virtual Vector2i		getIndices() const
		{
			return Vector2i((int)mX, (int)mZ);
		}

		/**
		@describe get local indices (x,z)
		@param
		@return
		*/
		virtual Vector2i		getLocalIndices() const
		{
			return Vector2i((int)mLocalX, (int)mLocalZ);
		}

		/************************************************************************/
		/* IResource::IListener interface                                                                     */
		/************************************************************************/
		/**
		@describe 
		@param
		@return
		*/
		virtual void	postLoad(const HRESOURCE& resource)
		{
			mPageResource = resource;
		}
		/**
		@describe when loading succeed
		@param
		@return
		*/
		virtual bool	onReady()
		{
			assert(mPageResource != NULL);
			if(mStatus == PS_LOADING)
				mStatus = PS_CACHED; 
			else
			{
				//unloaded right after perform caching
				assert(mStatus == PS_EMPTY);
				IResourceManager::getSingleton().unloadResource(mPageResource);
			}
			return true;
		}

		/**
		@describe when loading failed
		@param
		@return
		*/
		virtual bool	onFailed() { mStatus = PS_CACHED; return true; }

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		bool	addEntity(IEntity* entity, const AABB& aab, bool inLoading = false);

		/** @brief add entity while loading */
		bool	addEntity(IEntity* entity, bool inLoading = false);

		/** @brief  */
		bool	removeEntity(IEntity* entity);

		/** @brief  */
		bool	addLayer(const TString& name, const TStringParam& elementTypes, size_t subPageSize, size_t pageSize, scalar range, int priority);

		/** @brief  */
		bool	cachePage(const TString& path, IStage* stage, bool createOnFail, const ListenerSet& listeners);

		/** @brief */
		bool	loadPage(const TString& path, IStage* stage, bool createOnFail, const ListenerSet& listeners, const ProgressNotifier& notifier);

		/** @brief  */
		bool	savePage(const TString& path, IStage* stage, bool textFormat);

		/** @brief  */
		bool	unloadPage(Stage* stage, const ListenerSet& listeners, bool saveCurrent);

		/** @brief  */
		bool	unloadImpl(Stage* stage, bool destroyEntities = true);

		/** @brief load cached page */
		void	update(const Vector2& position, bool forceAsync);

		/** @brief  */
		inline void clear()
		{
			for(LayerMap::iterator i = mLayerMap.begin(); i != mLayerMap.end(); ++i)
				i->second->clear();

			if (mStatus == PS_LOADING)
			{
				assert(mPageResource == NULL);
				assert(this->isListening());
				this->cancel();
			}
			if (mPageResource != NULL)
				IResourceManager::getSingleton().unloadResource(mPageResource);
			mStatus = PS_EMPTY;
		}

		/** @brief  */
		inline size_t getEntities(const Box2& aab, const TStringParam& elementTypes, TPointerParam<IEntity>& outEntities, size_t limit) const
		{
			for (LayerMap::const_iterator i = mLayerMap.begin(); i != mLayerMap.end(); ++i)
			{
				if (i->second->getEntities(aab, elementTypes, outEntities, limit) >= limit)
					break;
			}
			return outEntities.size();
		}

		/** @brief  */
		inline PageState	getLoadStatus() const { return (PageState)mStatus; }

		//empty:	unloaded empty page, mPageResource should be NULL
		//loading:	loading page, mPageResource should be NULL
		//cached && mPageResource != NULL:	loaded page
		//cached && mPageResource == NULL:	unloaded page but not empty, with new entity added (edit mode only), mModified should be true.
		//									this usually happens when an full-map component created for each page (i.e. terrains), or new entity create & placed at 
		//									visible page bounds, the entity will also be put into empty neighbor pages.

		/** @brief  */
		inline bool isEmpty() const
		{
			assert(mStatus != PS_EMPTY || mPageResource == NULL);
			return mStatus == PS_EMPTY;
		}

		/** @brief  */
		inline bool isLoading() const
		{
			assert(mStatus != PS_LOADING || mPageResource == NULL);
			return mStatus == PS_LOADING;
		}

		/** @brief  */
		inline bool isLoaded() const
		{
			assert(mPageResource == NULL || mStatus == PS_CACHED);
			return mPageResource != NULL && mStatus == PS_CACHED;
		}

		/** @brief  */
		inline bool	isUnloaded() const
		{
			assert(!(mPageResource == NULL && mStatus == PS_CACHED) || mModified);
			assert(mPageResource == NULL || mStatus == PS_CACHED);
			return mPageResource == NULL && (mStatus == PS_EMPTY || mStatus == PS_CACHED);
		}

		/** @brief  */
		inline void	setCenter(const Vector2& pos) { mPosition = pos; }
		/** @brief  */
		inline const EntitySet& getEntities() const { return mEntities; }

	protected:
		EntitySet	mEntities;
		LayerMap	mLayerMap;
		PrioriyLayers mOrderedLayers;
		Vector2		mPosition;
		HRESOURCE	mPageResource;
		TStringList	mSharedList;

		uint16		mX;
		uint16		mZ;
		uint8		mLocalX;
		uint8		mLocalZ;
		uint8		mStatus;
		uint8		mModified : 1;
		uint8		mInitial : 1;	//initial resource: edit mode only. won't do auto save, and entities won't be destroyed
	};

	class PagingManager : public IPagingManager, public Allocatable
	{
	public:
		PagingManager(Stage* stage);
		virtual ~PagingManager();

		/**
		@describe get stage's descriptor
		@param
		@return
		*/
		virtual const DESC&	getDesc() const { return mDesc; }

		/**
		@describe
		@param
		@return
		*/
		virtual IPage*		getPage(size_t x, size_t z) const;

		/**
		@describe get page enclosing specified pos
		@param
		@return
		*/
		virtual IPage*		getPageByPosition(scalar x, scalar z) const;

		/**
		@describe 
		@param
		@return
		*/
		virtual size_t		getEntities(const AABB& aab, const TStringParam& elementTypes, TPointerParam<IEntity>& outEntities, size_t limit = -1) const;

		/**
		@describe
		@param
		@return
		*/
		virtual bool		addLayer(const TString& name, const TStringParam& elementTypes, size_t subPageSize, scalar range, int priority = IPage::DEFAULT_PRIORITY) const;

		/**
		@describe re-calculate paging by entity's geometry info (position, scale, rotation, bounding)
		@param
		@return
		*/
		virtual bool		resetPaging();

		/**
		@describe load page synchronously
		@param
		@return
		*/
		virtual bool		loadPages(const TString& path, const Vector3& position, const ProgressNotifier& notifier = ProgressNotifier::EMPTY);

		/**
		@describe
		@param
		@return
		*/
		virtual bool		savePages(const TString& path, bool textFormat) const;

		/**
		@describe
		@param
		@return
		*/
		virtual void		update(const Vector3& position);

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool		notifyEntity(IEntity* entity, bool newCreated = false, bool performLoad = false);

		/**
		@describe
		@param
		@return
		*/
		virtual bool		addListener(IListener* listener) { return listener != NULL && mListeners.insert(listener).second; }

		/**
		@describe
		@param
		@return
		*/
		virtual bool		removeListener(IListener* listener) { return mListeners.erase(listener) == 1; }

		/**
		@describe
		@param
		@return
		*/
		virtual void		setEditMode(bool editMode) { mEditMode = editMode; }

		/**
		@describe
		@param
		@return
		*/
		virtual bool		isEditMode() const { return mEditMode; }

		/************************************************************************/
		/*                                                                      */
		/************************************************************************/
		static const uint32 INVALID_PARAM = uint32(-1);
		/**
		@describe 
		@param 
		@return 
		*/
		void		preInit(uint32 pageSize, uint32 pageCount = INVALID_PARAM, uint32 visiblePage = INVALID_PARAM);

		/**
		@describe 
		@param defaultElements elements for default layer
		@return
		*/
		void		initialize(const TStringParam& defaultElements);

		/**
		@describe 
		@param
		@return
		*/
		bool		removeEntity(IEntity* entity);

		/** @brief  */
		inline void setStartIndex(size_t startX, size_t startZ)
		{
			mStartX = startX;
			mStartZ = startZ;
			Vector2 bmin = Vector2((scalar)mStartX, (scalar)mStartZ) * (scalar)mDesc.mPageSize;
			Vector2 bmax = bmin + Vector2::UNIT_ALL * (scalar)(mDesc.mVisiblePages*mDesc.mPageSize);
			mRange.set(bmin, bmax);
		}

	protected:

		/** @brief  */
		inline size_t		getPages(const AABB& worldAAB, TPointerParam<Page>& outPages) const
		{
			Vector2 bmin = Vector2(worldAAB.getMinPoint().x, worldAAB.getMinPoint().z);
			Vector2 bmax = Vector2(worldAAB.getMaxPoint().x, worldAAB.getMaxPoint().z);
			Box2 worldAAB2 = Box2(bmin, bmax);
			for (size_t i = 0; i < mPages.size(); ++i)
			{
				Page* page = mPages[i];
				const Vector2& center = page->getPosition();
				scalar halfSize = (scalar)mDesc.mPageSize / 2 - Math::LOW_EPSILON;
				Box2 pageBox = Box2(center - halfSize, center + halfSize);

				if (pageBox.intersect(worldAAB2))
					outPages.push_back(page);
			}
			return outPages.size();
		}

		typedef Vector<Page*>	PageList;
		typedef TStringMap<Page*> ShareList;

		Stage*		mStage;
		Box2		mRange;
		size_t		mStartX;
		size_t		mStartZ;

		DESC		mDesc;
		TString		mPath;
		PageList	mPages;
		ShareList	mSharedPages;
		ListenerSet	mListeners;
		bool		mEditMode;
	};
	
}//namespace Blade

#endif//__Blade_Page_h__