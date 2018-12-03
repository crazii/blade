/********************************************************************
	created:	2010/04/28
	filename: 	CameraElement.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <Element/CameraElement.h>
#include <interface/public/graphics/IGraphicsView.h>
#include <interface/public/graphics/GraphicsInterface_blang.h>
#include <interface/IRenderScheme.h>
#include "../Camera.h"

namespace Blade
{
	const TString MainCameraElement::MAIN_CAMERA_ELEMENT = BTString("_MAIN_CAMERA_ELEMENT_");

	//////////////////////////////////////////////////////////////////////////
	CameraElement::CameraElement()
		:GraphicsElement( BXLang(BLANG_CAMERA_ELEMENT) )
		,mCamera(NULL)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	CameraElement::~CameraElement()
	{
		BLADE_DELETE mCamera;
	}

	/************************************************************************/
	/* IElement interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	size_t				CameraElement::initParallelStates()
	{
		mParallelStates[CameraState::CAMERA] = mCameraData;
		mParallelStates[CameraState::FAR_PLANE] = mFar;
		mParallelStates[CameraState::NEAR_PLANE] = mNear;
		mParallelStates[CameraState::FIELD_OF_VIEW] = mFov;
		mParallelStates[CameraState::ASPECT] = mAspect;

		mAspect.setPriority((uint32)PP_HIGHEST);
		mAspect.setUnTouched(16.0f / 9.0f);
		mCameraData = static_cast<IGraphicsCamera*>(mCamera);

		return GraphicsElement::initParallelStates();
	}

	//////////////////////////////////////////////////////////////////////////
	void				CameraElement::onParallelStateChange(const IParaState& data)
	{
		if( data == mFar )
		{
			mCamera->setFarClipDistance( mFar );
		}
		else if( data == mNear )
		{
			mCamera->setNearClipDistance( mNear );
		}
		else if( data == mFov )
		{
			mCamera->setFieldOfView( mFov );
		}
		else if( data == mAspect )
		{
			mCamera->setAspectRatio( mAspect );
		}
		else
			return GraphicsElement::onParallelStateChange(data);
	}

	/************************************************************************/
	/* GraphicsElement interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void				CameraElement::onInitialize()
	{
		Camera* cam = BLADE_NEW Camera();
		if (mDesc->mType == MainCameraElement::MAIN_CAMERA_ELEMENT)
			cam->setMainCamera(cam);
		else
			cam->setMainCamera(this->getRenderScene()->getMainCameraImpl());
		mCamera = cam;
		mContent = static_cast<Camera*>(mCamera);
		cam->setElement(this);
	}
	
}//namespace Blade