/********************************************************************
	created:	2010/04/08
	filename: 	TBBTaskWorker.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#if BLADE_TASK_USE_TBB 
#include "TBBTaskWorker.h"
#include <tbb/tbb_exception.h>

namespace Blade
{
	namespace Impl
	{
		//////////////////////////////////////////////////////////////////////////
		static tbb::priority_t getTBBTaskPriority(ITask::ETaskPriority tp)
		{
			switch (tp)
			{
			case ITask::TP_HIGH:
				return tbb::priority_high;
				break;
			case ITask::TP_NORMAL:
				return tbb::priority_normal;
				break;
			case ITask::TP_LOW:
				return tbb::priority_low;
				break;
			case ITask::TP_BACKGROUND:
				return tbb::priority_low;
			default:
				assert(false);
				return tbb::priority_normal;
			}
		}

	}//namespace Impl



	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	TypeTask::TypeTask(ITask* ptask, int mode)
		:mTask(ptask)
		,mMode(mode)
		,mPriority(Impl::getTBBTaskPriority(ptask->getPriority()))
	{

	}

	//////////////////////////////////////////////////////////////////////////
	TypeTask::~TypeTask()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	tbb::task*	TypeTask::execute()
	{
		if( mMode == TBB_TASK_MODE_RUN)
			mTask->run();
		else
			mTask->update();

		return NULL;
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	RootTask::RootTask()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	RootTask::~RootTask()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	tbb::task*	RootTask::execute()
	{
		for( int i = 2; i >= 0; --i )
		{
			TaskList& list = mTaskList[i];
			for( TaskList::iterator n = list.begin(); n != list.end(); ++n )
			{
				this->set_ref_count(2);
				TypeTask& sub = *( new(this->allocate_child()) TypeTask(*n,mMode) );
				//sub.recycle_to_reexecute();
				this->spawn_and_wait_for_all(sub);
			}
		}
		return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	void		RootTask::setMode(int mode)
	{
		mMode = mode;
	}

	//////////////////////////////////////////////////////////////////////////
	struct FnPriorityGreater
	{
		bool	operator()(const ITask* lhs, const ITask* rhs) const
		{
			return lhs->getPriorityID() > rhs->getPriorityID();
		}
	};
	void		RootTask::addTask(ITask*	ptask)
	{
		ITask::ETaskPriority priority = ptask->getPriority();

		assert( priority != ITask::TP_BACKGROUND );

		TaskList& list = mTaskList[priority];
		list.push_back(ptask);
		list.sort( FnPriorityGreater() );
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	BackgroundTask::BackgroundTask(ITask*	ptask,TBBTaskWorker* owner)
		:mTask(ptask)
		,mOwner(owner)
		,mPriority(Impl::getTBBTaskPriority(ptask->getPriority()))
	{
		assert( mOwner != NULL );
		assert( ptask != NULL && ptask->getPriority() == ITask::TP_BACKGROUND );
	}

	//////////////////////////////////////////////////////////////////////////
	BackgroundTask::~BackgroundTask()
	{
		mOwner->pushFinishedBackgroundTask(mTask);
	}

	//////////////////////////////////////////////////////////////////////////
	tbb::task*	BackgroundTask::execute()
	{
		mTask->run();
		return NULL;
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	TBBTaskWorker::TBBTaskWorker()
		:mTbbTaskInit(tbb::task_scheduler_init::deferred)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	TBBTaskWorker::~TBBTaskWorker()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	bool	TBBTaskWorker::initialize(size_t maxTasktype, size_t /*maxBackgroundTasks*/)
	{
		if(maxTasktype == 0 )
			mTbbTaskInit.initialize(tbb::task_scheduler_init::automatic);
		else
			mTbbTaskInit.initialize((int)maxTasktype);
		return true;
	}


	//////////////////////////////////////////////////////////////////////////
	void	TBBTaskWorker::deinitialize()
	{
		mGroupedTask.clear();
		mTbbTaskInit.terminate();
	}

	//////////////////////////////////////////////////////////////////////////
	bool	TBBTaskWorker::acceptTask(ITask* ptask)
	{
		if( ptask->getPriority() == ITask::TP_BACKGROUND )
		{
			BackgroundTask& root = *( new(tbb::task::allocate_root() ) BackgroundTask(ptask, this) );
			tbb::task::enqueue(root, Impl::getTBBTaskPriority(ptask->getPriority()));
			mTotalBackgroundTaskCount.safeIncrement();
		}
		else
		{
			TypeGroupedList& list = mGroupedTask[ ptask->getType() ];
			list.push_back(ptask);
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	TBBTaskWorker::removeTask(ITask* ptask)
	{
		if( ptask->getPriority() == ITask::TP_BACKGROUND )
		{
			//TODO:
			//HOW TO?
			return true;
		}

		TypeGroupedList& list = mGroupedTask[ ptask->getType() ];

		TypeGroupedList::iterator i = std::find(list.begin(),list.end(),ptask);
		if( i != list.end() )
		{
			list.erase(i);
			return true;
		}
		return false;
	}


	//////////////////////////////////////////////////////////////////////////
	void	TBBTaskWorker::startWork()
	{
		this->startAllTasks(false);
	}

	//////////////////////////////////////////////////////////////////////////
	void	TBBTaskWorker::update()
	{
		this->startAllTasks(true);
	}

	//////////////////////////////////////////////////////////////////////////
	size_t	TBBTaskWorker::getBackgroundTaskCount() const
	{
		return mTotalBackgroundTaskCount.count();
	}

	//////////////////////////////////////////////////////////////////////////
	ITask*	TBBTaskWorker::popFinishedBackgroundTask()
	{
		if( mFinishedBackgroundTasks.empty() )
			return NULL;
		else
		{
			assert( mTotalBackgroundTaskCount.count() != 0);
			ITask* ptask = NULL;
			while( mFinishedBackgroundTasks.try_pop(ptask) == false );
			mTotalBackgroundTaskCount.safeDecrement();
			return ptask;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void	TBBTaskWorker::pushFinishedBackgroundTask(ITask* ptask)
	{
		if( ptask != NULL )
		{
			mFinishedBackgroundTasks.push(ptask);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void	TBBTaskWorker::startAllTasks(bool updating)
	{
		int mode = updating ? TBB_TASK_MODE_UPDATE : TBB_TASK_MODE_RUN;

		for (TypeGroupeMap::iterator i = mGroupedTask.begin(); i != mGroupedTask.end(); ++i)
		{
			if (i->first != ITask::ANY_TYPE)
			{
				RootTask& root = *(new(tbb::task::allocate_root()) RootTask());
				TypeGroupedList& list = i->second;

				for (TypeGroupedList::iterator n = list.begin(); n != list.end(); ++n)
					root.addTask(*n);

				root.setMode(mode);

				mRootList.push_back(root);
			}
			else
			{
				TypeGroupedList& list = i->second;
				for (TypeGroupedList::iterator n = list.begin(); n != list.end(); ++n)
				{
					RootTask& root = *(new(tbb::task::allocate_root()) RootTask());
					root.addTask(*n);
					root.setMode(mode);
					mRootList.push_back(root);
				}
			}
		}

		tbb::task::spawn_root_and_wait(mRootList);
	}
	
}//namespace Blade
#endif //BLADE_TASK_USE_TBB 