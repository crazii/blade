/********************************************************************
	created:	2014/06/14
	filename: 	Animation.h
	author:		Crazii
	
	purpose:	runtime animation instance of skeleton's animation resource
*********************************************************************/
#ifndef __Blade_Animation_h__
#define __Blade_Animation_h__
#include <math/DualQuaternion.h>
#include <utility/BladeContainer.h>
#include <interface/IModelConfigManager.h>
#include "AnimationResource.h"

namespace Blade
{
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class Animation;
	//note: use allocatable for common animation classes.
	//use model ModelAllocatable for large quantities of resource class objects.
	class AnimationState : public Allocatable
	{
	public:
		AnimationState(const AnimationResource* resource, Animation* parent);
		~AnimationState();

		/** @brief  */
		bool			setBoneWeights(const scalar* weights, size_t count);

		/** @brief  */
		void			setBoneWeight(index_t bone, scalar weight);

		/** @brief  */
		bool			initBoneWeights(scalar defaultWeight = 1.0f);

		/** @brief  */
		void			setCurrentPos(scalar pos);

		/** @brief  */
		void			setBlendWeight(scalar weight);

		/** @brief  */
		void			updateAnimation(DualQuaternion* dest, const BoneActivity* boneAcitvity, scalar weight = 1.0);

		/** @brief  */
		const BoneDQ&	getBoneTransform(uint32 boneIndex, scalar time) const;

		/** @brief  */
		inline scalar		getCurrentPos() const
		{
			return mPosition;
		}

		/** @brief  */
		inline scalar		getBlendWeight() const
		{
			return mWeight;
		}

		/** @brief  */
		const AnimationResource*	getAnimation() const
		{
			return mAnimtaion;
		}

		/** @brief  */
		const tchar*		getName() const
		{
			return this->getAnimation()->getName();
		}

		/** @brief length in time sec */
		inline scalar		getLength() const
		{
			return this->getAnimation()->getTotalTime();
		}

		/** @brief  */
		inline bool			isLooped() const
		{
			return this->getAnimation()->isLooped();
		}

		/** @brief whether contains large translations, if yes, model's bounding need to be translated */
		inline bool			hasTranslation() const
		{
			return this->getAnimation()->hasTranslation();
		}

	protected:
		const AnimationResource*	mAnimtaion;
		scalar		mPosition;		//current time position
		scalar		mWeight;		//blend weight of current animation
		scalar*		mBoneWeights;	//array of weights for each bone
		Animation*	mParent;
	};//class AnimationState

	/************************************************************************/
	/* blend tree                                                                     */
	/************************************************************************/
	class IBlendNode
	{
	public:
		virtual ~IBlendNode() {}

		/** @brief  */
		virtual bool	blend(scalar timePos, DualQuaternion* target, scalar weight, const BoneActivity* boneAcitvity) = 0;

		/** @brief  */
		virtual AnimationState* getAnimtaion() const = 0;

		/** @brief  */
		virtual size_t	getChildCount() const = 0;

		/** @brief  */
		virtual IBlendNode* getChild(index_t index) const = 0;

		/** @brief  */
		virtual scalar	getBlendWeight() const = 0;

		/** @brief  */
		virtual bool	setBlendWeight(scalar weight) = 0;
	};


	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class Animation : public Allocatable
	{
	public:
		Animation();
		~Animation();

		/** @brief  */
		bool	initialize(const AnimationTable* animationTable);

		/** @brief  */
		bool	cleanup();

		/** @brief  */
		bool	update(scalar timePos, DualQuaternion* destTransforms, const BoneActivity* boneAcitvity);

		/** @brief simple set for preview */
		bool	setAnimation(const TString& name);

		/** @brief  */
		inline size_t getBoneCount() const
		{
			return mAnimation->getBoneCount();
		}

		/** @brief  */
		inline size_t getAnimtaionCount() const
		{
			return (mAnimation != NULL) ? mAnimation->getAnimationResourceCount() : 0;
		}

		/** @brief  */
		inline const tchar* getAnimtaion(index_t index) const
		{
			return  (mAnimation != NULL) ? mAnimation->getAnimationResource(index)->getName() : NULL;
		}

		/** @brief  */
		inline AnimationState* getCurrentAnimation() const
		{
			AnimationState* state = NULL;
			if( mBlendingRoot != NULL )
				state = mBlendingRoot->getAnimtaion();
			return state;
		}

	protected:
		const AnimationTable*	mAnimation;

		struct AnimationStateLess
		{
		public:
			bool	operator()(const AnimationState* lhs, const AnimationState* rhs) const
			{
				//note: names are in string table for persistence
				return *lhs->getAnimation() < *rhs->getAnimation();
			}
		};

		typedef Set<AnimationState*, AnimationStateLess>	AnimationSet;
		AnimationSet	mStateSet;
		IBlendNode*		mBlendingRoot;	
	};//class Animation
	
}//namespace Blade


#endif // __Blade_Animation_h__