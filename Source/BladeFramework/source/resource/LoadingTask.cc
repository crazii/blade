/********************************************************************
created:	2018/11/17
filename: 	LoadingTask.cc
author:		Crazii
purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "LoadingTask.h"
#include <interface/ITaskManager.h>

namespace Blade 
{
	const ITask::Type LoadingTask::TYPE = BTString("Resource IO");

	//////////////////////////////////////////////////////////////////////////
	const HTASK&		ProcessTask::getProcessTask(const ITask::Type& type, WorkQueue& readyQueue)
	{
		BLADE_TS_CHECK(TS_MAIN_SYNC);	//ITaskManager::addTask need operate on sync mode
		ProcessTaskMap& tasks = ProcessTask::getTaskMap();

		HTASK& task = tasks[type];
		if (task != NULL)
		{
			assert(task->getType() == type);
			return task;
		}

		task.bind(BLADE_NEW ProcessTask(type, readyQueue));
		ITaskManager::getSingleton().addTask(task);
		return task;
	}

	//////////////////////////////////////////////////////////////////////////
	void			ProcessTask::removeAllProcessTasks()
	{
		ProcessTaskMap& tasks = ProcessTask::getTaskMap();
		ITaskManager& taskManager = ITaskManager::getSingleton();

		for (ProcessTaskMap::iterator i = tasks.begin(); i != tasks.end(); ++i)
			taskManager.removeTask(i->second);
	}

}//namespace Blade 