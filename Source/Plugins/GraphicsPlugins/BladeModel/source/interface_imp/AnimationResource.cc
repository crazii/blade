/********************************************************************
	created:	2014/02/25
	filename: 	AnimationResource.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "AnimationResource.h"
#include <interface/public/ISerializer.h>

namespace Blade
{
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	AnimationResource::AnimationResource(const TString& name, scalar time, scalar frameTime, uint32 flags, size_t boneCount, const KeyFrame* AnimationResourceKeys, const uint16* boneKeyCount)
		:mBoneSequences(NULL)
		,mKeyFrames(NULL)
		,mFrameCount(0)
		,mTime(time)
		,mFrameTime(frameTime)
		,mName(name)
	{
#if BLADE_DEBUG
		mBoneCount = boneCount;
#endif
		mFlags = flags;

		if( boneKeyCount != NULL && AnimationResourceKeys != NULL && boneCount != 0 )
		{
			this->reset(boneCount, AnimationResourceKeys, boneKeyCount);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	AnimationResource::~AnimationResource()
	{
		BLADE_DELETE[] mBoneSequences;
		BLADE_DELETE[] mKeyFrames;
	}

	//////////////////////////////////////////////////////////////////////////
	void	AnimationResource::reset(size_t boneCount, const KeyFrame* AnimationResourceKeys, const uint16* boneKeyCount)
	{
		BLADE_DELETE[] mBoneSequences;
		BLADE_DELETE[] mKeyFrames;

#if BLADE_DEBUG
		mBoneCount = boneCount;
#endif

		mFrameCount = 0;
		for(size_t i = 0; i < boneCount; ++i)
			mFrameCount += boneKeyCount[i];

		mKeyFrames = BLADE_NEW KeyFrame[mFrameCount];
		std::memcpy(mKeyFrames, AnimationResourceKeys, mFrameCount*sizeof(KeyFrame) );

		mBoneSequences = BLADE_NEW KeySequence[boneCount];
		KeyFrame* boneKeys = mKeyFrames;
		for(size_t i = 0; i < boneCount; ++i)
		{
			size_t keyframeCount = boneKeyCount[i];	//key frame count for this bone
			mBoneSequences[i] = KeySequence( boneKeys, keyframeCount, i);
			boneKeys += keyframeCount;
		}
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool		AnimationTable::initialize(size_t boneCount, uint32 FPS)
	{
		if( mBoneCount == 0 || mBoneCount == boneCount )
		{
			mFPS = (uint32)FPS;
			mBoneCount = boneCount;
			return true;
		}
		assert(false);
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		AnimationTable::clear()
	{
		mBoneCount = 0;
		mFPS = 0;
		mMap.clear();
		mNames.clear();
		for(size_t i = 0; i < mAnimationResources.size(); ++i)
		{
			AnimationResource* animation = mAnimationResources[i];
			BLADE_DELETE animation;
		}
		mAnimationResources.clear();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	AnimationResource*	AnimationTable::addAnimationResource(const TString& name, float duration, uint32 flags, const KeyFrame* keyframes, const uint16* boneKeyCounts, size_t boneCount)
	{
		assert( mAnimationResources.size() == mMap.size() );
		assert( mNames.size() == mMap.size() );
		assert( mBoneCount != 0 );
		assert( mBoneCount == boneCount );
		BLADE_UNREFERENCED(boneCount);

		std::pair<AnimationResourceMap::iterator, bool> ret = mMap.insert( std::make_pair(name, (AnimationResource*)NULL) );
		if( !ret.second)
		{
			//already exist with the same name
			assert(false);
			return NULL;
		}

		mNames.push_back(name);

		AnimationResource* Animation = BLADE_NEW AnimationResource(name, duration, scalar(1)/(scalar)mFPS, flags, mBoneCount, keyframes, boneKeyCounts);
		mAnimationResources.push_back(Animation);
		ret.first->second = Animation;
		return Animation;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		AnimationTable::setAnimationInfo(AnimationResource* res, const TString& newName, const uint32* flags)
	{
		if(res == NULL )
		{
			assert(false);
			return false;
		}

		if( this->getAnimationResource(res->mName) != res)
		{
			assert(false);
			return false;
		}
		if( newName == res->getName() )
			return true;

		AnimationResource* newSlot = mMap[newName];
		if( newSlot != NULL )
		{
			assert(false);
			return false;
		}

		if(mMap.erase(res->mName) == 1)
		{
			mNames.erase( std::find(mNames.begin(), mNames.end(), res->mName) );
			newSlot = res;
			res->mName = newName;
			if( flags != NULL )
				res->mFlags = *flags;
			mNames.push_back(newName);
			return true;
		}
		else
		{
			assert(false);
			mMap.erase(newName);
			return false;
		}
	}

	static const uint32 ANIMATIONRESOURCE_TABLE_MAGIC = BLADE_FCC('B','L','A','T');
	typedef struct BoneTransformFormat : public TempAllocatable
	{
		int16	rotation_i16x3n[3];
		fp16	position_f16x3[3];
		fp32	time_pos;
		//uint32 frame_id;
	}BT_FMT;
	static_assert(sizeof(BT_FMT) == 16, "size/ailgnment error!");

	//////////////////////////////////////////////////////////////////////////
	bool		AnimationTable::saveAnimationResource(const HSTREAM& stream) const
	{
		if( stream != NULL && stream->isValid() && (stream->getAccesMode()&IStream::AM_WRITE) )
		{
			BLADE_SERIALIZE_CHECK(0 != mBoneCount);
			BLADE_SERIALIZE_CHECK( mAnimationResources.size() == mMap.size() );
			BLADE_SERIALIZE_CHECK( mNames.size() == mMap.size() );

			bool ret = stream->writeAtom(ANIMATIONRESOURCE_TABLE_MAGIC) == sizeof(ANIMATIONRESOURCE_TABLE_MAGIC);
			BLADE_SERIALIZE_CHECK(ret);

			ret = StringTable::writeStrings(stream, mNames);
			BLADE_SERIALIZE_CHECK(ret);

			for(size_t ir = 0; ir < mAnimationResources.size(); ++ir)
			{
				AnimationResource* animation = mAnimationResources[ir];
				uint32 frameCount = (uint32)animation->mFrameCount;
				ret = stream->writeAtom(frameCount) == sizeof(frameCount);
				BLADE_SERIALIZE_CHECK(ret);

				fp32 time = animation->mTime;
				ret = stream->writeAtom(time) == sizeof(time);
				BLADE_SERIALIZE_CHECK(ret);
				uint32 flags = (uint32)animation->mFlags.getMask();
				ret = stream->writeAtom(flags) == sizeof(flags);
				BLADE_SERIALIZE_CHECK(ret);

				{
					uint16* boneKeyCountTable = BLADE_TMP_ALLOCT(uint16, mBoneCount);
					for(size_t i = 0; i < mBoneCount; ++i)
					{
						if( animation->mBoneSequences[i].getFrameCount() > (size_t)std::numeric_limits<uint16>::max() )
							BLADE_EXCEPT(EXC_OUT_OF_RANGE, BTString("too larget data."));

						boneKeyCountTable[i] = (uint16)animation->mBoneSequences[i].getFrameCount();
					}
					ret = (size_t)stream->write(boneKeyCountTable, mBoneCount) == mBoneCount*sizeof(uint16);
					BLADE_TMP_FREE(boneKeyCountTable);
				}

				size_t transformCount = frameCount;
				BT_FMT* transformFmt = BLADE_NEW BT_FMT[transformCount];
				for(size_t i = 0; i < transformCount; ++i)
				{
					const KeyFrame& keyframe = animation->mKeyFrames[i];
					BoneDQ transform = keyframe.getTransform();
					transform.normalize();
					Quaternion rotation = transform.real;
					Vector3 translation = transform.getTranslation();
					if( rotation.w < 0 )		//we are gonna drop .w, and re-construct on loading, so make it always > 0
						rotation = -rotation;	//-quat has exactly the same effect of quat

					transformFmt[i].rotation_i16x3n[0] = short(rotation.x * 32767);
					transformFmt[i].rotation_i16x3n[1] = short(rotation.y * 32767);
					transformFmt[i].rotation_i16x3n[2] = short(rotation.z * 32767);

					transformFmt[i].position_f16x3[0] = fp16( translation[0] );
					transformFmt[i].position_f16x3[1] = fp16( translation[1] );
					transformFmt[i].position_f16x3[2] = fp16( translation[2] );

					transformFmt[i].time_pos = keyframe.getTimePos();
				}

				IStream::Size bytes = (IStream::Size)(sizeof(BT_FMT)*transformCount);
				ret = stream->writeData( transformFmt, bytes) == bytes;
				BLADE_DELETE[] transformFmt;
				BLADE_SERIALIZE_CHECK(ret);
			}
			return true;
		}
		assert(false);
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		AnimationTable::loadAnimationResource(const HSTREAM& stream, size_t boneCount, size_t& totalFrameCount)
	{
		assert( mNames.size() == 0 );
		assert( mAnimationResources.size() == 0 );
		assert( mMap.size() == 0 );
		totalFrameCount = 0;

		typedef TempVector<BT_FMT> TransformCache;
		typedef TempVector<uint16> BoneFrameCountTable;
		typedef TempVector<KeyFrame> KeyframeCache;

		if( stream != NULL && stream->isValid() && (stream->getAccesMode()&IStream::AM_READ) )
		{
			BLADE_SERIALIZE_CHECK(mBoneCount != 0 );
			BLADE_SERIALIZE_CHECK(boneCount == mBoneCount);

			uint32 magic = 0;
			stream->readAtom(magic);
			BLADE_SERIALIZE_CHECK(magic == ANIMATIONRESOURCE_TABLE_MAGIC);

			bool ret = StringTable::readStrings(stream, mNames, IModelConfigManager::getSingleton().getModelPool() );
			BLADE_SERIALIZE_CHECK(ret);

			totalFrameCount = 0;
			for(size_t i = 0; i < mNames.size(); ++i)
			{
				uint32 frameCount = 0;
				ret = stream->readAtom(frameCount) == sizeof(frameCount);
				BLADE_SERIALIZE_CHECK(ret);
				BLADE_SERIALIZE_CHECK(frameCount != 0);
				totalFrameCount += frameCount;

				fp32 time;
				ret = stream->readAtom(time) == sizeof(time);
				BLADE_SERIALIZE_CHECK(ret);

				uint32 flags = 0;
				ret = stream->readAtom(flags) == sizeof(flags);
				BLADE_SERIALIZE_CHECK(ret);

				BoneFrameCountTable frameCountTable;
				frameCountTable.resize( mBoneCount );
				ret = (size_t)stream->read(&frameCountTable[0], mBoneCount) == mBoneCount*sizeof(uint16);
				BLADE_SERIALIZE_CHECK(ret);

				size_t transformCount = frameCount;
				TransformCache cache;
				cache.resize(transformCount);
				IStream::Size bytes = (IStream::Size)(sizeof(BT_FMT)*transformCount);
				ret = stream->readData(&cache[0], bytes) == bytes;
				BLADE_SERIALIZE_CHECK(ret);

				KeyframeCache keyFrames;
				keyFrames.resize(frameCount);
				{
					Quaternion rotation;
					Vector3 translation;
					BoneDQ dq;
					for(size_t j = 0; j < frameCount; ++j)
					{
						rotation[0] = (scalar)cache[j].rotation_i16x3n[0] / (scalar)32767.0f;
						rotation[1] = (scalar)cache[j].rotation_i16x3n[1] / (scalar)32767.0f;
						rotation[2] = (scalar)cache[j].rotation_i16x3n[2] / (scalar)32767.0f;
						assert(!isnan(rotation[0]) && !isnan(rotation[1]) && !isnan(rotation[2]));

						rotation[3] = std::sqrt(
							1
							- rotation[0]*rotation[0] 
						-rotation[1]*rotation[1]
						-rotation[2]*rotation[2]);

						translation[0] = cache[j].position_f16x3[0].getFloat();
						translation[1] = cache[j].position_f16x3[1].getFloat();
						translation[2] = cache[j].position_f16x3[2].getFloat();
						//assert(!isnan(translation[0]) && !isnan(translation[1]) && !isnan(translation[2]));

						rotation.normalize();
						dq.set(rotation, translation);
						dq.normalize();
						keyFrames[j] = KeyFrame(dq, cache[j].time_pos);
					}
				}

				AnimationResource* Animation = BLADE_NEW AnimationResource(mNames[i], time, scalar(1)/(scalar)mFPS, flags, mBoneCount, &keyFrames[0], &frameCountTable[0]);
				mAnimationResources.push_back(Animation);
				ret = mMap.insert( std::make_pair(mNames[i], Animation) ).second;
				if( !ret )
					BLADE_DELETE Animation;
				BLADE_SERIALIZE_CHECK(ret);
			}
			return true;
		}
		assert(false);
		return false;
	}
	
}//namespace Blade