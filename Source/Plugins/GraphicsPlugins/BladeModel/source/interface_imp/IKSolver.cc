/********************************************************************
	created:	2015/11/27
	filename: 	IKSolver.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include "BladePCH.h"
#include "IKSolver.h"
#include "IK.h"
#include <utility/Profiling.h>
#include <interface/IPlatformManager.h>

namespace Blade
{
	template class Factory<IKSolver>;

	static const scalar REFERENCE_SIZESQ = (1.5f*1.5f) + (2.0f*2.0f) + (0.5f*0.5f);
	static const scalar MIN_DISTANCE = 0.001f;
	static const scalar MIN_DISTANCESQ = (MIN_DISTANCE*MIN_DISTANCE) / REFERENCE_SIZESQ;

	//////////////////////////////////////////////////////////////////////////
	IKSolver::IKSolver()
	{
		assert(IModelConfigManager::getSingleton().getIKConfiguration() != ModelConsts::IK_NONE);
		mIK = NULL;
		mValidTargets = 0;
	}

	//////////////////////////////////////////////////////////////////////////
	IKSolver::~IKSolver()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	void	IKSolver::reset()
	{
		for(size_t i = 0; i < mTargets.size(); ++i)
			mTargets[i].enabled = false;

		mValidTargets = 0;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	IKSolver::setTarget(const TString& effectorType, index_t index, const Vector3& modelPosition)
	{
		EffectorTarget target = {effectorType, 0, modelPosition, false};

		//mTargets is already sorted
		TargetList::iterator i = std::lower_bound(mTargets.begin(), mTargets.end(), target);
		if( i != mTargets.end() && i+(indexdiff_t)index < mTargets.end() && !(target < *(i+(indexdiff_t)index)) )
		{
			i += (indexdiff_t)index;
			//targets with the same type should be sorted in group
			if( i->type == effectorType )
			{
				if( !i->enabled )
				{
					i->enabled = true;
					++mValidTargets;
				}
				i->pos = modelPosition;
				return true;
			}
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	IKSolver::removeTarget(const TString& effectorType, index_t index)
	{
		EffectorTarget target = {effectorType, 0, Vector3::ZERO, false};

		//mTargets is already sorted
		TargetList::iterator i = std::lower_bound(mTargets.begin(), mTargets.end(), target);
		if( i != mTargets.end() && i+(indexdiff_t)index < mTargets.end() && !(target < *(i+(indexdiff_t)index)) )
		{
			i += (indexdiff_t)index;
			//targets with the same type should be sorted in group
			if( i->type == effectorType )
			{
				if( i->enabled )
				{
					i->enabled = false;
					--mValidTargets;
				}
				return true;
			}
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t	IKSolver::getEffectorCount(const TString& effectorType) const
	{
		size_t count = 0;
		for(size_t i = 0; i < mTargets.size(); ++i)
		{
			if( mTargets[i].type == effectorType )
				++count;
		}
		return count;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	IKSolver::initialize(const IK* _IK)
	{
		assert(IModelConfigManager::getSingleton().getIKConfiguration() != ModelConsts::IK_FULLBODY);
		mIK = _IK;
		mValidTargets = 0;

		if(_IK == NULL || _IK->getChainCount() == 0)
			return false;

		//add to set first to avoid vector re-allocation
		TargetSet set;
		for(size_t i = 0; i < _IK->getChainCount(); ++i)
		{
			const IK::Chain& chain = _IK->getChain(i);
			assert(chain.mEffectorIndex != INVALID_INDEX);
			const IK::Joint& effector = chain[chain.mEffectorIndex];
			const TString& type = IK::getEffectorType(effector.typeIndex);
			assert(!type.empty());
			EffectorTarget target = {type, i, Vector3::ZERO, false};
			set.insert(target);
		}

		mTargets.reserve( set.size() );
		mTargets.insert(mTargets.end(), set.begin(), set.end());
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	IKSolver::solve(DualQuaternion* bonePalette, const BoneActivity* boneAcitvity, const BONE_DATA* /*boneData*/, size_t count)
	{
		//BLADE_LW_PROFILING_FUNCTION();
		assert(IModelConfigManager::getSingleton().getIKConfiguration() != ModelConsts::IK_FULLBODY);

		if( mIK == NULL || mValidTargets == 0 )
			return false;

		for(index_t targetIndex = 0; targetIndex < mTargets.size(); ++targetIndex)
		{
			const EffectorTarget& effectorTarget = mTargets[targetIndex];
			if( !effectorTarget.enabled )
				continue;
			//clear data
			//effectorTarget.enabled = false;

			if( effectorTarget.index >= mIK->getChainCount() )
			{
				assert(false);
				continue;
			}

			//solve one chain
			const IK::Chain& chain = mIK->getChain(effectorTarget.index);
			if(chain.size() == 0 || chain[0].getIndex() >= count || chain.mEffectorIndex >= chain.size() )
			{
				assert(false);
				continue;
			}

			const BONE_DATA* endEffector = chain[ chain.mEffectorIndex ].bone;

			if( boneAcitvity != NULL && boneAcitvity[endEffector->mIndex] == 0)
				continue;

			//BLADE_LW_PROFILING_FUNCTION();

			//cache local (bone) space transform
			TempVector<DualQuaternion> localTransformCache( chain.size() );
			for(size_t i = 0; i < chain.size(); ++i)
			{
				assert( chain[i].getIndex() < count && (chain[i].getParent() < count || chain[i].getParent() == uint32(-1)) );
				if( boneAcitvity[chain[i].getIndex()] > 0 )
				{
					if( chain[i].getParent() == uint32(-1) )
						localTransformCache[i] = bonePalette[ chain[i].getIndex() ];
					else
					{
						assert( boneAcitvity[chain[i].getParent()] > 0 );
						localTransformCache[i] = bonePalette[ chain[i].getIndex() ] * bonePalette[ chain[i].getParent() ].getConjugate();
					}
				}
				else
					localTransformCache[i] = DualQuaternion::IDENTITY;
			}

			const Vector3& target = effectorTarget.pos;
			bool isDone = false;

			const scalar tolerance = MIN_DISTANCESQ*mIK->getSquaredSize();
			static const index_t MAX_ITERATION_COUNT = 20;
			for(index_t iteration = 0; iteration < MAX_ITERATION_COUNT; ++iteration)
			{
				//for each bone in the chain
				for(size_t i = chain.mEffectorIndex + 1; i < chain.size()-1; ++i)
				{
					if( !chain[i].isActive() || boneAcitvity[chain[i].getIndex()] == 0)
						continue;

					const BONE_DATA* bone = chain[i].bone;
					assert( bone->mIndex < count );

					//bone pos(model space)
					Vector3 bonePos = bonePalette[bone->mIndex].getTranslation();
					//current end effector pos(model space)
					Vector3 endPos = bonePalette[endEffector->mIndex].getTranslation();
					scalar distanceSQ = endPos.getSquaredDistance(target);
					// check if reach the target
					isDone = distanceSQ <= tolerance;
					if( isDone )
						break;

					Quaternion invParent = bonePalette[ chain[i].getParent() ].getRotation().getConjugate();
					//get local bone space vector
					Vector3 curVector = (endPos - bonePos) * invParent;
					Vector3 targetVector = (target - bonePos) * invParent;

					curVector.normalize();
					targetVector.normalize();

					scalar cosAngle = curVector.dotProduct(targetVector);
					// rotate to the same direction
					if( cosAngle < 1.0f )
					{
						//rotation axis
						Vector3 axis = curVector.crossProduct(targetVector);
						axis.normalize();
						//rotation the angle
						scalar angle = std::acos(cosAngle);

						//rotate the bone
						{
							Vector3 t = localTransformCache[i].getTranslation();
							//apply rotation
							Quaternion r = localTransformCache[i].getRotation() * Quaternion(axis, angle);

							const IK_CONSTRAINTS& constraints = chain[i].getConstraints();
							scalar yaw, pitch, roll;
							//get rotated pose
							r.getYawPitchRoll(yaw, pitch, roll);

							//apply constraints
							pitch = Math::Clamp(pitch, constraints.mMinX, constraints.mMaxX);
							yaw = Math::Clamp(yaw, constraints.mMinY, constraints.mMaxY);
							roll = Math::Clamp(roll, constraints.mMinZ, constraints.mMaxZ);

							//fix singular direction problem, and apply heuristic direction on constraints
							if( pitch == constraints.mMinX && constraints.mMinX > -5e-2f )
								pitch = constraints.mMaxX*1e-2f;
							else if( pitch == constraints.mMaxX && constraints.mMaxX < 5e-2f )
								pitch = constraints.mMinX*1e-2f;

							if( yaw == constraints.mMinY && constraints.mMinY > -5e-2f )
								yaw = constraints.mMaxY*1e-2f;
							else if( yaw == constraints.mMaxY && constraints.mMaxY < 5e-2f )
								yaw = constraints.mMinY*1e-2f;

							if( roll == constraints.mMinZ && constraints.mMinZ > -5e-2f )
								roll = constraints.mMaxZ*1e-2f;
							else if( roll == constraints.mMaxZ && constraints.mMaxZ < 5e-2f )
								roll = constraints.mMinZ*1e-2f;

							//final pose
							localTransformCache[i].set(Quaternion(yaw, pitch, roll), t);
						}

						//update transform to model space for this bone & all children, till end effector
						for(index_t j = i; (indexdiff_t)j >= (indexdiff_t)chain.mEffectorIndex; --j)
						{
							//don't update inactive bones during iteration, add a final pass to update it
							if( boneAcitvity[chain[j].getIndex()] > 0 && chain[j].isActive() )
								bonePalette[ chain[j].getIndex() ] = localTransformCache[j] * bonePalette[ chain[j].getParent() ];
						}
					}

				}//for each bone in the chain

			}//for each iteration

			//update inactive bones in final pass
			for(index_t j = chain.size()-1; (indexdiff_t)j >= 0; --j)
			{
				if( boneAcitvity[chain[j].getIndex()] > 0 && !chain[j].isActive() )
					bonePalette[ chain[j].getIndex() ] = localTransformCache[j] * bonePalette[ chain[j].getParent() ];
			}

		}//for each chain

		return true;
	}


	//////////////////////////////////////////////////////////////////////////
	FullBodyIKSolver::FullBodyIKSolver()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	FullBodyIKSolver::~FullBodyIKSolver()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	bool	FullBodyIKSolver::initialize(const IK* _IK)
	{
		assert(IModelConfigManager::getSingleton().getIKConfiguration() == ModelConsts::IK_FULLBODY);
		mIK = _IK;
		mValidTargets = 0;

		if(_IK == NULL || _IK->getChainCount() != 1)
			return false;

		const IK::Chain& chain = _IK->getChain(0);
		assert(chain.mEffectorIndex == INVALID_INDEX);

		//add to set first to avoid vector re-allocation
		TargetSet set;
		for(size_t i = 0; i < chain.size(); ++i)
		{
			const IK::Joint& joint = chain[i];
			if( !joint.isEffector() )
				continue;

			const TString& type = IK::getEffectorType(joint.typeIndex);
			assert(!type.empty());
			EffectorTarget target = {type, i, Vector3::ZERO, false};
			set.insert(target);
		}

		mTargets.reserve( set.size() );
		mTargets.insert(mTargets.end(), set.begin(), set.end());
		return true;
	}


	//////////////////////////////////////////////////////////////////////////
	///inspired by FABRIK's multiple effector solution, yet this is a CCD solution for multiple effectors
	//////////////////////////////////////////////////////////////////////////
	bool	FullBodyIKSolver::solve(DualQuaternion* bonePalette, const BoneActivity* boneAcitvity, const BONE_DATA* boneData, size_t count)
	{
		//BLADE_LW_PROFILING_FUNCTION();

		assert(IModelConfigManager::getSingleton().getIKConfiguration() == ModelConsts::IK_FULLBODY);
		if( mIK == NULL || mValidTargets == 0 || mIK->getChainCount() != 1)
			return false;

		//if one or more target set, we need solve the whole IK body
		typedef TempVector<const IK::Joint*> JointCache;
		typedef TempVector<const EffectorTarget*> MultipleTarget;
		typedef TempVector< MultipleTarget > IKTree;
		typedef TempVector<bool> UpdatedMask;

		const IK::Chain& chain = mIK->getChain(0);

		TempVector<DualQuaternion> localTransformCache( count );
		for(size_t i = 0; i < count; ++i)
		{
			if( boneAcitvity[i] == 0 )
				continue;

			if( boneData[i].mParent == uint32(-1) )
				localTransformCache[i] = bonePalette[i];
			else
			{
				assert( boneAcitvity[i] > 0 );
				localTransformCache[i] = bonePalette[i] * bonePalette[ boneData[i].mParent ].getConjugate();
			}
		}

		IKTree initialTree( chain.size() );
		size_t realValidCount = 0;
		for(size_t i = 0; i < mTargets.size(); ++i)
		{
			EffectorTarget& target = mTargets[i];
			assert( target.index < chain.size() );
			if(boneAcitvity[chain[target.index].getIndex()] > 0)
			{
				++realValidCount;
				const IK::Joint& effector = chain[target.index];

				//if effect's target is not set, use animation's pos as a locked position, TODO:
				if(!target.enabled)
					target.pos = bonePalette[ effector.getIndex() ].getTranslation();

				initialTree[effector.getJointParent()].push_back( &target );
			}
		}

		if( realValidCount == 0 )
			return false;

		//TempVector<scalar> distances(mTargets.size(), FLT_MAX);
		static const index_t MAX_ITERATION_COUNT = 15;
		const scalar tolerance = (MIN_DISTANCE*MIN_DISTANCE)*(mIK->getSquaredSize());
		for(index_t iIter = 0; iIter < MAX_ITERATION_COUNT; ++iIter)
		{
			IKTree currentTree = initialTree;
			IKTree subBaseTree( chain.size() );

			//for each effector, trace it to a sub base
			//after all effector is calculated to a sub base, restart from the sub bases
			for(size_t branchIndex = 0; branchIndex < currentTree.size(); ++branchIndex)
			{
				const MultipleTarget& branch = currentTree[branchIndex];

				if( branch.size() > 0 )
				{
					assert(chain[branchIndex].isActive());

					JointCache subChainCache;
					subChainCache.reserve( branch.size()*8 );

					//note: for a sub base, all children joints of the sub base will be 
					//affected by sub base's change
					//put all active IK chains with effectors to subChainCache for update
					for(size_t i = 0; i < branch.size(); ++i)
					{
						const EffectorTarget& effectorTarget = *branch[i];
						const IK::Joint& effector = chain[ effectorTarget.index ];
						assert(effector.isActive());
						uint16 jointIndex = effector.getJointIndex();
						while( jointIndex != branchIndex)
						{
							subChainCache.push_back( &chain[jointIndex] );
							jointIndex = chain[jointIndex].getJointParent();
							assert(jointIndex != uint16(-1) );
						}
					}

					uint16 jointIndex = (uint16)branchIndex;
					uint16 parent = chain[branchIndex].getJointParent();
					while( parent != uint16(-1) )
					{
						const IK::Joint& joint = chain[jointIndex];
						subChainCache.push_back(&joint);
						IPlatformManager::prefetch<PM_READ>(&bonePalette[joint.getParent()]);

						const BONE_DATA* bone = joint.bone;
						assert( bone->mIndex < count );

						Quaternion rotation = Quaternion::IDENTITY;
						for(size_t i = 0; i < branch.size(); ++i)
						{
							const EffectorTarget& effectorTarget = *branch[i];
							const IK::Joint& effector = chain[ effectorTarget.index ];
							assert(effector.getIndex() != bone->mIndex );
							IPlatformManager::prefetch<PM_READ>(&bonePalette[effector.getIndex()]);

							//note: if effector has no target, it doesn't means we don't need update it, since
							//it will still affected a sub base, which is affected by sibling chains.
							//if( !effectorTarget.enabled )
							//	continue;

							const Vector3& target = effectorTarget.pos;
							Quaternion rot = Quaternion::IDENTITY;

							//bone pos(model space)
							Vector3 bonePos = bonePalette[bone->mIndex].getTranslation();
							//current end effector pos(model space)
							Vector3 endPos = bonePalette[effector.getIndex()].getTranslation();
							scalar distanceSQ = endPos.getSquaredDistance(target);
							// check if reach the target
							if( distanceSQ > tolerance )
							{
								Quaternion invParent = bonePalette[ joint.getParent() ].getRotation().getConjugate();
								//get local bone space vector
								Vector3 curVector = (endPos - bonePos) * invParent;
								Vector3 targetVector = (target - bonePos) * invParent;

								curVector.normalize();
								targetVector.normalize();

								scalar cosAngle = curVector.dotProduct(targetVector);
								//rotate to the same direction
								if( cosAngle < 1.0f )
								{
									//rotation axis
									Vector3 axis = curVector.crossProduct(targetVector);
									axis.normalize();
									//rotation the angle
									scalar angle = std::acos(cosAngle);
									rot = Quaternion(axis, angle);
								}
							}
							//average rotation for each effector (slerp for multiple quaternions with same weight)
							rotation.slerpWith(rot, 1.0f/(scalar)(i+1), true);
						}//for each effector in the branch							

						Vector3 t = localTransformCache[joint.getIndex()].getTranslation();
						Quaternion r = localTransformCache[joint.getIndex()].getRotation() * rotation;
						scalar yaw, pitch, roll;
						r.getYawPitchRoll(yaw, pitch, roll);
						const IK_CONSTRAINTS& constraints = joint.getConstraints();
						pitch = Math::Clamp(pitch, constraints.mMinX, constraints.mMaxX);
						yaw = Math::Clamp(yaw, constraints.mMinY, constraints.mMaxY);
						roll = Math::Clamp(roll, constraints.mMinZ, constraints.mMaxZ);

						//fix singular direction problem, and apply heuristic direction on constraints
						if( pitch == constraints.mMinX && constraints.mMinX > -5e-2f )
							pitch = constraints.mMaxX*1e-2f;
						else if( pitch == constraints.mMaxX && constraints.mMaxX < 5e-2f )
							pitch = constraints.mMinX*1e-2f;

						if( yaw == constraints.mMinY && constraints.mMinY > -5e-2f )
							yaw = constraints.mMaxY*1e-2f;
						else if( yaw == constraints.mMaxY && constraints.mMaxY < 5e-2f )
							yaw = constraints.mMinY*1e-2f;

						if( roll == constraints.mMinZ && constraints.mMinZ > -5e-2f )
							roll = constraints.mMaxZ*1e-2f;
						else if( roll == constraints.mMaxZ && constraints.mMaxZ < 5e-2f )
							roll = constraints.mMinZ*1e-2f;

						localTransformCache[joint.getIndex()].set(Quaternion(yaw, pitch, roll), t);

						//update transform to model space for this bone & all children, till end effector
						for(index_t i = subChainCache.size()-1; (indexdiff_t)i >= 0; --i)
						{
							//don't update inactive bones during iteration, add a final pass to update it
							uint32 boneIndex = subChainCache[i]->getIndex();
							if( boneAcitvity[boneIndex] > 0 )
							{
								IPlatformManager::prefetch<PM_WRITE>(&localTransformCache[boneIndex]);

								if( subChainCache[i]->getParent() != uint32(-1) )
									bonePalette[ subChainCache[i]->getIndex() ] = localTransformCache[boneIndex] * bonePalette[ subChainCache[i]->getParent() ];
								else
									bonePalette[ subChainCache[i]->getIndex() ] = localTransformCache[boneIndex];
							}
						}

						//each to sub base
						const IK::Joint& parentJoint = chain[parent];
						assert(parentJoint.isActive() && parentJoint.getIndex() == joint.getParent() && parentJoint.getJointIndex() == joint.getJointParent());
						if( parentJoint.isSubBase() )
						{
							//merge branch to sub base
							MultipleTarget& subBaseTarget = subBaseTree[ parent ];
							subBaseTarget.insert(subBaseTarget.end(), branch.begin(), branch.end());
							break;
						}
						jointIndex = parent;
						parent = parentJoint.getJointParent();
					}//while branch not reach sub base

				}//if( branch.size() > 0 )

				//switch to subBase tree
				if(branchIndex == currentTree.size() - 1)
				{
					currentTree = subBaseTree;
					bool doSwtich = false;
					for(size_t i = 0; i < subBaseTree.size(); ++i)
					{
						if( subBaseTree[i].size() > 0 )
							doSwtich = true;
						subBaseTree[i].clear();
					}

					if( doSwtich )
						branchIndex = (size_t)-1;
				}

			}//for each branch in the tree

			//check if each effector reaches target
			bool bContinue = false;
			for(size_t i = 0; !bContinue && i < mTargets.size(); ++i)
			{
				const IK::Joint& effector = chain[mTargets[i].index];
				scalar distanceSQ = bonePalette[effector.getIndex()].getTranslation().getSquaredDistance(mTargets[i].pos);

				// check if reach the target
				if(distanceSQ > tolerance /*&& distanceSQ > distances[i]*/)
					bContinue = true;

				//distances[i] = distanceSQ;
			}

			if( !bContinue )
				break;

		}//for each iteration

		UpdatedMask	updateMask(count);
		for(size_t i = 0; i < chain.size(); ++i)
			updateMask[ chain[i].getIndex() ] = true;

		//update whole skeleton for in-active joints
		for(size_t i = 0; i < count; ++i)
		{
			if( boneAcitvity[i] == 0 || updateMask[i] )
				continue;
			IPlatformManager::prefetch<PM_WRITE>(&localTransformCache[i]);

			if( boneData[i].mParent != uint32(-1) )
				bonePalette[i] = localTransformCache[i] * bonePalette[ boneData[i].mParent ];
			else
				bonePalette[i] = localTransformCache[i];
		}
		return true;
	}
	
}//namespace Blade