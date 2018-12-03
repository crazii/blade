/********************************************************************
	created:	2017/02/04
	filename: 	HDR.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_HDR_h__
#define __Blade_HDR_h__
#include <interface/public/IImageEffect.h>
#include <Technique.h>

namespace Blade
{
	class Material;
	namespace
	{
		static const scalar HDR_DEFAULT_BLOOM_FACTOR = 0.75f;
		static const uint32 HDR_DEFAULT_BLOOM_INTENSITY = 2;
		static const scalar HDR_DEFAULT_EXPOSURE = 1.0f;
	}

	class HDR : public ImageEffectBase, public Allocatable
	{
	public:
		static const TString NAME;
	public:
		HDR();
		~HDR();

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
		virtual void onEnable();

		/** @brief  */
		virtual void onDisable();

	protected:
		Material*	mMaterial;
		HTEXTURE	mLuminance;	//adapted luminance
		HTEXTURE	mLastLuminace;
		ShaderVariableMap::Handles mBloomFactorHandle;
		ShaderVariableMap::Handles mExposureHandle;
		fp32		mBloomFactor;
		fp32		mExposureScale;
		uint32		mBloomIntensity;
		bool		mEnableBloom;

		friend class BladePostFXPlugin;
	};
	
}//namespace Blade


#endif // __Blade_HDR_h__