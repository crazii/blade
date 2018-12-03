/********************************************************************
	created:	2011/11/02
	filename: 	CameraActor.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_CameraActor_h__
#define __Blade_CameraActor_h__
#include "LogicActor.h"
#include "ICameraController.h"
#include <math/BladeMath.h>

namespace Blade
{
	class CameraActor : public LogicActor
	{
	public:
		inline CameraActor(IEntity* entity, IGeometry* geom, ICameraController* controller)
			:LogicActor(entity, geom)
			,mController(controller)
			,mMoveSpeed(1.0f)
			,mRotateSpeed(Math::HALF_PI)		{}
		virtual ~CameraActor()					{}

		/** @brief  */
		inline ICameraController*	getController() const	{return mController;}

		/** @brief  */
		inline IGraphicsCamera*		getCamera() const		{return mController->getCamera();}

		/** @brief  */
		inline void		setMoveSpeed(scalar speed)			{mMoveSpeed = speed;}

		/** @brief  */
		inline void		setRotateSpeed(scalar speed)		{mRotateSpeed = speed;}

		/** @brief  */
		inline scalar	getMoveSpeed() const				{return mMoveSpeed;}

		/** @brief  */
		inline scalar	getRotateSpeed() const				{return mRotateSpeed;}

	protected:
		ICameraController*	mController;
		scalar				mMoveSpeed;
		scalar				mRotateSpeed;		//angular speed in radians
	};//class CameraActor

}//namespace Blade



#endif // __Blade_CameraActor_h__