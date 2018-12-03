/********************************************************************
	created:	2014/02/21
	filename: 	SkeletonResource.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_SkeletonResource_h__
#define __Blade_SkeletonResource_h__
#include <interface/ISkeletonResource.h>
#include "AnimationResource.h"

namespace Blade
{
	class IK;

	class SkeletonResource : public ISkeletonResource, public Allocatable
	{
	public:
		SkeletonResource();
		~SkeletonResource();

		/************************************************************************/
		/* IResource interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual const TString&	getType() const;

		/************************************************************************/
		/* ISkeletonResource interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool	initSkeleton(BONE_DATA* boneData, size_t boneCount, const TString* names, size_t nameCount, uint32 FPS);

		/*
		@describe 
		@param [in] keyframes key frame 2 dim array in count of [boneCount][boneKeyCount]
		@param [in] boneKeyCounts boneKeyCount for each bone. in count of boneCount
		@return 
		*/
		virtual bool	addAnimation(const TString& name, float duration, uint32 flags, const KeyFrame* keyframes, const uint16* boneKeyCounts);

		/*
		@describe get animation count
		@param
		@return
		*/
		virtual size_t	getAnimationCount() const
		{
			return mAnimTable.getAnimationResourceCount();
		}

		/*
		@describe get bone key count for each bone of specified animation.
		@param [in] boneKeyCount: array to receive key count for each bone. MUST in size of bone count
		@return
		*/
		virtual bool	getAnimtaionBoneKeyCount(index_t index, uint16* boneKeyCount) const;

		/*
		@copydoc ISkeletonResource::getAnimationData
		*/
		virtual bool	getAnimationData(index_t index, TString& name, float& duration, uint32& flags, KeyFrame* keyframes = NULL) const;

		/*
		@copydoc ISkeletonResource::setAnimationInfo
		*/
		virtual bool	setAnimationInfo(const TString& origianlName, const TString& targetName, const uint32* flags = NULL);

		/*
		@copydoc ISkeletonResource::reorderBones
		*/
		virtual bool	reorderBones(const ISkeletonResource& skeletenRef);

		/*
		@copydoc ISkeletonResource::setIKContraints
		*/
		virtual bool	setIKContraints(const IK_CONSTRAINTS* constraints, size_t boneCount);

		/*
		@copydoc ISkeletonResource::getBoneCount
		*/
		virtual size_t	getBoneCount() const
		{
			return mBoneTable.mBoneCount;
		}

		/*
		@copydoc ISkeletonResource::getBoneData
		*/
		virtual const BONE_DATA* getBoneData() const
		{
			return mBoneTable.mBoneData;
		}

		/** @brief  */
		virtual const tchar*	getBoneName(size_t index) const { return mBoneTable.getBoneName((uint32)index); }

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		inline const AnimationTable* getAnimation() const	{return &mAnimTable;}

		/** @brief  */
		const BONE_DATA*	getBoneData(size_t index) const	{return mBoneTable.getBoneData(index);}

		/** @brief  */
		index_t				findBone(const TString& name) const {return mBoneTable.findBone(name);}

		/** @brief  */
		const IK*			getIK() const	{return mIK;}

	protected:
		BoneTable		mBoneTable;
		AnimationTable	mAnimTable;
		uint32			mTotalFrameCount;

		//runtime data
		IK*				mIK;
		friend class SkeletonSerializer;
	};//class SkeletonResource
	
}//namespace Blade

#endif //  __Blade_SkeletonResource_h__