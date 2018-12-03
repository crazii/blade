/********************************************************************
	created:	2011/11/02
	filename: 	ILogicService.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_ILogicService_h__
#define __Blade_ILogicService_h__
#include <interface/InterfaceSingleton.h>
#include <BladeFramework.h>

namespace Blade
{
	class ILogicScene;
	class ParaStateQueue;

	class BLADE_FRAMEWORK_API ILogicService : public InterfaceSingleton<ILogicService>
	{
	public:
		virtual ~ILogicService()		{}

		/**
		@describe 
		@param
		@return
		*/
		virtual ILogicScene*		createLogicScene(const TString& name) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual ILogicScene*		getLogicScene(const TString& name) const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual ParaStateQueue*		getLogicStateQueue() = 0;

	};//class ILogicService
	
	extern template class BLADE_FRAMEWORK_API Factory<ILogicService>;

}//namespace Blade



#endif // __Blade_ILogicService_h__