/********************************************************************
	created:	2010/05/09
	filename: 	Quadtree.h
	author:		Crazii
	purpose:	regular quadtree
*********************************************************************/
#ifndef __Blade_Quadtree_h__
#define __Blade_Quadtree_h__
#include <utility/BladeContainer.h>
#include <interface/ISpaceData.h>
#include <interface/ISpace.h>
#include <SpaceBase.h>
#include <SpaceContent.h>
#include <interface/IPlatformManager.h>

namespace Blade
{

#define DELETE_EMPTY_CELL 0
#define FULL_CONTENT_CACHE 0

	class Quadtree : public ISpaceData, public Allocatable
	{
	public:
		static const TString QUADTREE_SPACE_TYPE;
	public:
		typedef enum EQuadtreeSubtreeIndex
		{
			QSI_INVALID_INDEX = INVALID_INDEX,
			QSI_START = 0,
			QSI_LEFT_UP = QSI_START,
			QSI_LEFT_DOWN,
			QSI_RIGHT_DOWN,
			QSI_RIGHT_UP,

			QSI_COUNT,
		}SUB_INDEX;
		//quad tree's content
		typedef Set<ISpaceContent*>	ContentSet;


		//	+--------x+	RIGHT
		//	|
		//	|
		//	|
		//	z+(DOWN)
	public:
		Quadtree(Quadtree* parent);
		Quadtree(Quadtree* parent, size_t depth, const Box2& bounding, uint32 partitionMask);
		virtual ~Quadtree();

		/************************************************************************/
		/* ISpaceData interface                                                                     */
		/************************************************************************/
		/**
		@describe get the name of the data
		@param
		@return
		*/
		virtual const TString&	getSpaceTypeName() { return Quadtree::QUADTREE_SPACE_TYPE; }

		/**
		@describe
		@param
		@return
		*/
		virtual uint32			getPartitionMask() const { return mPartitionMask; }

		/**
		@describe when attach to a camera
		@param
		@return
		*/
		virtual void			onAttach(ISpaceContent* /*content*/) {}

		/**
		@describe
		@param
		@return
		*/
		virtual void			onDetach(ISpaceContent* /*content*/) {}

		/************************************************************************/
		/* custom method                                                                     */
		/************************************************************************/
		/**
		@describe
		@param
		@return
		*/
		bool					removeContent(ISpaceContent* content);

		/**
		@describe
		@param
		@return
		*/
		void					findVisibleContent(const Frustum& frustum, IVisibleContentSet& vcs);

		/**
		@describe
		@param
		@return
		*/
		bool					queryContents(SpaceQuery& query, EContentSpaceFlag flag, scalar& limit) const;

		/**
		@describe
		@param
		@return
		*/
		size_t				getDepth() const { return mDepth; }

		/**
		@describe
		@param
		@return
		*/
		inline Quadtree*	getSubTree(SUB_INDEX index) const
		{
			assert(index >= 0 && index <= 3);
			return mSubtree[index];
		}

		/**
		@describe
		@param
		@return
		*/
		inline bool			isLeaf() const { return mDepth == ISpace::MAX_PARTITION_LEVEL; }

		/**
		  @describe
		  @param
		  @return
		*/
		inline bool			isRoot() const { return mDepth == 0; }

		/*
		@describe
		@param
		@return
		*/
		inline Quadtree*	getParent() const { return mParent; }

		/*
		@describe
		@param
		@return
		*/
#if FULL_CONTENT_CACHE
		BLADE_ALWAYS_INLINE void	fallOffTree(IVisibleContentSet& vcs) const
		{
			if (mFullContents.size() > 0)
			{
				for (ContentSet::const_iterator i = mFullContents.begin(); i != mFullContents.end();)
				{
					ISpaceContent* content = *(i++);
					//IPlatformManager::prefetch<PM_READ>(*i);
					SpaceBase::addVisibleContent(vcs, content);
				}
			}
		}
#else
		inline void	fallOffTree(IVisibleContentSet& vcs) const
		{
			for (ContentSet::const_iterator i = mContents.begin(); i != mContents.end();)
			{
				ISpaceContent* content = *(i++);
				SpaceBase::addVisibleContent(vcs, content);
			}
			for (int i = 0; i < 4; ++i)
			{
				Quadtree* subtree = this->getSubTree((Quadtree::SUB_INDEX)i);
				if (subtree != NULL)
					subtree->fallOffTree(vcs);
			}
		}
#endif

		/*
		@describe
		@param
		@return
		*/
		BLADE_ALWAYS_INLINE void	deleteChild(Quadtree* child)
		{
			for (int i = 0; i < (int)QSI_COUNT; ++i)
			{
				if (mSubtree[i] == child)
				{
					BLADE_DELETE mSubtree[i];
					mSubtree[i] = NULL;
					return;
				}
			}
			assert(false);
		}

