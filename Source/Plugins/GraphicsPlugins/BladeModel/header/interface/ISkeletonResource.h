/********************************************************************
	created:	2014/02/26
	filename: 	ISkeletonResource.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_ISkeletonResource_h__
#define __Blade_ISkeletonResource_h__
#include <BladeModel.h>
#include <utility/Version.h>
#include <utility/StringList.h>
#include <interface/public/IResource.h>
#include <interface/IModelConfigManager.h>
#include <math/DualQuaternion.h>

namespace Blade
{
	//note: VS2012/15 don't do empty base optimization as expected, but still it is confronted to the standard.
	//empty base optimization is mandatory since C++11

	//note: if member/base have the same sub object as this class, C++ standard ensure the sub objects have different address.
	//this will make object larger. this is not about EBO. so BoneDQ & (KeyFrame/BONE_DATA) cannot both inherit ModelAllocatable.
	//unless ModelAllocatable is template.
#if BLADE_COMPILER != BLADE_COMPILER_MSVC || BLADE_STDCPP >= BLADE_STDCPP11
#define EBO_PRESENT 1
#else
#define EBO_PRESENT 0
#endif

	class BoneDQ : public DualQuaternion
#if EBO_PRESENT
		, public ModelAllocatable<BoneDQ>
#endif
	{
	public:
		inline BoneDQ() {}
		inline BoneDQ(const Quaternion& _p, const Quaternion& _q):DualQuaternion(_p, _q)	{}
		inline BoneDQ(const Quaternion& r, const Vector3& t)								{ this->set(r, t); }
		inline BoneDQ(const DualQuaternion& src) :DualQuaternion(src)						{}

		inline BoneDQ& operator=(const DualQuaternion& rhs)
		{
			if( this != &rhs )
				(DualQuaternion&)(*this) = rhs;
			return *this;
		}

#if EBO_PRESENT
		using ModelAllocatable::operator new;
		using ModelAllocatable::operator delete;
		using ModelAllocatable::operator new[];
		using ModelAllocatable::operator delete[];
#else
		inline static void*	operator new(size_t size)
		{
			return BLADE_MODEL_ALLOC(size);
		}
		inline static void*	operator new(size_t size,const MEMINFO*)
		{
			return BLADE_MODEL_ALLOC(size);
		}
		inline static void*	operator new(size_t size,const char* sourcefile,int lineNO,const MEMINFO*)
		{
			return BLADE_MODEL_ALLOC_DEBUG(size,sourcefile,lineNO);
		}
		inline static void*	operator new[](size_t size)
		{
			return BLADE_MODEL_ALLOC(size);
		}
		inline static void*	operator new[](size_t size,const MEMINFO*)
		{
			return BLADE_MODEL_ALLOC(size);
		}
		inline static void*	operator new[](size_t size,const char* sourcefile,int lineNO,const MEMINFO*)
		{
			return BLADE_MODEL_ALLOC_DEBUG(size,sourcefile,lineNO);
		}

		inline static void		operator delete(void* ptr)
		{
			return BLADE_MODEL_FREE(ptr);
		}
		inline static void		operator delete(void* ptr,const MEMINFO*)
		{
			return BLADE_MODEL_FREE(ptr);
		}
		inline static void		operator delete(void* ptr,const char* /*sourcefile*/,int /*lineNO*/,const MEMINFO*)
		{
			return BLADE_MODEL_FREE(ptr);
		}
		inline static void		operator delete[](void* ptr)
		{
			return BLADE_MODEL_FREE(ptr);
		}
		inline static void		operator delete[](void* ptr,const MEMINFO*)
		{
			return BLADE_MODEL_FREE(ptr);
		}
		inline static void		operator delete[](void* ptr,const char* /*sourcefile*/,int /*lineNO*/,const MEMINFO*)
		{
			return BLADE_MODEL_FREE(ptr);
		}
