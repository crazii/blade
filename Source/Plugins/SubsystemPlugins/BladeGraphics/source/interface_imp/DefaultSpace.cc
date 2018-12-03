/********************************************************************
	created:	2010/05/07
	filename: 	DefaultSpace.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "DefaultSpace.h"
#include <SpaceContent.h>
#include <interface/ISpaceCoordinator.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	DefaultSpace::DefaultSpace()
		:SpaceBase(BTString("DefaultSpaceType"))
		,mPartitionMask(0)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	DefaultSpace::~DefaultSpace()
	{

	}

	/************************************************************************/
	/* ISpace interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool					DefaultSpace::initialize(ISpaceCoordinator* coorinator, const AABB& maxBound, uint32 partitionMask)
	{
		if( mCoordinator != coorinator )
		{
			mCoordinator = coorinator;
			mBox = maxBound;
			mPartitionMask = partitionMask;
			return true;
		}
		else
		{
			assert(false);
			return false;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void					DefaultSpace::update()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	bool					DefaultSpace::addContentImpl(ISpaceContent* content, uint32 /*partitionMask*/, bool /*strictFit = false*/)
	{
		bool success = false;
		if( content == NULL )
			return success;

		ISpace* space = this;
		if( mCoordinator == NULL )
			BLADE_EXCEPT(EXC_NEXIST,BTString("no coordinator set.") );

		if( content->getSpace() == space )
			return true;
		else if( content->getSpace() != NULL && (content->getSpace() != space || content->getSpace() != mCoordinator ) )
			BLADE_EXCEPT(EXC_INVALID_OPERATION,BTString("conent already in another space.") );

		if( content->getSpace() != space )
			success = mSpaceContents.insert(content).second;
		if (success)
		{
			content->setSpacePartitionMask(mPartitionMask);
			mBox.merge(content->getWorldAABB());
		}
		return success;
	}

	//////////////////////////////////////////////////////////////////////////
	bool					DefaultSpace::removeContentImpl(ISpaceContent* content)
	{
		return mSpaceContents.erase(content) == 1;
	}

	//////////////////////////////////////////////////////////////////////////
	void					DefaultSpace::findVisibleContent(const Frustum& frustum, IVisibleContentSet& vcs)
	{
		for( SpaceContentSet::iterator i = mSpaceContents.begin(); i != mSpaceContents.end(); ++i)
		{
			ISpaceContent* content = *i;
			SpaceBase::cullContent(frustum, vcs, content);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool					DefaultSpace::queryContents(SpaceQuery& query, EContentSpaceFlag flag, scalar& distLimit) const
	{
		scalar distance = distLimit;
		if( !mBox.isInfinite() )
		{
			POS_VOL pv = query.intersect(mBox, distance);
			if( pv == PV_OUTSIDE || (query.isQueringNearest() && distance > distLimit) || distance > query.getDistanceLimit())
				return false;
		}
		if(!query.isPrecise())
			distLimit = distance;
		for( SpaceContentSet::const_iterator i = mSpaceContents.begin(); i != mSpaceContents.end(); ++i)
		{
			ISpaceContent* content = *i;
			if (!SpaceBase::queryContent(query, content, flag, distLimit))
				break;
		}//for

		return query.size() > 0;
	}

	//////////////////////////////////////////////////////////////////////////
	void					DefaultSpace::clearContents()
	{
		for (SpaceContentSet::const_iterator i = mSpaceContents.begin(); i != mSpaceContents.end(); ++i)
		{
			ISpaceContent* content = *i;
			ISpace::clearSpace(content, false);
		}
		mSpaceContents.clear();
	}

	//////////////////////////////////////////////////////////////////////////
	void					DefaultSpace::translateEmptySpaceImpl(const Vector3& center)
	{
		assert(mSpaceContents.empty());
		mBox.offset(center - mBox.getCenter());
	}
	
}//namespace Blade