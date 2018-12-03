/********************************************************************
	created:	2012/03/22
	filename: 	GraphicsTaskController.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "GraphicsTaskController.h"
#include "GraphicsSystem.h"
#include <interface/public/window/IWindow.h>
#include <interface/public/graphics/IGraphicsService.h>
#include <interface/IRenderScene.h>
#include <interface/IStage.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	GraphicsTaskController::GraphicsTaskController(IRenderScene* pScene)
		:mScene(pScene)
		,mSkippAll(false)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	GraphicsTaskController::~GraphicsTaskController()
	{

	}

	/************************************************************************/
	/* IGraphicsTaskController interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool	GraphicsTaskController::isTaskRunNeeded(GraphicsTask* ptask,uint32& sleeptime) const
	{
		BLADE_UNREFERENCED(ptask);
		if( mSkippAll 
			|| ptask->getRenderScheme() == NULL 
			|| mScene->getStage() == NULL 
			|| mScene->getStage()->getWindow() == NULL
			|| !mScene->getStage()->getWindow()->isVisible() )
		{
			//sleeptime = 30 / (uint32)IGraphicsService::getSingleton().getGraphicsSceneCount();
			if( IGraphicsService::getSingleton().getGraphicsSceneCount() <= 1 )
				sleeptime = 30;
			else
				sleeptime = 0;
			return false;
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void	GraphicsTaskController::onTaskAccepted(GraphicsTask* ptask)
	{
		BLADE_UNREFERENCED(ptask);
	}

	//////////////////////////////////////////////////////////////////////////
	void	GraphicsTaskController::onTaskRemoved(GraphicsTask* ptask)
	{
		BLADE_UNREFERENCED(ptask);
	}

	//////////////////////////////////////////////////////////////////////////
	void	GraphicsTaskController::onTaskDestroy(GraphicsTask* ptask)
	{
		mScene->unuseRenderTarget( ptask->getRenderTarget() );
	}

	//////////////////////////////////////////////////////////////////////////
	void	GraphicsTaskController::updateThreadID(Thread::ID id)
	{
		GraphicsSystem::getSingleton().setGraphicsTaskID(id);
	}

}//namespace Blade