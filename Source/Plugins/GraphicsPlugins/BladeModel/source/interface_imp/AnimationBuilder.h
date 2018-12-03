/********************************************************************
	created:	2015/09/07
	filename: 	AnimationBuilder.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_AnimationBuilder_h__
#define __Blade_AnimationBuilder_h__
#include <interface/IAnimationBuilder.h>
#include <utility/BladeContainer.h>


namespace Blade
{
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	typedef TempVector<BONE_DATA>	BoneList;
	typedef TempVector<uint32>		BoneIDList;
	typedef TempList<KeyFrame>		BoneKeyframeList;
	typedef TempVector<KeyFrame>	BoneKeyframeArray;
	typedef TempVector<uint16>		BoneKeyCountList;
	typedef Blade::TempList<scalar> KeyTimeList;
	typedef Blade::TempVector<scalar> ErrorList;

	typedef TempVector< std::pair<KeyFrame*, index_t> >		BoneKeyframePtrList;
	typedef TempMap<scalar, BoneKeyframePtrList>			TimeKeyFrameList;


	class AnimationBuilder : public IAnimationBuilder, public TempAllocatable
	{
	public:
		AnimationBuilder()
			:mFrequency(10.0f)
			,mAngleError(0.4f)
			,mPosError(0.004f)
		{
		}

		/************************************************************************/
		/* IAnimationBuilder interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param
		@return
		*/
		virtual bool	initialize(scalar animFrequency, scalar angleError, scalar posError)
		{
			mFrequency = animFrequency;
			mAngleError = angleError;
			mPosError = posError;
			return true;
		}

		/*
		@describe 
		@param
		@return
		*/
		virtual bool	buildAnimation(IAnimationCollector* collector, ISkeletonResource* target, ProgressNotifier& callback);

	protected:
		scalar	mFrequency;
		scalar	mAngleError;
		scalar	mPosError;
	};//class AnimationBuilder

	
}//namespace Blade


#endif // __Blade_AnimationBuilder_h__