/********************************************************************
	created:	2010/05/08
	filename: 	GameElement.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <GameElement.h>
#include <interface/IGameSystem.h>
#include "interface_imp/GameScene.h"

namespace Blade
{
	const TString GameElement::GAME_SYSTEM_TYPE = BTString("Game");

	//////////////////////////////////////////////////////////////////////////
	GameElement::GameElement(const TString& type)
		:ElementBase(GAME_SYSTEM_TYPE, type,PP_HIGH)
	{
		mParallelStates.setQueue( IGameSystem::getSingleton().getLogicStateQueue() );
	}

	//////////////////////////////////////////////////////////////////////////
	GameElement::~GameElement()
	{

	}

	/************************************************************************/
	/* IElement interface                                                                     */
	/************************************************************************/

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void				GameElement::initialize(IScene* scene)
	{
		assert(scene != NULL && scene->getSceneType() == GameScene::GAME_SCENE_TYPE);
		this->setScene(scene);
	}

}//namespace Blade