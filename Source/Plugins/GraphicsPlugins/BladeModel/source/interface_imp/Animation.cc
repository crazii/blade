/********************************************************************
	created:	2014/06/14
	filename: 	Animation.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "Animation.h"
#include <interface/IModelConfigManager.h>
#include <interface/IPlatformManager.h>

namespace Blade
{
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	AnimationState::AnimationState(const AnimationResource* resource, Animation* parent)
		:mAnimtaion(resource)
		,mPosition(0)
		,mWeight(1.0)
		,mBoneWeights(NULL)
		,mParent(parent)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	AnimationState::~AnimationState()
	{
		BLADE_MODEL_FREE(mBoneWeights);
	}

	//////////////////////////////////////////////////////////////////////////
	bool		AnimationState::setBoneWeights(const scalar* weights, size_t count)
	{
		if( weights != NULL && count == mParent->getBoneCount() )
		{
			BLADE_MODEL_FREE(mBoneWeights);
			mBoneWeights = BLADE_MODEL_ALLOCT(scalar, count);
			std::memcpy(mBoneWeights, weights, count*sizeof(scalar));
			return true;
		}
		else
		{
			assert(false);
			return false;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void		AnimationState::setBoneWeight(index_t bone, scalar weight)
	{
		if( mBoneWeights != NULL && bone < mParent->getBoneCount() )
			mBoneWeights[bone] = weight;
		else
			BLADE_EXCEPT(EXC_INVALID_PARAM, BTString("no weights created or index out of range."));
	}

	//////////////////////////////////////////////////////////////////////////
	bool		AnimationState::initBoneWeights(scalar defaultWeight/* = 1.0f*/)
	{
		assert(mParent != NULL );

		size_t count = mParent->getBoneCount();
		if( mBoneWeights == NULL &&  count != 0)
		{
			mBoneWeights = BLADE_MODEL_ALLOCT(scalar, mParent->getBoneCount() );
			for(size_t i = 0; i < count; ++i)
				mBoneWeights[i] = defaultWeight;
			return true;
		}
		else
		{
			assert(false);
			return false;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void		AnimationState::setCurrentPos(scalar pos)
	{
		mPosition = pos;
		scalar length = this->getLength();
		if( this->isLooped() )
		{
			mPosition = std::fmod(mPosition, length + /*Math::HIGH_EPSILON*/mAnimtaion->getFrameTime()*0.8f);
			if( mPosition < 0 )
				mPosition += length;
		}
		else
			Math::Clamp(mPosition, scalar(0), length);
	}

	//////////////////////////////////////////////////////////////////////////
	void		AnimationState::setBlendWeight(scalar weight)
	{
		mWeight = weight;
	}

	//////////////////////////////////////////////////////////////////////////
	void		AnimationState::updateAnimation(DualQuaternion* dest, const BoneActivity* boneAcitvity, scalar weight/* = 1.0*/)
	{
		if( weight*mWeight < 1e-3 )
			return;

		const bool loop = this->isLooped();

		size_t boneCount = mParent->getBoneCount();
		for(size_t i = 0; i < boneCount; ++i)
		{
			if( boneAcitvity[i] == 0 )
				continue;
			scalar timePos = mPosition;

			//get sequence for this bone
			const KeySequence& seq = mAnimtaion->getBoneSequence(i);
			IPlatformManager::prefetch<PM_READ>( &seq + 1, sizeof(KeySequence) );
			IPlatformManager::prefetch<PM_WRITE>( &dest[i+1], sizeof(DualQuaternion) );

			assert( seq.getBoneIndex() == i );
			BoneDQ transform;

			if( seq.getFrameCount() > 1 )
			{
				const KeyFrame* next = seq.find(timePos);
				assert( next >= seq.begin() );
				const KeyFrame* kf = next - 1;

				IPlatformManager::prefetch<PM_READ>( kf, sizeof(KeyFrame)*2 );
				scalar prevTime = kf->getTimePos();

				if( next >= seq.end() )	//not found : probably fp precision error, or time out of range
				{
					assert( next == seq.getKeyFrame( seq.getFrameCount()-1 ) + 1);
					assert( seq.getKeyFrame( seq.getFrameCount()-1 )->getTimePos() <= timePos );
					assert( kf == seq.getKeyFrame( seq.getFrameCount()-1 ) );
					if( loop )
					{
						//usually loop animation start/end are almost the same,
						//align to first frame for pos overflow.
#if 0
						kf = seq.getKeyFrame( 0 );
						next = kf + 1;
						timePos -= prevTime;
						prevTime = kf->getTimePos();
#else
						next = seq.getKeyFrame(0);
						timePos = timePos - prevTime + Math::LOW_EPSILON;
						prevTime = 0;
#endif
					}
					else
					{
						next = kf;
						kf = next - 1;
					}
				}
				scalar nextTime = next->getTimePos();

				//interpolate between kf & next
				transform = kf->getTransform();
				if( kf != next && !Math::Equal<scalar>(prevTime, nextTime, Math::LOW_EPSILON) )
				{
					assert( nextTime > prevTime );
					assert(timePos >= prevTime );
					scalar t = (timePos - prevTime) / (nextTime - prevTime);
					assert( t >= -1e-1 && t < 1001e-1);
					if( t > Math::LOW_EPSILON )
					{
						//transform.sclerpWith(next->getTransform(), t, true);	//precise but slow
						transform.nlerpWith(next->getTransform(), t, true);		//fast approximate
					}
				}
			}
			else
				transform = seq.getKeyFrame( 0 )->getTransform();

			if( mBoneWeights != NULL )
				weight *= mBoneWeights[i];
			weight *= mWeight;

			/*
			from: Applications of Dual Quaternions in Three
			Dimensional Transformation and Interpolation

			Input:		Two skeletons S1 and S2 with identical structure and the interpolation parameter t
			Output:		The interpolated skeleton S3
						Create the structure of S3 with identical structure to S1 and S2 ;

			for Each bone B1 of S1 do
				Take the corresponding bone B2 of S2;
				Take the transformation r1 of B1 and r2 of B2;
				r3 = Interpolate(r1;r2;t);
				Set the transformation of the bone B3 to r3;
			end
			*/

			//DLB: fast approximate
			dest[i] += transform*weight;	//DIB iteration for accuracy: save transform & weight for iteration?
			//dest[i].normalize();

			//interpolation original & current
			//note: the weight is multiplied by parent weight already,
			//so *lerp is not correct with that weight
			//dest[i].sclerpWith(transform, weight);	//precise but slow
			//dest[i].nlerpWith(transform, weight);		//fast approximate
		}
	}

	//////////////////////////////////////////////////////////////////////////
	const BoneDQ&	AnimationState::getBoneTransform(uint32 boneIndex, scalar time) const
	{
		const KeySequence& seq = mAnimtaion->getBoneSequence(boneIndex);
		const KeyFrame* frame = seq.find(time);
		if( frame == seq.end() )
			--frame;
		return frame->getTransform();
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class BlendAtom : public IBlendNode, public Allocatable
	{
	public:
		BlendAtom(AnimationState* state) :mState(state)	{}
		~BlendAtom()	{}

		/** @brief  */
		virtual bool	blend(scalar timePos, DualQuaternion* target, scalar weight, const BoneActivity* boneAcitvity)
		{
			assert( mState != NULL );
			mState->setCurrentPos(timePos);
			mState->updateAnimation(target, boneAcitvity, weight);
			return true;
		}

		/** @brief  */
		virtual AnimationState* getAnimtaion() const		{return mState;}

		/** @brief  */
		virtual size_t	getChildCount() const				{return 0;}

		/** @brief  */
		virtual IBlendNode* getChild(index_t /*index*/) const	{return NULL;}

		/** @brief  */
		virtual scalar	getBlendWeight() const				{return mState->getBlendWeight();}

		/** @brief  */
		virtual bool	setBlendWeight(scalar weight)		{mState->setBlendWeight(weight); return true;}
	protected:
		AnimationState*		mState;
	};

	class BlendNode : public IBlendNode, public Allocatable
	{
	public:
		BlendNode(size_t initCount)
		{
			mChildren.reserve(initCount);
		}
		~BlendNode()
		{
			for(size_t i = 0; i < mChildren.size(); ++i)
				BLADE_DELETE mChildren[i];
		}

		/** @brief  */
		virtual bool	blend(scalar timePos, DualQuaternion* target, scalar weight, const BoneActivity* boneAcitvity)
		{
			weight *= mBlendWeight;
#if BLADE_DEBUG
			scalar total = 0;
#endif
			for(size_t i = 0; i < mChildren.size(); ++i)
			{
#if BLADE_DEBUG
				total += mChildren[i]->getBlendWeight();
#endif
				mChildren[i]->blend(timePos, target, weight, boneAcitvity);
			}
#if BLADE_DEBUG
			assert( Math::Equal<scalar>(total, 1.0, Math::LOW_EPSILON) );
#endif
			return true;
		}

		/** @brief  */
		virtual AnimationState* getAnimtaion() const		{return NULL;}

		/** @brief  */
		virtual size_t	getChildCount() const				{return mChildren.size();}

		/** @brief  */
		virtual IBlendNode* getChild(index_t index) const	{return index < mChildren.size() ? mChildren[index] : NULL;}

		/** @brief  */
		virtual scalar	getBlendWeight() const				{return mBlendWeight;}

		/** @brief  */
		virtual bool	setBlendWeight(scalar weight)		{mBlendWeight = weight; return true;}
	protected:
		typedef Vector<IBlendNode*>	BlendingList;
		BlendingList	mChildren;
		scalar			mBlendWeight;
	};


	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	Animation::Animation()
		:mAnimation(NULL)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	Animation::~Animation()
	{
		this->cleanup();
	}

	//////////////////////////////////////////////////////////////////////////
	bool	Animation::initialize(const AnimationTable* animationTable)
	{
		if( animationTable == mAnimation )
			return true;

		if( animationTable == NULL || mAnimation != NULL)
		{
			assert(false);
			return false;
		}
		mAnimation = animationTable;

		//init runtime data
		for(size_t i = 0; i < mAnimation->getAnimationResourceCount(); ++i)
		{
			const AnimationResource* animRes = mAnimation->getAnimationResource(i);
			assert( animRes != NULL );
			AnimationState* state = BLADE_NEW AnimationState(animRes, this);
			std::pair<AnimationSet::iterator,bool> ret = mStateSet.insert( state );
			assert(ret.second);
			if( !ret.second )
				BLADE_DELETE state;
		}

		if( mStateSet.size() > 0 )
			mBlendingRoot = BLADE_NEW BlendAtom( *mStateSet.begin() );
		else
			mBlendingRoot = NULL;

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	Animation::cleanup()
	{
		for( AnimationSet::iterator i = mStateSet.begin(); i != mStateSet.end(); ++i)
			BLADE_DELETE *i;
		mStateSet.clear();
		BLADE_DELETE mBlendingRoot;
		mBlendingRoot = NULL;
		mAnimation = NULL;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	Animation::update(scalar timePos, DualQuaternion* destTransforms, const BoneActivity* boneAcitvity)
	{
		assert(destTransforms != NULL);
		if( mBlendingRoot != NULL && destTransforms != NULL )
		{
			return mBlendingRoot->blend(timePos, destTransforms, 1.0f, boneAcitvity);
		}
		else
			return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	Animation::setAnimation(const TString& name)
	{
		//use temp object to find
		AnimationResource res(name, 0, 0, false, 0, NULL, NULL);
		AnimationState state(&res, this);
		AnimationSet::const_iterator iter = mStateSet.find(&state);
		if( iter != mStateSet.end() )
		{
			//temp hack.
			//TODO: fix it
			BLADE_DELETE mBlendingRoot;
			(*iter)->setCurrentPos(0);
			mBlendingRoot = BLADE_NEW BlendAtom( *iter );
			return true;
		}
		return false;
	}
	
}//namespace Blade