		/**
		  @describe get content count, include sub nodes' contents
		  @param
		  @return
		*/
		inline size_t getFullContentCount() const
		{
#if FULL_CONTENT_CACHE
			return mFullContents.size();
#else
			return mTotalCount;
#endif
		}

		/**
		  @describe 
		  @param
		  @return
		*/
		inline bool addToFullContent(ISpaceContent* content)
		{
#if FULL_CONTENT_CACHE
			return mFullContents.insert(content).second;
#else
			BLADE_UNREFERENCED(content);
			++mTotalCount;
			return true;
#endif
		}

	protected:
		/*
		@describe recursive method to add content
		@param
		@return
		*/
		bool				climbUpTree(ISpaceContent* content,const AABB& box,const Vector3& center,const Vector3& halfsize, bool strictFit);

		/**
		@describe 
		@param
		@return
		*/
		static bool			jumpUpTree(Quadtree* root, ISpaceContent* content, uint32 partitionMask, bool strictFit);

		/*
		@describe recursive method to get content
		@param
		@return
		*/
		static void			climbDownTree(Quadtree* tree,const Frustum& frustum,IVisibleContentSet& vcs,unsigned int frustumPlaneMask = Frustum::FPM_ALL);

		/*
		@describe
		@param
		@return
		*/
		void				clear();

		/*
		@describe
		@param
		@return indicates this tree node is deleted or not
		*/
		bool				notifyContentRemoved(ISpaceContent* content, bool notifyParent);

		/** @brief  */
		inline const AABB	getBounding() const { return AABB(mCenter - mHalfSize, mCenter + mHalfSize); }

		/** @brief  */
		inline void			setBounding(const AABB& aab) { mCenter = aab.getCenter(); mHalfSize = aab.getHalfSize(); }

		/** @brief  */
		inline Box2			getSubBound(SUB_INDEX index) const
		{
			const Vector3& center = mCenter;
			AABB bounding = this->getBounding();

			switch (index)
			{
			case QSI_LEFT_UP:
				return Box2(bounding.getMinPoint().x, bounding.getMinPoint().z, center.x, center.z);
			case QSI_LEFT_DOWN:
				return Box2(bounding.getMinPoint().x, center.z, center.x, bounding.getMaxPoint().z);
			case QSI_RIGHT_DOWN:
				return Box2(center.x, center.z, bounding.getMaxPoint().x, bounding.getMaxPoint().z);
			case QSI_RIGHT_UP:
				return Box2(center.x, bounding.getMinPoint().z, bounding.getMaxPoint().x, center.z);
			default:
				assert(false);
			}
			return Box2(BT_NULL);
		}

		/** @brief  */
		Quadtree*		createSubNode(SUB_INDEX index);

		/** @brief  */
		inline void		tranlsate(const Vector3& _offset)
		{
			mCenter += _offset;

			for (size_t i = 0; i < QSI_COUNT; ++i)
			{
				Quadtree* subNode = mSubtree[i];
				if (subNode != NULL)
					subNode->tranlsate(_offset);
			}
		}

#if FULL_CONTENT_CACHE
		/** @brief  */
		inline void		directClear()
		{
			mHalfSize.y = 0;

			mFullContents.clear();

			mContents.clear();

			for (size_t i = 0; i < QSI_COUNT; ++i)
			{
				Quadtree* subNode = mSubtree[i];
				if (subNode != NULL)
					subNode->directClear();
			}
		}
#else
		/** @brief  */
		template<typename FnClear>
		inline void		directClear(FnClear fn)
		{
			mHalfSize.y = 0;

			mTotalCount = 0;
			for (ContentSet::const_iterator i = mContents.begin(); i != mContents.end(); ++i)
			{
				ISpaceContent* content = *i;	//prefetch next?
				IPlatformManager::prefetch<PM_WRITE>(content);
				fn(content);
			}
			mContents.clear();

			for (size_t i = 0; i < QSI_COUNT; ++i)
			{
				Quadtree* subNode = mSubtree[i];
				if (subNode != NULL)
					subNode->directClear<FnClear>(fn);
			}
		}
#endif

		Quadtree*		mSubtree[4];
		Quadtree*		mParent;
		Vector3			mCenter;
		Vector3			mHalfSize;
		uint32			mPartitionMask;
		uint32			mDepth : 8;
#if FULL_CONTENT_CACHE
		ContentSet		mFullContents;		//content including sub tree's content
#else
		uint32			mTotalCount : 24;
#endif
		ContentSet		mContents;

		friend class QuadtreeSpace;
	};//class Quadtree
	
}//namespace Blade


#endif //__Blade_Quadtree_h__