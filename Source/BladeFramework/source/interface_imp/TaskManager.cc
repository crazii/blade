/********************************************************************
	created:	2010/04/08
	filename: 	TaskManager.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <ExceptionEx.h>
#include "TaskManager.h"
#include "ITaskWorker.h"


namespace Blade
{
	//global state:
	extern size_t BLADE_TASK_STATE;

	//////////////////////////////////////////////////////////////////////////
	TaskManager::TaskManager()
		:mInitialized(false)
		,mWorker(NULL)
		,mTaskState(TS_MAIN_SYNC)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	TaskManager::~TaskManager()
	{
		this->shutdown();
	}

	//////////////////////////////////////////////////////////////////////////
	void		TaskManager::initialize(const TString& mode,size_t maxTaskType/* = 0*/, size_t maxBackgroundTasks/* = 0*/)
	{
		if( mInitialized )
		{
			BLADE_EXCEPT(EXCE_STATE_ERROR, BTString("Task Manger has already initialized.") );
			return;
		}

		mInitialized = true;

		mWorker = ITaskWorker::interchange(mode);

		mWorker->initialize(maxTaskType, maxBackgroundTasks);
	}

	//////////////////////////////////////////////////////////////////////////
	void		TaskManager::shutdown()
	{
		if( mInitialized )
		{
			mInitialized = false;
			assert(mWorker != NULL);
			while( mWorker->getBackgroundTaskCount() != 0)
				this->autoRemoveBackgourndTasks();
			mWorker->deinitialize();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool		TaskManager::addTask(ITask* ptask)
	{
		BLADE_TS_CHECK(TS_MAIN_SYNC);
		if( ptask == NULL )
		{
			assert(false);
			return false;
		}

		assert( mWorker != NULL );
		if( mWorker == NULL || !mInitialized )
			BLADE_EXCEPT(EXCE_STATE_ERROR,BTString("task manager not initialized yet."));

		ITask*& emptytask =  mTaskMap[ ptask->getName() ];
		if( emptytask != NULL )
			BLADE_EXCEPT(EXCE_STATE_ERROR,BTString("task with the same name already exist:")+ ptask->getName());

		emptytask = ptask;
		ptask->onAccepted();
		return mWorker->acceptTask(ptask);
	}

	//////////////////////////////////////////////////////////////////////////
	bool		TaskManager::removeTask(const TString& TaskName)
	{

		assert( mWorker != NULL );
		if( mWorker == NULL || !mInitialized )
			BLADE_EXCEPT(EXCE_STATE_ERROR,BTString("task manager not initialized yet."));

		TaskMap::iterator i = mTaskMap.find( TaskName );

		if(i == mTaskMap.end() )
		{
			BLADE_LOG(Error, TEXT("task not found - task name:'")
				<< TaskName << TEXT("' in function '")
				<< BLADE_TFUNCTION << TEXT("'") );
			return false;
		}

		ITask* ptask = i->second;
		bool result = mWorker->removeTask(ptask);
		assert(result);
		mTaskMap.erase(i);
		ptask->onRemoved();
		return result;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		TaskManager::removeTask(ITask* ptask)
	{
		if( ptask == NULL )
			return false;

		return removeTask( ptask->getName() );
	}

	//////////////////////////////////////////////////////////////////////////
	size_t		TaskManager::getTaskCount() const
	{
		return mTaskMap.size();
	}

	//////////////////////////////////////////////////////////////////////////
	bool		TaskManager::runAllTasks()
	{
		assert( mWorker != NULL );
		if( mWorker == NULL || !mInitialized )
		{
			BLADE_EXCEPT(EXCE_STATE_ERROR,BTString("task manager not initialized yet."));
			//return false;
		}

		Lock::set(&mTaskState, TS_ASYNC_RUN);
		Lock::set(&BLADE_TASK_STATE, TS_ASYNC_RUN);

		mWorker->startWork();
		Lock::set(&mTaskState, TS_MAIN_SYNC);
		Lock::set(&BLADE_TASK_STATE, TS_MAIN_SYNC);
		return true;
	}


	//////////////////////////////////////////////////////////////////////////
	bool		TaskManager::updateAllTasks()
	{
		Lock::set(&mTaskState, TS_ASYNC_UPDATE);
		Lock::set(&BLADE_TASK_STATE, TS_ASYNC_UPDATE);

		assert( mWorker != NULL );
		if( mWorker == NULL || !mInitialized )
		{
			BLADE_EXCEPT(EXCE_STATE_ERROR,BTString("task manager not initialized yet."));
			//return false;
		}

		mWorker->update();
		this->autoRemoveBackgourndTasks();

		Lock::set(&mTaskState, TS_MAIN_SYNC);
		Lock::set(&BLADE_TASK_STATE, TS_MAIN_SYNC);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	TASK_STATE	TaskManager::getCurrentState()
	{
		return (TASK_STATE)mTaskState;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	TaskManager::dispatchTasks(IDispatchableTask** tasks, size_t count, IDispatchableTask::Handles* outHandle/* = NULL*/)
	{
		if (count == 0 || tasks == NULL)
			return false;

		mWorker->dispatchTasks(tasks, count, outHandle);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void		TaskManager::autoRemoveBackgourndTasks()
	{
		//auto remove background tasks
		ITask* pBackgroundTask;
		do
		{
			pBackgroundTask = mWorker->popFinishedBackgroundTask();
			if( pBackgroundTask != NULL )
				this->removeTask(pBackgroundTask);
		} while (pBackgroundTask != NULL);
	}
	
}//namespace Blade