/********************************************************************
	created:	2014/10/01
	filename: 	ModelViewTrack.h
	author:		Crazii
	
	purpose:	skeletal animation track for model viewer
*********************************************************************/
#ifndef __Blade_ModelViewTrack_h__
#define __Blade_ModelViewTrack_h__
#include <interface/public/ITrack.h>
#include <interface/ISyncAnimation.h>

namespace Blade
{
	class ModelViewTrack : public ITrack, public Allocatable
	{
	public:
		ModelViewTrack(ISyncAnimation* animation) :mAnimation(animation)	{}
		virtual ~ModelViewTrack()	{}

		/************************************************************************/
		/* ITrack interface                                                                     */
		/************************************************************************/

		/** @brief  */
		virtual scalar	getDuration() const
		{
			return mAnimation->getAnimationLength();
		}

		/** @brief  */
		virtual scalar	getFrameTime() const
		{
			return mAnimation->getFrameTime();
		}

		/** @brief get current play pos */
		virtual scalar	getPosition() const
		{
			return mAnimation->getAnimationPos();
		}

		/** @brief FEATURE_MASK  */
		virtual int		getFeatures() const
		{
			return TF_SEEK;
		}

		/** @brief  */
		virtual const tchar* getCurrentAnimation() const
		{
			return mAnimation->getCurrentAnimation();
		}

		/** @brief set animation position */
		virtual bool	setPosition(scalar pos)
		{
			return mAnimation->setAnimationPos(pos);
		}
	protected:
		ISyncAnimation*	mAnimation;
	};
	
}//namespace Blade


#endif // __Blade_ModelViewTrack_h__