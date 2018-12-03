/********************************************************************
	created:	2010/05/03
	filename: 	CameraController.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "CameraController.h"


namespace Blade
{
	const TString CameraController::CAMERA_CONTROLLER_TYPE = BTString("CameraController");

	//////////////////////////////////////////////////////////////////////////
	CameraController::CameraController()
		:GameElement(CAMERA_CONTROLLER_TYPE)
		,mFarPlane(&mFarData)
		,mNearPlane(&mNearData)
		,mAspect(&mAspectRatioData)
		,mFieldOfView(&mFOVData)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	CameraController::~CameraController()
	{
	}

	/************************************************************************/
	/* IElement interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	size_t			CameraController::initParallelStates()
	{
		mParallelStates[CameraState::CAMERA] = mCamera;
		mParallelStates[CameraState::FAR_PLANE] = mFarPlane;
		mParallelStates[CameraState::NEAR_PLANE] = mNearPlane;
		mParallelStates[CameraState::FIELD_OF_VIEW] = mFieldOfView;
		mParallelStates[CameraState::ASPECT] = mAspect;

		mCamera.setPriority(PP_LOWEST);
		mAspect.setPriority(PP_LOWEST);

		mCamera.setUnTouched(NULL);
		mFarPlane = 1500.0f;
		mNearPlane = 0.2f;
		mAspect = 1.3333f;
		mFieldOfView = Math::Degree2Radian(86);
		return GameElement::initParallelStates();
	}

	/************************************************************************/
	/* ICameraController interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	IGraphicsCamera*CameraController::getCamera() const
	{
		return mCamera;
	}

	//////////////////////////////////////////////////////////////////////////
	void			CameraController::setFarDistance(scalar fFar)
	{
		mFarPlane = fFar;
	}

	//////////////////////////////////////////////////////////////////////////
	scalar			CameraController::getFarDistance() const
	{
		return mFarPlane;
	}

	//////////////////////////////////////////////////////////////////////////
	void			CameraController::setNearDistance(scalar fNear)
	{
		mNearPlane = fNear;
	}

	//////////////////////////////////////////////////////////////////////////
	scalar			CameraController::getNearDistance() const
	{
		return mNearPlane;
	}

	//////////////////////////////////////////////////////////////////////////
	scalar			CameraController::getAspectRatio() const
	{
		return mAspect;
	}

	//////////////////////////////////////////////////////////////////////////
	void			CameraController::setFOV(scalar fFov)
	{
		mFieldOfView = fFov;
	}

	//////////////////////////////////////////////////////////////////////////
	scalar			CameraController::getFOV() const
	{
		return mFieldOfView;
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void			CameraController::onConfigChange(void* data)
	{
		if( data == &mFarData)
		{
			mFarPlane.applyChange();
		}
		else if( data == &mNearData )
		{
			mNearPlane.applyChange();
		}
		else if( data == &mAspectRatioData )
		{
			mAspect.applyChange();
		}
		else if( data == &mFOVData )
		{
			mFieldOfView.applyChange();
		}
	}
	
}//namespace Blade