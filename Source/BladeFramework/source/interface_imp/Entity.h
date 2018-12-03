/********************************************************************
	created:	2010/04/08
	filename: 	Entity.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_Entity_h__
#define __Blade_Entity_h__
#include <new>
#include <limits>
#include <memory>
#include <utility/BladeContainer.h>
#include <interface/IEntity.h>
#include <parallel/ParaStateGroup.h>
#include <BladeBase_blang.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	typedef enum EEntityStatus
	{
		ES_READY,
		ES_LOADING,			//entity or elements being loaded
		ES_DELETE_LOADING,	//delete while in async loading
		ES_INVALID,			//not used, only as parameter
	}ENTITY_SATUS;

	static const TString ENTITY_BNDING_CONFIG = BTString("entity");
	const TString ENTITY_NAME = BTString(BLANG_NAME);
	const TString ENTITY_PAGES = BTString("pages");

	class Entity : public IEntity, public Bindable, public Allocatable
	{
	public:
		Entity(const TString& name, IStage* stage);
		virtual ~Entity();

		/************************************************************************/
		/* IEntity interface                                                                     */
		/************************************************************************/
		/**
		@describe
		@param
		@return
		*/
		virtual const TString&	getName() const
		{
			return mName;
		}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual IStage*		getStage() const
		{
			return mStage;
		}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void		removeElement(const TString& name);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual IElement*	getElement(const TString& name) const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual size_t		getAllElements(TStringParam& nameList, ElementListParam& elemList) const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual size_t		getElementsByType(ElementListParam& elemList, const TString& type, TStringParam* nameList = NULL) const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual size_t		getElementsBySystemType(ElementListParam& elemList, const TString& sytemType) const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void		addElement(const TString& name,const HELEMENT& elem);

		/*
		@describe get interface supported by elements
		@param
		@return
		*/
		virtual bool		getInterfaces(InterfaceName type, TPointerParam<Interface>& interfaceList) const;

		/************************************************************************/
		/* custom methods                                                                      */
		/************************************************************************/
		typedef	TStringMap<HELEMENT>		ElementMap;

		/*
		@describe this method returns STL object, so it can (/better) not be called across DLL boundaries
		@param 
		@return 
		*/
		const ElementMap&	getElements() const;

		/** @brief notify all elements prepare saving */
		void		preSave() const;
		/** @brief notify all elements afeter saving  */
		void		postSave() const;

		/** @brief  */
		void		instantLoad(const ProgressNotifier& notifier);
		inline void instantLoad()
		{
			ProgressNotifier notifier;
			return this->instantLoad(notifier);
		}

		/** @brief notify entity & elements loaded (element resource not loaded) */
		void		postLoad(const ProgressNotifier& notifier, bool succeeded);
		inline void	postLoad(bool succeeded)
		{
			ProgressNotifier notifier;
			return this->postLoad(notifier, succeeded);
		}

		/** @brief notify all elements resource loaded (success or failed) */
		void		postProcess(const ProgressNotifier& notifier);
		inline void	postProcess()
		{
			ProgressNotifier notifier;
			this->postProcess(notifier);
		}

		/** @brief  */
		inline bool isReady() const { return mStatus == ES_READY; }

		/** @brief  */
		inline bool	isLoading() const
		{
			bool ret = mStatus == ES_LOADING;
			assert(!ret || mResource == NULL);
			return ret;
		}

		/** @brief  */
		inline bool	isPendingDeleted() const { return mStatus == ES_DELETE_LOADING; }

		/** @brief  */
		inline bool hasResource() const
		{
			bool ret = mResource != NULL;
			return ret;
		}

		/** @brief  */
		HRESOURCE&	getResource()
		{
			return mResource;
		}

		/** @brief  */
		inline void	setResource(const HRESOURCE& res)
		{
			mResource = res;
		}

		/** @brief  */
		inline void setStatus(ENTITY_SATUS es)
		{
			mStatus = (uint8)es;
		}

		/** @brief  */
		inline ENTITY_SATUS	getStatus() const
		{
			return (ENTITY_SATUS)mStatus;
		}

		/** @brief  */
		size_t	getElementLoadingCount() { return mLoadingCount; }

		/** @brief  */
		void changeElementLoadingCount(int16 count) { assert(count >= 0 || mLoadingCount >= -count); mLoadingCount += count; }

		/** @brief  */
		inline void		setName(const TString& newName)
		{
			mName = newName;
		}

		/** @brief  */
		inline void		enablePaging(uint16 crossedPages)
		{
			mMaxPages = crossedPages;
		}

		/** @brief  */
		inline bool		isPagingEnabled() const
		{
			return mMaxPages > 0;
		}

		/** @brief  */
		inline uint16	getMaxPages() const { return mMaxPages; }

		/** @brief  */
		inline void		incrementPage() { ++mCurrentPages; assert(mCurrentPages >= 1 && mCurrentPages <= mMaxPages); }

		/** @brief  */
		inline uint16	decrementPage() { --mCurrentPages; assert(mCurrentPages >= 0 && mCurrentPages < mMaxPages); return mCurrentPages; }

		/** @brief  */
		inline uint16	getPageCount() const { return mCurrentPages; }

		/** @brief  */
		const TString&	getName(index_t) const { return mName; }
		const uint16&	getMaxPages(index_t) const { return mMaxPages; }
		bool			setMaxPages(index_t, const uint16& pages) { mMaxPages = pages; return true; }

	protected:
		/*
		@describe 
		@param 
		@return 
		*/
		void		addElement(const HELEMENT& elem);

		/*
		@describe 
		@param 
		@return 
		*/
		void		removeAllElements();

		/**
		@describe 
		@param
		@return
		*/
		void		addParallelStates(IElement* element);

	protected:
		typedef TStringMap<ParaStateGroup>	StateGroupMap;
		TString			mName;
		ElementMap		mElementMap;
		StateGroupMap	mGroupMap;

		IStage*			mStage;
		HRESOURCE		mResource;
		uint16			mMaxPages;
		uint16			mCurrentPages;
		uint16			mLoadingCount;			//loading element count
		uint8			mStatus;				//states managed by stage
		uint8			mSynchronized;			//entity postLoad called, element synchronized, element resources may not loaded
	};//class Entity

	//////////////////////////////////////////////////////////////////////////
	struct FnEntityPtrLess
	{
		bool	operator()(const Entity* lhs, const Entity* rhs) const
		{
			return FnTStringFastLess::compare(lhs->getName(), rhs->getName());
		}
	};

	//////////////////////////////////////////////////////////////////////////
	class EntitySet : public Set<Entity*, FnEntityPtrLess>
	{
	public:
		/** @brief  */
		Lock&	getLock() const { return mLock; }
	protected:
		mutable Lock	mLock;
	};

	//////////////////////////////////////////////////////////////////////////
	class EntityFinder : public Entity
	{
	public:
		EntityFinder(const TString& name)
			:Entity(name, NULL)
		{
			
		}

		~EntityFinder()
		{
			
		}
	};

	typedef List<Entity*> EntityList;
	
}//namespace Blade


#endif //__Blade_Entity_h__