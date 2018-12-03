/********************************************************************
	created:	2011/09/01
	filename: 	GlobalLightVariables.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_GlobalLightVariables_h__
#define __Blade_GlobalLightVariables_h__

#include <utility/BladeContainer.h>
#include "GlobalLightShaderVariableUpdaters.h"

namespace Blade
{

	class GlobalLightVariables //: public IGlobalLightVariables
	{
	public:
		GlobalLightVariables();
		~GlobalLightVariables();

		/************************************************************************/
		/*                                                                      */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		void					markLightVariableDirty();

		/*
		@describe
		@param
		@return
		*/
		AutoShaderVariable*		getBuiltInVariable(const TString& semantic);

		/*
		@describe 
		@param 
		@return 
		*/
		static ForwardGlobalLightVectorVariableUpdater&	getLightVectorUpdater();

		/*
		@describe 
		@param 
		@return 
		*/
		static ForwardGlobalLightCountVariableUpdater&	getLightCountUpdater();

		/**
		@describe 
		@param
		@return
		*/
		static GlobalVariableUpdater&	getViewLightVectorUpdater();

	protected:
		typedef StaticTStringMap<AutoShaderVariable*>	VariableMap;
		typedef StaticVector<LightVariableUpdater*>		LightVariableupdaterList;
		typedef StaticVector<AutoShaderVariable*>		LightVariables;

		VariableMap					mVariableMap;
		LightVariables				mLightVariables;
	};
	

}//namespace Blade



#endif // __Blade_GlobalLightVariables_h__