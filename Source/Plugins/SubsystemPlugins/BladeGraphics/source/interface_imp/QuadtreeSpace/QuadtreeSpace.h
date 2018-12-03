/********************************************************************
	created:	2010/05/09
	filename: 	QuadtreeSpace.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_QuadtreeSpace_h__
#define __Blade_QuadtreeSpace_h__
#include "Quadtree.h"

namespace Blade
{
	class QuadtreeSpace : public SpaceBase, public Quadtree
	{
	public:
		QuadtreeSpace();
		virtual ~QuadtreeSpace();

		/************************************************************************/
		/* ISpace interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param [in] maxBound: max limit of the whole space.
		@param [in] maxLevel: sub space divide level.
		@return 
		@remarks: maxBound & maxLevel are just hint for the implementation, they may not be used.
		TODO: use implementation defined config via IConfig interface.
		*/
		virtual bool					initialize(ISpaceCoordinator* coorinator, const AABB& maxBound, uint32 partitionMask);

		/**
		@describe update space internal hierarchy if content moved
		@param
		@return
		*/
		virtual void					update();

		/*
		@describe get space bounds
		@param
		@return
		*/
		virtual const AABB&	getSpaceBound() const
		{
			return mMaxBound;
		}

		/*
		@describe directly set the space bound, maybe invalid operation and return false on some implementation.
		@param
		@return
		*/
		virtual bool					setSpaceBound(const AABB& /*box*/)
		{
			return false;
		}

		/**
		@describe
		@param
		@return
		*/
		virtual bool					notifyContentChanged(ISpaceContent* content);

		/*
		@describe add content to space
		@param [in] strictFit: check content's size against space's internal cell size, should be exactly match, or return false.
		@return
		*/
		virtual bool					addContentImpl(ISpaceContent* content, uint32 partitionMask, bool strictFit = false);

		/*
		@describe
		@param [in] content's spaceData if has any
		@return
		*/
		virtual bool					removeContentImpl(ISpaceContent* content);

		/*
		@describe
		@param
		@return
		*/
		virtual void					findVisibleContent(const Frustum& frustum, IVisibleContentSet& vcs);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool					queryContents(SpaceQuery& query, EContentSpaceFlag flag, scalar& distLimit) const;

		/**
		@describe clear all contents
		@param
		@return
		*/
		virtual void					clearContents();

		/**
		@describe move space to another position
		@param
		@return
		*/
		virtual void					translateEmptySpaceImpl(const Vector3& center);

		using ISpace::setSpaceData;

	protected:

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		static inline void ClearContentData(ISpaceContent* content)
		{
			ISpace::clearSpace(content, false);
			ISpace::setSpaceData(content, NULL);
		}

	protected:
		typedef TempSet<ISpaceContent*>	UpdateList;

		AABB		mMaxBound;
		Lock		mChangedLock;
		UpdateList	mChangedContents;
	};//QuadtreeSpaceManager
	
}//namespace Blade


#endif //__Blade_QuadtreeSpace_h__