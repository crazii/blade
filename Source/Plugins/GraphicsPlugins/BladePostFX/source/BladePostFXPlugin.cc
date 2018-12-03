/********************************************************************
	created:	2017/1/24
	filename: 	BladePostFXPlugin.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <databinding/DataSourceWriter.h>
#include "BladePostFXPlugin.h"
#include "MotionVector.h"
#include "TAA.h"
#include "HDR.h"
#include "HBAO.h"

namespace Blade
{
	static const TString PLUGIN_NAME = BTString("BladePostFX");
	BLADE_DEF_PLUGIN(BladePostFXPlugin);

	//////////////////////////////////////////////////////////////////////////
	BladePostFXPlugin::BladePostFXPlugin()
		:PluginBase(PLUGIN_NAME)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	BladePostFXPlugin::~BladePostFXPlugin()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void				BladePostFXPlugin::getSupportList(TStringParam& supportList) const
	{
		supportList.push_back(BTString("PostFX"));
	}

	//////////////////////////////////////////////////////////////////////////
	void				BladePostFXPlugin::getDependency(TStringParam& dependencyList) const
	{
		dependencyList.push_back(BTString("GraphicsService"));
	}

	//////////////////////////////////////////////////////////////////////////
	void				BladePostFXPlugin::install()
	{
		NameRegisterFactory(MotionVector, IImageEffect, MotionVector::NAME);
		NameRegisterFactory(TemporalAA, IImageEffect, TemporalAA::NAME);
		NameRegisterFactory(HDR, IImageEffect, HDR::NAME);
		NameRegisterFactory(HBAO, IImageEffect, HBAO::NAME);

		{
			const TString BLOOM = BTString("Enable Bloom");
			const TString BLOOM_FACTOR = BTString("Bloom Factor");
			const TString BLOOM_INTENSITY = BTString("Bloom Intensity");	//bloom pass count
			const TString EXPOSURE_SCALE = BTString("Exposure Sale");
			DataSourceWriter<HDR> writer(HDR::NAME);
			writer.beginVersion(Version(0, 2));
			writer << BLOOM << CAF_NORMAL_DEVMODE << Variant(true) << &HDR::mEnableBloom;
			writer << BLOOM_FACTOR << CAF_NORMAL_DEVMODE << ConfigAtom::Range(0.0f, 1.0f, 0.001f, HDR_DEFAULT_BLOOM_FACTOR) << &HDR::mBloomFactor;
			writer << BLOOM_INTENSITY << CAF_NORMAL_DEVMODE << ConfigAtom::Range(uint32(1), uint32(4), uint32(1), uint32(HDR_DEFAULT_BLOOM_INTENSITY)) << &HDR::mBloomIntensity;
			writer << EXPOSURE_SCALE << CAF_NORMAL_DEVMODE << ConfigAtom::Range(0.001f, 10.0f, 0.001f, HDR_DEFAULT_EXPOSURE) << &HDR::mExposureScale;
			writer.endVersion();
		}

		{
			const TString BLUR_SHARPNESS = BTString("Blur Sharpness");
			const TString ANGLE_BIAS = BTString("Angle Bias");
			const TString AO_SCALE = BTString("AO Intensity");
			const TString AO_RANGE = BTString("AO Range");
			const TString AO_MAX_DISTANCE = BTString("Max Distance");
			const TString DIR_COUNT = BTString("Direction Count");
			const TString STEP_COUNT = BTString("Step Count");
			const TString HALF_RES = BTString("Resolution");

			DataSourceWriter<HBAO> writer(HBAO::NAME);
			writer.beginVersion(Version(0, 2));
			writer << BLUR_SHARPNESS << ConfigAtom::Range(1.0f, 60.0f, 1.0f, AO_DEF_BLURSHARPNESS) << &HBAO::mSharpness;
			writer << ANGLE_BIAS << ConfigAtom::Range(0.01f, 0.5f, 0.01f, AO_DEF_ANGLE_BIAS) << &HBAO::mAngleBias;
			writer << AO_SCALE << ConfigAtom::Range(0.01f, 30.0f, 0.01f, AO_DEF_AO_SCALE) << &HBAO::mAOScale;
			writer << AO_RANGE << ConfigAtom::Range(0.01f, 5.0f, 0.01f, AO_DEF_RANGE) << &HBAO::mAORange;
			writer << AO_MAX_DISTANCE << ConfigAtom::Range(1.0f, 2000.0f, 1.0f, AO_DEF_MAX_DISTANCE) << &HBAO::mAOMaxDistance;
			writer << DIR_COUNT << ConfigAtom::Range((uint32)1u, (uint32)8u, (uint32)1u, (uint32)AO_DEF_DIR_COUNT) << &HBAO::onConfigChange << &HBAO::mDirectionCount;
			writer << STEP_COUNT << ConfigAtom::Range((uint32)1u, (uint32)8u, (uint32)1u, (uint32)AO_DEF_STEP_COUNT) << &HBAO::mStepCount;
			writer << HALF_RES << ConfigAtom::Range(0.5f, 1.0f, 0.1f, AO_DEF_RESOLUTION) << &HBAO::mResolutionScale;
			writer.endVersion();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void				BladePostFXPlugin::uninstall()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void				BladePostFXPlugin::initialize()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void				BladePostFXPlugin::shutdown()
	{

	}
	
}//namespace Blade