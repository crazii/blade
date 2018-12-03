/********************************************************************
	created:	2010/04/08
	filename: 	TBBTaskWorker.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_TBBTaskWorker_h__
#define __Blade_TBBTaskWorker_h__
#define BLADE_TASK_USE_TBB 0

#if BLADE_TASK_USE_TBB
#include "ITaskWorker.h"
#include <RefCount.h>
#include <utility/BladeContainer.h>

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable:4365 4574 4347)
#endif
#include <tbb/task.h>
#include <tbb/task_scheduler_init.h>
#include <tbb/concurrent_queue.h>
#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(pop)
#endif

namespace Blade
{
	static const int TBB_TASK_MODE_RUN		= 0;
	static const int TBB_TASK_MODE_UPDATE	= 1;

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class TypeTask : public tbb::task
	{
	public:
		TypeTask(ITask* ptask,int mode);
		~TypeTask();

		/** @brief  */
		virtual tbb::task*	execute();

		/** @brief  */
		virtual tbb::priority_t group_priority() const { return mPriority; }

	protected:
		ITask*	mTask;
		int		mMode;
		tbb::priority_t mPriority;
	};//class TypeTask

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class RootTask : public tbb::task
	{
	public:
		RootTask();
		~RootTask();

		/** @brief  */
		virtual tbb::task*	execute();

		/** @brief  */
		void		setMode(int mode);

		/** @brief  */
		void		addTask(ITask*	ptask);

	protected:
		typedef		TempList<ITask*>	TaskList;

		TaskList		mTaskList[3];
		int				mMode;
	};//class RootTask

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class TBBTaskWorker;
	class BackgroundTask : public tbb::task
	{
	public:
		BackgroundTask(ITask*	ptask,TBBTaskWorker* owner);
		~BackgroundTask();

		/** @brief  */
		virtual tbb::task*	execute();

		/** @brief  */
		virtual tbb::priority_t group_priority() const { return mPriority; }

	protected:
		ITask*	mTask;
		TBBTaskWorker*	mOwner;
		tbb::priority_t mPriority;
	};// class BackgroundTask

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class TBBTaskWorker : public ITaskWorker ,public Singleton<TBBTaskWorker>
	{
	public:
		TBBTaskWorker();
		~TBBTaskWorker();

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool	initialize(size_t maxTasktype, size_t maxBackgroundTasks);


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
		@describe remove task
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

		/************************************************************************/
		/*                                                                      */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		void			pushFinishedBackgroundTask(ITask* ptask);

		/**
		@describe 
		@param
		@return
		*/
		void			startAllTasks(bool updating);

	protected:
		typedef		List<ITask*>					TypeGroupedList;
		typedef		Map<TString,TypeGroupedList>	TypeGroupeMap;
		typedef		tbb::strict_ppl::concurrent_queue<ITask*>	BackgroundTaskList;

		tbb::task_scheduler_init mTbbTaskInit;

		TypeGroupeMap		mGroupedTask;
		tbb::task_list		mRootList;
		BackgroundTaskList	mFinishedBackgroundTasks;
		RefCount			mTotalBackgroundTaskCount;
	};//class RootTask

	
}//namespace Blade

#endif//BLADE_TASK_USE_TBB 

#endif //__Blade_TBBTaskWorker_h__