/********************************************************************
	created:	2011/11/02
	filename: 	ILogic.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_ILogic_h__
#define __Blade_ILogic_h__
#include <utility/String.h>
#include <Handle.h>
#include <Factory.h>
#include <BladeFramework.h>

namespace Blade
{
	class ILogicScene;
	class IStage;

	class BLADE_FRAMEWORK_API ILogic
	{
	public:
		virtual ~ILogic()		{}

		/**
		@describe
		@param
		@return
		*/
		virtual const TString&	getName() = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual void	onAddToScene(IStage* stage,ILogicScene* scene) = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual void	onRemove() = 0;

		/**
		@describe task running (asynchronous)
		@param
		@return
		*/
		virtual void	onRun() = 0;

		/**
		@describe task update (asynchronous)
		@param
		@return
		*/
		virtual void	onUpdate() = 0;

		/**
		@describe  main loop update (synchronous)
		@param 
		@return 
		*/
		virtual void	onMainLoopUpdate() = 0;

	};//class ILogic

	extern template class BLADE_FRAMEWORK_API Factory<ILogic>;

	typedef Handle<ILogic>	HLOGIC;
	typedef Factory<ILogic> LogicFactory;
	

}//namespace Blade



#endif // __Blade_ILogic_h__