/********************************************************************
	created:	2017/12/02
	filename: 	DefaultTaskWorker.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "DefaultTaskWorker.h"

namespace Blade
{
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	DefaultTaskWorker::DefaultTaskWorker()
		:mMainThreadTasks(TString::EMPTY)
	{
		mThreads.reserve(4);
		mHosts.reserve(8);
		mTasks.reserve(8);
	}

	//////////////////////////////////////////////////////////////////////////
	DefaultTaskWorker::~DefaultTaskWorker()
	{

	}

	/************************************************************************/
	/* ITaskWorker interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool	DefaultTaskWorker::initialize(size_t maxTaskType, size_t maxBackgroundTasks)
	{
		if (maxTaskType == 0)
			maxTaskType = Thread::getHardwareConcurrency();
		if (maxBackgroundTasks == 0)
			maxBackgroundTasks = std::max(Thread::getHardwareConcurrency() - maxTaskType, std::max<size_t>(maxTaskType, 4u) );

		mMaxTaskType = maxTaskType;

		mThreads.resize(maxTaskType + maxBackgroundTasks);
		mHosts.resize(maxTaskType + maxBackgroundTasks);
		for (size_t i = 0; i < mThreads.size(); ++i)
		{
			TaskQueue* backgroundQueue = (i >= maxTaskType ? &mBackgroundQueue : NULL);
			mHosts[i].bind(BLADE_NEW TaskHost(&mTaskQueue, backgroundQueue));
			mThreads[i].bind(BLADE_NEW Thread(mHosts[i]));
		}

		mDispatchableThreads.resize(maxTaskType * 4);
		mDispatchableHost.resize(maxTaskType * 4);
		mDispatchMask.resize(mMaxTaskType * 4);
		mDispatchLock.resize(mMaxTaskType * 4);
		for (size_t i = 0; i < mDispatchableThreads.size(); ++i)
		{
			mDispatchableHost[i].bind(BLADE_NEW TaskHost(NULL, NULL));
			mDispatchableThreads[i].bind(BLADE_NEW Thread(mDispatchableHost[i]));
			mDispatchMask[i] = false;
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void	DefaultTaskWorker::deinitialize()
	{
		for (size_t i = 0; i < mHosts.size(); ++i)
			mHosts[i]->exit();

		for (size_t i = 0; i < mThreads.size(); ++i)
			mThreads[i]->join();

		mThreads.clear();
		mHosts.clear();

		mTasks.clear();
		mBackgroundTasks.clear();

		for (size_t i = 0; i < mDispatchableHost.size(); ++i)
		{
			//TODO: unfinished handle will be dangling
			//@see DefaultDispatchableTaskHandle
			//all returned handles should be deleted at this point, otherwise there will be a crash
			ScopedLock sl(mDispatchLock[i]);
			mDispatchableHost[i]->wait();
			mDispatchableHost[i]->exit();
		}

		for (size_t i = 0; i < mDispatchableThreads.size(); ++i)
			mDispatchableThreads[i]->join();

		mDispatchableThreads.clear();
		mDispatchableThreads.clear();

		mMaxTaskType = 0;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	DefaultTaskWorker::acceptTask(ITask* ptask)
	{
		BLADE_TS_VERIFY(TS_MAIN_SYNC);

		assert(ptask != NULL);
		const ITask::Type& type = ptask->getType();

		if (ptask->getPriority() == ITask::TP_BACKGROUND)
		{
			HTYPEDTASK task(BLADE_NEW TypedTask(type.name));
			task->addTask(ptask);
			mBackgroundTasks.push_back(task);
			mBackgroundTasks.sort(FnTypedTaskHandlePriorityCmp());
			return true;
		}

		if (type != ITask::ANY_TYPE)
		{
			if (type.affinity != ITask::TA_MAIN)
			{
				TaskList::iterator i = std::find_if(mTasks.begin(), mTasks.end(), FnTypedTaskHandleTypeFinder(type.name));
				if (i == mTasks.end())
				{
					bool threadAffinity = type.affinity == ITask::TA_ANY;
					HTYPEDTASK task(BLADE_NEW TypedTask(type.name, threadAffinity));
					i = mTasks.insert(i, task);
					//sort by highest priority
					std::sort(mTasks.begin(), mTasks.end(), FnTypedTaskHandlePriorityCmp());

					if (threadAffinity)
					{
						//find an empty guested host and assign it
						for (size_t j = 0; j < mHosts.size(); ++j)
						{
							if (mHosts[j]->getFixedGuest() == task)
							{
								mHosts[j]->setFixedGuest(task);
								break;
							}
						}
					}
				}

				return static_cast<TypedTask*>(*i)->addTask(ptask);
			}
			else
			{
				if (mMainThreadTasks.getTaskCount() == 0)
				{
					mMainThreadTasks.changeType(type.name);
					mMainThreadTasks.addTask(ptask);
				}
				else if (mMainThreadTasks.getType() == type.name)
					mMainThreadTasks.addTask(ptask);
				else
				{
					assert(false);
					return false;
				}
				return true;
			}
		}
		else
		{
			HTYPEDTASK task(BLADE_NEW TypedTask(type.name));
			task->addTask(ptask);
			mTasks.push_back(task);
			std::sort(mTasks.begin(), mTasks.end(), FnTypedTaskHandlePriorityCmp());
			return true;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool	DefaultTaskWorker::removeTask(ITask* ptask)
	{
		if (ptask->getPriority() == ITask::TP_BACKGROUND)
			return true;
		else if (ptask->getType() == ITask::ANY_TYPE)
		{
			assert(ptask != NULL);
			const ITask::Type& type = ptask->getType();
			TaskList::iterator i = mTasks.begin();
			while (i != mTasks.end())
			{
				i = std::find_if(i, mTasks.end(), FnTypedTaskHandleTypeFinder(type.name));
				if (i != mTasks.end())
				{
					if((*i)->removeTask(ptask))
						return true;
					++i;
				}
			}
			return false;
		}
		else
		{
			assert(ptask != NULL);
			const ITask::Type& type = ptask->getType();
			if (type.affinity != ITask::TA_MAIN)
			{
				TaskList::iterator i = std::find_if(mTasks.begin(), mTasks.end(), FnTypedTaskHandleTypeFinder(type.name));
				if (i != mTasks.end())
					return (*i)->removeTask(ptask);
			}
			else
				return mMainThreadTasks.removeTask(ptask);
			return false;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void	DefaultTaskWorker::startWork()
	{
		this->startAll(false);
	}

	//////////////////////////////////////////////////////////////////////////
	void	DefaultTaskWorker::update()
	{
		this->startAll(true);
	}

	//////////////////////////////////////////////////////////////////////////
	size_t	DefaultTaskWorker::getBackgroundTaskCount() const
	{
		return mBackgroundTasks.size();
	}

	//////////////////////////////////////////////////////////////////////////
	ITask*	DefaultTaskWorker::popFinishedBackgroundTask()
	{
		for (BackgroundTaskList::iterator i = mBackgroundTasks.begin(); i != mBackgroundTasks.end(); ++i)
		{
			const HTYPEDTASK& task = *i;
			if (task->isDoneBackground())
			{
				assert(task->isInBackgoundQueue());
				ITask* t = task->getBackgroundTask();
				mBackgroundTasks.erase(i);
				return t;
			}
		}
		return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	class DefaultDispatchableTaskHandle : public  IDispatchableTask::IHandle, public Allocatable
	{
	public:
		DefaultTaskWorker::TaskHost* mHost;				 
		Lock*	mLock;
#if BLADE_DEBUG
		DefaultTaskWorker* mWorker;
#endif

		DefaultDispatchableTaskHandle(DefaultTaskWorker::TaskHost* host, Lock* l, DefaultTaskWorker* worker) 
			:mHost(host)
			,mLock(l)
#if BLADE_DEBUG
			,mWorker(worker)
#endif
		{
#if !BLADE_DEBUG
			BLADE_UNREFERENCED(worker);
#endif
		}
		virtual ~DefaultDispatchableTaskHandle()
		{
			this->DefaultDispatchableTaskHandle::sync();
		}

		virtual void sync()
		{
			ScopedLock sl(*mLock);
			mHost->wait();
		}
	};
	void	DefaultTaskWorker::dispatchTasks(IDispatchableTask** tasks, size_t count, IDispatchableTask::Handles* outHandle/* = NULL*/)
	{
		TempVector<TaskHost*> selectedhosts;
		TempVector<size_t> selectedIndices;
		selectedhosts.reserve(count);
		selectedIndices.reserve(count);

		{
			size_t i = 0;
			size_t total = mDispatchMask.size();

			while (selectedhosts.size() < count)
			{
				if (mDispatchLock[i].tryLock())
				{
					if (!mDispatchMask[i])
					{
						mDispatchMask[i] = true;
						selectedhosts.push_back(mDispatchableHost[i]);
						selectedIndices.push_back(i);
					}
					else
						mDispatchLock[i].unlock();
				}

				i = (i + 1) % total;
			}

			assert(selectedIndices.size() == count);
			assert(selectedhosts.size() == count);
		}

		for (size_t i = 0; i < count; ++i)
			selectedhosts[i]->startOnce(tasks[i], &mDispatchLock[selectedIndices[i]], &mDispatchMask[selectedIndices[i]]);

		if (outHandle != NULL)
		{
			outHandle->resize(count);
			for (size_t i = 0; i < count; ++i)
				(*outHandle)[i] = BLADE_NEW DefaultDispatchableTaskHandle(selectedhosts[i], &mDispatchLock[selectedIndices[i]], this);
		}

		for (size_t i = 0; i < count; ++i)
			mDispatchLock[selectedIndices[i]].unlock();
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	inline void	DefaultTaskWorker::startAll(bool updating)
	{
		assert(mTaskQueue.empty());

		//prepare queue for all hosts
		for (size_t i = 0; i < mTasks.size(); ++i)
		{
			if (mTasks[i]->size() > 0)
			{
				mTasks[i]->setUpdating(updating);
				if(!mTasks[i]->getTheadAffinity())	//task with thread affinity is bound to host directly, not put in queue.
					mTaskQueue.pushSafe(mTasks[i]);
			}
		}

		//put background tasks at last
		if (!updating)
		{
			for (BackgroundTaskList::iterator i = mBackgroundTasks.begin(); i != mBackgroundTasks.end(); ++i)
			{
				const HTYPEDTASK& task = *i;
				assert(task->isBackground());
				if (task->size() > 0 && !task->isDoneBackground() && !task->isInBackgoundQueue())
				{
					task->setUpdating(false);
					task->setInBackgroundQueue();
					mBackgroundQueue.pushSafe(task);
				}
			}
		}

		size_t count = std::min(mTasks.size(), mMaxTaskType);
		//start all main host
		for (size_t i = 0; i < count; ++i)
			mHosts[i]->start();
		
		count = std::min(mHosts.size(), mMaxTaskType + mBackgroundQueue.size());
		//start background host
		for (size_t i = mMaxTaskType; i < count; ++i)
			mHosts[i]->start();

		mMainThreadTasks.setUpdating(updating);
		mMainThreadTasks.run();

		//wait all hosts
		for (size_t i = 0; i < mHosts.size(); ++i)
			mHosts[i]->wait();

		assert(mTaskQueue.empty());
	}

}//namespace Blade