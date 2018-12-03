/********************************************************************
	created:	2010/04/08
	filename: 	TaskManager.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_TaskManager_h__
#define __Blade_TaskManager_h__
#include <interface/ITaskManager.h>
#include <utility/BladeContainer.h>

namespace Blade
{
	class ITaskWorker;

	class TaskManager : public ITaskManager , public Singleton<TaskManager>
	{
	public:
		using Singleton<TaskManager>::getSingleton;
	public:
		TaskManager();
		~TaskManager();

		/**
		@describe 
		@param [in] maxTaskType: max type of task that execute in parallel. if 0, implementation will decide the value.
		@param [in] maxBackgroundTasks: max number of background tasks that execute in parallel. if 0, implementation will decide the value.
		some implementation doesn't use this value.
		@return
		*/
		virtual void		initialize(const TString& mode,size_t maxTaskType = 0, size_t maxBackgroundTasks = 0);

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void		shutdown();

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool		addTask(ITask* ptask);

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool		removeTask(const TString& TaskName);

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool		removeTask(ITask* ptask);

		/**
		@describe 
		@param 
		@return 
		*/
		virtual size_t		getTaskCount() const;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool		runAllTasks();

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool		updateAllTasks();

		/**
		@describe 
		@param 
		@return 
		*/
		virtual TASK_STATE	getCurrentState();

		/**
		@describe start tasks in list and resume. if block is needed, call sync for the returned handle. By using handle, you can sync at any time after function returns.
		@param
		@return
		*/
		virtual bool	dispatchTasks(IDispatchableTask** tasks, size_t count, IDispatchableTask::Handles* outHandle = NULL);

	protected:

		/** @brief  */
		void		autoRemoveBackgourndTasks();


		typedef		TStringMap<ITask*>	TaskMap;
		typedef		List<ITask*>		TaskList;

		/** @brief  */
		bool			mInitialized;
		/** @brief proxy for real task runner */
		ITaskWorker*	mWorker;
		/** @brief task registration map */
		TaskMap			mTaskMap;
		size_t			mTaskState;
		friend class ITaskWorker;
	};//class TaskManager


	
}//namespace Blade


#endif //__Blade_TaskManager_h__