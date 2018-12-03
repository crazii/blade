/********************************************************************
	created:	2010/09/06
	filename: 	InstanceShaderVariables.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_InstanceShaderVariables_h__
#define __Blade_InstanceShaderVariables_h__
#include <utility/BladeContainer.h>
#include "ShaderVariableUpdaters.h"
#include "InstanceShaderVariableUpdaters.h"

namespace Blade
{
	struct RenderOperation;

	class InstanceShaderVariables
	{
	public:
		InstanceShaderVariables();
		~InstanceShaderVariables();

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
		void					markDirty(const RenderOperation* rop);

		/*
		@describe
		@param
		@return
		*/
		AutoShaderVariable*		getBuiltInVariable(const TString& semantic);

	protected:
		typedef StaticTStringMap<AutoShaderVariable*>			VariableMap;
		typedef StaticVector<InstanceVariableUpdater*>			UpdaterList;
		typedef StaticVector<InstanceTextureProjectionUpdater>	TexProjectUpdaterList;
		
		VariableMap						mVariableMap;
		UpdaterList						mUpdaters;
		TexProjectUpdaterList			mTexProjectUpdaters;
		ForwardLocalLightUpdater*		mForwardLocalLightUpdater;
	};//class InstanceShaderVariables
	
}//namespace Blade


#endif //__Blade_InstanceShaderVariables_h__