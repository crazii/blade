/********************************************************************
	created:	2010/05/07
	filename: 	DefaultSpace.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_DefaultSpaceManager_h__
#define __Blade_DefaultSpaceManager_h__
#include <SpaceBase.h>
#include <utility/BladeContainer.h>

namespace Blade
{
	class DefaultSpace : public SpaceBase, public Allocatable
	{
	public:
		DefaultSpace();
		~DefaultSpace();

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

		/*
		@describe get space bounds
		@param
		@return
		*/
		inline virtual const AABB&	getSpaceBound() const
		{
			return mBox;
		}

		/**
		@describe update space internal hierarchy if content moved
		@param
		@return
		*/
		virtual void						update();

		/*
		@describe directly set the space bound, maybe invalid operation and return false on some implementation.
		@param
		@return
		*/
		inline virtual bool					setSpaceBound(const AABB& box)
		{
			mBox = box;
			return true;
		}

		/**
		@describe
		@param
		@return
		*/
		virtual bool					notifyContentChanged(ISpaceContent* /*content*/) { return true; }

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

	protected:
		typedef Set<ISpaceContent*>		SpaceContentSet;

		SpaceContentSet		mSpaceContents;
		AABB				mBox;
		uint32				mPartitionMask;
	};//class DefaultSpace
	
}//namespace Blade


#endif //__Blade_DefaultSpaceManager_h__