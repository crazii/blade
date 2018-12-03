/********************************************************************
	created:	2015/11/27
	filename: 	IK.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include "BladePCH.h"
#include "IK.h"

namespace Blade
{
	StaticList<TString>	IK::msEffectorTypes;

	//////////////////////////////////////////////////////////////////////////
	void			IK::initEffectorTypes()
	{
		if( msEffectorTypes.size() == 0)
		{
			msEffectorTypes.push_back(ModelConsts::EFFECTOR_HAND);
			msEffectorTypes.push_back(ModelConsts::EFFECTOR_FOOT);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	size_t			IK::getEffectorTypeCount()
	{
		IK::initEffectorTypes();
		return msEffectorTypes.size();
	}

	//////////////////////////////////////////////////////////////////////////
	const TString&	IK::getEffectorType(index_t index)
	{
		IK::initEffectorTypes();
		if( index < msEffectorTypes.size() )
		{
			TypeList::iterator i = msEffectorTypes.begin();
			std::advance(i, index);
			return *i;
		}
		else
		{
			assert(false);
			return TString::EMPTY;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	IKChainBuilder::IKChainBuilder()
	{
		assert(IModelConfigManager::getSingleton().getIKConfiguration() != ModelConsts::IK_NONE);
		mEnableHand = IModelConfigManager::getSingleton().getIKConfiguration() != ModelConsts::IK_FOOT;
		mFullBody = IModelConfigManager::getSingleton().getIKConfiguration() == ModelConsts::IK_FULLBODY;
	}

	//////////////////////////////////////////////////////////////////////////
	IKChainBuilder::~IKChainBuilder()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	bool	IKChainBuilder::buildIKChians(const BoneTable& boneTable, IK& chainSet)
	{
		chainSet.mChains.clear();
		size_t count = boneTable.mBoneCount;

		if( count == 0)
			return false;

		const BONE_DATA* bones = boneTable.mBoneData;

		//calculate skeleton bounds
		Vector3 min(FLT_MAX, FLT_MAX, FLT_MAX), max(-FLT_MAX, -FLT_MAX, -FLT_MAX);
		TempVector<size_t> childCounter(count, 0);	//counter for direct children
		for(size_t i = 0; i < count; ++i)
		{
			const BONE_DATA* bone = bones + i;
			min = Vector3::getMinVector(min, bone->mInitialPose.getTranslation());
			max = Vector3::getMaxVector(max, bone->mInitialPose.getTranslation());

			if( IKChainBuilder::isEffector(bone, boneTable) )
				this->countChainReference(&childCounter[0], bone, bones, count);
		}
		chainSet.mSquaredSize = max.getSquaredDistance(min);

		//collect chain info
		EffectorMap effectorMap;

		for(size_t i = 0; i < count; ++i)
		{
			const BONE_DATA* bone = bones + i;
			uint32 effectorType = (uint32)-1;
			if( IKChainBuilder::isEffector(bone, boneTable, &effectorType) )
			{
				if( mFullBody )
				{
					//full body need merge all chains into one, so that it contains multiple end effector
					//there's only one chain(full skeleton) in full body mode.
					JointSet& jointSet = effectorMap[NULL];
					std::pair<JointSet::iterator,bool> result = jointSet.insert(IK::Joint(bone, IKF_ACTIVE|IKF_EFFECTOR, effectorType));
					assert(result.second); BLADE_UNREFERENCED(result);
					//collect active joints
					this->collectJoints(bone, jointSet, bones, &childCounter[0], count);
				}
				else
				{
					JointSet& jointSet = effectorMap[bone];
					jointSet.insert(IK::Joint(bone, IKF_ACTIVE|IKF_EFFECTOR, effectorType));
					this->collectJoints(bone, jointSet, bones, &childCounter[0], count);
				}
			}
		}

		//if( mFullBody )//extra pass to collect all joints except active joints
		//{
		//	JointSet& jointSet = effectorMap[NULL];
		//	this->collectChildJoints(bones+0, jointSet, bones, count); //0 element in bone array is the root bone
		//}

		//generate chains
		for(EffectorMap::iterator i = effectorMap.begin(); i != effectorMap.end(); ++i)
		{
			const BONE_DATA* bone = i->first;
			const JointSet& set = i->second;

			chainSet.mChains.push_back( IK::Chain() );
			IK::Chain& chain = chainSet.mChains.back();

			chain.resize(set.size());
			index_t index = 0;
			for(JointSet::const_iterator j = set.begin(); j != set.end(); ++j)
			{
				const IK::Joint& joint = *j;

				joint.setJointIndex( (uint16)index);
				if( joint.bone->mParent != (uint32)-1 )
				{
					JointSet::const_iterator parent = set.find(IK::Joint(bones+joint.bone->mParent, IKF_NONE));
					if( parent != set.end() )
						joint.setJointParent( (uint16)std::distance(set.begin(), parent) );
				}
				assert(joint.jointIndex != joint.jointParent);

				chain[index] = joint;
				if( joint.bone == bone )
					chain.mEffectorIndex = index;
				index++;
			}
		}

		//TODO: fix constraints: currently constraints are sampled from skeleton animations,
		//if animations are too simple, then constraints may be too limited.
		//i.e. for a series of animations which don't have a bent knee, 
		//its constraints are limited to be narrow angles / straight too, and IK will never bent knee.

		return chainSet.mChains.size() > 0;
	}

	//////////////////////////////////////////////////////////////////////////
	void	IKChainBuilder::countChainReference(size_t* childCounting, const BONE_DATA* effector, const BONE_DATA* boneData, size_t count)
	{
		BLADE_UNREFERENCED(count);
		uint32 parent = effector->mParent;

		while( parent != (uint32)-1 )
		{
			assert(parent < count);
			++childCounting[parent];
			parent = boneData[parent].mParent;
		}
	}
	
	//////////////////////////////////////////////////////////////////////////
	void	IKChainBuilder::collectChildJoints(const BONE_DATA* bone, JointSet& set, const BONE_DATA* boneData, size_t count)
	{
		for(size_t i = 0; i < count; ++i)
		{
			const BONE_DATA* currentBone = boneData + i;
			if( currentBone->mParent == bone->mIndex )
			{
				assert( currentBone->mParent != (uint32)-1 );
				assert( currentBone != bone );
				std::pair<JointSet::iterator, bool> result = set.insert(IK::Joint(currentBone, IKF_NONE));
				if( result.second || mFullBody ) //don't continue if bone exists
				{
					this->collectChildJoints(currentBone, set, boneData, count);
				}
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void	IKChainBuilder::collectJoints(const BONE_DATA* effector, JointSet& set, const BONE_DATA* boneData, const size_t* childCounting, size_t count)
	{
		//collect children: not active
		if( !mFullBody )//full body only collect active joints
			this->collectChildJoints(effector, set, boneData, count);

		//collect parent: active
		uint32 parent = effector->mParent;
		const BONE_DATA* bone = effector;
		while( parent != (uint32)-1 )
		{
			assert((size_t)parent < count );
			const BONE_DATA* parentBone = boneData+parent;
			assert(parentBone->mIndex == parent);
			std::pair<JointSet::iterator,bool> result = set.insert(IK::Joint(parentBone, IKF_ACTIVE));
			parent = boneData[parent].mParent;

			assert(childCounting != NULL);
			if( childCounting[parentBone->mIndex] > 1 )
			{
				//stop at parent shared by IK chains
				if( !mFullBody )
					break;
				else if( result.second && childCounting[parentBone->mIndex] > childCounting[bone->mIndex] )
					result.first->raiseFlags(IKF_SUBBASE);
			}

			//note: bone is already added to set, collector siblings: not active
			if( !mFullBody )//full body only collect active joints
				this->collectChildJoints(parentBone, set, boneData, count);

			bone = parentBone;
		}
	}

	
}//namespace Blade