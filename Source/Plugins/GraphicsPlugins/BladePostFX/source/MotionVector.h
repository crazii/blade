/********************************************************************
	created:	2017/01/26
	filename: 	MotionVector.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_MotionVector_h__
#define __Blade_MotionVector_h__
#include <interface/public/IImageEffect.h>
#include <math/Matrix44.h>
#include <Technique.h>

namespace Blade
{
	class Material;

	class MotionVector : public ImageEffectBase, public Allocatable
	{
	public:
		static const TString NAME;
	public:
		MotionVector();
		~MotionVector();

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
		Material*	mMaterial;
		ShaderVariableMap::Handles mHandleReprojection;
	};

	
}//namespace Blade


#endif // __Blade_MotionVector_h__