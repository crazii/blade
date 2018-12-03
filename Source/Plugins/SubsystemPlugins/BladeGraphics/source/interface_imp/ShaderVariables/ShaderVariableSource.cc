/********************************************************************
	created:	2010/09/05
	filename: 	ShaderVariableSource.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <interface/IShaderVariableSource.h>
#include <interface/ICamera.h>
#include <Material.h>
#include <interface/public/IRenderable.h>
#include "ShaderVariableSource.h"
#include "ShaderVariableUpdaters.h"
#include "GlobalShaderVariableUpdaters.h"

namespace Blade
{

	//////////////////////////////////////////////////////////////////////////
	ShaderVariableSource::ShaderVariableSource()
	{
		mCamera = NULL;
		mView = NULL;
		mROP = NULL;
		mPropertySet = NULL;
		mLightManager = NULL;

		mLastPassProperty = NULL;
		mLastCustomPassProperty = NULL;
		mLastInstanceProperty = NULL;

		mCurrentOutput = NULL;
		mCurrentIFXOutput = NULL;
		mShadowManager = NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	ShaderVariableSource::~ShaderVariableSource()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void					ShaderVariableSource::onShaderSwitch(bool dynamicSwithing)
	{
		mInstanceShaderVariables.onShaderSwitch(dynamicSwithing);
		mPropertyhaderVariables.onShaderSwitch(dynamicSwithing);
	}

	//////////////////////////////////////////////////////////////////////////
	AutoShaderVariable*		ShaderVariableSource::getBuiltInVariable(const TString& semantic)
	{
		AutoShaderVariable* asv;
		if( (asv=mGlobalShaderVariables.getBuiltInVariable(semantic)) == NULL )
		{
			asv = mInstanceShaderVariables.getBuiltInVariable(semantic);
			if(asv == NULL )
				asv = mPropertyhaderVariables.getPassVariable(semantic);
			if( asv == NULL )
				asv = mGlobalLightVariables.getBuiltInVariable(semantic);
		}

		if(asv == NULL )
			BLADE_EXCEPT(EXC_NEXIST,BTString("shader variable with the semantic not found:\"") + semantic + BTString("\".") );
		return asv;
	}

	//////////////////////////////////////////////////////////////////////////
	const HAUTOSHADERVARIABLE&	ShaderVariableSource::createGlobalCustomVariable(const TString& name,SHADER_CONSTANT_TYPE type,size_t count)
	{
		return mGlobalShaderVariables.createCustomVariable(name,type,count);
	}

	//////////////////////////////////////////////////////////////////////////
	const HAUTOSHADERVARIABLE&	ShaderVariableSource::getGlobalCustomVariable(const TString& name, SHADER_CONSTANT_TYPE type, size_t count)
	{
		return mGlobalShaderVariables.getGlobalCustomVariable(name, type, count);
	}

	//////////////////////////////////////////////////////////////////////////
	bool					ShaderVariableSource::setGlobalCustomVariable(const HAUTOSHADERVARIABLE& variable, const void* source, size_t bytes)
	{
		return mGlobalShaderVariables.setGlobalCustomVariable(variable, source, bytes);
	}

	//////////////////////////////////////////////////////////////////////////
	void					ShaderVariableSource::setCamera(const ICamera* camera)
	{
		if( mCamera != camera )
		{
			mCamera = camera;
			if( mCamera != NULL )
				mGlobalShaderVariables.markCameraVariableDirty();
			mGlobalShaderVariables.markShadowVariableDirty(true);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void					ShaderVariableSource::setView(const IRenderView* view)
	{
		if( mView != view )
		{
			mView = view;
			if( mView != NULL )
				mGlobalShaderVariables.markViewVariableDirty();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void					ShaderVariableSource::setRenderOperation(const RenderOperation* rop)
	{
		if( mROP != rop )
		{
			mROP = rop;
			mInstanceShaderVariables.markDirty(rop);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool					ShaderVariableSource::setRenderProperties(const RenderPropertySet* passProperty, 
		const RenderPropertySet* customPassProperty, const RenderPropertySet* instanceProperty)
	{
		if( mLastPassProperty != passProperty || mLastCustomPassProperty != customPassProperty || mLastInstanceProperty != instanceProperty)
		{
			mLastPassProperty = passProperty;
			mLastCustomPassProperty = customPassProperty;
			mLastInstanceProperty = instanceProperty;

			int count = 0;
			const RenderPropertySet* uniqueProperty = NULL;
			if( passProperty != NULL )
			{
				++count;
				uniqueProperty = passProperty;
			}
			if( customPassProperty != NULL )
			{
				++count;
				uniqueProperty = customPassProperty;
			}
			if( instanceProperty != NULL )
			{
				++count;
				uniqueProperty = instanceProperty;
			}

			if(count > 1)
			{
				//merging order matters
				if( passProperty != NULL )
					mMergedProperty = *passProperty;
				else
					mMergedProperty = *customPassProperty;

				if( passProperty != NULL && customPassProperty != NULL )
					mMergedProperty.mergeProperties(*customPassProperty);

				if( instanceProperty != NULL )
					mMergedProperty.mergeProperties(*instanceProperty);

				mPropertySet = &mMergedProperty;
			}
			else if( uniqueProperty != NULL && mPropertySet != uniqueProperty )
				mPropertySet = uniqueProperty;

			mPropertyhaderVariables.markDirty();
			return true;
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	void					ShaderVariableSource::setLightManager(ILightManager* lm)
	{
		if( mLightManager != lm )
		{
			mLightManager = lm;
			if( mLightManager != NULL )
				mGlobalLightVariables.markLightVariableDirty();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void					ShaderVariableSource::setRenderOutput(const RenderOutput* output, bool forceApply/* = false*/)
	{
		if (mCurrentOutput != output || forceApply)
		{
			mCurrentOutput = output;
			mGlobalShaderVariables.markRenderOutputVariableDirty();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void					ShaderVariableSource::setImageEffectOutput(const ImageEffectOutput* output, bool forceApply/* = false*/)
	{
		if (mCurrentIFXOutput != output || forceApply)
		{
			mCurrentIFXOutput = output;
			mGlobalShaderVariables.markPostFXOutputVariableDirty();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void					ShaderVariableSource::setShadowManager(const ShadowManager* shadowManager)
	{
		if (mShadowManager != shadowManager)
		{
			mShadowManager = shadowManager;
			mGlobalShaderVariables.markShadowVariableDirty(false);
		}
	}
	
}//namespace Blade