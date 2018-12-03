/********************************************************************
	created:	2010/09/05
	filename: 	GlobalShaderVariables.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "GlobalShaderVariables.h"
#include "GlobalShaderVariableUpdaters.h"
#include "GlobalLightVariables.h"
#include "CameraViewShaderVariableUpaters.h"
#include "OutputShaderVariableUpdaters.h"
#include "../RenderScheme/RenderSchemeDesc.h"

namespace Blade
{
	static CameraPositionUpdater								EyePosUpdater;
	static ViewMatrixUpdater									ViewUpdater;
	static ViewVectorUpdater									ViewDirUpdater;
	static ProjectionMatrixUpdater								ProjUpdater;
	static ViewDepthUpdater										DepthUpdater(&ProjUpdater);
	static ViewProjectionMatrixUpdater							ViewProjUpdater(&ViewUpdater, &ProjUpdater);
	static InverseViewMatrixUpdater								InvViewUpdater;
	static InverseProjectionMatrixUpdater						InvProjUpdater;
	static InverseViewProjectionMatrixUpdater					InvViewProjUpdater(&InvViewUpdater, &InvProjUpdater, &ViewProjUpdater);
	static TransposeViewMatrixUpdater							TansposeViewUpdater;
	static TransposeProjectionMatrixUpdater						TansposeProjUpdater;
	static TransposeViewProjectionMatrixUpdater					TransposeViewProjUpdater(&ViewProjUpdater);
	static InvTransposeViewMatrixUpdater						InvTransViewUpdater(&InvViewUpdater);
	static InvTransposeProjectionMatrixUpdater					InvTransProjUpdater(&InvProjUpdater);
	static InvTransposeVPMatrixUpdater							InvTransViewProjUpdater(&InvViewProjUpdater);
	static BillboardViewMatrixUpdater							BillboardViewUpdater(&EyePosUpdater);
	static BillboardViewProjectionMatrixUpdater					BillboardViewProjectionUpdater(&BillboardViewUpdater, &ProjUpdater);
	static ViewSizeUpdater										ViewWidthHeighUpdater;
	static ViewPosUpdater										ViewRelativePosSizeUpdater;
	static CameraDepthUpdater									CameraNearFarUpdater;
	static ViewFrustumRayUpdater								FrustumRayUpdater(&ProjUpdater);
	static ScreenSpaceUVFlipUpdater								UVFlipUpdater;
	static const size_t MAX_INPUTBUFFER_UPDATER = MAX_INPUT_COUNT;
	static RenderBufferUpdater									InputBufferUpdater[MAX_INPUTBUFFER_UPDATER];
	static RenderBufferUVSPaceUpdater							InputBufferUVUpdater[MAX_INPUTBUFFER_UPDATER];
	static RenderBufferSizeUpdater								InputBufferTexelSizeUpdater[MAX_INPUTBUFFER_UPDATER];
	static RenderTargetSizeUpdater								TargetSizeUpdater;
	static PostFXInputUpdater									PostFXBufferUpdater;
	static PostFXInputUVSPaceUpdater							PostFXBufferUVUpdater;
	static PostFXInputSizeUpdater								PostFXBufferSizeUpdater;

	static TimeUpdater											TimeSourceUpdater;
	static SinCosTimeUpdater									SinTimeUpdater;
	static ShadowMatrixUpdater									ShadowMatricesUpdater;
	static ShadowDistancesUpdater								ShadowDistanceUpdater;
	static ShadowTextureCoordinateUpdate						ShadowTextureCoordinateTransformUpdater;

	static ShadowDepthMatrixUpdater								ShadowDepthMatricesUpdater;
	static CurrentShadowDepthMatrixUpdater						CurrentShadowDpethUpdater(ShadowDepthMatricesUpdater);


	//////////////////////////////////////////////////////////////////////////
	GlobalShaderVariables::GlobalShaderVariables()
		:mCustomVariables(SVU_GLOBAL_CUSTOM)
	{
		mVariableMap[BTString("EYE_POS")] = EyePosUpdater.getVariable();
		mVariableMap[BTString("VIEW_DIR")] = ViewDirUpdater.getVariable();
		mVariableMap[BTString("VIEW_MATRIX")] = ViewUpdater.getVariable();
		mVariableMap[BTString("PROJ_MATRIX")] = ProjUpdater.getVariable();
		mVariableMap[BTString("VIEW_DEPTH_FACTOR")] = DepthUpdater.getVariable();
		mVariableMap[BTString("VIEWPROJ_MATRIX")] = ViewProjUpdater.getVariable();
		mVariableMap[BTString("INV_VIEW_MATRIX")] = InvViewUpdater.getVariable();
		mVariableMap[BTString("INV_PROJ_MATRIX")] = InvProjUpdater.getVariable();
		mVariableMap[BTString("INV_VIEWPROJ_MATRIX")] = InvViewProjUpdater.getVariable();
		mVariableMap[BTString("TRANSPOSE_VIEW_MATRIX")] = TansposeViewUpdater.getVariable();
		mVariableMap[BTString("TRANSPOSE_PROJ_MATRIX")] = TansposeProjUpdater.getVariable();
		mVariableMap[BTString("TRANSPOSE_VIEWPROJ_MATRIX")] = TransposeViewProjUpdater.getVariable();
		mVariableMap[BTString("INV_TRANS_VIEW_MATRIX")] = InvTransViewUpdater.getVariable();
		mVariableMap[BTString("INV_TRANS_PROJ_MATRIX")] = InvTransProjUpdater.getVariable();
		mVariableMap[BTString("INV_TRANS_VIEWPROJ_MATRIX")] = InvTransViewProjUpdater.getVariable();
		mVariableMap[BTString("BILLBOARD_VIEW_MATRIX")] = BillboardViewUpdater.getVariable();
		mVariableMap[BTString("BILLBOARD_VIEWPROJ_MATRIX")] = BillboardViewProjectionUpdater.getVariable();
		mVariableMap[BTString("VIEW_SIZE")] = ViewWidthHeighUpdater.getVariable();
		mVariableMap[BTString("VIEW_POS")] = ViewRelativePosSizeUpdater.getVariable();
		mVariableMap[BTString("CAMERA_DEPTH")] = CameraNearFarUpdater.getVariable();
		mVariableMap[BTString("VIEW_FRUSTUM_RAYS")] = FrustumRayUpdater.getVariable();
		mVariableMap[BTString("UV_FLIP")] = UVFlipUpdater.getVariable();
		mVariableMap[BTString("IMAGEFX_INPUT")] = PostFXBufferUpdater.getVariable();
		mVariableMap[BTString("IMAGEFX_INPUT_UVSPACE")] = PostFXBufferUVUpdater.getVariable();
		mVariableMap[BTString("IMAGEFX_INPUT_SIZE")] = PostFXBufferSizeUpdater.getVariable();
		mVariableMap[BTString("TIME")] = TimeSourceUpdater.getVariable();
		mVariableMap[BTString("SINCOS_TIME")] = SinTimeUpdater.getVariable();
		mVariableMap[BTString("SHADOW_MATRIX")] = ShadowMatricesUpdater.getVariable();
		mVariableMap[BTString("SHADOW_DEPTH_MATRIX")] = ShadowDepthMatricesUpdater.getVariable();
		mVariableMap[BTString("CURRENT_SHADOW_DEPTH_MATRIX")] = CurrentShadowDpethUpdater.getVariable();
		mVariableMap[BTString("SHADOW_DISTANCES")] = ShadowDistanceUpdater.getVariable();
		mVariableMap[BTString("SHADOW_TEXCOORDINATES")] = ShadowTextureCoordinateTransformUpdater.getVariable();

		static const size_t INIT_SIZE = 64;
		mCameraVariables.reserve(INIT_SIZE);
		mViewVariables.reserve(INIT_SIZE);

		mCameraVariables.push_back(EyePosUpdater.getVariable());
		mCameraVariables.push_back(ViewDirUpdater.getVariable());
		mCameraVariables.push_back(ViewUpdater.getVariable());
		mCameraVariables.push_back(ProjUpdater.getVariable());
		mCameraVariables.push_back(DepthUpdater.getVariable());
		mCameraVariables.push_back(ViewProjUpdater.getVariable());
		mCameraVariables.push_back(InvViewUpdater.getVariable());
		mCameraVariables.push_back(InvProjUpdater.getVariable());
		mCameraVariables.push_back(InvViewProjUpdater.getVariable());
		mCameraVariables.push_back(TansposeViewUpdater.getVariable());
		mCameraVariables.push_back(TansposeProjUpdater.getVariable());
		mCameraVariables.push_back(TransposeViewProjUpdater.getVariable());
		mCameraVariables.push_back(InvTransViewUpdater.getVariable());
		mCameraVariables.push_back(InvTransProjUpdater.getVariable());
		mCameraVariables.push_back(InvTransViewProjUpdater.getVariable());
		mCameraVariables.push_back(CameraNearFarUpdater.getVariable());
		mCameraVariables.push_back(FrustumRayUpdater.getVariable());
		mViewVariables.push_back(ViewWidthHeighUpdater.getVariable());
		mViewVariables.push_back(ViewRelativePosSizeUpdater.getVariable());
		mViewVariables.push_back(GlobalLightVariables::getViewLightVectorUpdater().getVariable());

		mOutputVariables.reserve(MAX_INPUTBUFFER_UPDATER*3+1);
		for (size_t i = 0; i < MAX_INPUTBUFFER_UPDATER; ++i)
		{
			assert(i < MAX_INPUTBUFFER_UPDATER);
			TString strIndex = TStringHelper::fromUInt(i);

			TString name = BTString("RENDERBUFFER_INPUT") + strIndex;
			InputBufferUpdater[i].setIndex(i);
			mVariableMap[name] = InputBufferUpdater[i].getVariable();
			mOutputVariables.push_back(InputBufferUpdater[i].getVariable());

			TString uvName = BTString("RENDERBUFFER_UVSPACE") + strIndex;
			InputBufferUVUpdater[i].setIndex(i);
			mVariableMap[uvName] = InputBufferUVUpdater[i].getVariable();
			mOutputVariables.push_back(InputBufferUVUpdater[i].getVariable());

			TString texelName = BTString("RENDERBUFFER_SIZE") + strIndex;
			InputBufferTexelSizeUpdater[i].setIndex(i);
			mVariableMap[texelName] = InputBufferTexelSizeUpdater[i].getVariable();
			mOutputVariables.push_back(InputBufferTexelSizeUpdater[i].getVariable());
		}
		mVariableMap[BTString("RENDERTARGET_SIZE")] = TargetSizeUpdater.getVariable();
		mOutputVariables.push_back(TargetSizeUpdater.getVariable());

		mPostFXVariables.reserve(INIT_SIZE);
		mPostFXVariables.push_back(PostFXBufferUpdater.getVariable());
		mPostFXVariables.push_back(PostFXBufferUVUpdater.getVariable());
		mPostFXVariables.push_back(PostFXBufferSizeUpdater.getVariable());

		mPerFrameVariables.reserve(INIT_SIZE);
		mPerFrameVariables.push_back(TimeSourceUpdater.getVariable());
		mPerFrameVariables.push_back(SinTimeUpdater.getVariable());

		mShadowVariables.reserve(INIT_SIZE);
		mShadowVariables.push_back(ShadowMatricesUpdater.getVariable());
		mShadowVariables.push_back(ShadowDepthMatricesUpdater.getVariable());
		mShadowVariables.push_back(ShadowDistanceUpdater.getVariable());
		mShadowVariables.push_back(ShadowTextureCoordinateTransformUpdater.getVariable());
		mShadowVariables.push_back(CurrentShadowDpethUpdater.getVariable());
	}

	//////////////////////////////////////////////////////////////////////////
	GlobalShaderVariables::~GlobalShaderVariables()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void					GlobalShaderVariables::markFrameVariableDirty()
	{
		for (size_t i = 0; i < mPerFrameVariables.size(); ++i)
			mPerFrameVariables[i]->markDirty();
	}

	//////////////////////////////////////////////////////////////////////////
	void					GlobalShaderVariables::markCameraVariableDirty()
	{
		for (size_t i = 0; i < mCameraVariables.size(); ++i)
		{
			mCameraVariables[i]->markDirty();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void					GlobalShaderVariables::markViewVariableDirty()
	{
		for (size_t i = 0; i < mViewVariables.size(); ++i)
		{
			mViewVariables[i]->markDirty();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void					GlobalShaderVariables::markRenderOutputVariableDirty()
	{
		for (size_t i = 0; i < mOutputVariables.size(); ++i)
		{
			mOutputVariables[i]->markDirty();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void					GlobalShaderVariables::markPostFXOutputVariableDirty()
	{
		for (size_t i = 0; i < mPostFXVariables.size(); ++i)
		{
			mPostFXVariables[i]->markDirty();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void					GlobalShaderVariables::markShadowVariableDirty(bool depthMatricesOnly)
	{
		if (depthMatricesOnly)
		{
			ShadowDepthMatricesUpdater.getVariable()->markDirty();
			CurrentShadowDpethUpdater.getVariable()->markDirty();
		}
		else
		{
			for (size_t i = 0; i < mShadowVariables.size(); ++i)
				mShadowVariables[i]->markDirty();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	AutoShaderVariable*		GlobalShaderVariables::getBuiltInVariable(const TString& semantic)
	{
		VariableMap::iterator i = mVariableMap.find(semantic);
		if (i != mVariableMap.end())
			return i->second;
		else
			return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	const HAUTOSHADERVARIABLE&	GlobalShaderVariables::createCustomVariable(const TString& name, SHADER_CONSTANT_TYPE type, size_t count)
	{
		if (mCustomVariables.getVariable(name) == NULL)
			mCustomVariables.createVariable(name, type, count);
		return mCustomVariables.getVariable(name);
	}

	//////////////////////////////////////////////////////////////////////////
	const HAUTOSHADERVARIABLE&	GlobalShaderVariables::getGlobalCustomVariable(const TString& name, SHADER_CONSTANT_TYPE type, size_t count)
	{
		const HAUTOSHADERVARIABLE& hv = mCustomVariables.getVariable(name);
		if (hv == NULL || hv->getType() != type || hv->getCount() != count)
			return HAUTOSHADERVARIABLE::EMPTY;
		return hv;
	}

	//////////////////////////////////////////////////////////////////////////
	bool						GlobalShaderVariables::setGlobalCustomVariable(const HAUTOSHADERVARIABLE& variable, const void* source, size_t bytes)
	{
		if (variable == NULL || bytes > variable->getRealSize())
			return false;
		ShaderVariableMap::writeData(ShaderVariableMap::getVariableData(variable), source, variable->getType(), variable->getCount());
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	ViewMatrixUpdater&		GlobalShaderVariables::getViewMatrixUpdater()
	{
		return ViewUpdater;
	}

	//////////////////////////////////////////////////////////////////////////
	ProjectionMatrixUpdater&	GlobalShaderVariables::getProjectionMatrixUpdater()
	{
		return ProjUpdater;
	}

	//////////////////////////////////////////////////////////////////////////
	ViewProjectionMatrixUpdater&	GlobalShaderVariables::getViewProjectionUpdater()
	{
		return ViewProjUpdater;
	}

	//////////////////////////////////////////////////////////////////////////
	CameraPositionUpdater&	GlobalShaderVariables::getCameraPositionUpdater()
	{
		return EyePosUpdater;
	}

	//////////////////////////////////////////////////////////////////////////
	BillboardViewMatrixUpdater& GlobalShaderVariables::getBillboardViewUpdater()
	{
		return BillboardViewUpdater;
	}

}//namespace Blade