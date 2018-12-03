/********************************************************************
	created:	2009/03/14
	filename: 	IEnvironmentManager.h
	author:		Crazii
	
	purpose:	manager for the global evironment variables,and the excution result
				this differs from the IConfigManager because the varialbes here may be changed,
				while the conifgmanager holds the variables that would not be changed.
*********************************************************************/
#ifndef __Blade_IEnvironmentManager_h__
#define __Blade_IEnvironmentManager_h__
#include <BladeFramework.h>
#include <interface/InterfaceSingleton.h>
#include <ConstDef.h>

namespace Blade
{
	class Variant;

	class IEnvironmentManager : public InterfaceSingleton<IEnvironmentManager>
	{
	public:
		virtual ~IEnvironmentManager()	{}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool addVariable(const TString& name,const Variant& var) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const Variant& getVariable(const TString& name) const = 0;
		
		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool setVariable(const TString& name,const Variant& var) = 0;

	};//class IEnvironmentManager


	extern template class BLADE_FRAMEWORK_API Factory<IEnvironmentManager>;
	
}//namespace Blade

#endif // __Blade_IEnvironmentManager_h__