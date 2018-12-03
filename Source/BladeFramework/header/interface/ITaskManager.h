/********************************************************************
	created:	2010/04/08
	filename: 	ITaskManager.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_ITaskManager_h__
#define __Blade_ITaskManager_h__
#include <BladeFramework.h>
#include <interface/InterfaceSingleton.h>
#include <interface/public/ITask.h>

namespace Blade
{
	class ITaskManager : public InterfaceSingleton<ITaskManager>
	{
	public:
		virtual	~ITaskManager() {}

		/**
		@describe 
		@param [in] maxTaskType: max type of task that execute in parallel. if 0, implementation will decide the value.
		@param [in] maxBackgroundTasks: max number of background tasks that execute in parallel. if 0, implementation will decide the value.
					some implementation doesn't use this value.
		@return 
		*/
		virtual void		initialize(const TString& mode, size_t maxTaskType = 0, size_t maxBackgroundTasks = 0) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void		shutdown() = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool		addTask(ITask* ptask) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool		removeTask(const TString& TaskName) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool		removeTask(ITask* ptask) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual size_t		getTaskCount() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool		runAllTasks() = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool		updateAllTasks() = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual TASK_STATE	getCurrentState() = 0;

		/**
		@describe start tasks in list and resume. if block is needed, call sync for the returned handle. By using handle, you can sync at any time after function returns.
		@param
		@return
		*/
		virtual bool	dispatchTasks(IDispatchableTask** tasks, size_t count, IDispatchableTask::Handles* outHandle = NULL) = 0;

	};//class ITaskManager

	extern template class BLADE_FRAMEWORK_API Factory<ITaskManager>;

}//namespace Blade



#endif //__Blade_ITaskManager_h__