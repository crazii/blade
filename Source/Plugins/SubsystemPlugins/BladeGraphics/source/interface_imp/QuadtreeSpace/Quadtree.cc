/********************************************************************
	created:	2010/05/09
	filename: 	Quadtree.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "Quadtree.h"
#include <utility/Profiling.h>
#include <interface/IPlatformManager.h>
#include <interface/public/graphics/GraphicsInterface_blang.h>
#include "QuadtreeSpace.h"

namespace Blade
{
#if !BLADE_DEBUG && BLADE_ARCHITECTURE == BLADE_ARCHITECTURE_32BIT && !FULL_CONTENT_CACHE
	static const size_t SIZE = sizeof(Quadtree);
	static_assert(sizeof(Quadtree) <= 64, "size too large, neeed optimze");	//better keep a cache friendly size
#endif

	const TString Quadtree::QUADTREE_SPACE_TYPE = BTString(BLANG_QUADTREE_SPACE);
	//////////////////////////////////////////////////////////////////////////
	Quadtree::Quadtree(Quadtree* parent)
		:mParent(parent)
		,mPartitionMask(0)
		,mDepth(0)
	{
		mSubtree[0] = mSubtree[1] = mSubtree[2] = mSubtree[3] = NULL;
		mCenter = mHalfSize = Vector3::ZERO;
	}

	//////////////////////////////////////////////////////////////////////////
	Quadtree::Quadtree(Quadtree* parent, size_t depth, const Box2& bounding, uint32 partitionMask)
		:mParent(parent)
		,mPartitionMask(partitionMask)
		,mDepth((uint)depth)
	{
		mSubtree[0] = mSubtree[1] = mSubtree[2] = mSubtree[3] = NULL;
		AABB aab;
		aab.set(bounding.getMinPoint().x,0,bounding.getMinPoint().y,bounding.getMaxPoint().x,0,bounding.getMaxPoint().y);
		this->setBounding(aab);
	}

	//////////////////////////////////////////////////////////////////////////
	Quadtree::~Quadtree()
	{
		this->clear();
	}

	/************************************************************************/
	/* custom method                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool					Quadtree::removeContent(ISpaceContent* content)
	{
		if( content != NULL && content->getSpaceData() == this )
		{
			bool ret = mContents.erase(content) == 1;
			if(ret)
			{	
				QuadtreeSpace::setSpaceData(content, NULL);
				this->notifyContentRemoved(content, true);
			}
			assert(ret);
			return ret;
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	void					Quadtree::findVisibleContent(const Frustum& frustum,IVisibleContentSet& vcs)
	{
		//BLADE_LW_PROFILING_FUNCTION();
		Quadtree::climbDownTree(this, frustum, vcs);
	}

	//////////////////////////////////////////////////////////////////////////
	bool					Quadtree::queryContents(SpaceQuery& query, EContentSpaceFlag flag, scalar& limit) const
	{
		if( this->getFullContentCount() == 0)
			return false;

		scalar distance = limit;
		POS_VOL pos = query.intersect(this->getBounding(), distance);
		if (pos == PV_OUTSIDE || (query.isQueringNearest() && distance > limit) || distance > query.getDistanceLimit())
			return false;

		if(!query.isPrecise())
			limit = distance;
#if FULL_CONTENT_CACHE
		const ContentSet& contents = (pos == PV_INTERSECTED) ? mContents : mFullContents;
#else
		const ContentSet& contents = mContents;
#endif

		//query contents
		for (ContentSet::const_iterator i = contents.begin(); i != contents.end(); ++i)
		{
			ISpaceContent* content = *i;
			IPlatformManager::prefetch<PM_READ>(content);

			if (!SpaceBase::queryContent(query, content, flag, limit))
				break;
		}

#if FULL_CONTENT_CACHE
		if( pos == PV_INTERSECTED )
#endif
		{
			//query subtree
			for(int i = 0; i < 4; ++i)
			{
				Quadtree* subtree = mSubtree[i];
				if(subtree != NULL )
					subtree->queryContents(query, flag, limit);
			}		
		}
		return query.size() > 0;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				Quadtree::climbUpTree(ISpaceContent* content,const AABB& box,const Vector3& center,const Vector3& halfsize, bool strictFit)
	{
		AABB bounding = this->getBounding();
		if( strictFit )
		{
			assert(bounding.getMinPoint().x <= box.getMinPoint().x );
			assert(bounding.getMinPoint().z <= box.getMinPoint().z );
			assert(bounding.getMaxPoint().x >= box.getMinPoint().x );
			assert(bounding.getMaxPoint().z >= box.getMinPoint().z );
		}

		bool changeMinY = bounding.getMinPoint().y > box.getMinPoint().y;
		bool changeMaxY = bounding.getMaxPoint().y < box.getMaxPoint().y;

		if( changeMinY || changeMaxY )
		{
			if( changeMinY )
				bounding.setMinY( box.getMinPoint().y );

			if( changeMaxY )
				bounding.setMaxY( box.getMaxPoint().y );

			this->setBounding(bounding);
		}

		bool ret = false;
		bool isEnd = this->isLeaf();
		if (!isEnd)
		{
			isEnd = (halfsize.x + Math::LOW_EPSILON >= mHalfSize.x && halfsize.z + Math::LOW_EPSILON >= mHalfSize.z);
			if (!isEnd && !strictFit)
				isEnd = ((mCenter.x >= box.getMinPoint().x && mCenter.x <= box.getMaxPoint().x) || (mCenter.z >= box.getMinPoint().z && mCenter.z <= box.getMaxPoint().z));
		}

		//atom cell
		if(isEnd)
		{
			if( strictFit )
			{
				scalar alignMinX = box.getMinPoint().x <= Math::LOW_EPSILON ? 0 : bounding.getMinPoint().x / box.getMinPoint().x;
				scalar alignMaxX = box.getMaxPoint().x <= Math::LOW_EPSILON ? 0 : bounding.getMaxPoint().x / box.getMaxPoint().x;
				scalar alignMinZ = box.getMinPoint().z <= Math::LOW_EPSILON ? 0 : bounding.getMinPoint().z / box.getMinPoint().z;
				scalar alignMaxZ = box.getMaxPoint().z <= Math::LOW_EPSILON ? 0 : bounding.getMaxPoint().z / box.getMaxPoint().z;
				if( alignMinX != std::ceil(alignMinX)
					|| alignMinZ != std::ceil(alignMinZ)
					|| alignMaxX != std::ceil(alignMaxX)
					|| alignMaxZ != std::ceil(alignMaxZ) )
				{
					assert(false);
					return false;
				}
			}

			assert( !box.getMinPoint().equal(box.getMaxPoint(), Math::LOW_EPSILON) );
			ret = mContents.insert(content).second;
			assert(ret);
			QuadtreeSpace::setSpaceData(content, this);
			content->setSpacePartitionMask(this->getPartitionMask());
		}
		else
		{
			assert( mHalfSize.x > halfsize.x );
			assert( mHalfSize.z > halfsize.z );
			//split
			SUB_INDEX index = QSI_INVALID_INDEX;
			Box2 bounds;

			if( center.x < mCenter.x )
			{
				if( center.z < mCenter.z )
					index = QSI_LEFT_UP;
				else
					index = QSI_LEFT_DOWN;
			}
			else
			{
				if( center.z >= mCenter.z )
					index = QSI_RIGHT_DOWN;
				else
					index = QSI_RIGHT_UP;
			}

			if (mSubtree[index] == NULL)
				mSubtree[index] = this->createSubNode(index);
			ret = mSubtree[index]->climbUpTree(content, box, center, halfsize, strictFit);
		}

		assert(ret);
		if(ret)
		{
			//add to root : skip recursion
			ret = this->addToFullContent(content);
			assert(ret);
			if( !ret )
				BLADE_EXCEPT(EXC_REDUPLICATE, BTString("same content already inside space.") );
		}
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				Quadtree::jumpUpTree(Quadtree* root, ISpaceContent* content, uint32 partitionMask, bool strictFit)
	{
		bool ret = false;

		Quadtree* node = root;

		while (node != NULL)
		{
			bool match = ISpace::matchPartition(node->mPartitionMask, partitionMask);

			if (match)
			{
				ret = node->mContents.insert(content).second;
				QuadtreeSpace::setSpaceData(content, node);
				break;
			}
			else if (!node->isLeaf())
			{
				SUB_INDEX index = (SUB_INDEX)ISpace::extractIndex(partitionMask, node->mDepth + 1);
				if (index >= QSI_COUNT || index < QSI_START)
				{
					assert(false);
					return false;
				}

				if (node->mSubtree[index] == NULL)
					node->mSubtree[index] = node->createSubNode(index);
				node = node->mSubtree[index];
			}
			else
				node = NULL;
		};

		assert(ret);
		if (ret)
		{
			assert(node != NULL);

			const AABB& aab = content->getWorldAABB();
			if (aab.isInfinite() || aab.isNull())
			{
				//add to root : skip recursion
				assert(node->mDepth == 0);
				ret = node->addToFullContent(content);
			}
			else
			{
				while (node != NULL)
				{
					ret = node->addToFullContent(content);
					assert(ret);
					if (!ret)
						BLADE_EXCEPT(EXC_REDUPLICATE, BTString("same content already inside space."));

					AABB bounding = node->getBounding();

					bool changeMinY = bounding.getMinPoint().y > aab.getMinPoint().y;
					bool changeMaxY = bounding.getMaxPoint().y < aab.getMaxPoint().y;

					if (changeMinY || changeMaxY)
					{
						if (changeMinY)
							bounding.setMinY(aab.getMinPoint().y);

						if (changeMaxY)
							bounding.setMaxY(aab.getMaxPoint().y);

						node->setBounding(bounding);
					}
					assert(!strictFit || bounding.contains(aab));
					assert(strictFit || bounding.intersect(aab));
					BLADE_UNREFERENCED(strictFit);
					node = node->mParent;
				}
			}
		}
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	void				Quadtree::climbDownTree(Quadtree* tree, const Frustum& frustum, IVisibleContentSet& vcs, unsigned int frustumPlaneMask/* = Frustum::FPM_ALL*/)
	{
		if(tree->getFullContentCount() > 0)
		{
			const POINT3& center = tree->mCenter;
			const Vector3 half = tree->mHalfSize;

			ICamera::VISIBILITY vis = frustum.getVisibility(center, half, &frustumPlaneMask);
			if( vis == ICamera::CV_ALL )
				tree->fallOffTree(vcs);
			else if( vis != ICamera::CV_NULL || tree->isRoot())
			{
				//contents
				for(ContentSet::const_iterator i = tree->mContents.begin(); i != tree->mContents.end(); ++i)
				{
					ISpaceContent* content = *i;

					//note: prefetch may not have improvement on some situations( may have slightly unnoticeable drawbacks, only noticeable on profiling)
					//even if the code is same, prefetch will have different result on different environments (threading, memory usage, other processes' CPU usage)
					//so on average, we better use prefetch to prevent cache miss
					IPlatformManager::prefetch<PM_READ>(content);

					SpaceBase::cullContent(frustum, vcs, content, frustumPlaneMask);
				}

				//try sub nodes
				for( int i = 0; i < 4; ++i )
				{
					Quadtree* subtree = tree->getSubTree( (Quadtree::SUB_INDEX)i );
					if( subtree != NULL )
						Quadtree::climbDownTree(subtree, frustum, vcs, frustumPlaneMask);
				}
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void				Quadtree::clear()
	{
		while( !mContents.empty() )
		{
			ContentSet::iterator i = mContents.begin(); 
			ISpaceContent* content = *i;
			this->removeContent(content);
		}

		for( int i = 0; i < 4; ++i)
		{
			BLADE_DELETE mSubtree[i];
			mSubtree[i] = NULL;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool				Quadtree::notifyContentRemoved(ISpaceContent* content, bool notifyParent)
	{
		bool deleted = false;

#if FULL_CONTENT_CACHE
		if( mFullContents.erase(content) != 1 )
		{
			assert(false);
			BLADE_EXCEPT(EXCE_STATE_ERROR, BTString("invalid state.") );
		}
#else
		assert(mTotalCount > 0);
		--mTotalCount;
#endif

		if (notifyParent)
		{
			//note: if parent is deleted, this will no longer exist!
			//so load parent from this in advance
			Quadtree* parent = mParent;

			if (parent != NULL)
			{
				deleted = parent->notifyContentRemoved(content, notifyParent);
				if (!deleted && this->getFullContentCount() == 0)
				{
#if DELETE_EMPTY_CELL
					parent->deleteChild(this);
					deleted = true;
#endif
				}
			}
		}
		return deleted;
	}

	//////////////////////////////////////////////////////////////////////////
	Quadtree*		Quadtree::createSubNode(SUB_INDEX index)
	{
		size_t depth = mDepth + 1;
		uint32 partitionMask = ISpace::generatePartitionMask(index, depth, mPartitionMask);
		return BLADE_NEW Quadtree(this, depth, this->getSubBound(index), partitionMask);
	}
	
}//namespace Blade