#endif
	};
	static_assert(sizeof(BoneDQ) == 32, "size error");
	static_assert(sizeof(BoneDQ) == sizeof(DualQuaternion), "size error");

	//////////////////////////////////////////////////////////////////////////
	//resource data, held by skeleton resources
	//key frame for one bone
	//////////////////////////////////////////////////////////////////////////
	class KeyFrame : public ModelAllocatable<KeyFrame>
	{
	public:
		KeyFrame()	{ std::memset(this, 0, sizeof(*this)); }
		KeyFrame(const BoneDQ& transform, scalar time) : mTransform(transform),mTime(time) {}
		~KeyFrame()	{}

		/** @brief  */
		const BoneDQ&	getTransform() const	{return mTransform;}
		/** @brief  */
		scalar			getTimePos() const		{return mTime;}

	protected:
		BoneDQ		mTransform;
		fp32		mTime;		//time point
	};
	static_assert(sizeof(KeyFrame) == 36, "size error");

	typedef struct SIKConstraints
	{
		fp32	mMinX;
		fp32	mMaxX;
		fp32	mMinY;
		fp32	mMaxY;
		fp32	mMinZ;
		fp32	mMaxZ;
		inline SIKConstraints()
		{
			mMinX = mMinY = mMinZ = FLT_MAX;
			mMaxX = mMaxY = mMaxZ = -FLT_MAX;
		}
		inline SIKConstraints(fp32 minx, fp32 maxx, fp32 miny, fp32 maxy, fp32 minz, fp32 maxz)
		{
			mMinX = minx; mMaxX = maxx;
			mMinY = miny; mMaxY = maxy;
			mMinZ = minz; mMaxZ = maxz;
		}

		inline void merge(const SIKConstraints& rhs)
		{
			mMinX = std::min(mMinX, rhs.mMinX);
			mMaxX = std::max(mMaxX, rhs.mMaxX);
			mMinY = std::min(mMinY, rhs.mMinY);
			mMaxY = std::max(mMaxY, rhs.mMaxY);
			mMinZ = std::min(mMinZ, rhs.mMinZ);
			mMaxZ = std::max(mMaxZ, rhs.mMaxZ);
		}
	}IK_CONSTRAINTS;

	//////////////////////////////////////////////////////////////////////////
	typedef struct SBoneData : public ModelAllocatable<SBoneData>
	{
	public:
		BoneDQ	mInitialPose;	///binding pose: initial transform in model space. note it is no inversed.
		uint32	mParent;		///parent data index
		uint32	mIndex;			///bone index
		IK_CONSTRAINTS	mConstraints;
	}BONE_DATA;
	static_assert(sizeof(BONE_DATA) == 64, "size/alignment error");


	class BLADE_MODEL_API ISkeletonResource : public IResource
	{
	public:
		static const Version SKELETON_LATEST_SERIALIXER_VERSION;

	public:
		virtual ~ISkeletonResource()	{}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool	initSkeleton(BONE_DATA* boneData, size_t boneCount, const TString* Names, size_t nameCount, uint32 FPS) = 0;

		/**
		@describe 
		@param [in] keyframes key frame array in count of sum(boneKeyCounts)
		@param [in] boneKeyCounts boneKeyCount for each bone. in count of boneCount
		@return 
		*/
		virtual bool	addAnimation(const TString& name, float duration, uint32 flags, const KeyFrame* keyframes, const uint16* boneKeyCounts) = 0;

		/**
		@describe get animation count
		@param
		@return
		*/
		virtual size_t	getAnimationCount() const = 0;

		/**
		@describe get bone key count for each bone of specified animation.
		@param [in] boneKeyCount: array to receive key count for each bone. MUST in size of bone count
		@return
		*/
		virtual bool	getAnimtaionBoneKeyCount(index_t index, uint16* boneKeyCount) const = 0;

		/**
		@describe get key frames for animation
		@param [in] keyFrames: array to receive key frames, MUSTt in size of sum of array in getAnimtaionBoneKeyCount()
		example:
		index_t animIndex = 0;
		size_t boneCount = animRes->getBoneCount();
		uint16* boneKeyCount = BLADE_NEW uint16[boneCount];
		if( animRes->getAnimtaionBoneKeyCount(animIndex, boneKeyCount) )
		{
			size_t totalFrames = 0;
			for(size_t i = 0; i < boneCount; ++i)
				totalFrames += boneKeyCount[i];
			KeyFrame* frames = BLADE_NEW KeyFrame[totalFrames];
			TString name; float duration; bool loop;
			animRes->getAnimation(animIndex, name, duration, loop, frames);
		}
		@return
		*/
		virtual bool	getAnimationData(index_t index, TString& name, float& duration, uint32& flags, KeyFrame* keyframes = NULL) const = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual bool	setAnimationInfo(const TString& origianlName, const TString& targetName, const uint32* flags = NULL) = 0;

		/**
		@describe re-order bones using in put skeletenRef, so that this skeleton matches the skeletenRef' bone orders.
		input skeletenRef MUST be equal or larger than current bones in count
		if input bone count is larger, this will adding IDENTITY key frames for bones that does not exist in current skeleton
		@@note this function is used by tools only.
		@param
		@return
		*/
		virtual bool	reorderBones(const ISkeletonResource& skeletenRef) = 0;

		/**
		@describe set bone constraints. this will MERGE the input parameters with current constraints existing within this skeleton
		@param
		@return
		*/
		virtual bool	setIKContraints(const IK_CONSTRAINTS* constraints, size_t boneCount) = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual size_t	getBoneCount() const = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual const BONE_DATA* getBoneData() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const tchar*	getBoneName(size_t index) const = 0;
	};
	
}//namespace Blade

#endif //  __Blade_ISkeletonResource_h__