/********************************************************************
	created:	2010/04/28
	filename: 	GameScene.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_GameScene_h__
#define __Blade_GameScene_h__
#include <interface/IGameScene.h>
#include <Handle.h>
#include <BladeGame.h>
#include <utility/BladeContainer.h>
#include <interface/public/ITask.h>

namespace Blade
{

	class GameScene : public IGameScene, public Allocatable
	{
	public:
		static const TString GAME_SCENE_TYPE;
	public:
		GameScene();
		~GameScene();

		/************************************************************************/
		/* ISerializable interface                                                                     */
		/************************************************************************/
		/** @brief called after loading data */
		virtual void			postProcess(const ProgressNotifier& notifier);

		/************************************************************************/
		/* IScene Interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual const TString&	getSceneType() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			getAvailableElementTypes(ElementTypeList& elemList) const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual IElement*		createElement(const TString& type);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			onAttachToStage(IStage* pstage);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			onDetachFromStage(IStage* pstage);

		/************************************************************************/
		/* IGameScene interface                                                                     */
		/************************************************************************/

		/*
		@describe
		@param
		@return
		*/
		virtual void	addLogic(const HLOGIC& logic);

		/*
		@describe
		@param
		@return
		*/
		virtual void	addLogic(const TString& GameLogicName);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void	setLogicWorld(ILogicWorld* gameworld)		{mGameworld = gameworld;}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual ILogicWorld*	getLogicWorld() const					{return mGameworld;}

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/

		/*
		@describe update on main loop : this is in main thread (synchronizing state), \n
		at this time, there is no task running, except background tasks
		@param 
		@return 
		*/
		void			onMainLoopUpdate();

	protected:
		typedef	Set<HLOGIC>	LogicList;

		LogicList				mLogicList;
		HTASK					mTask;
		IStage*					mStage;
		ILogicWorld*			mGameworld;
	};//class GameScene

	//template class BLADE_GAME_API Handle<GameScene>;
	//typedef Handle<GameScene>	HGAMESCENE;
	
}//namespace Blade


#endif //__Blade_GameScene_h__