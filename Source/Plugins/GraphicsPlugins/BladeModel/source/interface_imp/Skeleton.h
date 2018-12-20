/********************************************************************
	created:	2014/06/14
	filename: 	Skeleton.h
	author:		Crazii
	
	purpose:	runtime skeleton instance for skeleton resource
*********************************************************************/
#ifndef __Blade_Skeleton_h__
#define __Blade_Skeleton_h__
#include "SkeletonResourceState.h"
#include "Animation.h"

namespace Blade
{
	class IKSolver;

	class Skeleton : public Allocatable
	{
	public:
		Skeleton();
		~Skeleton();

		/** @brief  */
		bool	initialize(const HRESOURCE& skeleton, uint64 loopID);

		/** @brief  */
		bool	cleanup();

		/** @brief  */
		bool	updateBoneTransforms(uint64 loopID, scalar time, scalar frameTime, bool forceUpdate);

		/** @brief  */
		bool	needUpdateBoneTransforms(uint64 loopID) const { return mActiveBones.size() > 0 && loopID != mLoopID; }

		/** @brief  */
		inline const DualQuaternion*	getBlendedTransform() const {return mBoneDQ;}

		/** @brief  */
		bool	activateBones(const uint32* bones, size_t count, bool active);

		/** @brief  */
		size_t	getAnimtaionCount() const;

		/** @brief  */
		const tchar* getAnimation(index_t index) const;

		/** @brief  */
		bool	setAnimation(const TString& name);

		/** @brief  */
		AnimationState* getCurrentAnimation() const;

		/** @brief  */
		inline void	setUpdateFrequency(scalar freq)	{mFrequency = freq;}

		/** @brief  */
		inline bool			isBoneActive(index_t boneIndex) const	{return boneIndex < mActiveBones.size() ? mActiveBones[boneIndex] > 0 : false;}

		/** @brief  */
		const BONE_DATA*	getBoneData(index_t boneIndex) const;

		/** @brief  */
		IKSolver*			getIKSolver() const	{return mSolver;}

	protected:
		/** @brief  */
		size_t	activateBone(uint32 boneIndex, bool active);

		SkeletonResourceState	mSkeletonRes;
		uint64					mLoopID;
		BoneDQ*					mBoneDQ;
		scalar					mFrameTime;
		scalar					mFrequency;
		Animation				mAnimation;
		IKSolver*				mSolver;
		
		typedef Vector<BoneActivity>	ActiveBone;	//bone ref count by animations
		ActiveBone				mActiveBones;
	};//class Skeleton
	
}//namespace Blade


#endif // __Blade_Skeleton_h__