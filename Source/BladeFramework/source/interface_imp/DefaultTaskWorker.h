/********************************************************************
	created:	2017/12/02
	filename: 	DefaultTaskWorker.h
	author:		Crazii
	purpose:	Blade's default task management implementation	
*********************************************************************/
#ifndef ___DefaultTaskWorker_h__
#define ___DefaultTaskWorker_h__
#include "ITaskWorker.h"
#include <threading/Thread.h>
#include <threading/ConditionVariable.h>
#include <threading/Mutex.h>

namespace Blade
{

	class DefaultTaskWorker : public ITaskWorker, public Singleton<DefaultTaskWorker>
	{
	public:
		//task group for one type
		class TypedTask : public Allocatable
		{
		public:
			TypedTask(const TString& type, bool threadAffinity = false)
				:mType(type), mHighestPriority(ITask::TP_NORMAL), mUpdating(false), mDone(false), mInBackgroundQueue(false), mThreadAffinity(threadAffinity)
			{
			}

			virtual ~TypedTask() {}

			/** @brief  */
			inline const TString& getType() const { return mType; }

			/** @brief  */
			inline size_t getTaskCount() const { return mTasks.size(); }

			/** @brief  */
			virtual void run()
			{
				if (!mUpdating)
				{
					for (TaskList::iterator i = mTasks.begin(); i != mTasks.end(); ++i)
						(*i)->run();
					if (this->isBackground())
						mDone = true;
				}
				else
				{
					assert(!this->isBackground());
					for (TaskList::iterator i = mTasks.begin(); i != mTasks.end(); ++i)
						(*i)->update();
				}
			}

			/** @brief  */
			inline bool operator<(const TypedTask& rhs) const
			{
				return mHighestPriority > rhs.mHighestPriority;
			}

			struct FnTaskCmp
			{
				inline bool operator()(const ITask* lhs, const ITask* rhs) const
				{
					return lhs->getPriority() > rhs->getPriority() && lhs->getPriorityID() > rhs->getPriorityID();
				}
			};

			/** @brief  */
			inline void setUpdating(bool updating)
			{
				mUpdating = updating;
			}

			/** @brief  */
			inline bool addTask(ITask* task)
			{
				assert(task != NULL);
				TaskList::iterator i = std::find(mTasks.begin(), mTasks.end(), task);
				if (i != mTasks.end())
				{
					assert(false);
					return false;
				}

				mTasks.push_back(task);
				//sort task by priority
				mTasks.sort(FnTaskCmp());
				return true;
			}

			/** @brief  */
			inline bool	removeTask(ITask* task)
			{
				TaskList::iterator i = std::find(mTasks.begin(), mTasks.end(), task);
				if (i != mTasks.end())
				{
					mTasks.erase(i);
					return true;
				}
				return false;
			}

			/** @brief  */
			inline size_t size() const { return mTasks.size(); }

			/** @brief  */
			inline bool isBackground() const
			{
				assert(mTasks.size() != 0);
				return (*mTasks.begin())->getPriority() == ITask::TP_BACKGROUND;
			}

			/** @brief  */
			inline bool	isInBackgoundQueue() const { return mInBackgroundQueue; }

			/** @brief  */
			inline void	setInBackgroundQueue() { mInBackgroundQueue = true; }

			/** @brief  */
			inline bool isDoneBackground() const
			{
				assert(this->isBackground());
				return mDone;
			}

			/** @brief  */
			inline ITask* getBackgroundTask() const
			{
				assert(this->isBackground());
				assert(mTasks.size() == 1);
				return *mTasks.begin();
			}

			/** @brief  */
			inline bool	getTheadAffinity() const { return mThreadAffinity; }

			/** @brief  */
			inline void changeType(const TString& type) { mType = type; }

		protected:
			typedef List<ITask*> TaskList;

			TString		mType;
			TaskList	mTasks;
			ITask::ETaskPriority mHighestPriority;
			bool		mUpdating;
			bool		mDone;		//done for background task
			bool		mInBackgroundQueue;
			bool		mThreadAffinity;
		};// class TypedTask
		typedef Handle<TypedTask> HTYPEDTASK;
		typedef ConQueue<TypedTask*> TaskQueue;

		struct FnTypedTaskHandlePriorityCmp
		{
			inline bool operator()(const HTYPEDTASK& lhs, const HTYPEDTASK& rhs) const
			{
				return *lhs < *rhs;
			}
		};

		struct FnTypedTaskHandleTypeFinder : public NonAssignable
		{
			const TString& type;
			FnTypedTaskHandleTypeFinder(const TString& _type) :type(_type) {}

			inline bool operator()(const HTYPEDTASK& target) const
			{
				return target->getType() == type;
			}
		};

		class NoneTypedTask : public TempAllocatable
		{
		public:
			typedef void(*PFN)(void*);

