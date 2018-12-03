/********************************************************************
	created:	2010/04/28
	filename: 	GameScene.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "GameScene.h"
#include <interface/ITaskManager.h>
#include <interface/IStage.h>
#include <interface/public/logic/ILogicWorld.h>

#include <GameElement.h>
#include "GameTask.h"


namespace Blade
{
	const TString GameScene::GAME_SCENE_TYPE = GameConsts::GAME_SCENE_TYPE;
	
	//////////////////////////////////////////////////////////////////////////
	GameScene::GameScene()
		:mStage(NULL)
		,mGameworld(NULL)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	GameScene::~GameScene()
	{
	}

	/************************************************************************/
	/* IScene Interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void			GameScene::postProcess(const ProgressNotifier& /*callback*/)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	const TString&	GameScene::getSceneType() const
	{
		return GAME_SCENE_TYPE;
	}

	//////////////////////////////////////////////////////////////////////////
	void			GameScene::getAvailableElementTypes(ElementTypeList& elemList) const
	{
		size_t count = GameElementFactory::getSingleton().getNumRegisteredClasses();

		for( size_t i = 0; i < count; ++i)
		{
			const TString& name = GameElementFactory::getSingleton().getRegisteredClass(i);
			elemList.push_back( name );
		}

	}

	//////////////////////////////////////////////////////////////////////////
	IElement*		GameScene::createElement(const TString& type)
	{
		if( !GameElementFactory::getSingleton().isClassRegistered(type) )
			return NULL;

		GameElement* elem = BLADE_FACTORY_CREATE(GameElement,type );
		elem->initialize(this);
		return elem;;
	}

	//////////////////////////////////////////////////////////////////////////
	void			GameScene::onAttachToStage(IStage* pstage)
	{
		GameTask* task = BLADE_NEW GameTask( pstage->getName()+TEXT("'s Game Logic Task") );

		mTask.bind( task );
		ITaskManager::getSingleton().addTask(task);
		mStage = pstage;

		for( LogicList::iterator i = mLogicList.begin(); i != mLogicList.end(); ++i )
		{
			task->addLogic( (*i) );
			(*i)->onAddToScene(pstage,this);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void			GameScene::onDetachFromStage(IStage* pstage)
	{
		assert( mStage == pstage );
		BLADE_UNREFERENCED(pstage);

		for( LogicList::iterator i = mLogicList.begin(); i != mLogicList.end() ; ++i)
		{
			(*i)->onRemove();
		}

		mStage = NULL;
		ITaskManager::getSingleton().removeTask(mTask);
	}


	/************************************************************************/
	/* IGameScene interface                                                                     */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	void			GameScene::addLogic(const HLOGIC& logic)
	{
		assert( logic != NULL );

		if( mLogicList.insert(logic).second )
		{
			if( mStage != NULL )
				logic->onAddToScene(mStage,this);

			if( mTask != NULL)
			{
				GameTask* gt = static_cast<GameTask*>(mTask);
				gt->addLogic(logic );
			}
		}
		else
			BLADE_EXCEPT(EXC_REDUPLICATE,BTString("the game logic already added.")+logic->getName() );
	}

	//////////////////////////////////////////////////////////////////////////
	void	GameScene::addLogic(const TString& GameLogicName)
	{
		this->addLogic(HLOGIC(BLADE_FACTORY_CREATE(ILogic, GameLogicName)));
	}

	//////////////////////////////////////////////////////////////////////////
	void	GameScene::onMainLoopUpdate()
	{
		for(LogicList::iterator i = mLogicList.begin(); i != mLogicList.end(); ++i)
		{
			ILogic* logic = *i;
			logic->onMainLoopUpdate();
		}
	}


}//namespace Blade