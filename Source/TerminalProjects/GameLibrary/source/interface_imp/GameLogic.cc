/********************************************************************
	created:	2015/02/11
	filename: 	GameLogic.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "GameLogic.h"
#include <interface/IGameScene.h>
#include <interface/IStage.h>
#include <interface/public/graphics/IGraphicsService.h>

namespace Blade
{
	const TString GameLogic::NAME = BTString("GameLogic");

	//////////////////////////////////////////////////////////////////////////
	GameLogic::GameLogic()
		:mStage(NULL)
		,mScene(NULL)
	{
		mFSM = BLADE_NEW GameFSM();
	}

	//////////////////////////////////////////////////////////////////////////
	GameLogic::~GameLogic()
	{
		assert( mStage == NULL );
		assert( mScene == NULL );
		BLADE_DELETE mFSM;
	}

	/************************************************************************/
	/* ILogic interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	const TString&	GameLogic::getName()
	{
		return GameLogic::NAME;
	}

	//////////////////////////////////////////////////////////////////////////
	void	GameLogic::onAddToScene(IStage* stage, ILogicScene* scene)
	{
		assert( stage != NULL && scene != NULL );
		//must be game scene?
		assert( scene->getSceneType() == GameConsts::GAME_SCENE_TYPE);
		mStage = stage;
		mScene = scene;

		//init FSM
		mFSM->initialize(IGameState::INIT_STATE);
		stage->setTitle( mFSM->getCurrentState()->getName() );
	}

	//////////////////////////////////////////////////////////////////////////
	void	GameLogic::onRemove()
	{
		mStage = NULL;
		mScene = NULL;
		mFSM->shutdown();
	}

	//////////////////////////////////////////////////////////////////////////
	void	GameLogic::onRun()
	{
		mFSM->run();
	}

	//////////////////////////////////////////////////////////////////////////
	void	GameLogic::onUpdate()
	{
		mFSM->update();
	}

	//////////////////////////////////////////////////////////////////////////
	void	GameLogic::onMainLoopUpdate()
	{
		IGameState* state = mFSM->getCurrentState();
		mFSM->syncUpdate();
		if( mStage != NULL && state != mFSM->getCurrentState() )
			mStage->setTitle( mFSM->getCurrentState()->getName() );
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	
}//namespace Blade