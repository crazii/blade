/********************************************************************
	created:	2010/09/06
	filename: 	InstanceShaderVariables.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "InstanceShaderVariables.h"
#include "GlobalShaderVariables.h"
#include "GlobalLightVariables.h"

namespace Blade
{
	ViewMatrixUpdater& ViewUpdater						= GlobalShaderVariables::getViewMatrixUpdater();
	ViewProjectionMatrixUpdater& ViewProjectionUpdater	= GlobalShaderVariables::getViewProjectionUpdater();
	ProjectionMatrixUpdater& ProjectionUpdater			= GlobalShaderVariables::getProjectionMatrixUpdater();
	CameraPositionUpdater& CameraPosUpdater				= GlobalShaderVariables::getCameraPositionUpdater();
	ForwardGlobalLightVectorVariableUpdater& LightVectorUpdater		= GlobalLightVariables::getLightVectorUpdater();
	ForwardGlobalLightCountVariableUpdater& LightCountUpdater		= GlobalLightVariables::getLightCountUpdater();
	BillboardViewMatrixUpdater& BillboardViewUpdater	= GlobalShaderVariables::getBillboardViewUpdater();

	static WorldMatrixUpdater							WorldUpdater;
	static WorldPositionUpdater							WorldPosUpdater(&WorldUpdater);
	static WorldViewMatrixUpdater						WorldViewUpater(&WorldUpdater, &ViewUpdater);
	static WorldViewProjectionMatrixUpdater				WorldViewProjectionUpdater(&WorldUpdater, &ViewProjectionUpdater);
	static InverseWorldMatrixUpdater					InvWorldUpdater(&WorldUpdater);
	static InverseWorldViewMatrixUpdater				InvWorldViewUpdater(&WorldViewUpater);
	static InverseWorldViewProjectionMatrixUpdater		InvWorldViewProjectionUpdater(&WorldViewProjectionUpdater);
	static TransposeWorldMatrixUpdater					TransWorldUpdater(&WorldUpdater);
	static TransposeWorldViewMatrixUpdater				TransWorldViewUpdater(&WorldViewUpater);
	static TransposeWorldViewProjectionMatrixUpdater	TransWorldViewProjectionUpdater(&WorldViewProjectionUpdater);
	static InvTransposeWorldMatrixUpdater				InvTransWorldUpdater(&InvWorldUpdater);
	static InvTransposeWorldViewMatrixUpdater			InvTransWorldViewUpdater(&InvWorldViewUpdater);
	static InvTransposeWorldViewProjectionMatrixUpdater	InvTransWorldViewProjectionUpdater(&InvWorldViewProjectionUpdater);
	static InstanceCameraPositionUpdater				ObjectSpaceEyePosUpdater(&CameraPosUpdater, &InvWorldUpdater);
	static ForwardLocalLightCountUpdater				LocalLightCountUpdater(ForwardLocalLightUpdater::getSingletonPtr());
	static ForwardLocalLightPositionUpdater				LocalLightPositionUpdater(ForwardLocalLightUpdater::getSingletonPtr());
	static ForwardLocalLightDirectionUpdater			LocalLightDirectionUpdater(ForwardLocalLightUpdater::getSingletonPtr());
	static ForwardLocalLightDiffuseUpdater				LocalLightDiffuseUpdater(ForwardLocalLightUpdater::getSingletonPtr());
	static ForwardLocalLightSpecularUpdater				LocalLightSpecularUpdater(ForwardLocalLightUpdater::getSingletonPtr());
	static ForwardLocalLightAttenuationUpdater			LocalLightAttenuationUpdater(ForwardLocalLightUpdater::getSingletonPtr());
	static BillboardWorldViewMatrixUpdater				BillboardWorldViewUpdater(&BillboardViewUpdater, &WorldUpdater);
	static BillboardWorldViewProjectionMatrixUpdater	BillboardWorldViewProjectionUpdater(&BillboardWorldViewUpdater, &ProjectionUpdater);
	static BonePaletteUpdater							SkinBonePaletteUpdater;
	static ProjectedPosUpdater							ProjectedPositionUpdater(&WorldViewProjectionUpdater);
	static ViewSpacePositionUpdater						ViewSpacePosUpdater(&ViewUpdater, &WorldUpdater);
	static ViewSpaceBoundingnUpdater					ViewSpaceBoundingUpdater(&ViewUpdater);
	static SamplerCountUpdater							TextureCountUpdater;
	static DeferredLightPositionUpdater					LightPositionUpdater;
	static DeferredLightViewSpacePositionUpdater		LightPositionViewSpaceUpdater(&ViewSpacePosUpdater);
	static DeferredLightDiffuseUpdater					LightDiffuseUpdater;
	static DeferredLightSpecularUpdater					LightSpeculardater;
	static DeferredLightAttenuationUpdater				LightAttenuationUpdater;
	static DeferredLightSpotDirectionUpdater			LightDirectionUpdater;

	//////////////////////////////////////////////////////////////////////////
	InstanceVariableUpdater* InstanceVariableUpdater::msRootInstanceUpdater = &WorldUpdater;

	//////////////////////////////////////////////////////////////////////////
	InstanceShaderVariables::InstanceShaderVariables()
	{
		mVariableMap[BTString("WORLD_MATRIX")] = WorldUpdater.getVariable();
		mVariableMap[BTString("WORLD_POSITION")] = WorldPosUpdater.getVariable();
		mVariableMap[BTString("WORLD_VIEW_MATRIX")] = WorldViewUpater.getVariable();
		mVariableMap[BTString("WORLD_VIEWPROJ_MATRIX")] = WorldViewProjectionUpdater.getVariable();
		mVariableMap[BTString("INV_WORLD_MATRIX")] = InvWorldUpdater.getVariable();
		mVariableMap[BTString("INV_WORLD_VIEW_MATRIX")] = InvWorldViewUpdater.getVariable();
		mVariableMap[BTString("INV_WORLD_VIEWPROJ_MATRIX")] = InvWorldViewProjectionUpdater.getVariable();
		mVariableMap[BTString("TRANSPOSE_WORLD_MATRIX")] = TransWorldUpdater.getVariable();
		mVariableMap[BTString("TRANSPOSE_WORLD_VIEW_MATRIX")] = TransWorldViewUpdater.getVariable();
		mVariableMap[BTString("TRANSPOSE_WORLD_VIEWPROJ_MATRIX")] = TransWorldViewProjectionUpdater.getVariable();
		mVariableMap[BTString("INV_TRANS_WORLD_MATRIX")] = InvTransWorldUpdater.getVariable();
		mVariableMap[BTString("INV_TRANS_WORLD_VIEW_MATRIX")] = InvTransWorldViewUpdater.getVariable();
		mVariableMap[BTString("INV_TRANS_WORLD_VIEWPROJ_MATRIX")] = InvTransWorldViewProjectionUpdater.getVariable();

		mVariableMap[BTString("EYE_POS_OBJECT_SPACE")] = ObjectSpaceEyePosUpdater.getVariable();
		mVariableMap[BTString("LIGHT_COUNT")] = LocalLightCountUpdater.getVariable();
		mVariableMap[BTString("LIGHT_POSITION")] = LocalLightPositionUpdater.getVariable();
		mVariableMap[BTString("LIGHT_DIRECTION")] = LocalLightDirectionUpdater.getVariable();
		mVariableMap[BTString("LIGHT_DIFFUSE")] = LocalLightDiffuseUpdater.getVariable();
		mVariableMap[BTString("LIGHT_SPECULAR")] = LocalLightSpecularUpdater.getVariable();
		mVariableMap[BTString("LIGHT_ATTENUATION")] = LocalLightAttenuationUpdater.getVariable();
		mVariableMap[BTString("BILLBOARD_WORLD_VIEW_MATRIX")] = BillboardWorldViewUpdater.getVariable();
		mVariableMap[BTString("BILLBOARD_WORLD_VIEWPROJ_MATRIX")] = BillboardWorldViewProjectionUpdater.getVariable();
		mVariableMap[BTString("BONE_PALETTE")] = SkinBonePaletteUpdater.getVariable();
		mVariableMap[BTString("PROJECTED_POS")] = ProjectedPositionUpdater.getVariable();
		mVariableMap[BTString("VIEW_SPACE_POSITION")] = ViewSpacePosUpdater.getVariable();
		mVariableMap[BTString("VIEW_SPACE_BOUNDING")] = ViewSpaceBoundingUpdater.getVariable();
		mVariableMap[BTString("SAMPLER_COUNT")] = TextureCountUpdater.getVariable();
		mVariableMap[BTString("DEFERRED_LIGHT_POSITION")] = LightPositionUpdater.getVariable();
		mVariableMap[BTString("DEFERRED_LIGHT_POSITION_VIEW_SPACE")] = LightPositionViewSpaceUpdater.getVariable();
		mVariableMap[BTString("DEFERRED_LIGHT_DIFFUSE")] = LightDiffuseUpdater.getVariable();
		mVariableMap[BTString("DEFERRED_LIGHT_SPECULAR")] = LightSpeculardater.getVariable();
		mVariableMap[BTString("DEFERRED_LIGHT_ATTENUATION")] = LightAttenuationUpdater.getVariable();
		mVariableMap[BTString("DEFERRED_LIGHT_DIRECTION")] = LightDirectionUpdater.getVariable();

		//
		mTexProjectUpdaters.resize(MaterialInstance::TEX_PROJECTION_LIMIT);
		const TString TEX_PROJECT_SEMANTIC = BTString("TEX_PROJECTION_VP_MATRIX");
		for(size_t i = 0; i < mTexProjectUpdaters.size(); ++i )
		{
			mTexProjectUpdaters[i].setIndex(i);

			TString semantic = TEX_PROJECT_SEMANTIC + TStringHelper::fromUInt(i);
			mVariableMap[semantic] = mTexProjectUpdaters[i].getVariable();
		}

		mUpdaters.resize(mVariableMap.size());
		size_t index = 0;
		for(VariableMap::const_iterator i = mVariableMap.begin(); i != mVariableMap.end(); ++i)
			mUpdaters[index++] = static_cast<InstanceVariableUpdater*>( i->second->getUpdater() );
		//sort updater (static variables) by address to avoid cache miss
		std::sort(mUpdaters.begin(), mUpdaters.end());

		mForwardLocalLightUpdater = ForwardLocalLightUpdater::getSingletonPtr();
	}

	//////////////////////////////////////////////////////////////////////////
	InstanceShaderVariables::~InstanceShaderVariables()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void					InstanceShaderVariables::onShaderSwitch(bool dynamicSwithing)
	{
		for (index_t i = 0; i < mUpdaters.size(); ++i)
			mUpdaters[i]->onShaderSwitch(dynamicSwithing);
	}

	//////////////////////////////////////////////////////////////////////////
	void					InstanceShaderVariables::markDirty(const RenderOperation* rop)
	{
		for (index_t i = 0; i < mUpdaters.size(); ++i)
			mUpdaters[i]->setRenderOperation(rop);
		mForwardLocalLightUpdater->reset();
	}

	//////////////////////////////////////////////////////////////////////////
	AutoShaderVariable*		InstanceShaderVariables::getBuiltInVariable(const TString& semantic)
	{
		VariableMap::iterator i = mVariableMap.find(semantic);
		if( i != mVariableMap.end() )
			return i->second;
		else
			return NULL;
	}
	
}//namespace Blade