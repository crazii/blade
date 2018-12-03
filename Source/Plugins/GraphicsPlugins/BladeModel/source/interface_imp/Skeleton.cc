/********************************************************************
	created:	2014/06/14
	filename: 	Skeleton.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "Skeleton.h"
#include <utility/Profiling.h>
#include <interface/IPlatformManager.h>
#include "IKSolver.h"

namespace Blade
{
	//human eyes max resolution rate 48Hz, set to 60Hz for precision
	static const scalar SKELETON_UPDATE_MAX_FREQUENCY = 1.0f/60.0f;

	//////////////////////////////////////////////////////////////////////////
	Skeleton::Skeleton()
		:mLoopID( 0 )
		,mBoneDQ(NULL)
		,mFrameTime(0)
		,mFrequency(SKELETON_UPDATE_MAX_FREQUENCY)
		,mSolver(NULL)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	Skeleton::~Skeleton()
	{
		this->cleanup();
	}

	//////////////////////////////////////////////////////////////////////////
	bool	Skeleton::initialize(const HRESOURCE& skeleton, uint64 loopID)
	{
		if( mBoneDQ != NULL )
			return false;

		mSkeletonRes.setManualResource(true, skeleton);

		SkeletonResource* skeletonRes = mSkeletonRes.getSkeletonResource();

		if( mAnimation.initialize( skeletonRes->getAnimation() ) )
		{
			mLoopID = loopID;
			size_t boneCount = skeletonRes->getAnimation()->getBoneCount();
			mBoneDQ = BLADE_NEW BoneDQ[ boneCount ];
			mActiveBones.resize(boneCount, 0);

			if( IModelConfigManager::getSingleton().isIKEnabled() && skeletonRes->getIK() != NULL )
			{
				IKSolver* solver = BLADE_FACTORY_CREATE(IKSolver, IModelConfigManager::getSingleton().getIKConfiguration());
				bool ret = solver->initialize(skeletonRes->getIK());
				if(!ret)
					BLADE_DELETE solver;
				else
					mSolver = solver;
			}

			return true;
		}
		else
			return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	Skeleton::cleanup()
	{
		BLADE_DELETE mSolver;
		BLADE_DELETE[] mBoneDQ;
		mActiveBones.clear();
		return mAnimation.cleanup();
	}
	
	//////////////////////////////////////////////////////////////////////////
	bool	Skeleton::updateBoneTransforms(uint64 loopID, scalar time, scalar frameTime, bool forceUpdate)
	{
		if( mLoopID != loopID)
		{
			//BLADE_LW_PROFILING_FUNCTION();
			mLoopID = loopID;

			//check frequency
			if( frameTime >= mFrequency*2 )	//we must update on low fps
				mFrameTime = 0;
			else
			{
				mFrameTime += frameTime;
				if((mFrameTime < mFrequency) && !forceUpdate)
					//BLADE_LOG(DebugOutput, BTString("skip animation update because frame time=") << mFrameTime );
					return false;
				mFrameTime = std::fmod(mFrameTime, mFrequency);
			}

			SkeletonResource* skeletonRes = mSkeletonRes.getSkeletonResource();
			size_t boneCount = skeletonRes->getAnimation()->getBoneCount();
			assert( mActiveBones.size() == boneCount );
			if( boneCount == 0 )
				return false;

			//reset blending data
			std::memset(mBoneDQ, 0, sizeof(BoneDQ)*boneCount);

			//update bone local transform
			bool result = mAnimation.update(time, mBoneDQ, &mActiveBones[0]);

			if (result)
			{
				//BLADE_LW_PROFILING_FUNCTION();
				const BONE_DATA* boneData = skeletonRes->getBoneData(0);

				//update bone hierarchy & calculate bone transforms
				for (size_t i = 0; i < boneCount; ++i)
				{
					if (mActiveBones[i] == 0) //bone not used by any animation
						continue;
					IPlatformManager::prefetch<PM_WRITE>(&mBoneDQ[i], sizeof(DualQuaternion));
					mBoneDQ[i].normalize();

					//apply hierarchy
					uint32 parent = boneData[i].mParent;
					if (parent != uint32(-1))
					{
						//bones already sorted in linear order (by animation exporter), parent always calculated before children
						//@see BoneTable::sortBoneData
						assert(parent < (uint32)i);
						mBoneDQ[i] = mBoneDQ[i] * mBoneDQ[parent];
					}
				}

				if (IModelConfigManager::getSingleton().isIKEnabled() && mSolver != NULL)
					mSolver->solve(mBoneDQ, &mActiveBones[0], boneData, boneCount);

				//apply inversed binding pose
				for (size_t i = 0; i < boneCount; ++i)
				{
					if (mActiveBones[i] == 0) //bone not used by any animation
						continue;

					IPlatformManager::prefetch<PM_READ>(&boneData[i], sizeof(BONE_DATA));
					IPlatformManager::prefetch<PM_WRITE>(&mBoneDQ[i], sizeof(DualQuaternion));

					//reset bone matrices to init pose (T pose) to prepare animation
					const BoneDQ& tposeDQ = boneData[i].mInitialPose;
					//note: tposeDQ is normalized after loading and never modified
					//and Inverse(dq) == Conjugate(dq), if dq is normalized
					BoneDQ inversedBindingPose = tposeDQ.getConjugate();

					mBoneDQ[i] = inversedBindingPose * mBoneDQ[i];
				}
			}

			assert(result);
			return result;
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	Skeleton::activateBones(const uint32* bones, size_t count, bool active)
	{
		SkeletonResource* skeletonRes = mSkeletonRes.getSkeletonResource();
		if( bones == NULL || count == 0 || skeletonRes == NULL )
			return false;

		size_t changedCount = 0;
		for(size_t i = 0; i < count; ++i)
		{
			uint32 boneIndex = bones[i];
			if( boneIndex == (uint32)INVALID_INDEX )
				continue;

			changedCount += this->activateBone(boneIndex, active);
		}
		return changedCount > 0;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t	Skeleton::getAnimtaionCount() const
	{
		return mAnimation.getAnimtaionCount();
	}

	//////////////////////////////////////////////////////////////////////////
	const tchar* Skeleton::getAnimation(index_t index) const
	{
		return mAnimation.getAnimtaion(index);
	}

	//////////////////////////////////////////////////////////////////////////
	bool	Skeleton::setAnimation(const TString& name)
	{
		return mAnimation.setAnimation(name);
	}

	//////////////////////////////////////////////////////////////////////////
	AnimationState* Skeleton::getCurrentAnimation() const
	{
		return mAnimation.getCurrentAnimation();
	}

	//////////////////////////////////////////////////////////////////////////
	const BONE_DATA*	Skeleton::getBoneData(index_t boneIndex) const
	{
		SkeletonResource* skeletonRes = mSkeletonRes.getSkeletonResource();
		if( skeletonRes != NULL )
			return skeletonRes->getBoneData(boneIndex);
		else
			return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t	Skeleton::activateBone(uint32 boneIndex, bool active)
	{
		if (boneIndex >= mActiveBones.size())
		{
			assert(false);
			BLADE_EXCEPT(EXC_INTERNAL_ERROR, BTString("data corrupted"));
		}
		if (active)
		{
			assert(mActiveBones[boneIndex] < std::numeric_limits<BoneActivity>::max());	//overflow check
			++mActiveBones[boneIndex];
		}
		else if (mActiveBones[boneIndex] > 0)
			--mActiveBones[boneIndex];
		else
			assert(false);	//activating/deactivating count mismatch?

		//note: active bones for mesh already contain parent
#if 0
		SkeletonResource* skeletonRes = mSkeletonRes.getSkeletonResource();
		const BONE_DATA* boneData = skeletonRes->getBoneData(0);

		const BONE_DATA& data = boneData[boneIndex];
		return 1 + (data.mParent != uint32(-1) ? this->activateBone(data.mParent, active) : 0);
#else
		return 1;
#endif
	}

}//namespace Blade