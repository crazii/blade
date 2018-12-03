/********************************************************************
	created:	2010/09/05
	filename: 	GlobalShaderVariables.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_GlobalShaderVariables_h__
#define __Blade_GlobalShaderVariables_h__
#include <utility/BladeContainer.h>
#include <interface/public/graphics/IRenderView.h>

#include <AutoShaderVariable.h>
#include <interface/ICamera.h>
#include "ShaderVariableUpdaters.h"
#include <ShaderVariableMap.h>

namespace Blade
{
	class RenderOutput;
	class ViewMatrixUpdater;
	class ProjectionMatrixUpdater;
	class ViewProjectionMatrixUpdater;
	class CameraPositionUpdater;
	class BillboardViewProjectionMatrixUpdater;
	class BillboardViewMatrixUpdater;

	class GlobalShaderVariables
	{
	public:
		GlobalShaderVariables();
		~GlobalShaderVariables();

		/**
		@describe 
		@param
		@return
		*/
		void					markFrameVariableDirty();

		/*
		@describe
		@param
		@return
		*/
		void					markCameraVariableDirty();

		/*
		@describe
		@param
		@return
		*/
		void					markViewVariableDirty();

		/**
		@describe 
		@param
		@return
		*/
		void					markRenderOutputVariableDirty();

		/**
		@describe 
		@param
		@return
		*/
		void					markPostFXOutputVariableDirty();

		/**
		@describe 
		@param
		@return
		*/
		void					markShadowVariableDirty(bool depthMatricesOnly);

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
		const HAUTOSHADERVARIABLE&	createCustomVariable(const TString& name,SHADER_CONSTANT_TYPE type,size_t count);

		/*
		@describe
		@param
		@return
		*/
		const HAUTOSHADERVARIABLE&	getGlobalCustomVariable(const TString& name, SHADER_CONSTANT_TYPE type, size_t count);

		/**
		@describe
		@param
		@return
		*/
		bool						setGlobalCustomVariable(const HAUTOSHADERVARIABLE& variable, const void* source, size_t bytes);

		/*
		@describe
		@param
		@return
		*/
		static ViewMatrixUpdater&		getViewMatrixUpdater();

		/*
		@describe 
		@param 
		@return 
		*/
		static ProjectionMatrixUpdater&	getProjectionMatrixUpdater();

		/*
		@describe
		@param
		@return
		*/
		static ViewProjectionMatrixUpdater&	getViewProjectionUpdater();

		/*
		@describe 
		@param 
		@return 
		*/
		static CameraPositionUpdater&	getCameraPositionUpdater();

		/*
		@describe 
		@param 
		@return 
		*/
		static BillboardViewMatrixUpdater& getBillboardViewUpdater();

	protected:
		typedef StaticTStringMap<AutoShaderVariable*>	VariableMap;
		typedef StaticVector<AutoShaderVariable*>		CategoryVariables;
		typedef StaticVector<CameraVariableUpdater*>	CameraVariableUpdaterList;
		typedef StaticVector<ViewVariableUpdater*>		ViewVariableUpdaterList;
		
		VariableMap					mVariableMap;
		CategoryVariables			mPerFrameVariables;
		CategoryVariables			mCameraVariables;
		CategoryVariables			mViewVariables;
		CategoryVariables			mOutputVariables;
		CategoryVariables			mPostFXVariables;
		CategoryVariables			mShadowVariables;

		ShaderVariableMapBase		mCustomVariables;
	};//class GlobalShaderVariables
	
}//namespace Blade


#endif //__Blade_GlobalShaderVariables_h__