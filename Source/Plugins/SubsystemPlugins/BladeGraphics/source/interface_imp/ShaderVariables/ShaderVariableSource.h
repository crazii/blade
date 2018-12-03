/********************************************************************
	created:	2010/09/05
	filename: 	ShaderVariableSource.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_ShaderVariableSource_h__
#define __Blade_ShaderVariableSource_h__
#include <interface/IShaderVariableSource.h>
#include "GlobalShaderVariables.h"
#include "InstanceShaderVariables.h"
#include "PropertyShaderVariables.h"
#include "GlobalLightVariables.h"

namespace Blade
{
	class ImageEffectOutput;
	class ShadowManager;

	class ShaderVariableSource : public IShaderVariableSource, public Singleton<ShaderVariableSource>
	{
	public:
		using Singleton<ShaderVariableSource>::getSingleton;
		using Singleton<ShaderVariableSource>::getSingletonPtr;
	public:
		ShaderVariableSource();
		~ShaderVariableSource();

		/**
		@describe 
		@param
		@return
		*/
		virtual void					onShaderSwitch(bool dynamicSwithing);

		/*
		@describe get engine built-in semantics, maybe global or per-instance
		@param
		@return
		*/
		virtual AutoShaderVariable*		getBuiltInVariable(const TString& semantic);

		/*
		@describe create/get global custom shader variables, this type of variable is shared among all shaders
		this is usually auto called automatically by framework, but you can create it manually
		you can set global custom variable by calling IShaderVariableSource::setGlobalCustomVariable
		@param 
		@return 
		@remarks create variable if it not exist, otherwise get the existing variable
		*/
		virtual const HAUTOSHADERVARIABLE&	createGlobalCustomVariable(const TString& name,SHADER_CONSTANT_TYPE type,size_t count);

		/*
		@describe
		@param
		@return
		*/
		virtual const HAUTOSHADERVARIABLE&	getGlobalCustomVariable(const TString& name, SHADER_CONSTANT_TYPE type, size_t count);

		/**
		@describe
		@param
		@return
		*/
		virtual bool					setGlobalCustomVariable(const HAUTOSHADERVARIABLE& variable, const void* source, size_t bytes);

		/*
		@describe
		@param
		@return
		*/
		virtual void					setCamera(const ICamera* camera);

		/*
		@describe
		@param
		@return
		*/
		virtual void					setView(const IRenderView* view);

		/*
		@describe
		@param
		@return
		*/
		virtual void					setRenderOperation(const RenderOperation* rop);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool					setRenderProperties(const RenderPropertySet* passProperty, 
			const RenderPropertySet* customPassProperty, const RenderPropertySet* instanceProperty);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void					setLightManager(ILightManager* lm);

		/**
		@describe 
		@param
		@return
		*/
		virtual void					beginFrame()	{mGlobalShaderVariables.markFrameVariableDirty();}

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		void	shutdown()
		{
			for (int i = RPT_BEGIN; i < RPT_COUNT; ++i)
				mMergedProperty.removeProperty(RENDER_PROPERTY(i));
		}

		/**
		@describe
		@param
		@return
		*/
		void					setRenderOutput(const RenderOutput* output, bool forceApply = false);
		inline const RenderOutput*		getRenderOutput() const
		{
			return mCurrentOutput;
		}

		/**
		@describe
		@param
		@return
		*/
		void					setImageEffectOutput(const ImageEffectOutput* output, bool forceApply = false);
		inline const ImageEffectOutput*	getImageEffectOutput() const
		{
			return mCurrentIFXOutput;
		}

		/**
		@describe 
		@param
		@return
		*/
		void					setShadowManager(const ShadowManager* shadowManager);
		inline const ShadowManager* getShadowManager() const
		{
			return mShadowManager;
		}

	protected:
		GlobalShaderVariables	mGlobalShaderVariables;
		InstanceShaderVariables	mInstanceShaderVariables;
		PropertyShaderVariables	mPropertyhaderVariables;
		GlobalLightVariables	mGlobalLightVariables;

		RenderPropertySet		mMergedProperty;
		const RenderPropertySet*mLastPassProperty;
		const RenderPropertySet*mLastCustomPassProperty;
		const RenderPropertySet*mLastInstanceProperty;

		const RenderOutput*		mCurrentOutput;
		const ImageEffectOutput*mCurrentIFXOutput;
		const ShadowManager* mShadowManager;
	};


}//namespace Blade


#endif //__Blade_ShaderVariableSource_h__
