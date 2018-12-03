/********************************************************************
	created:	2017/8/1
	filename: 	HBAO.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_HBAO_h__
#define __Blade_HBAO_h__

#include <interface/public/IImageEffect.h>
#include <math/Matrix44.h>
#include <Technique.h>

namespace Blade
{
	class Material;

	namespace
	{
		static const fp32 AO_DEF_BLURSHARPNESS = 20.0f;
		static const fp32 AO_DEF_ANGLE_BIAS = 0.15;
		static const fp32 AO_DEF_AO_SCALE = 3.0f;
		static const fp32 AO_DEF_RANGE = 0.6f;
		static const fp32 AO_DEF_RESOLUTION = 1.0f;
		static const fp32 AO_DEF_MAX_DISTANCE = 1000;
		static const int32 AO_DEF_DIR_COUNT = 4;
		static const int32 AO_DEF_STEP_COUNT = 4;
	}

	class HBAO : public ImageEffectBase, public Allocatable
	{
	public:
		static const TString NAME;
	public:
		HBAO();
		~HBAO();

		/** @brief  */
		virtual void initialize();

		/** @brief  */
		virtual void shutdown();

		/** @brief  */
		virtual void prepareImpl();

		/** @brief  */
		virtual bool process_inputoutput(const HTEXTURE& chainIn, const HTEXTURE& chainOut, const HTEXTURE& depth,
			const HTEXTURE* extraInputs, size_t inputCount, const HTEXTURE* extraOuputs, size_t outputCount);

		/** @brief  */
		virtual void finalizeImpl();

	protected:
		/** @brief  */
		void	initRandomTexture();
		/** @brief  */
		void	setupDirections();

		MaterialInstance*	mMaterial;
		HTEXTURE	mNoiseTexture;
		ShaderVariableMap::Handles mDirectionHandle;
		ShaderVariableMap::Handles mDirection3DHandle;
		ShaderVariableMap::Handles mParameterHandle;

		fp32		mSharpness;
		fp32		mAngleBias;
		fp32		mAOScale;
		fp32		mAORange;
		fp32		mResolutionScale;
		fp32		mAOMaxDistance;
		uint32		mDirectionCount;
		uint32		mStepCount;
		bool		mSSAO3DRay;
		void		onConfigChange(void* data);

		friend class BladePostFXPlugin;
	};//class HBAO;


}//namespace Blade

#endif//__Blade_HBAO_h__