/********************************************************************
	created:	2014/02/25
	filename: 	SkeletonResource.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "SkeletonResource.h"
#include "IK.h"

namespace Blade
{

	const Version ISkeletonResource::SKELETON_LATEST_SERIALIXER_VERSION = Version(0,1,'A');


	//////////////////////////////////////////////////////////////////////////
	SkeletonResource::SkeletonResource()
	{
		mIK = NULL;
		mTotalFrameCount = 0;
	}

	//////////////////////////////////////////////////////////////////////////
	SkeletonResource::~SkeletonResource()
	{
		BLADE_DELETE mIK;
	}

	/************************************************************************/
	/* IResource interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	const TString&	SkeletonResource::getType() const
	{
		return ModelConsts::SKELETON_RESOURCE_TYPE;
	}

	/************************************************************************/
	/* ISkeletonResource interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool	SkeletonResource::initSkeleton(BONE_DATA* boneData, size_t boneCount, const TString* names, size_t nameCount, uint32 FPS)
	{
		assert( boneData != NULL && names != NULL);
		assert( boneCount != 0 && nameCount != 0);

		if( mBoneTable.mBoneCount == 0 )
		{
			assert( mBoneTable.mBoneData == NULL);
			assert( mBoneTable.mNameTable.size() == 0);

			mBoneTable.mBoneCount = boneCount;
			mBoneTable.mBoneData = BLADE_NEW BONE_DATA[boneCount];
			std::memcpy(mBoneTable.mBoneData, boneData, sizeof(BONE_DATA)*boneCount);

			mBoneTable.mNameTable.resize( nameCount );
			for(size_t i = 0; i < nameCount; ++i)
				mBoneTable.mNameTable[i] = names[i];

			return mAnimTable.initialize(mBoneTable.mBoneCount, FPS);
		}
		else
		{
			assert(false);
			return false;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool	SkeletonResource::addAnimation(const TString& name, float duration, uint32 flags, const KeyFrame* keyframes, const uint16* boneKeyCounts)
	{
		//need to init skeleton first
		if( mBoneTable.mBoneData == NULL || mBoneTable.mBoneCount == 0 || mBoneTable.mNameTable.size() == 0 )
		{
			assert(mBoneTable.mBoneData == NULL);
			assert(mBoneTable.mBoneCount == 0);
			assert( mBoneTable.mNameTable.size() == 0);
			assert( false );
			return false;
		}
		if( keyframes == NULL || boneKeyCounts == NULL)
		{
			assert(false);
			return false;
		}

		AnimationResource* animation = mAnimTable.addAnimationResource(name, duration, flags, keyframes, boneKeyCounts, mBoneTable.mBoneCount);

		if( animation != NULL )
			mTotalFrameCount += animation->getTotalFrameCount();

		return animation != NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	SkeletonResource::getAnimtaionBoneKeyCount(index_t index, uint16* boneKeyCount) const
	{
		assert(mBoneTable.mBoneCount == mAnimTable.getBoneCount() );
		if(index < mAnimTable.getAnimationResourceCount() )
		{
			const AnimationResource* anim = mAnimTable.getAnimationResource(index);
			for(size_t i = 0; i < mAnimTable.getBoneCount(); ++i)
				boneKeyCount[i] = (uint16)anim->getBoneSequence(i).getFrameCount();
			return true;
		}
		assert(false);
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	SkeletonResource::getAnimationData(index_t index, TString& name, float& duration, uint32& flags, KeyFrame* keyframes/* = NULL*/) const
	{
		assert(mBoneTable.mBoneCount == mAnimTable.getBoneCount() );
		if(index < mAnimTable.getAnimationResourceCount() )
		{
			const AnimationResource* anim = mAnimTable.getAnimationResource(index);
			name = anim->getName();
			duration = anim->getTotalTime();
			flags = anim->getFlags();

			if( keyframes != NULL )
			{
				for(size_t i = 0; i < mAnimTable.getBoneCount(); ++i)
				{
					const KeySequence& seq = anim->getBoneSequence(i);
					for(size_t j = 0; j < seq.getFrameCount(); ++j)
						*(keyframes++) = *seq.getKeyFrame(j);
				}
			}
			return true;
		}
		assert(false);
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	SkeletonResource::setAnimationInfo(const TString& origianlName, const TString& targetName, const uint32* flags/* = NULL*/)
	{
		AnimationResource* anim = mAnimTable.getAnimationResource(origianlName);
		if( anim == NULL )
			return false;
		return mAnimTable.setAnimationInfo(anim, targetName, flags);
	}

	//////////////////////////////////////////////////////////////////////////
	bool	SkeletonResource::reorderBones(const ISkeletonResource& skeletenRef)
	{
		size_t referenceCount = skeletenRef.getBoneCount();
		if( referenceCount < mBoneTable.mBoneCount )
		{
			assert(false);
			return false;
		}
		const SkeletonResource& boneReferences = static_cast<const SkeletonResource&>(skeletenRef);

		//early out: check if exact match in order
		bool exactMatch = (referenceCount == mBoneTable.mBoneCount);
		if( exactMatch )
		{
			for(size_t i = 0; i < referenceCount; ++i)
			{
				const BONE_DATA& boneRef = *boneReferences.getBoneData(i);
				const TString& refName = boneReferences.mBoneTable.mNameTable[boneRef.mIndex];
				if( refName != this->getBoneName(i) )
				{
					exactMatch = false;
					break;
				}
			}
			if(exactMatch)
				return true;
		}
	
		TempVector<uint32> indexMap(mBoneTable.mBoneCount, uint32(-1));
		TempVector<uint32> invIndexMap(referenceCount, uint32(-1));
		TStringParam nameTable;
		nameTable.reserve( mBoneTable.mNameTable.size() + boneReferences.mBoneTable.mNameTable.size() );
		BONE_DATA* data = BLADE_NEW BONE_DATA[ referenceCount ];

		//checks
		bool inserting = (referenceCount > mBoneTable.mBoneCount);
		BLADE_UNREFERENCED(inserting);
		size_t insertedCount = 0;

		for(size_t i = 0; i < referenceCount; ++i)
		{
			const BONE_DATA& boneRef = *boneReferences.getBoneData(i);
			const TString& refName = boneReferences.mBoneTable.mNameTable[boneRef.mIndex];
			index_t index = mBoneTable.findBone(refName);
			invIndexMap[i] = (uint32)index;

			if( index != INVALID_INDEX )
			{
				indexMap[index] = (uint32)i;
				data[i] = *mBoneTable.getBoneData(index);
				nameTable.push_back( mBoneTable.mNameTable[i] );

				data[i].mIndex = (uint32)i;
				if(data[i].mParent != (uint32)-1 )
				{
					assert(data[i].mParent < index );	//it should be sorted already
					assert(index < indexMap.size() && indexMap[ data[i].mParent ] != uint32(-1) );
					uint32 newParent = indexMap[ data[i].mParent ];
					if( boneRef.mParent != newParent )	//input hierarchy is different, treat it as an error
					{
						//for this object, boneA is a child of boneB
						//but for input, boneA is not a child of boneB
						//force using input or using this both may cause errors.
						if(boneRef.mParent == uint32(-1))
						{
							//this function is used for tools only, use detail log
							BLADE_DETAIL_LOG(Error, BTString("bone hierarchy is diffent: bone '") << refName 
								<< BTString("' has a parent '") << mBoneTable.getBoneName(data[i].mParent) 
								<< BTString("', but input refernce's bone has no parent."));
						}
						else
						{
							BLADE_DETAIL_LOG(Error, BTString("bone hierarchy is diffent: bone '") << refName 
								<< BTString("' has a parent '") << mBoneTable.getBoneName(data[i].mParent) 
								<< BTString("', but input refernce's bone's parent is '") << boneReferences.getBoneName(boneRef.mParent)
								<< BTString("'."));
						}
						BLADE_DELETE[] data;
						return false;
					}
					data[i].mParent = newParent;
				}
			}
			else
			{
				assert(inserting);
				++insertedCount;
				//new bone inserted
				data[i].mIndex = (uint32)i;
				data[i].mParent = boneRef.mIndex;
				nameTable.push_back(refName);
				data[i].mInitialPose = boneRef.mInitialPose;
			}
		}

		if( insertedCount > 0 )
		{
			mBoneTable.mNameTable.clear();
			StringTableCache::persist(nameTable, mBoneTable.mNameTable, IModelConfigManager::getInterface().getModelPool());
		}
		else
			assert( !inserting );

		assert(insertedCount == referenceCount - mBoneTable.mBoneCount);

		//bone table re-ordered, now keyframes
		size_t boneCount = this->getBoneCount();
		TempVector<uint16> boneKeyCount(boneCount);
		for(size_t i = 0; i < this->getAnimationCount(); ++i)
		{
			this->getAnimtaionBoneKeyCount(i, &boneKeyCount[0]);
			size_t totalFrames = 0;
			for(size_t j = 0; j < boneCount; ++j)
				totalFrames += boneKeyCount[j];

			TempVector<KeyFrame> frames(totalFrames + insertedCount*2);

			boneKeyCount.resize( referenceCount );

			AnimationResource* anim = mAnimTable.getAnimationResource(i);
			index_t frameIndex = 0;
			for(size_t j = 0; j < referenceCount; ++j)
			{
				index_t boneIndex = invIndexMap[j];
				if( boneIndex == INVALID_INDEX )
				{
					//new inserted bone, add identity key frames for bone
					assert(insertedCount > 0);
					frames[frameIndex++] = KeyFrame(BoneDQ::IDENTITY, 0);
					frames[frameIndex++] = KeyFrame(BoneDQ::IDENTITY, anim->getTotalTime());
					boneKeyCount[j] = 2u;
				}
				else
				{
					const KeySequence& seq = anim->getBoneSequence(boneIndex);
					boneKeyCount[j] = (uint16)seq.getFrameCount();

					for(size_t k = 0; k < seq.getFrameCount(); ++k)
						frames[frameIndex++] = *seq.getKeyFrame(k);
				}
			}
			assert(frameIndex == totalFrames);
			anim->reset(referenceCount, &frames[0], &boneKeyCount[0]);
		}

		BLADE_DELETE[] mBoneTable.mBoneData;
		mBoneTable.mBoneData = data;
		mBoneTable.mBoneCount = referenceCount;

		return false;
	}


	//////////////////////////////////////////////////////////////////////////
	bool	SkeletonResource::setIKContraints(const IK_CONSTRAINTS* constraints, size_t boneCount)
	{
		if( constraints == NULL || boneCount != mBoneTable.mBoneCount )
		{
			assert(false);
			return false;
		}

		for(size_t i = 0; i < mBoneTable.mBoneCount; ++i)
			mBoneTable.mBoneData[i].mConstraints.merge(constraints[i]);

		return true;
	}

}//namespace Blade