			/** @brief  */
			NoneTypedTask(PFN fn, void* data, TString name)
			{
				mFN = fn;
				mFNInput = data;
				mName = name;
			}

			/** @brief  */
			inline const TString& getName() const
			{
				return mName;
			}

			/** @brief  */
			inline void run()
			{
				mFN(mFNInput);
			}

		protected:
			PFN		mFN;
			void*	mFNInput;
			TString	mName;
		};

		class TaskHost : public Thread::IExecuter, public Allocatable
		{
		public:
			TaskHost(TaskQueue* taskQueue, TaskQueue* backgroundQueue)
				:mTaskQueue(taskQueue)
				,mBackgroundTaskQueue(backgroundQueue)
				,mRunOnce(NULL)
				,mRunOnceLock(NULL)
				,mFixedGuest(NULL)
				,mRunOnceFinishBit(NULL)
				,mExit(false)
				,mWorking(false)
				,mStarted(false)
				,mBackground(false)
			{
			}

			TaskHost()
				:mTaskQueue(NULL)
				,mBackgroundTaskQueue(NULL)
				,mRunOnce(NULL)
				,mRunOnceLock(NULL)
				,mFixedGuest(NULL)
				,mRunOnceFinishBit(NULL)
				,mExit(false)
				,mWorking(false)
				,mStarted(false)
				,mBackground(false)
			{
			}

			/** @brief  */
			inline volatile bool isWorking() const { return mWorking; }

			/** @brief  */
			inline volatile bool isStarted() const { return mStarted; }

			struct FnPredicateWorking : public NonAssignable
			{
				const TaskHost& host;
				FnPredicateWorking(const TaskHost& h) :host(h) {}
				inline bool operator()() const
				{
					return !host.isWorking();
				}
			};

			struct FnPredicateStarted : public NonAssignable
			{
				const TaskHost& host;
				FnPredicateStarted(const TaskHost& h) :host(h) {}
				inline bool operator()() const
				{
					return host.isStarted();
				}
			};

			/** @brief  */
			virtual void run()
			{
				this->updateThreadName(NULL);

				while (!mExit)
				{
					TypedTask* backgroundTask = NULL;

					{
						TypedTask* work = NULL;
						UniqueLock lock(mMutex);
						mStartCV.wait(lock, FnPredicateStarted(*this));
						mStarted = false;

						if (mRunOnce == NULL)
						{
							if (mFixedGuest != NULL)
							{
								this->updateThreadName(mFixedGuest);
								mFixedGuest->run();
								this->updateThreadName(NULL);
							}
							else if (mTaskQueue != NULL)
							{
								//spin on the queue to fetch & run task
								while (!mTaskQueue->empty())
								{
									if (!mTaskQueue->popSafe(work))
										continue;

									this->updateThreadName(work);
									work->run();
									this->updateThreadName(NULL);
								}
							}

							//prefetch background task
							if (mBackgroundTaskQueue != NULL)
							{
								while (!mBackgroundTaskQueue->empty())
								{
									if (!mBackgroundTaskQueue->popSafe(backgroundTask))
										continue;
								}
							}
							mBackground = (backgroundTask != NULL);
						} //mRunOnce == NULL
						else
						{
							this->updateThreadName(mRunOnce->getName());
							mRunOnce->run();
							mRunOnce = NULL;
						}

						this->updateThreadName(NULL);
						//set flag & notify
						mWorking = false;
						mFinishCV.notifyOne();
					}
					//release it out if mutex, or there will be deadlock between DefaultDispatchableTaskHandle's wait()
					if (mRunOnceLock != NULL)
					{
						ScopedLock sl(*mRunOnceLock);
						*mRunOnceFinishBit = false;
						mRunOnceLock = NULL;
						mRunOnceFinishBit = NULL;
					}

					//now that all main tasks are done & main thread notified, continue working on background tasks
					while (backgroundTask != NULL)
					{
						this->updateThreadName(backgroundTask);

						assert(!backgroundTask->isDoneBackground());
						backgroundTask->run();
						assert(backgroundTask->isDoneBackground());

						backgroundTask = NULL;
						while (!mBackgroundTaskQueue->empty())
						{
							assert(mBackground);

							if (!mBackgroundTaskQueue->popSafe(backgroundTask))
								continue;
						}
					}

					{
						UniqueLock lock(mMutex);
						mBackground = false;
					}

				}//while(!mExit)
			}

			/** @brief  */
			inline void updateThreadName(TypedTask* task)
			{
#if BLADE_DEBUG 
				if (task != NULL)
					Thread::setThreadName(Thread::getCurrentID(), task->getType() + BTString(" - Blade"));
				else
					Thread::setThreadName(Thread::getCurrentID(), BTString("BladeWorker"));
#else
				BLADE_UNREFERENCED(task);
#endif
			}

