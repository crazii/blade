/********************************************************************
	created:	2017/1/25
	filename: 	TAA.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_TAA_h__
#define __Blade_TAA_h__
#include <interface/public/IImageEffect.h>
#include <math/Vector2.h>
#include <math/Matrix44.h>
#include <Technique.h>

namespace Blade
{
	class MaterialInstance;

	class TemporalAA : public ImageEffectBase, public Allocatable
	{
	public:
		static const TString NAME;
	public:
		TemporalAA();
		~TemporalAA();

		/** @brief  */
		virtual void initialize();

		/** @brief  */
		virtual void shutdown();

		/** @brief  */
		virtual void prepareImpl();

		/** @brief  */
		virtual bool process(const HTEXTURE& input, const HTEXTURE& output);

		/** @brief  */
		virtual void finalizeImpl();

		/** @brief  */
		virtual void onDisable();

	protected:
		scalar		mHalton2x3_16[16];
		size_t		mIndex;
		size_t		mHistoryIndex;
		Vector2		mJitter;
		scalar		mSharpness;
		Matrix44	mProjectionMatrix;
		HTEXTURE	mHistory[2];
		ShaderVariableMap::Handles mHandleHistoryMatrix;
		ShaderVariableMap::Handles mHandleSampleWeights;
		ShaderVariableMap::Handles mHandleLowpassWeights;
		ShaderVariableMap::Handles mHandlePlusWeights;
		MaterialInstance*	mMaterial;
	};
	
}//namespace Blade


#endif//__Blade_TAA_h__