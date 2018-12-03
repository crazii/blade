/********************************************************************
	created:	2011/09/01
	filename: 	GlobalLightVariables.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "GlobalShaderVariables.h"
#include "CameraViewShaderVariableUpaters.h"
#include "GlobalLightVariables.h"

namespace Blade
{
	static ViewMatrixUpdater&									ViewUpdater = GlobalShaderVariables::getViewMatrixUpdater();

	//////////////////////////////////////////////////////////////////////////
	class ForwardGlobalViewLightVectorVariableUpdater : public GlobalVariableUpdater
	{
	public:
		ForwardGlobalViewLightVectorVariableUpdater(ForwardGlobalLightVectorVariableUpdater* lightUpdater, ViewMatrixUpdater* viewUpdater)
			:GlobalVariableUpdater(SCT_FLOAT4, BLADE_MAX_LIGHT_COUNT)
			,mLightVectorUpdater(lightUpdater)
			,mViewMatrixUpdater(viewUpdater)
		{}

		/** @brief  */
		virtual const void*	updateData() const
		{
			size_t MaxCount = std::min<size_t>( IShaderVariableSource::getSingleton().getLightManager()->getGlobalLightCount(), BLADE_MAX_LIGHT_COUNT);
			const Vector4* lightVectors = (const Vector4*)mLightVectorUpdater->getVariable()->getData();
			const Matrix44* viewMatrix = (const Matrix44*)mViewMatrixUpdater->getVariable()->getData();
			for(size_t i = 0; i < MaxCount; ++i)
				mVectors[i] = lightVectors[i] * (*viewMatrix);
			return mVectors;
		}
		ForwardGlobalLightVectorVariableUpdater* mLightVectorUpdater;
		ViewMatrixUpdater*			mViewMatrixUpdater;
		mutable Vector4	mVectors[BLADE_MAX_LIGHT_COUNT];
	};

	static GlobalAmbientVariableUpdater							GlobalAmbientUpdater;
	static ForwardGlobalLightCountVariableUpdater				LightCountUpdater;
	static ForwardGlobalLightVectorVariableUpdater				LightVectorUpdater;
	static ForwardGlobalLightDiffuseVariableUpdater				LightDiffuseUpdater;
	static ForwardGlobalLightSpecularVariableUpdater			LightSpecularUpdater;
	static ForwardGlobalViewLightVectorVariableUpdater			ViewLightUpdater(&LightVectorUpdater, &ViewUpdater);
	
	static DeferredDirectionalLightCountUpdater					DeferredDirectionalCountUpdater;
	static DeferredDirectionalLightDirectionUpdater				DeferredDirectionalDirectionUpdater(&DeferredDirectionalCountUpdater);
	static DeferredDirectionalLightDiffuseUpdater				DeferredDirectionalDiffuseUpdater(&DeferredDirectionalCountUpdater);
	static DeferredDirectionalLightSpecularUpdater				DeferredDirectionalSpecularUpdater(&DeferredDirectionalCountUpdater);

	//////////////////////////////////////////////////////////////////////////
	GlobalLightVariables::GlobalLightVariables()
	{
		mLightVariables.push_back( GlobalAmbientUpdater.getVariable() );
		mLightVariables.push_back( LightCountUpdater.getVariable() );
		mLightVariables.push_back( LightVectorUpdater.getVariable() );
		mLightVariables.push_back( ViewLightUpdater.getVariable() );
		mLightVariables.push_back( LightDiffuseUpdater.getVariable() );
		mLightVariables.push_back( LightSpecularUpdater.getVariable() );

		mLightVariables.push_back( DeferredDirectionalCountUpdater.getVariable() );
		mLightVariables.push_back( DeferredDirectionalDirectionUpdater.getVariable() );
		mLightVariables.push_back( DeferredDirectionalDiffuseUpdater.getVariable() );
		mLightVariables.push_back( DeferredDirectionalSpecularUpdater.getVariable() );

		mVariableMap[BTString("GLOBAL_AMBIENT")] = GlobalAmbientUpdater.getVariable();
		//forward shading
		mVariableMap[BTString("MAIN_LIGHT_COUNT")] = LightCountUpdater.getVariable();
		mVariableMap[BTString("MAIN_LIGHT_VECTOR")] = LightVectorUpdater.getVariable();
		mVariableMap[BTString("MAIN_LIGHT_VECTOR_VIEW_SPACE")] = ViewLightUpdater.getVariable();
		mVariableMap[BTString("MAIN_LIGHT_DIFFUSE")] = LightDiffuseUpdater.getVariable();
		mVariableMap[BTString("MAIN_LIGHT_SPECULAR")] = LightSpecularUpdater.getVariable();
		//deferred shading
		mVariableMap[BTString("DEFERRED_LIGHT_COUNT")] = DeferredDirectionalCountUpdater.getVariable();
		mVariableMap[BTString("DEFERRED_LIGHT_DIRECTIONS")] = DeferredDirectionalDirectionUpdater.getVariable();
		mVariableMap[BTString("DEFERRED_LIGHT_DIFFUSES")] = DeferredDirectionalDiffuseUpdater.getVariable();
		mVariableMap[BTString("DEFERRED_LIGHT_SPECULARS")] = DeferredDirectionalSpecularUpdater.getVariable();
	}

	//////////////////////////////////////////////////////////////////////////
	GlobalLightVariables::~GlobalLightVariables()
	{

	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	void					GlobalLightVariables::markLightVariableDirty()
	{
		for(size_t i = 0; i < mLightVariables.size(); ++i )
		{
			mLightVariables[i]->markDirty();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	AutoShaderVariable*		GlobalLightVariables::getBuiltInVariable(const TString& semantic)
	{
		VariableMap::iterator i = mVariableMap.find(semantic);
		if( i != mVariableMap.end() )
			return i->second;
		else
			return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	ForwardGlobalLightVectorVariableUpdater&	GlobalLightVariables::getLightVectorUpdater()
	{
		return LightVectorUpdater;
	}

	//////////////////////////////////////////////////////////////////////////
	ForwardGlobalLightCountVariableUpdater&	GlobalLightVariables::getLightCountUpdater()
	{
		return LightCountUpdater;
	}

	//////////////////////////////////////////////////////////////////////////
	GlobalVariableUpdater&		GlobalLightVariables::getViewLightVectorUpdater()
	{
		return ViewLightUpdater;
	}
	

}//namespace Blade
