/********************************************************************
	created:	2011/06/01
	filename: 	PropertyShaderVariables.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "PropertyShaderVariables.h"

namespace Blade
{
	static PropertyDiffuseUpdater	DiffuseUpdater;
	static PropertySpecularUpdater	SpecularUpdater;
	static PropertyEmissiveUpdater	EmissiveUpdater;
	static PropertyAlphaUpdater		BlendFactorUpdater;

	//////////////////////////////////////////////////////////////////////////
	PropertyShaderVariables::PropertyShaderVariables()
	{
		mVariableMap[BTString("OBJECT_DIFFUSE")] = DiffuseUpdater.getVariable();
		mVariableMap[BTString("OBJECT_SPECULAR")] = SpecularUpdater.getVariable();
		mVariableMap[BTString("OBJECT_EMISSIVE")] = EmissiveUpdater.getVariable();
		mVariableMap[BTString("BLEND_FACTOR")] = BlendFactorUpdater.getVariable();
	}

	//////////////////////////////////////////////////////////////////////////
	PropertyShaderVariables::~PropertyShaderVariables()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void					PropertyShaderVariables::onShaderSwitch(bool dynamicSwithing)
	{
		DiffuseUpdater.onShaderSwitch(dynamicSwithing);
		SpecularUpdater.onShaderSwitch(dynamicSwithing);
		EmissiveUpdater.onShaderSwitch(dynamicSwithing);
		BlendFactorUpdater.onShaderSwitch(dynamicSwithing);
	}

	//////////////////////////////////////////////////////////////////////////
	void					PropertyShaderVariables::markDirty()
	{
		for(VariableMap::iterator i = mVariableMap.begin(); i != mVariableMap.end(); ++i )
			i->second->markDirty();
	}

	//////////////////////////////////////////////////////////////////////////
	AutoShaderVariable*		PropertyShaderVariables::getPassVariable(const TString& semantic)
	{
		VariableMap::iterator i = mVariableMap.find(semantic);
		if( i == mVariableMap.end() )
		{
			//BLADE_EXCEPT(EXC_NEXIST,BTString("shader variable with the semantic not found:\"") + semantic + BTString("\".") );
			return NULL;
		}
		else
			return i->second;
	}

}//namespace Blade
