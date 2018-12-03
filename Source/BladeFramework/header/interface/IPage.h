/********************************************************************
	created:	2016/04/20
	filename: 	IPage.h
	author:		Crazii
	purpose:	paging for streaming static geometry in large scale scenes
*********************************************************************/
#ifndef __Blade_IPage_h__
#define __Blade_IPage_h__
#include <BladeFramework.h>
#include <math/Vector2i.h>
#include <utility/String.h>
#include <interface/public/IProgressCallback.h>

namespace Blade
{
	class IEntity;

	class IPage
	{
	public:
		//paging
		static const uint32 VISIBLE_PAGE_BITCOUNT = 3;
		static const uint32 VISIBlE_PAGE_MASK = (1 << VISIBLE_PAGE_BITCOUNT) - 1;
		static const uint32 MAX_VISIBLE_PAGE = 1 << VISIBLE_PAGE_BITCOUNT;
		static const uint32 MAX_PAGE_COUNT = 16;
		static const uint32 MIN_SUBPAGE_SIZE = 16;
		static const int DEFAULT_PRIORITY = 0xFFFF;

		static BLADE_FRAMEWORK_API const TString DEFAULT_LAYER;
	public:
		virtual ~IPage() {}

		/**
		@describe get page center position
		@param
		@return
		*/
		virtual const Vector2&	getPosition() const = 0;

		/**
		@describe get page indices (x,z)
		@param
		@return
		*/
		virtual Vector2i		getIndices() const = 0;

		/**
		@describe get local indices (x,z) within the range of [0,IPagingManager::DESC::mVisiblePages)
		@note local indices are fixed and irrelevant to current loading position
		@param
		@return
		*/
		virtual Vector2i		getLocalIndices() const = 0;
	};


	class IPagingManager
	{
	public:
		typedef struct SDesc
		{
			uint32		mVisiblePages;	///page per side. total visible = mVisiblePages*mVisiblePages
			uint32		mPageSize;		///size (loading range) for each page
			uint32		mPageCount;		///total page per side.
		}DESC;

		class IListener
		{
		public:
			//note: pages are always unloaded first, then load to new data (even it is initially empty)
			//that means the two event always appears in pair, the same event is never successively called twice.
			//also means onPageUnload is guaranteed to be always dispatched before onPageLoad
			//note2: if onPageUnload for a page happens in one main loop, onPageLoad for another page is guaranteed to happen in the same loop
			//note3: don't keep any references of the page parameter
			/** @brief */
			virtual void onPageUnload(IPage* page) = 0;
			/** @brief  */
			virtual void onPageLoad(IPage* page) = 0;
		};

	public:
		virtual ~IPagingManager() {}

		/**
		@describe get stage's descriptor
		@param
		@return
		*/
		virtual const DESC&	getDesc() const = 0;

		inline uint32		getPageSize() const { return this->getDesc().mPageSize; }
		inline uint32		getVisiblePageCount() const { return this->getDesc().mVisiblePages; }
		inline uint32		getPageCount() const { return this->getDesc().mPageCount; }

		/**
		@describe 
		@param
		@return
		*/
		virtual IPage*		getPage(size_t x, size_t z) const = 0;

		/**
		@describe get page enclosing specified pos
		@param
		@return
		*/
		virtual IPage*		getPageByPosition(scalar x, scalar z) const = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual size_t		getEntities(const AABB& aab, const TStringParam& elementTypes, TPointerParam<IEntity>& outEntities, size_t limit = -1) const = 0;

		inline size_t	getEntities(const AABB& aab, const TString& elementType, TPointerParam<IEntity>& outEntities, size_t limit = -1) const
		{
			TStringParam l;
			l.push_back(elementType);
			return this->getEntities(aab, l, outEntities, limit);
		}

		/**
		@describe add layer info for paging system, shared by all stages' paging manager
		@param name: specify layer name
		@param elmentTypes: list of element types managed by this layer. managed layer will be loaded/unloaded by paging system
		@param subPageSize: layers are divided to sub pages. minimal sub page size is MIN_SUBPAGE_SIZE
		@param range: loading range, sub pages will be loaded if they pass range intersection, unloaded if they out of range. there is a buffer zone to avoid frequent loads/unloads.
		@param priority: loading order of the layer, layer with high priority value will loaded first
		@return
		*/
		virtual bool		addLayer(const TString& name, const TStringParam& elementTypes, size_t subPageSize, scalar range, int priority = IPage::DEFAULT_PRIORITY) const = 0;

		/**
		@describe re-calculate paging by entity's geometry info (position, scale, rotation, bounding)
		@param
		@return
		*/
		virtual bool		resetPaging() = 0;

		/**
		@describe load page synchronously. all entity within radius of paging strategy will be loaded synchronously
		@param
		@return
		*/
		virtual bool		loadPages(const TString& path, const Vector3& position, const ProgressNotifier& notifier = ProgressNotifier::EMPTY) = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual bool		savePages(const TString& path, bool textFormat) const = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual void		update(const Vector3& position) = 0;

		/**
		@describe notify if a new static entity created or static entity geometry changed(pos,scale,rotation,bounds).
		@param 
		@return 
		*/
		virtual bool		notifyEntity(IEntity* entity, bool newCreated = false, bool performLoad = false) = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual bool		addListener(IListener* listener) = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual bool		removeListener(IListener* listener) = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual void		setEditMode(bool editMode) = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual bool		isEditMode() const = 0;
	};
	
}//namespace Blade
#endif//__Blade_IPage_h__