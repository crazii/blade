/********************************************************************
	created:	2010/05/09
	filename: 	QuadtreeSpace.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "QuadtreeSpace.h"
#include <interface/ISpaceCoordinator.h>
#include <interface/IGraphicsSystem.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	QuadtreeSpace::QuadtreeSpace()
		:SpaceBase(QUADTREE_SPACE_TYPE)
		,Quadtree(NULL) 
	{
	}

	//////////////////////////////////////////////////////////////////////////
	QuadtreeSpace::~QuadtreeSpace()
	{

	}

	/************************************************************************/
	/* ISpace interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool					QuadtreeSpace::initialize(ISpaceCoordinator* coorinator, const AABB& maxBound, uint32 partitionMask)
	{
		if( mCoordinator == NULL )
		{
			mDepth = 0;
			mMaxBound = maxBound;
			this->setBounding(maxBound);
			mCoordinator = coorinator;
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
	void					QuadtreeSpace::update()
	{
		BLADE_TS_VERIFY(TS_ASYNC_UPDATE);

		if (!(mFlag&SF_DYNAMIC))
			return;

		for (UpdateList::const_iterator i = mChangedContents.begin(); i != mChangedContents.end(); ++i)
		{
			ISpaceContent* content = *i;

			if(!(content->getSpaceFlags()&CSF_DYNAMIC))	//static object edited
				continue;

			const AABB& aab = content->getWorldAABB();
			if (aab.isNull() || aab.isInfinite())
			{
				if (static_cast<Quadtree*>(content->getSpaceData()) != this) //not in root?
				{
					static_cast<Quadtree*>(content->getSpaceData())->removeContent(content);
					ISpace::clearSpace(content, true);
					//put into root
					this->addContent(content);
				}
				continue;
			}

			Box2 contentBox2(aab.getMinPoint().x, aab.getMinPoint().z, aab.getMaxPoint().x, aab.getMaxPoint().z);

			Quadtree* curNode = static_cast<Quadtree*>(content->getSpaceData());
			Quadtree* node = curNode;
#if DELETE_EMPTY_CELL
			bool validNode = false;
#else
			bool validNode = true;
#endif
			bool contained = false;

			while (node != NULL)
			{
				const AABB box = node->getBounding();
				Box2 box2(box.getMinPoint().x, box.getMinPoint().z, box.getMaxPoint().x, box.getMaxPoint().z);

#if DELETE_EMPTY_CELL
				//node with single content may be deleted if sub node deleted
				//so node may be invalid pointer after calling curNode->removeContent(content);
				if (node->mFullContents.size() > 1)
					validNode = true;
#endif

				if (!contained)
				{
					if (box2.intersect(contentBox2) == PV_INSIDE)
						contained = true;
				}

				if (contained && validNode)
					break;

				node = node->getParent();
			}

			if (node == NULL)
				node = this;

			if (node != curNode || curNode == this)
			{
				bool ret = curNode->removeContent(content);
				assert(ret); BLADE_UNREFERENCED(ret);
				node->climbUpTree(content, aab, aab.getCenter(), aab.getHalfSize(), false);
			}
		}

		mChangedContents.clear();
	}

	//////////////////////////////////////////////////////////////////////////
	bool					QuadtreeSpace::notifyContentChanged(ISpaceContent* content)
	{
		//only should be called in async run state from other sub systems
		//if within graphics system, any state is allowed
		BLADE_TS_VERITY_GRAPHICS_PUBLIC_ACCESS();
		ScopedLock lock(mChangedLock);
		return mChangedContents.insert(content).second;
	}

	//////////////////////////////////////////////////////////////////////////
	bool					QuadtreeSpace::addContentImpl(ISpaceContent* content, uint32 partitionMask, bool strictFit /*= false*/)
	{
		assert(content != NULL);
		if (content == NULL)
			return false;

		assert( mCoordinator != NULL );
		if( content->getSpace() == this  )
			return true;
		else if( content->getSpace() != NULL && (content->getSpace() != this || content->getSpace() != mCoordinator) )
			BLADE_EXCEPT( EXC_NEXIST,BTString("content already in another space.") );

		if (!(partitionMask&ISpace::INVALID_PARTITION))
		{
			bool ret = Quadtree::jumpUpTree(this, content, partitionMask & ISpace::PARTITION_MASK, strictFit);
			assert(ret);
			return ret;
		}

		const AABB& aab = content->getWorldAABB();
		const POINT3& pmax = aab.getMaxPoint();
		const POINT3& pmin = aab.getMinPoint();
		AABB bounding = this->getBounding();

		if (!aab.isFinite()
			|| (bounding.getMinPoint().x > pmin.x || bounding.getMinPoint().z > pmin.z || bounding.getMaxPoint().x < pmax.x || bounding.getMaxPoint().z < pmax.z))
		{
			bool ret = mContents.insert(content).second;
			if(ret )
				this->addToFullContent(content);
			
			ISpace::setSpaceData(content, this);
			content->setSpacePartitionMask(this->getPartitionMask());
			return true;
		}

		POINT3 center = aab.getCenter();
		Vector3 halfsize = aab.getHalfSize();
		bool ret = this->climbUpTree(content, aab, center, halfsize, strictFit);
		assert(ret);
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	bool					QuadtreeSpace::removeContentImpl(ISpaceContent* content)
	{
		if( content != NULL )
		{
			assert( content->getSpace() == this );
			ISpaceData* data = content->getSpaceData();
			if (data == NULL || data->getSpaceTypeName() != QUADTREE_SPACE_TYPE)
			{
				assert(false);
				return false;
			}

			Quadtree* quadtree = static_cast<Quadtree*>(data);
			return quadtree->removeContent(content);
		}
		assert(false);
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	void					QuadtreeSpace::findVisibleContent(const Frustum& frustum, IVisibleContentSet& vcs)
	{
		return Quadtree::findVisibleContent(frustum, vcs);
	}

	//////////////////////////////////////////////////////////////////////////
	bool					QuadtreeSpace::queryContents(SpaceQuery& query, EContentSpaceFlag flag, scalar& distLimit) const
	{
		return Quadtree::queryContents(query, flag, distLimit);
	}

	//////////////////////////////////////////////////////////////////////////
	void					QuadtreeSpace::clearContents()
	{
#if FULL_CONTENT_CACHE
		for (ContentSet::const_iterator i = mFullContents.begin(); i != mFullContents.end(); ++i)
		{
			ISpaceContent* content = *i;	//prefetch next?
			IPlatformManager::prefetch<PM_WRITE>(content);
			ISpace::clearSpace(content, false);
			ISpace::setSpaceData(content, NULL);
		}
		this->directClear();
#else
		this->directClear(&QuadtreeSpace::ClearContentData);
#endif
		AABB bounding = this->getBounding();
		bounding.setMaxY(mMaxBound.getMaxPoint().y);
		bounding.setMinY(mMaxBound.getMinPoint().y);
		this->setBounding(bounding);
	}

	//////////////////////////////////////////////////////////////////////////
	void					QuadtreeSpace::translateEmptySpaceImpl(const Vector3& center)
	{
		assert(this->getFullContentCount() == 0);

		Vector3 _offset = center - mCenter;
		mMaxBound.offset(_offset);

		if(std::fabs(_offset.x) > Math::LOW_EPSILON || std::fabs(_offset.y) > Math::LOW_EPSILON || std::fabs(_offset.z) > Math::LOW_EPSILON)
			this->Quadtree::tranlsate(_offset);
	}

}//namespace Blade