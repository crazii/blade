/********************************************************************
	created:	2009/03/14
	filename: 	EnvironmentManager.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <ExceptionEx.h>
#include "EnvironmentManager.h"

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	EnvironmentManager::EnvironmentManager()
	{
		Variant TermiVar = false;
		this->addVariable(ConstDef::EnvBool::WORK_TERMINATE,TermiVar );
		Variant devMode = false;
		this->addVariable(ConstDef::EnvBool::DEVELOPER_MODE, devMode);

		Variant ModeVar = BTString("default");
		this->addVariable(ConstDef::EnvString::WORKING_MODE,ModeVar);
		this->addVariable(ConstDef::EnvString::STARTUP_IMAGE, TString::EMPTY);
	}

	//////////////////////////////////////////////////////////////////////////
	EnvironmentManager::~EnvironmentManager()
	{

	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool EnvironmentManager::addVariable(const TString& name,const Variant& var)
	{
		if( var.getType() == VI_UNDEFINED )
		{
			assert(false);
			return false;
		}

		Variant& empty = mVariableMap[name];

		if( empty.getType() != VI_UNDEFINED )	//already have the variable
		{
			assert(false);
			return false;
		}
		empty.reset(var);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	const Variant& EnvironmentManager::getVariable(const TString& name) const
	{
		VarMap::const_iterator i = mVariableMap.find(name);
		if( i != mVariableMap.end() )
			return i->second;
		else
			BLADE_EXCEPT(EXC_NEXIST,BTString("environment variable with the name \"") + name + BTString("\".") );
	}
	//////////////////////////////////////////////////////////////////////////
	bool	EnvironmentManager::setVariable(const TString& name,const Variant& var)
	{
		if( var.getType() == VI_UNDEFINED )
		{
			assert(false);
			return false;
		}

		VarMap::iterator i = mVariableMap.find(name);
		if( i== mVariableMap.end() )
		{
			assert(false);
			return false;
		}

		Variant& target = i->second;

		assert( target.getType() != VI_UNDEFINED );

		target = var;
		return true;
	}
	
}//namespace Blade