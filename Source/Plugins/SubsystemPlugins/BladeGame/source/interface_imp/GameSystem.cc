/********************************************************************
	created:	2010/04/28
	filename: 	GameSystem.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "GameSystem.h"

namespace Blade
{
	const TString GameSystem::GAME_SYSTEM_NAME = BTString("BladeGameSystem");

	//////////////////////////////////////////////////////////////////////////
	GameSystem::GameSystem()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	GameSystem::~GameSystem()
	{

	}

	/************************************************************************/
	/* ISubsystem Interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	const TString&	GameSystem::getName()
	{
		return GAME_SYSTEM_NAME;
	}

	//////////////////////////////////////////////////////////////////////////
	void		GameSystem::install()
	{
		RegisterSingleton(GameSystem, ILogicService);
	}

	//////////////////////////////////////////////////////////////////////////
	void		GameSystem::uninstall()
	{
		UnregisterSingleton(GameSystem, ILogicService);
	}

	//////////////////////////////////////////////////////////////////////////
	void		GameSystem::initialize()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	void		GameSystem::update()
	{
		for(SceneList::iterator i = mScenes.begin(); i != mScenes.end(); ++i)
		{
			GameScene* scene = i->second;
			scene->onMainLoopUpdate();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void		GameSystem::shutdown()
	{
		mScenes.clear();
	}

	//////////////////////////////////////////////////////////////////////////
	IScene*		GameSystem::createScene(const TString& name)
	{

		SceneHandle& scene = mScenes[name];
		if( scene != NULL )
		{
			BLADE_EXCEPT(EXC_REDUPLICATE,BTString("cannot create scene,a scene with the same name already exist:")+name );
		}
		scene.bind( BLADE_NEW GameScene() );

		return scene;
	}

	//////////////////////////////////////////////////////////////////////////
	void		GameSystem::destroyScene(const TString& name)
	{
		mScenes.erase(name);
	}

	//////////////////////////////////////////////////////////////////////////
	IScene*		GameSystem::getScene(const TString& name) const
	{
		SceneList::const_iterator i = mScenes.find(name);
		if( i == mScenes.end() )
			return NULL;
		else
			return i->second;
	}

	/************************************************************************/
	/* ILogicService interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	IGameScene* GameSystem::getLogicScene(const TString& name) const
	{
		return this->getGameScene(name);
	}

	//////////////////////////////////////////////////////////////////////////
	ParaStateQueue*	GameSystem::getLogicStateQueue()
	{
		return &mQueue;
	}

	/************************************************************************/
	/* IGameSystem interface                                                                     */
	/************************************************************************/
	IGameScene* GameSystem::getGameScene(const TString& name) const
	{
		SceneList::const_iterator i = mScenes.find(name);
		if( i == mScenes.end() )
			return NULL;
		else
			return i->second;
	}
	
}//namespace Blade