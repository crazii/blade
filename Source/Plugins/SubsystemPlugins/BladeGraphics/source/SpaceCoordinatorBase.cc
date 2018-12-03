/********************************************************************
	created:	2010/04/30
	filename: 	SpaceCoordinatorBase.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <SpaceCoordinatorBase.h>
#include <interface/IEventManager.h>
#include <interface/ITaskManager.h>
#include <interface/public/graphics/GraphicsSynchronizedEvent.h>
#include <utility/Profiling.h>
#include "interface_imp/DefaultSpace.h"
#include "internal/GraphicsConfig.h"

namespace Blade
{
	namespace Impl
	{
		//////////////////////////////////////////////////////////////////////////
		void getPageFromPatitionMask(uint32 partitionMask, uint32& x, uint32& z)
		{
			uint32 pageMask = (partitionMask & ISpace::EXTRA_PARTITION_MASK) >> ISpace::PARTITION_BIT_COUNT;
			x = pageMask & IPage::VISIBlE_PAGE_MASK;
			z = (pageMask >> IPage::VISIBLE_PAGE_BITCOUNT) & IPage::VISIBlE_PAGE_MASK;
			assert(x <= IPage::MAX_VISIBLE_PAGE && z <= IPage::MAX_VISIBLE_PAGE);
		}

		//////////////////////////////////////////////////////////////////////////
		uint32 makePageMask(uint32 x, uint32 z)
		{
			assert(x <= IPage::MAX_VISIBLE_PAGE && z <= IPage::MAX_VISIBLE_PAGE);
			assert((x&(~IPage::VISIBlE_PAGE_MASK)) == 0 && (x&(~IPage::VISIBlE_PAGE_MASK)) == 0);
			uint32 mask = (((z& IPage::VISIBlE_PAGE_MASK) << IPage::VISIBLE_PAGE_BITCOUNT) | (x&IPage::VISIBlE_PAGE_MASK)) << ISpace::PARTITION_BIT_COUNT;
			assert((mask&ISpace::PARTITION_MASK) == 0);
			return mask;
		}
		
	}//namespace Impl

	//////////////////////////////////////////////////////////////////////////
	SpaceCoordinatorBase::SpaceCoordinatorBase(const TString& coordinatorType)
		:mCoordinatorType(coordinatorType)
		,mBound(BT_NULL)
		,mDesc(NULL)
		,mReference(NULL)
	{
		mSpaces.reserve(16);
	}

	//////////////////////////////////////////////////////////////////////////
	SpaceCoordinatorBase::~SpaceCoordinatorBase()
	{
		for (size_t i = 0; i < mSpaces.size(); ++i)
			BLADE_DELETE mSpaces[i];

		for (size_t i = 0; i < mDynamicSpaces.size(); ++i)
			BLADE_DELETE mDynamicSpaces[i];

		BLADE_DELETE mDefualtImpl;
	}

	/************************************************************************/
	/* ISpace interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool					SpaceCoordinatorBase::addContent(ISpaceContent* content, uint32 partitionMask/* = ISpace::INVALID_PARTITION*/, bool strictFit/* = false*/)
	{
		if (mDefualtImpl == NULL || mDynamicSpaces.size() == 0 || mSpaces.size() == 0)
		{
			assert(false);
			return false;
		}

		partitionMask = content->getSpacePartitionMask();
		if (partitionMask&ISpace::INVALID_PARTITION)
		{
			const AABB& bounds = content->getWorldAABB();

			if (bounds.isInfinite() || bounds.isNull() || ((content->getSpaceFlags()&CSF_INDEPENDENT)))
			{
				return mDefualtImpl->addContent(content, partitionMask, strictFit);
			}

			bool dynamic = ((content->getSpaceFlags()&CSF_DYNAMIC) != 0);
			for (uint32 z = 0; z < mDesc->visiblePage; ++z)
			{
				for (uint32 x = 0; x < mDesc->visiblePage; ++x)
				{
					ISpace* space = this->getSpaceofPage(x, z, dynamic);
					const AABB& spaceBounds = space->getSpaceBound();
					if (spaceBounds.contains(bounds.getCenter()))
					{
#if BLADE_DEBUG
						bool result = space->addContent(content, Impl::makePageMask(x, z) | ISpace::INVALID_PARTITION, strictFit);
						uint32 px = 0, pz = 0;
						Impl::getPageFromPatitionMask(content->getSpacePartitionMask(), px, pz);
						assert(px == x && pz == z && result);
						return result;
#else
						return space->addContent(content, Impl::makePageMask(x, z) | ISpace::INVALID_PARTITION, strictFit);
#endif
					}
				}
			}

			//assert(false && "out of range");
			//BLADE_LOG(Error, BTString("object out of space range."));
			return mDefualtImpl->addContent(content, partitionMask, strictFit);
		}

		uint32 x = 0, z = 0;
		Impl::getPageFromPatitionMask(partitionMask, x, z);

		ISpace* space = this->getSpaceofPage(x, z, false);
		if (space != NULL)
		{
			assert(/*space->getSpaceBound().contains(content->getPosition()) || */space->getSpaceBound().contains(content->getWorldAABB().getCenter()) );
			return space->addContent(content, partitionMask, strictFit);
		}
		assert(false);
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool					SpaceCoordinatorBase::removeContent(ISpaceContent* content)
	{
		if (content == NULL || (content->getSpace() != this && (content->getSpace() == NULL || content->getSpace()->getCoordinator() != this)) )
		{
			assert(false);
			return false;
		}
		if ((content->getSpaceFlags()&CSF_DYNAMIC) || !content->getWorldAABB().isFinite())
		{
			ISpace* space = content->getSpace();
			assert(space->getCoordinator() == this && (space->getSpaceFlag()&SF_DYNAMIC));
			return space->removeContent(content);
		}
		else
		{
			//extract runtime page index from mask
			uint32 partitionMask = content->getSpacePartitionMask();
			uint32 x = 0, z = 0;
			Impl::getPageFromPatitionMask(partitionMask, x, z);

			if (x >= mDesc->visiblePage || z >= mDesc->visiblePage)
			{
				assert(false);
				return false;
			}
			ISpace* space = this->getSpaceofPage(x, z, false);
			if(space != NULL)
				return space->removeContent(content);
			assert(false);
			return false;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool				SpaceCoordinatorBase::notifyContentChanged(ISpaceContent* content)
	{
		ISpace* space = content->getSpace();
		assert(space != NULL && space->getCoordinator() == this);

		//space not containing content, re-add 
		if ((content->getWorldAABB().isFinite() && !space->getSpaceBound().contains(content->getWorldAABB().getCenter())) || !(content->getSpaceFlags()&CSF_DYNAMIC))
		{
			//static object changed
			//probably on initialization, or editor's editing (translate/rotate/scale etc)
			//assert(!(space->getSpaceFlag()&SF_DYNAMIC));

			space->removeContent(content);
			content->setSpacePartitionMask(INVALID_PARTITION);
			return this->addContent(content);
		}
		else
		{
			assert((space->getSpaceFlag()&SF_DYNAMIC));
			return space->notifyContentChanged(content);
		}
	}

#if MULTITHREAD_CULLING
	struct CullingTask : public IDispatchableTask
	{
		TempVector<ISpace*> mSpaces;
		const Frustum* frustum;
		IVisibleContentSet* result;

		virtual ~CullingTask() {}	//make compiler happy
		virtual const TString& getName() const
		{
#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable:4640)//construction of local static object is not thread-safe
#endif
			static const TString TYPE = BTString("Culling task"); return TYPE; 
#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(pop)
#endif
		}
		virtual void run()
		{
			for(size_t i = 0; i < mSpaces.size(); ++i)
				mSpaces[i]->findVisibleContent(*(frustum), *(result));
		}
	};
#endif
	//////////////////////////////////////////////////////////////////////////
	void					SpaceCoordinatorBase::findVisibleContent(const Frustum& frustum, IVisibleContentSet& vcs)
	{
		//BLADE_LW_PROFILING_FUNCTION();
		mDefualtImpl->findVisibleContent(frustum, vcs);

#if MULTITHREAD_CULLING
		const size_t MAX_CULLING_TASK_COUNT = 4;
		if (mSpaces.size() > 1)
		{
			size_t taskCount = std::min(MAX_CULLING_TASK_COUNT, mSpaces.size());
			size_t spacePerTask = mSpaces.size() / taskCount;
			size_t remainder = mSpaces.size() % taskCount;
			assert(spacePerTask >= 1);
			DispatchableTaskList<CullingTask> tasks(taskCount);
			for (size_t i = 0; i < taskCount; ++i)
			{
				tasks[i].frustum = &frustum;
				tasks[i].result = &vcs;

				for (size_t j = 0; j < spacePerTask; ++j)
				{
					tasks[i].mSpaces.push_back(mSpaces[i*spacePerTask+j]);
					tasks[i].mSpaces.push_back(mDynamicSpaces[i*spacePerTask+j]);
				}
				//last task remainder
				if (i == taskCount - 1)
				{
					for (size_t j = 0; j < remainder; ++j)
					{
						tasks[i].mSpaces.push_back(mSpaces[(i + 1)*spacePerTask + j]);
						tasks[i].mSpaces.push_back(mDynamicSpaces[(i + 1)*spacePerTask + j]);
					}
				}
			}
			IDispatchableTask::Handles handles;
			ITaskManager::getSingleton().dispatchTasks(tasks.getTasks(), tasks.size(), &handles);
			handles.sync();
		}
		else
#endif
		{
			for (size_t i = 0; i < mDynamicSpaces.size(); ++i)
				mDynamicSpaces[i]->findVisibleContent(frustum, vcs);

			for (size_t i = 0; i < mSpaces.size(); ++i)
				mSpaces[i]->findVisibleContent(frustum, vcs);
		}

		//for_ech sub space
		size_t n = this->getNumSpaces();
		for( size_t i = 0; i < n; ++i )
		{
			ISpace* space = this->getSpace(i);
			Mask spaceFlag = space->getSpaceFlag();
			if( !spaceFlag.checkBits(SF_SEPARATED) && spaceFlag.checkBits(SF_SOLID) )
				space->findVisibleContent(frustum,vcs);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void					SpaceCoordinatorBase::update()
	{
		mDefualtImpl->update();

		for (size_t i = 0; i < mDynamicSpaces.size(); ++i)
			mDynamicSpaces[i]->update();

		//for_ech sub space
		size_t n = this->getNumSpaces();
		for( size_t i = 0; i < n; ++i )
		{
			ISpace* space = this->getSpace(i);
			Mask spaceFlag = space->getSpaceFlag();
			if( spaceFlag.checkBits(SF_DYNAMIC) )
				space->update();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool					SpaceCoordinatorBase::queryContents(SpaceQuery& query, EContentSpaceFlag flag, scalar& distLimit) const
	{
		bool result = false;

		{
			scalar dist = distLimit;
			result = mDefualtImpl->queryContents(query, flag, distLimit);
			if (!result)
				distLimit = dist;
		}
		

		for (uint32 z = 0; z < mDesc->visiblePage; ++z)
		{
			for (uint32 x = 0; x < mDesc->visiblePage; ++x)
			{
				scalar dist = distLimit;
				result |= this->getSpaceofPage(x, z, true)->queryContents(query, flag, distLimit);
				if (!result)
					distLimit = dist;

				result |= this->getSpaceofPage(x, z, false)->queryContents(query, flag, distLimit);
			}
		}

		bool subresult = false;
		size_t n = this->getNumSpaces();
		for( size_t i = 0; i < n; ++i )
		{
			ISpace* space = this->getSpace(i);
			Mask spaceFlag = space->getSpaceFlag();
			if( !spaceFlag.checkBits(SF_SEPARATED) && spaceFlag.checkBits(SF_SOLID) )
				subresult = space->queryContents(query, flag, distLimit);
		}
		return result || subresult;
	}

	/************************************************************************/
	/*ISpaceCoordinator interface                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool				SpaceCoordinatorBase::initialize(const COORDINATOR_DESC* desc)
	{
		if (mDynamicSpaces.size() != 0 || mSpaces.size() != 0 || mDesc != NULL || desc == NULL
			|| desc->type != mCoordinatorType || desc->visiblePage > (ISpace::EXTRA_PARTITION_MASK+1) )
		{
			assert(false);
			return false;
		}

		mDesc = desc;
		scalar size = (scalar)mDesc->pageSize;
		Vector3 spaceSize = Vector3(size, size, size);
		AABB fullAAB(Vector3(0, -size, 0), spaceSize * Vector3((scalar)mDesc->visiblePage, 1, (scalar)mDesc->visiblePage) );
		mBound = fullAAB;

		//default space containing null/infinite contents
		mDefualtImpl = BLADE_NEW DefaultSpace();
		mDefualtImpl->initialize(this, fullAAB, INVALID_PARTITION | SUB_PARTITION_ROOTEND);
		mDefualtImpl->setSpaceFlag(SF_SOLID | SF_DYNAMIC | SF_VISIBLE);

		mSpaces.resize(mDesc->visiblePage*mDesc->visiblePage);
		mDynamicSpaces.resize(mDesc->visiblePage*mDesc->visiblePage);
		for (size_t i = 0; i < mSpaces.size(); ++i)
		{
			//generate space's index mask using EXTRA_PARTITION_MASK,
			//its only used for remove contents (runtime only),
			//not for adding content, since content partition mask's saved during serialization, but page index is volatile,
			//loaded mask contains page index on last save, which may not match on this time.
			size_t x = i % (size_t)mDesc->visiblePage;
			size_t z = i / (size_t)mDesc->visiblePage;
			uint32 mask = Impl::makePageMask((uint32)x, (uint32)z);

			Vector3 min = Vector3(x*size, -size, z*size);
			Vector3 max = min + spaceSize * Vector3(1,2,1);
			AABB aab(min, max);

			ISpace* space = BLADE_FACTORY_CREATE(ISpace, desc->defaultSpace);
			space->initialize(this, aab, mask | SUB_PARTITION_ROOTEND);
			space->setSpaceFlag(SF_SOLID | SF_VISIBLE);
			mSpaces[i] = space;

			//dynamic spaces
			ISpace* dynamicSpace = BLADE_FACTORY_CREATE(ISpace, desc->defaultSpace);
			dynamicSpace->initialize(this, aab, INVALID_PARTITION | SUB_PARTITION_ROOTEND);
			dynamicSpace->setSpaceFlag(SF_SOLID | SF_DYNAMIC | SF_VISIBLE);
			mDynamicSpaces[i] = dynamicSpace;
		}

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void SpaceCoordinatorBase::onPageUnload(IPage* page)
	{
		Vector2i localIndices = page->getLocalIndices();
		assert((uint32)localIndices.x < mDesc->visiblePage && (uint32)localIndices.y < mDesc->visiblePage);

		ISpace* subSpace = this->getSpaceofPage((index_t)localIndices.x, (index_t)localIndices.y, false);
		this->unLoadPage(page, subSpace);

		subSpace = this->getSpaceofPage((index_t)localIndices.x, (index_t)localIndices.y, true);
		this->unLoadPage(page, subSpace);
	}

	//////////////////////////////////////////////////////////////////////////
	void SpaceCoordinatorBase::onPageLoad(IPage* page)
	{
		BLADE_TS_VERIFY(TS_MAIN_SYNC);

		Vector2i localIndices = page->getLocalIndices();
		assert((uint32)localIndices.x < mDesc->visiblePage && (uint32)localIndices.y < mDesc->visiblePage);

		ISpace* subSpace = this->getSpaceofPage((index_t)localIndices.x, (index_t)localIndices.y, false);
		this->movePage(page, subSpace);

		subSpace = this->getSpaceofPage((index_t)localIndices.x, (index_t)localIndices.y, true);
		this->movePage(page, subSpace);
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void		SpaceCoordinatorBase::unLoadPage(IPage* page, ISpace* space)
	{
		BLADE_TS_VERIFY(TS_MAIN_SYNC);

		assert(space != NULL);
		assert(!(space->getSpaceFlag()&SF_EMPTY));

#if BLADE_DEBUG
		const Vector2& pos2 = page->getPosition();
		Vector3 pos(pos2.x, 0, pos2.y);
		assert(space->getSpaceBound().contains(pos));
#endif
		BLADE_UNREFERENCED(page);

		space->clearContents();
		space->setSpaceFlag(space->getSpaceFlag() | SF_EMPTY);
	}
	
	//////////////////////////////////////////////////////////////////////////
	void		SpaceCoordinatorBase::movePage(IPage* page, ISpace* space)
	{
		assert(space != NULL);
		assert(space->getSpaceFlag()&SF_EMPTY);

		const Vector2& pos2 = page->getPosition();
		Vector3 pos(pos2.x, 0, pos2.y);

		space->translateEmptySpace(pos);
		space->setSpaceFlag((space->getSpaceFlag() & (uint32)(~SF_EMPTY)));
	}

	//////////////////////////////////////////////////////////////////////////
	ISpace*		SpaceCoordinatorBase::getSpaceofPage(index_t x, index_t z, bool dynamic) const
	{
		if (x >= mDesc->visiblePage || z >= mDesc->visiblePage)
		{
			assert(false);
			return NULL;
		}

		const SpaceForPages& pages = dynamic ? mDynamicSpaces : mSpaces;
		return pages[z * mDesc->visiblePage + x];
	}
	
}//namespace Blade