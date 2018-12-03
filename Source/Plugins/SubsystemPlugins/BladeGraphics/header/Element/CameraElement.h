/********************************************************************
	created:	2010/04/28
	filename: 	CameraElement.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_CameraElement_h__
#define __Blade_CameraElement_h__
#include <Element/GraphicsElement.h>
#include <interface/ICamera.h>
#include <interface/public/graphics/CameraState.h>

namespace Blade
{

	class CameraElement : public GraphicsElement, public Allocatable
	{
	public:
		CameraElement();
		~CameraElement();

		/************************************************************************/
		/* IElement interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual size_t		initParallelStates();

		/** @brief  */
		virtual void		onParallelStateChange(const IParaState& data);

		/** @brief  */
		virtual Interface*			getInterface(InterfaceName type)
		{
			CHECK_RETURN_INTERFACE(IGraphicsCamera, type, mCamera);
			return GraphicsElement::getInterface(type);
		}

		/************************************************************************/
		/* GraphicsElement interface                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		virtual void				onInitialize();

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		ICamera*					getCamera() const { return mCamera; }

	protected:
		ICamera*		mCamera;

		ParaCameraData	mCameraData;
		ParaScalar		mNear;
		ParaScalar		mFar;
		ParaScalar		mAspect;
		ParaScalar		mFov;
	};//class CameraElement


	class MainCameraElement : public CameraElement
	{
	public:
		//internal hacking
		static const TString MAIN_CAMERA_ELEMENT;

		MainCameraElement()
		{
			mDesc->mType = MAIN_CAMERA_ELEMENT;
		}
	};
	
}//namespace Blade



#endif //__Blade_CameraElement_h__