/********************************************************************
	created:	2010/05/07
	filename: 	GeometryTask.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "GeometryTask.h"
#include <parallel/ParaStateQueue.h>
#include "GeometryUpdater.h"

namespace Blade
{
	const ITask::Type GeometryTask::TYPE = ITask::ANY_TYPE;

	//////////////////////////////////////////////////////////////////////////
	GeometryTask::GeometryTask(const TString& name,GeometryUpdater& updater, ParaStateQueue& queue,ITask::ETaskPriority priority/* = ITask::TP_NORMAL*/)
		:TaskBase(name,priority)
		,mMethod(GUM_MANUAL)
		,mQueue(queue)
		,mUpdater(updater)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	GeometryTask::~GeometryTask()
	{

	}

	/************************************************************************/
	/* ITask interface                                                                     */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	const ITask::Type&	GeometryTask::getType() const
	{
		return TYPE;
	}

	//////////////////////////////////////////////////////////////////////////
	void			GeometryTask::run()
	{
		if( mMethod == GUM_ON_TASK_RUN )
			this->updateData();
	}

	//////////////////////////////////////////////////////////////////////////
	void			GeometryTask::update()
	{
		if( mMethod == GUM_ON_TASK_UPDATE )
			this->updateData();
	}

	//////////////////////////////////////////////////////////////////////////
	void			GeometryTask::onAccepted()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void			GeometryTask::onRemoved()
	{

	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	GEOM_UPDATE_METHOD	GeometryTask::getUpdateMethod() const
	{
		return mMethod;
	}

	//////////////////////////////////////////////////////////////////////////
	void				GeometryTask::setUpdateMethod(GEOM_UPDATE_METHOD method)
	{
		mMethod = method;
	}

	//////////////////////////////////////////////////////////////////////////
	void				GeometryTask::updateData()
	{
		mUpdater.updateNodes();
		mQueue.update();
	}
	
}//namespace Blade