/********************************************************************
	created:	2010/04/28
	filename: 	GameSystem.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_GameSystem_h__
#define __Blade_GameSystem_h__
#include <StaticHandle.h>
#include <interface/IGameSystem.h>
#include <utility/BladeContainer.h>
#include <parallel/ParaStateQueue.h>
#include "GameScene.h"

namespace Blade
{
	class GameSystem : public IGameSystem , public Singleton<GameSystem>
	{
	public:
		static const TString GAME_SYSTEM_NAME;
	public:
		GameSystem();
		~GameSystem();
		
		/************************************************************************/
		/* ISubsystem Interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual const TString&	getName();


		/*
		@describe this would be called when added to Framework
		@param 
		@return 
		*/
		virtual void		install();

		/*
		@describe
		@param
		@return
		*/
		virtual void		uninstall();

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void		initialize();

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void		update();

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void		shutdown();

		/*
		@describe 
		@param 
		@return 
		*/
		virtual IScene*		createScene(const TString& name);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void		destroyScene(const TString& name);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual IScene*		getScene(const TString& name) const;

		/************************************************************************/
		/* ILogicService interface                                                                     */
		/************************************************************************/
		/**
		@describe 
		@param
		@return
		*/
		virtual ILogicScene*		createLogicScene(const TString& name)
		{
			return static_cast<ILogicScene*>(this->createScene(name));
		}

		/*
		@describe
		@param
		@return
		*/
		virtual IGameScene*		getLogicScene(const TString& name) const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual ParaStateQueue*	getLogicStateQueue();


		/************************************************************************/
		/* IGameSystem interface                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		virtual IGameScene* getGameScene(const TString& name) const;

	protected:
		typedef StaticHandle<GameScene>		SceneHandle;

		typedef TStringMap<SceneHandle>		SceneList;

		SceneList		mScenes;
		ParaStateQueue	mQueue;
	};//class GameSystem
	
}//namespace Blade


#endif //__Blade_GameSystem_h__