/********************************************************************
	created:	2015/02/11
	filename: 	GameLogic.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_GameLogic_h__
#define __Blade_GameLogic_h__
#include <GameLibrary.h>
#include <interface/public/logic/ILogic.h>
#include "GameFSM.h"

namespace Blade
{
	class GameLogic : public ILogic, public Allocatable
	{
	public:
		static const TString NAME;
	public:
		GameLogic();
		~GameLogic();

		/************************************************************************/
		/* ILogic interface                                                                     */
		/************************************************************************/
		/**
		@describe
		@param
		@return
		*/
		virtual const TString&	getName();

		/**
		@describe
		@param
		@return
		*/
		virtual void	onAddToScene(IStage* stage, ILogicScene* scene);

		/**
		@describe
		@param
		@return
		*/
		virtual void	onRemove();

		/**
		@describe task running (asynchronous)
		@param
		@return
		*/
		virtual void	onRun();

		/**
		@describe task update (asynchronous)
		@param
		@return
		*/
		virtual void	onUpdate();

		/**
		@describe  main loop update (synchronous)
		@param 
		@return 
		*/
		virtual void	onMainLoopUpdate();

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/

	protected:
		IStage*		mStage;
		ILogicScene*mScene;
		GameFSM*	mFSM;
	};//class GameLogic 
	
}//namespace Blade


#endif // __Blade_GameLogic_h__