			/** @brief  */
			inline void updateThreadName(const TString& name)
			{
#if BLADE_DEBUG
				Thread::setThreadName(Thread::getCurrentID(), name + BTString(" - Blade"));
#else
				BLADE_UNREFERENCED(name);
#endif
			}

			/** @brief  */
			inline void start()
			{
				{
					UniqueLock lock(mMutex);
					//if there are too many background tasks, then hosts will be all occupied, and main tasks will have no hosts for them,
					//now extra pool slots (EXTRA_BACKGROUND_HOST) handles background tasks, leave enough hosts to do main task

					if (!mBackground)	//leave back ground hosts continue working, they will be signaled after background tasks done.
					{
						mStarted = true;
						mWorking = true;
					}
				}
				mStartCV.notifyOne();
			}

			/** @brief  */
			inline void startOnce(IDispatchableTask* task, Lock* finishLock, int* finishBit)
			{
				if (task == NULL)
					return;

				{
					UniqueLock lock(mMutex);
					assert(!mBackground);
					assert(mRunOnce == NULL);
					mRunOnce = task;
					mRunOnceLock = finishLock;
					mRunOnceFinishBit = finishBit;

					mStarted = true;
					mWorking = true;
				}
				mStartCV.notifyOne();
			}

			/** @brief called from main. to wait this done */
			inline void wait()
			{
				UniqueLock lock(mMutex);
				mFinishCV.wait(lock, FnPredicateWorking(*this));
			}

			/** @brief  */
			inline void exit()
			{
				UniqueLock lock(mMutex);
				mExit = true;
				mBackground = false;
				this->start();
			}

			/** @brief  */
			inline TypedTask* getFixedGuest() const { return mFixedGuest; }

			/** @brief  */
			inline void setFixedGuest(TypedTask* guest) { mFixedGuest = guest; }

		protected:
			Mutex				mMutex;
			ConditionVariable	mFinishCV;
			ConditionVariable	mStartCV;	//note: the start cv is not shared among all hosts, so they cannot be signaled at once using notifyAll()
			TaskQueue*			mTaskQueue;
			TaskQueue*			mBackgroundTaskQueue;
			IDispatchableTask*volatile	mRunOnce;
			Lock*				mRunOnceLock;
			TypedTask*			mFixedGuest;
			int*				mRunOnceFinishBit;
			volatile bool		mExit;
			volatile bool		mWorking;
			volatile bool		mStarted;
			volatile bool		mBackground;
		};// class TaskHost
		typedef Handle<TaskHost> HTASKHOST;

	public:
		DefaultTaskWorker();
		~DefaultTaskWorker();

		/************************************************************************/
		/* ITaskWorker interface                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		virtual bool	initialize(size_t maxTaskType, size_t maxBackgroundTasks);

		/*
		@describe
		@param
		@return
		*/
		virtual void	deinitialize();

		/*
		@describe
		@param
		@return
		*/
		virtual bool	acceptTask(ITask* ptask);

		/*
		@describe
		@param
		@return
		*/
		virtual bool	removeTask(ITask* ptask);

		/*
		@describe
		@param
		@return
		*/
		virtual void	startWork();

		/*
		@describe
		@param
		@return
		*/
		virtual void	update();


		/*
		@describe get the un-finished (sill running) background task count
		@param
		@return
		*/
		virtual size_t	getBackgroundTaskCount() const;

		/*
		@describe get the finished background task and remove from list
		@param
		@return
		*/
		virtual ITask*	popFinishedBackgroundTask();

		/**
		@describe 
		@param
		@return
		*/
		virtual void	dispatchTasks(IDispatchableTask** tasks, size_t count, IDispatchableTask::Handles* outHandle = NULL);

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
	protected:
		/** @brief  */
		inline void	startAll(bool updating);

		typedef Vector<HTHREAD> ThreadPool;
		typedef Vector<HTASKHOST> TaskHostList;
		typedef Vector<HTYPEDTASK> TaskList;
		typedef List<HTYPEDTASK> BackgroundTaskList;
		typedef Vector<int> DispatchMask;
		typedef Vector<Lock> DispatchLock;

		ThreadPool	mThreads;
		TaskHostList mHosts;
		TaskList	mTasks;
		TaskQueue	mTaskQueue;
		TaskQueue	mBackgroundQueue;

		TypedTask	mMainThreadTasks;

		BackgroundTaskList		mBackgroundTasks;

		ThreadPool		mDispatchableThreads;
		TaskHostList	mDispatchableHost;
		DispatchMask	mDispatchMask;
		DispatchLock	mDispatchLock;
		size_t		mMaxTaskType;
	};
	
}//namespace Blade
#endif//___DefaultTaskWorker_h__