/********************************************************************
	created:	2011/11/02
	filename: 	ILogicScene.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_ILogicScene_h__
#define __Blade_ILogicScene_h__
#include <interface/public/IScene.h>
#include <interface/public/logic/ILogic.h>

namespace Blade
{
	class ILogicWorld;

	class BLADE_FRAMEWORK_API ILogicScene : public IScene
	{
	public:
		virtual ~ILogicScene()		{}

		/**
		@describe
		@param
		@return
		*/
		virtual void	addLogic(const HLOGIC& logic) = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual void	addLogic(const TString& LogicName) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void	setLogicWorld(ILogicWorld* gameworld) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual ILogicWorld*	getLogicWorld() const = 0;

	};//class ILogicScene
	

}//namespace Blade



#endif // __Blade_ILogicScene_h__