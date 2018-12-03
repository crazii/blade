/********************************************************************
	created:	2010/04/08
	filename: 	ITaskWorker.h
	author:		Crazii
	
	purpose:	
	note:		this is not a public interface ,it's used internally and for internal extension in framework level
*********************************************************************/
#ifndef __Blade_ITaskWorker_h__
#define __Blade_ITaskWorker_h__
#include <BladeFramework.h>
#include <interface/InterfaceSingleton.h>
#include <Factory.h>
#include <interface/public/ITask.h>

namespace Blade
{
	class ITaskWorker : public InterfaceSingleton<ITaskWorker>
	{
	public:
		virtual ~ITaskWorker()
		{
		}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool	initialize(size_t maxTasktype, size_t maxBackgroundTasks) = 0;


		/*
		@describe 
		@param 
		@return 
		*/
		virtual void	deinitialize() = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool	acceptTask(ITask* ptask) = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool	removeTask(ITask* ptask) = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void	startWork() = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void	update() = 0;

		
		/*
		@describe get the un-finished (sill running) background task count
		@param 
		@return 
		*/
		virtual size_t	getBackgroundTaskCount() const = 0;

		/*
		@describe get the finished background task and remove from list
		@param 
		@return 
		*/
		virtual ITask*	popFinishedBackgroundTask() = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual void	dispatchTasks(IDispatchableTask** tasks, size_t count, IDispatchableTask::Handles* outHandle = NULL) = 0;

	};//class ITaskWorker
	

	extern template class BLADE_FRAMEWORK_API Factory<ITaskWorker>;
	typedef Factory<ITaskWorker> TaskWorkerFactory;

}//namespace Blade


#endif //__Blade_ITaskWorker_h__