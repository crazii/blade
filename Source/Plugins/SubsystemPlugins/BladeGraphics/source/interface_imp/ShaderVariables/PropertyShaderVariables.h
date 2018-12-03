/********************************************************************
	created:	2011/06/01
	filename: 	PropertyShaderVariables.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_PassShaderVariables_h__
#define __Blade_PassShaderVariables_h__
#include "PropertyShaderVariableUpdater.h"
#include <utility/BladeContainer.h>
#include <interface/public/graphics/IShader.h>

namespace Blade
{

	class PropertyShaderVariables
	{
	public:
		PropertyShaderVariables();
		~PropertyShaderVariables();
		/**
		@describe 
		@param
		@return
		*/
		void					onShaderSwitch(bool dynamicSwithing);

		/*
		@describe
		@param
		@return
		*/
		void					markDirty();

		/*
		@describe
		@param
		@return
		*/
		AutoShaderVariable*		getPassVariable(const TString& semantic);

	protected:
		typedef StaticTStringMap<AutoShaderVariable*>	VariableMap;
		typedef StaticVector<PropertyVariableUpdater*>		PassVariableUpdaterList;

		VariableMap					mVariableMap;
	};//class PassShaderVariables

}//namespace Blade



#endif // __Blade_PassShaderVariables_h__