/********************************************************************
	created:	2010/04/30
	filename: 	SpaceCoordinatorBase.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_SpaceCoordinatorBase_h__
#define __Blade_SpaceCoordinatorBase_h__
#include <interface/ISpaceCoordinator.h>

namespace Blade
{

	class BLADE_GRAPHICS_API SpaceCoordinatorBase : public ISpaceCoordinator, public Allocatable
	{
	public:
		SpaceCoordinatorBase(const TString& coordinatorType);
		virtual ~SpaceCoordinatorBase();

		/************************************************************************/
		/* ISpace interface                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		inline virtual const TString&	getSpaceType() const
		{
			return mCoordinatorType;
		}

		/*
		@describe
		@param
		@return
		*/
		inline virtual ISpaceCoordinator*getCoordinator() const
		{
			return const_cast<SpaceCoordinatorBase*>(this);
		}

		/*
		@describe
		@param
		@return
		*/
		inline virtual bool				initialize(ISpaceCoordinator* /*pcoordinator*/, const AABB& /*maxBound*/, uint32 /*partitionMask*/)
		{
			return false;
		}

		/*
		@describe 
		@param 
		@return 
		*/
		inline virtual uint32				getSpaceFlag() const
		{
			return SF_DYNAMIC|SF_VISIBLE|SF_SOLID;
		}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void					setSpaceFlag(uint32 /*flag*/) { assert(false); }

		/**
		@describe 
		@param [in] partitionMask: used for fast locating partition, space should mask out EXTRA_PARITION_MASK by using PARTITION_MASK
		@param [in] strictFit: check content's size against space's internal cell size, should be exactly match, or return false.
		@return
		*/
		virtual bool					addContent(ISpaceContent* content, uint32 partitionMask = ISpace::INVALID_PARTITION, bool strictFit = false);

		/*
		@describe
		@param
		@return
		*/
		virtual bool					removeContent(ISpaceContent* content);

		/**
		@describe
		@param
		@return
		*/
		virtual bool					notifyContentChanged(ISpaceContent* content);

		/*
		@describe
		@param
		@return
		*/
		virtual void					findVisibleContent(const Frustum& frustum,IVisibleContentSet& vcs);

		/*
		@describe
		@param
		@return
		*/
		inline virtual const AABB&		getSpaceBound() const
		{
			return mBound;
		}

		/**
		@describe update space internal hierarchy if content moved
		@param
		@return
		*/
		virtual void						update();

		/*
		@describe
		@param
		@return
		*/
		inline virtual bool					setSpaceBound(const AABB& /*box*/)
		{
			return false;
		}

		/*
		@describe
		@param
		@return
		*/
		virtual bool					queryContents(SpaceQuery& query, EContentSpaceFlag flag, scalar& distLimit) const;

		/**
		@describe 
		@param
		@return
		*/
		virtual void					clearContents() {}

		/**
		@describe move space to another position
		@param
		@return
		*/
		virtual void					translateEmptySpaceImpl(const Vector3& /*center*/) {}

		/************************************************************************/
		/*ISpaceCoordinator interface                                                                      */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		inline virtual const TString&	getCoordinatorType() const
		{
			return mCoordinatorType;
		}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool				initialize(const COORDINATOR_DESC* desc);

		/**
		@describe 
		@param
		@return
		*/
		inline virtual const COORDINATOR_DESC*	getDesc() const
		{
			return mDesc;
		}

		/*
		@describe
		@param
		@return
		*/
		virtual bool				setPositionReference(IPositionReference* treference) { mReference = treference; return true; }

		/*
		@describe 
		@param 
		@return 
		*/
		virtual IPositionReference*	getPositionReference() const { return mReference; }

		/**
		@describe x,z in range of [0,COORDINATOR_DESC.visiblePage]
		@param
		@return
		*/
		virtual ISpace*				getSpaceofPage(index_t x, index_t z) const
		{
			return this->getSpaceofPage(x, z, false);
		}

		/************************************************************************/
		/* IPagingManager::IListener interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual void onPageUnload(IPage* page);

		/** @brief  */
		virtual void onPageLoad(IPage* page);

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		void		unLoadPage(IPage* page, ISpace* space);
		/** @brief  */
		void		movePage(IPage* page, ISpace* space);

		/** @brief  */
		ISpace*		getSpaceofPage(index_t x, index_t z, bool dynamic) const;

	protected:
		typedef Vector<ISpace*>	SpaceForPages;
		TString			mCoordinatorType;
		AABB			mBound;

		const COORDINATOR_DESC*	mDesc;	//observer
		IPositionReference*	mReference;
		SpaceForPages	mSpaces;
		SpaceForPages	mDynamicSpaces;
		ISpace*			mDefualtImpl;
	};
	
}//namespace Blade


#endif //__Blade_SpaceCoordinatorBase_h__