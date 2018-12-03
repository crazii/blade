/********************************************************************
	created:	2010/05/03
	filename: 	CameraController.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_CameraController_h__
#define __Blade_CameraController_h__
#include <Pimpl.h>
#include <interface/public/graphics/IGraphicsView.h>
#include <interface/public/graphics/CameraState.h>

#include <interface/public/logic/ICameraController.h>
#include <GameElement.h>

namespace Blade
{
	class BLADE_GAME_API CameraController : public ICameraController, public GameElement, public Allocatable
	{
	private:
		CameraController(const CameraController&);
		CameraController& operator=(const CameraController&);
	public:
		static const TString CAMERA_CONTROLLER_TYPE;
	public:
		CameraController();
		~CameraController();

		/************************************************************************/
		/* IElement interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual size_t			initParallelStates();

		/************************************************************************/
		/* ICameraController interface                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		virtual IGraphicsCamera*getCamera() const;

		/*
		@describe
		@param
		@return
		*/
		virtual void			setFarDistance(scalar fFar);

		/*
		@describe
		@param
		@return
		*/
		virtual scalar			getFarDistance() const;

		/*
		@describe
		@param
		@return
		*/
		virtual void			setNearDistance(scalar fNear);

		/*
		@describe
		@param
		@return
		*/
		virtual scalar			getNearDistance() const;

		/*
		@describe
		@param
		@return
		*/
		virtual scalar			getAspectRatio() const;

		/*
		@describe
		@param
		@return
		*/
		virtual void			setFOV(scalar fFovy);

		/*
		@describe
		@param
		@return
		*/
		virtual scalar			getFOV() const;

	protected:

		//data binding register
		friend class BladeGamePlugin;
		void			onConfigChange(void* data);
		scalar			mFarData;
		scalar			mNearData;
		scalar			mAspectRatioData;
		scalar			mFOVData;

		//parallel data
		ParaScalar		mFarPlane;
		ParaScalar		mNearPlane;
		ParaScalar		mAspect;
		ParaScalar		mFieldOfView;
		ParaCameraData	mCamera;
	private:

	};//class CameraController

	typedef Handle<CameraController>	HCAMERACTRLER;
	
}//namespace Blade



#endif //__Blade_CameraController_h__