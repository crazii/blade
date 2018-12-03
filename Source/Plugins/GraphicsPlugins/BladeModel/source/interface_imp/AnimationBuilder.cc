/********************************************************************
	created:	2015/09/07
	filename: 	AnimationBuilder.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "AnimationBuilder.h"
#include <utility/StringTable.h>
#include "Bone.h"
#include "AnimationResource.h"

#define ANIM_EXPORT_SPEED_OPTIMIZE 1
#define OPTIMIZE_FRAME 1
#define INCREASE_ERROR_ON_DEPTH 0

namespace Blade
{
	const TString IAnimationBuilder::ANIMATION_BUILDER_CLASS = BTString("AnimBuilder");
	template class Factory<IAnimationBuilder>;

	/************************************************************************/
	/* IAnimationBuilder interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool	AnimationBuilder::buildAnimation(IAnimationCollector* collector, ISkeletonResource* target, ProgressNotifier& callback)
	{
		if( collector == NULL || target == NULL )
		{
			assert(false);
			return false;
		}

		size_t boneCount = collector->getBoneCount();
		const BONE_DATA* bones = collector->getBones();

		if( bones == NULL || boneCount == 0)
		{
			assert(false);
			return false;
		}

		BoneList boneData(boneCount);
		std::memcpy(&boneData[0], bones, sizeof(BONE_DATA)*boneCount);
	
		//skip update index: index is used by collector, we will update it later
		BoneIDList boneOldIndices(boneCount);
		BoneTable::sortBoneData(&boneData[0], boneCount, true, &boneOldIndices[0]);

		TStringParam names;
		names.reserve( boneCount );
		for(size_t i = 0; i < boneCount; ++i)
			names.push_back( collector->getBoneName( boneOldIndices[i] ) );

		TempVector<IK_CONSTRAINTS> constraints( boneCount );

		bool ret = target->initSkeleton(&boneData[0], boneCount, names.getBuffer(), names.size(), (uint)mFrequency);
		assert(ret);

		size_t animationCount = collector->getAnimationCount();
		BoneDQ transform;
		//export animations(sets of key frames):
		for(size_t animIndex = 0; animIndex < animationCount; ++animIndex)
		{
			callback.beginStep( 1/scalar(animationCount)  );
			
			const AnimationDef& animDef = *collector->getAnimationDef(animIndex);
			const TString& name = animDef.Name;
			uint32 startFrame = animDef.StartFrame;
			uint32 endFrame = animDef.EndFrame;
			bool loop = animDef.Loop;
			//BLADE_LOG(DebugOutput, BTString("build animation:") << name);

			collector->setCurrentAnimation(name);
			scalar startTime = collector->getFrameTime(startFrame);
			scalar endTime = collector->getFrameTime(endFrame);
			//note: animation length is longer than last key frame time pos, to add small duration for last key frame.
			double duration = (double)endTime - (double)startTime;
			size_t maxBoneDepth = 0;

			BoneKeyframeList keyFrames;
			BoneKeyCountList keyCountList;
			keyCountList.resize( boneCount, 0);
			TimeKeyFrameList allKeyFrames;

			//collect key frame times
			KeyTimeList timeList;

			//init constant sampling rate interval for key frames
			scalar samplingInterval = 1/mFrequency;
			for(scalar t = startTime; t <= endTime;)
			{
				timeList.push_back(t);
				t += samplingInterval;
				if( t > endTime && t < endTime + samplingInterval )
					timeList.push_back(endTime);
			}

#if ANIM_EXPORT_SPEED_OPTIMIZE
			callback.beginStep( 0.1f );
#else
			callback.beginStep( 0.8f );
#endif
			for(size_t i = 0; i < boneCount; ++i)
			{
				callback.onStep(i, boneCount);

				{
					size_t boneDepth = 0;
					const BONE_DATA* node = &boneData[i];
					while( node->mParent != (uint32)-1 )
					{
						++boneDepth;
						node = &boneData[node->mParent];
					}
					if( boneDepth > maxBoneDepth )
						maxBoneDepth = boneDepth;
				}

				//export key frames using collected key times
				for(KeyTimeList::const_iterator iter = timeList.begin(); iter != timeList.end(); ++iter)
				{
#if ANIM_EXPORT_SPEED_OPTIMIZE
					keyFrames.push_back( KeyFrame() );
					allKeyFrames[ *iter ].push_back( std::make_pair(&keyFrames.back(), i) );
#else
					collector->getBoneTransform(transform, boneOldIndices[i], *iter);
					keyFrames.push_back( KeyFrame(transform, *iter) );
#endif
				}
				keyCountList[i] = (uint16)timeList.size();
			}//for each bone

#if ANIM_EXPORT_SPEED_OPTIMIZE
			callback.beginStep( 0.8f );
			size_t step = 0;
			for( TimeKeyFrameList::iterator i = allKeyFrames.begin(); i != allKeyFrames.end(); ++i)
			{
				callback.onStep(step++, allKeyFrames.size() );

				scalar t = i->first;
				BoneKeyframePtrList& list = i->second;

				double time = (double)t - (double)startTime;
				for(size_t j = 0; j < list.size(); ++j)
				{
					BoneKeyframePtrList::value_type type = list[j];
					KeyFrame* kf = type.first;
					index_t boneIndex = type.second;
					collector->getBoneTransform(transform, boneOldIndices[boneIndex], t);
					*kf = KeyFrame(transform, (scalar)time);
				}
			}
#endif
			callback.beginStep( 0.1f );

			//optimize out key frames that can be interpolated from neighbors
			//adjust threshold based on node depth, deeper nodes have larger threshold(deeper nodes don't need much precision)
			//this will save about 20%~50% space for animation files

#if OPTIMIZE_FRAME

			ErrorList errorDepth;
			errorDepth.resize(maxBoneDepth+1u);
			//accumulated error
			float accumAngle = Math::Degree2Radian(mAngleError);
			float accumPos = mPosError;

#if INCREASE_ERROR_ON_DEPTH
			for(size_t i = 0; i < maxBoneDepth; ++i)
				errorDepth[i] = (scalar)i+1;
			float initAngle = accumAngle / (maxBoneDepth*(maxBoneDepth+1)/2);
			float initPos = accumPos / (maxBoneDepth*(maxBoneDepth+1)/2);
#else
			for(size_t i = 0; i < maxBoneDepth; ++i)
				errorDepth[i] = 1;
			float initAngle = accumAngle / maxBoneDepth;
			float initPos = accumPos / maxBoneDepth;
#endif

			BoneKeyframeList::iterator  start = keyFrames.begin();
			for(index_t i = 0; i < boneCount; ++i)
			{
				callback.onStep(i, boneCount);

				//calculate depth
				size_t boneDepth = 0;
				{
					const BONE_DATA* node = &boneData[i];
					while( node->mParent != (uint32)-1 )
					{
						++boneDepth;
						node = &boneData[node->mParent];
					}
				}

				size_t keyCount = keyCountList[i];
				BoneKeyframeList::iterator iter = start;
				assert(keyCount > 0);
				++iter;
				for(size_t index = 1; index+1 < keyCount; )
				{
					//assert( std::distance(start, iter) == index ); //debug too slow, uncomment if needed
					float angleThreshold = initAngle * errorDepth[boneDepth];
					float posThreshold = initPos * errorDepth[boneDepth];

					const KeyFrame& kf = *iter;
					const KeyFrame& prev = *(--iter);
					++iter;
					const KeyFrame& next = *(++iter);
					--iter;
					scalar t = (kf.getTimePos() - prev.getTimePos()) / (next.getTimePos() - prev.getTimePos());
					assert( t > 0 && t < 1); //possibly iter position error across two bone key frame sequences
					BoneDQ interpolated = prev.getTransform();
					interpolated.sclerpWith(next.getTransform(), t, true);
					interpolated.normalize();

					const BoneDQ& dq = kf.getTransform();
					if( interpolated.real.equal(dq.real, angleThreshold)
						&& interpolated.getTranslation().equal(dq.getTranslation(), posThreshold)
						)
					{
						iter = keyFrames.erase(iter);
						--keyCount;
					}
					else
					{
						++iter;
						++index;
					}
				}
				keyCountList[i] = (uint16)keyCount;
				if( keyCount >= 2 )
					start = ++iter;
				else
					start = iter;
			}
#endif
			BoneKeyframeArray keyFrameArray( keyFrames.begin(), keyFrames.end() );

			uint32 flags = 0;
			if( loop )
				flags |= AF_LOOP;

			//calculate longest bone chain
			scalar maxSize = 0;
			for(int i = (int)boneCount-1; i >= 0; --i)
			{
				scalar chainSize = 0;
				BoneDQ dq = boneData[(index_t)i].mInitialPose;
				uint32 index = boneData[(index_t)i].mParent;
				while( index != (uint32)-1 )
				{
					const BONE_DATA& parent = boneData[(index_t)index];
					chainSize += parent.mInitialPose.getTranslation().getDistance( dq.getTranslation() );

					index = parent.mParent;
					dq = parent.mInitialPose;
				}
				if( maxSize < chainSize )
					maxSize = chainSize;
			}

			//maxSize *= 0.5;
			//check if translation is too large
			const BoneDQ& rootDQ = boneData[0].mInitialPose;
			size_t rootKeyCount = (size_t)keyCountList[0];
			for(size_t i = 0; i < rootKeyCount; ++i)
			{
				const BoneDQ& rootKeyDQ = keyFrameArray[i].getTransform();
				Vector3 translation = rootKeyDQ.getTranslation() - rootDQ.getTranslation();
				if( translation.getLength() > maxSize )
					flags |= AF_TRANSLATION;
			}

			size_t index = 0;
			for(size_t i = 0; i < boneCount; ++i)
			{
				scalar initPitch = 0, initYaw = 0, initRoll = 0;
				//get binding pose's rotation
				//boneData[i].mInitialPose.getRotation().getYawPitchRoll(initYaw, initPitch, initRoll);
				//get key frames' rotation
				for(size_t j = 0; j < keyCountList[i]; ++j)
				{
					const BoneDQ& keyDQ = keyFrameArray[index++].getTransform();
					scalar yaw, pitch, roll;
					keyDQ.real.getYawPitchRoll(yaw, pitch, roll);
					scalar minPitch = std::min(pitch, initPitch);
					scalar maxPitch = std::max(pitch, initPitch);
					scalar minYaw = std::min(yaw, initYaw);
					scalar maxYaw = std::max(yaw, initYaw);
					scalar minRoll = std::min(roll, initRoll);
					scalar maxRoll = std::max(roll, initRoll);
					constraints[i].merge( IK_CONSTRAINTS(minPitch, maxPitch, minYaw, maxYaw, minRoll, maxRoll) );
				}
			}

			ret = target->addAnimation(name, (scalar)duration, flags, &keyFrameArray[0], &keyCountList[0]);
			assert(ret);

		}//for each animation

		ret = target->setIKContraints(&constraints[0], boneCount);
		assert(ret);

		return true;
	}

	
}//namespace Blade