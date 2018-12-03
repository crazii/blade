/********************************************************************
	created:	2014/02/21
	filename: 	AnimationResource.h
	author:		Crazii
	purpose:	skeletal/skinned AnimationResources: a set of key frames for all bones in skeleton
				this is the resource object, held by skeleton resources, it is not a standard resource(IResource).
*********************************************************************/
#ifndef __Blade_AnimationResource_h__
#define __Blade_AnimationResource_h__
#include "Bone.h"

namespace Blade
{
	enum EAnimationFlags
	{
		AF_LOOP			= 0x00000001,
		AF_TRANSLATION	= 0x00000002,
	};

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class AnimationResource : public Allocatable
	{
	public:
		AnimationResource(const TString& name, scalar time, scalar frameTime, uint32 flags, size_t boneCount, const KeyFrame* AnimationResourceKeys, const uint16* boneKeyCount);
		~AnimationResource();

		/** @brief note: shared buffer (T)String object cannot be copied directly */
		const tchar*		getName() const			{return mName.c_str();}

		/** @brief total duration */
		inline scalar		getTotalTime() const	{return mTime;}

		/** @brief constant FPS info */
		inline scalar		getFrameTime() const	{return mFrameTime;}

		/** @brief  */
		inline uint32		getTotalFrameCount() const{return mFrameCount;}

		/** @brief  */
		inline uint32		getFlags() const		{return mFlags.getMask();}

		/** @brief  */
		inline bool			isLooped() const		{return mFlags.checkBits(AF_LOOP);}

		/** @brief  */
		inline bool			hasTranslation() const	{return mFlags.checkBits(AF_TRANSLATION);}

		/** @brief  */
		inline bool operator<(const AnimationResource& rhs) const
		{
			return FnTStringFastLess::compare( mName, rhs.mName );
		}

		/** @brief get one key sequence object for one bone */
		inline const KeySequence&		getBoneSequence(index_t boneIndex) const
		{
#if BLADE_DEBUG
			assert( boneIndex < mBoneCount );
#endif
			return mBoneSequences[boneIndex];
		}

		/** @brief  */
		void	reset(size_t boneCount, const KeyFrame* AnimationResourceKeys, const uint16* boneKeyCount);

	protected:
		KeySequence*	mBoneSequences;	//sequence for all bones
		KeyFrame*		mKeyFrames;		//key frame for all bones, in linear storage

		uint32		mFrameCount;	//total key frame count for all bones
#if BLADE_DEBUG
		size_t		mBoneCount;		//debugging bone count
#endif
		scalar		mTime;			//total time of this AnimationResource
		scalar		mFrameTime;		//FPS used by this animation
		TString		mName;
		Mask		mFlags;
		friend class AnimationTable;
	};

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/

	class AnimationTable
	{
	public:
		typedef Vector<AnimationResource*>	AnimationResourceList;
		typedef TStringMap<AnimationResource*> AnimationResourceMap;

		inline AnimationTable()		{mBoneCount = 0;}
		inline ~AnimationTable()	{this->clear();}

		/** @brief  */
		bool		initialize(size_t boneCount, uint32 FPS);

		/** @brief  */
		bool		clear();

		/** @brief add AnimationResource on runtime (usually on exporter) */
		AnimationResource*	addAnimationResource(const TString& name, float duration, uint32 flags, const KeyFrame* keyframes, const uint16* boneKeyCounts, size_t boneCount);

		/** @brief  */
		bool		setAnimationInfo(AnimationResource* res, const TString& newName, const uint32* flags);

		/** @brief  */
		bool		saveAnimationResource(const HSTREAM& stream) const;

		/** @brief  load AnimationResources from stream */
		bool		loadAnimationResource(const HSTREAM& stream, size_t boneCount, size_t& totalFrameCount);

		/** @brief  */
		inline size_t	getAnimationResourceCount() const
		{
			assert( mAnimationResources.size() == mMap.size() );
			assert( mNames.size() == mMap.size() );
			return mAnimationResources.size();
		}

		/** @brief  */
		inline AnimationResource*	getAnimationResource(const index_t index)
		{
			assert( mAnimationResources.size() == mMap.size() );
			assert( mNames.size() == mMap.size() );
			if( index < mAnimationResources.size() )
				return mAnimationResources[index];
			else
				return NULL;
		}

		/** @brief  */
		inline const AnimationResource*	getAnimationResource(const index_t index) const
		{
			assert( mAnimationResources.size() == mMap.size() );
			assert( mNames.size() == mMap.size() );
			if( index < mAnimationResources.size() )
				return mAnimationResources[index];
			else
				return NULL;
		}

		/** @brief  */
		inline AnimationResource*	getAnimationResource(const TString& name)
		{
			assert( mAnimationResources.size() == mMap.size() );
			assert( mNames.size() == mMap.size() );
			AnimationResourceMap::const_iterator i = mMap.find(name);
			if( i != mMap.end() )
				return i->second;
			else
				return NULL;
		}

		inline const AnimationResource*	getAnimationResource(const TString& name) const
		{
			assert( mAnimationResources.size() == mMap.size() );
			assert( mNames.size() == mMap.size() );
			AnimationResourceMap::const_iterator i = mMap.find(name);
			if( i != mMap.end() )
				return i->second;
			else
				return NULL;
		}

		/** @brief  */
		inline size_t				getBoneCount() const
		{
			return mBoneCount;
		}

		/** @brief  */
		inline uint32				getFPS() const
		{
			return mFPS;
		}
	protected:
		NameTable		mNames;
		AnimationResourceList	mAnimationResources;
		AnimationResourceMap	mMap;
		size_t			mBoneCount;
		uint32			mFPS;
	};
	
}//namespace Blade

#endif //  __Blade_AnimationResource_h__