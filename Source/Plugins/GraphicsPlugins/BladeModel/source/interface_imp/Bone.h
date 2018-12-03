/********************************************************************
	created:	2014/02/25
	filename: 	Bone.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_Bone_h__
#define __Blade_Bone_h__
#include <BladeModel.h>
#include <math/BladeMath.h>
#include <math/Vector3.h>
#include <math/Quaternion.h>
#include <utility/BladeContainer.h>
#include <utility/StringTable.h>
#include <interface/ISkeletonResource.h>


namespace Blade
{
	typedef StringTable NameTable;
	typedef uint8 BoneActivity;

	//////////////////////////////////////////////////////////////////////////
	//utility helpers
	//////////////////////////////////////////////////////////////////////////
	//important: sort bones so that parent comes first, this is an optimization for animation runtime
	struct FnBoneDataCompare
	{
		const BONE_DATA* root;
		FnBoneDataCompare(const BONE_DATA* _root) :root(_root)	{}

		//check if rhs is descendant of lhs
		inline bool isDescendant(const BONE_DATA* left, const BONE_DATA* right) const
		{
			size_t rightLevel = 0;
			while(right->mParent != (uint32)-1 )
			{
				right = root + right->mParent;
				++rightLevel;
			}

			size_t leftLevel = 0;
			while(left->mParent!= (uint32)-1)
			{
				left = root + left->mParent;
				++leftLevel;
			}
			return leftLevel < rightLevel;// || (leftLevel == rightLevel && left < right);
		}

		bool operator()(const BONE_DATA& lhs, const BONE_DATA& rhs) const
		{
			if( this->isDescendant(&lhs, &rhs) )
				return true;
			else
				return false;
		}
	};

	struct FnBoneDataFind
	{
		uint32 index;
		FnBoneDataFind(uint32 idx) :index(idx)	{}

		bool operator()(const BONE_DATA& target) const
		{
			return target.mIndex == index;
		}
	};

	//////////////////////////////////////////////////////////////////////////
	//resource data, held by skeleton resources
	//////////////////////////////////////////////////////////////////////////
	class BoneTable
	{
	public:
		NameTable	mNameTable;
		BONE_DATA*	mBoneData;
		size_t		mBoneCount;

		BoneTable() :mBoneData(NULL), mBoneCount(0)	{}
		~BoneTable() {BLADE_DELETE[] mBoneData;}

		/** @brief get bone name by NAME INDEX */
		inline const tchar* getBoneName(uint32 index) const	{return index < (uint32)mNameTable.size() ? mNameTable[index].c_str() : NULL;}

		/** @brief  */
		const BONE_DATA*	getBoneData(index_t index) const {return mBoneData != NULL && index < mBoneCount ? &mBoneData[index] : NULL;}

		/** @brief  */
		inline index_t		findBone(const TString& name) const
		{
			//TODO: optimize
			for(size_t i = 0; i < mBoneCount; ++i)
			{
				if( mNameTable[i] == name )
					return i;
			}
			return INVALID_INDEX;
		}

		/** @brief sort the bone by the order that parent comes before children */
		/* @note: this is an offline function. data should be sorted before runtime using */
		static bool sortBoneData(BONE_DATA* boneData, size_t boneCount, bool updateIndex, uint32 oriIndex[] = NULL)
		{
			if(boneData == NULL || boneCount == 0)
			{
				assert(false);
				return false;
			}

			//make a copy of original array: the parent index should indexing the array of original order,
			//on sorting, the order is changing
			TempVector<BONE_DATA> boneCache(boneCount);
			std::memcpy(&boneCache[0], boneData, boneCount*sizeof(BONE_DATA));

			//note: sort will change the boneData order, causing mismatch of index with original data
			std::sort(&boneData[0], &boneData[0]+boneCount, FnBoneDataCompare( &boneCache[0] ) );
			//update parent index
			for(size_t i = 0; i < boneCount; ++i)
			{
				BONE_DATA& data = boneData[i];
				assert( data.mIndex != (uint32)-1);

				if(data.mParent != (uint32)-1)
				{
					//locate parent in new sorted array
					BONE_DATA* item = std::find_if(&boneData[0], &boneData[0]+boneCount, FnBoneDataFind(data.mParent) );
					assert(item != &boneData[0]+boneCount );
					data.mParent = uint32(item - &boneData[0]);
				}
			}

			//update new index
			for(size_t i = 0; i < boneCount; ++i)
			{
				BONE_DATA& data = boneData[i];
				
				if( oriIndex != NULL )
					oriIndex[i] = data.mIndex;

				if( updateIndex)
					data.mIndex = (uint32)i;
			}
			return true;
		}

	};

	//////////////////////////////////////////////////////////////////////////
	// key sequence for one bone
	// resource data
	//////////////////////////////////////////////////////////////////////////
	class KeySequence : public ModelAllocatable<KeySequence>
	{
	public:
		KeySequence(const KeyFrame* keyframes, size_t count, index_t boneIndex)
			:mKeyFrames(keyframes),mBonIndex(uint16(boneIndex)),mFrameCount((uint16)count)
		{
			//overflow check
			assert( count <= 0xFFFF );
			assert( boneIndex <= 0xFFFF );
		}
		KeySequence()	:mKeyFrames(NULL),mBonIndex( uint16(-1)),mFrameCount(0)	{}
		~KeySequence()	{}

		/** @brief  */
		inline size_t	getBoneIndex() const				{return mBonIndex;}

		/** @brief get number of key frames */
		inline size_t	getFrameCount() const				{return mFrameCount;}

		/** @brief  */
		inline const KeyFrame*	getKeyFrame(index_t index)	const	{return (mKeyFrames != NULL && index < mFrameCount) ? &mKeyFrames[index] : NULL;}

		/** @brief  */
		inline const KeyFrame*	begin() const				{return mKeyFrames + 0;}
		/** @brief  */
		inline const KeyFrame*	end() const					{return mKeyFrames + mFrameCount;}
		/** @brief get upper bound(>timepos) by time pos */
		const KeyFrame*			find(scalar timePos) const	{return std::upper_bound(this->begin(), this->end(), timePos, timeCompareLess);}

		/** @brief  */
		static inline bool	timeCompareLess(scalar time, const KeyFrame& target)
		{
			return time < target.getTimePos();
		}
	protected:
		const KeyFrame*	mKeyFrames;
		uint16			mBonIndex;
		uint16			mFrameCount;
	};

	//////////////////////////////////////////////////////////////////////////
	//runtime data, held by skeleton instances
	//currently NOT used
	//////////////////////////////////////////////////////////////////////////
#if 0
	class Bone : public Allocatable
	{
	public:
		Bone();
		~Bone();

		/** @brief  */
		inline const BONE_DATA*	getBoneData() const		{return mBoneTable->getBoneData(mDataIndex);}

		/** @brief  */
		inline uint32			getIndex() const		{return this->getBoneData()->mIndex;}

		/** @brief  */
		inline const tchar*		getName() const			{return mBoneTable->getBoneName( this->getBoneData()->mIndex );}

	protected:
		BoneTable*	mBoneTable;
		size_t		mDataIndex;

		//runtime transform data
		Quaternion	mLocalRotation;
		POINT3		mLocalTranslate;
		scalar		mLocalScale;

		//bone final transform
		Quaternion	mRotation;
		POINT3		mTranslate;
		scalar		mScale;
	};
#endif
	
}//namespace Blade

#endif //  __Blade_Bone_h__