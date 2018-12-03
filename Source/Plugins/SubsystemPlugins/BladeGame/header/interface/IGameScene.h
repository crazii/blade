/********************************************************************
	created:	2010/05/08
	filename: 	IGameScene.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_IGameScene_h__
#define __Blade_IGameScene_h__
#include <BladeGame.h>
#include <interface/public/logic/ILogic.h>
#include <interface/public/logic/ILogicScene.h>

namespace Blade
{
	class BLADE_GAME_API IGameScene : public ILogicScene
	{
	public:
		virtual ~IGameScene()		{}

		/*
		@describe
		@param
		@return
		*/
		virtual void	addLogic(const HLOGIC& logic) = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual void	addLogic(const TString& GameLogicName) = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void	setLogicWorld(ILogicWorld* gameworld) = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual ILogicWorld*	getLogicWorld() const = 0;

	};//class IGameScene
	
}//namespace Blade


#endif //__Blade_IGameScene_h__