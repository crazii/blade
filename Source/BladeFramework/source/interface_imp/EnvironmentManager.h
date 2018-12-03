/********************************************************************
	created:	2009/03/14
	filename: 	EnvironmentManager.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_EnvironmentManager_h__
#define __Blade_EnvironmentManager_h__
#include <interface/IEnvironmentManager.h>
#include <Singleton.h>
#include <utility/ConfigFile.h>
#include <utility/BladeContainer.h>

namespace Blade
{

	class EnvironmentManager : public IEnvironmentManager ,public Singleton<EnvironmentManager>
	{
	public:
		//ctor & dector
		EnvironmentManager();
		virtual ~EnvironmentManager();
		
		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool addVariable(const TString& name,const Variant& var);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const Variant& getVariable(const TString& name) const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool setVariable(const TString& name,const Variant& var);

	protected:
		typedef StaticTStringMap<Variant>	VarMap;
		VarMap		mVariableMap;
	};//class EnvironmentManager
	
}//namespace Blade


#endif // __Blade_EnvironmentManager_h__