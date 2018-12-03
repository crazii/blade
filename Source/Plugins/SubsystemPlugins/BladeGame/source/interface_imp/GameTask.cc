/********************************************************************
	created:	2010/05/01
	filename: 	GameTask.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "GameTask.h"
#include <interface/IGameSystem.h>
#include <parallel/ParaStateQueue.h>

namespace Blade
{
	const ITask::Type GameTask::TYPE = BTString("Logic");

	//////////////////////////////////////////////////////////////////////////
	GameTask::GameTask(const TString& name,ITask::ETaskPriority priority/* = TP_NORMAL*/)
		:TaskBase(name,priority)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	GameTask::~GameTask()
	{

	}

	/************************************************************************/
	/* ITask class interfaces                                                                     */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	const ITask::Type&	GameTask::getType() const
	{
		return TYPE;
	}

	//////////////////////////////////////////////////////////////////////////
	void			GameTask::run()
	{
		for( GameLgocSet::iterator i = mLgoicSet.begin(); i != mLgoicSet.end(); ++i)
		{
			(*i)->onRun();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void			GameTask::update()
	{
		for( GameLgocSet::iterator i = mLgoicSet.begin(); i != mLgoicSet.end(); ++i)
		{
			(*i)->onUpdate();
		}

		ParaStateQueue& queue = *IGameSystem::getSingleton().getLogicStateQueue();
		queue.update();
	}

	//////////////////////////////////////////////////////////////////////////
	void			GameTask::onAccepted()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void			GameTask::onRemoved()
	{

	}



	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void					GameTask::addLogic(ILogic* logic)
	{
		if( logic != NULL )
		{
			if( !mLgoicSet.insert(logic).second )
			{
				BLADE_EXCEPT(EXC_REDUPLICATE,BTString("logic already added into task:")+logic->getName());
			}
		}
	}
	
}//namespace Blade