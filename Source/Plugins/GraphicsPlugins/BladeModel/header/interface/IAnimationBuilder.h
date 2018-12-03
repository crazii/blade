/********************************************************************
	created:	2015/09/07
	filename: 	IAnimationBuilder.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_IAnimationBuilder_h__
#define __Blade_IAnimationBuilder_h__
#include <math/Vector2.h>
#include <math/Vector3.h>
#include <math/Vector4.h>
#include <math/Quaternion.h>
#include <math/Matrix33.h>
#include <interface/public/ISerializable.h>
#include <interface/Interface.h>
#include "ISkeletonResource.h"

namespace Blade
{

	struct AnimationDef
	{
		TString	Name;
		uint32	StartFrame;
		uint32  EndFrame;
		bool	Loop;

		inline bool operator==(const AnimationDef& rhs) const
		{
			return Name == rhs.Name;
		}
	};

	class IAnimationCollector
	{
	public:
		/** @brief  */
		virtual size_t			getAnimationCount() = 0;
		/** @brief  */
		virtual AnimationDef*	getAnimationDef(index_t index) = 0;

		/** @brief  */
		virtual size_t			getBoneCount() = 0;

		/** @brief  */
		virtual TString			getBoneName(index_t index) = 0;

		/** @brief get bone array */
		virtual const BONE_DATA*getBones() = 0;

		/** @brief  */
		virtual void			setCurrentAnimation(const TString& name) = 0;

		/** @brief  */
		virtual scalar			getFrameTime(uint32 frame) = 0;

		/** @brief  */
		virtual bool			getBoneTransform(BoneDQ& transform, index_t boneIndex, scalar time) = 0;

	};//class IAnimationCollector

	class IAnimationBuilder : public Interface
	{
	public:
		BLADE_MODEL_API static const TString ANIMATION_BUILDER_CLASS;
	public:
		virtual ~IAnimationBuilder()	{}

		/*
		@describe 
		@param
		@return
		*/
		virtual bool	initialize(scalar animFrequency, scalar angleError, scalar posError) = 0;

		/*
		@describe 
		@param
		@return
		*/
		virtual bool	buildAnimation(IAnimationCollector* collector, ISkeletonResource* target, ProgressNotifier& callback) = 0;
	};//class IAnimationBuilder

	extern template class BLADE_MODEL_API Factory<IAnimationBuilder>;
	
}//namespace Blade


#endif // __Blade_IAnimationBuilder_h__