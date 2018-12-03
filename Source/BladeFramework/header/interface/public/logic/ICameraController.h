/********************************************************************
	created:	2011/11/02
	filename: 	ICameraController.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_ICameraController_h__
#define __Blade_ICameraController_h__
#include <BladeFramework.h>
#include <interface/public/IElement.h>

namespace Blade
{
	class IGraphicsCamera;

	class BLADE_FRAMEWORK_API ICameraController
	{
	public:
		virtual ~ICameraController()	{}

		/**
		@describe
		@param
		@return
		*/
		virtual IGraphicsCamera*getCamera() const = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual void			setFarDistance(scalar fFar) = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual scalar			getFarDistance() const = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual void			setNearDistance(scalar fNear) = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual scalar			getNearDistance() const = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual scalar			getAspectRatio() const = 0;

		/**
		@describe horizontal field-of-view
		@param
		@return
		*/
		virtual void			setFOV(scalar fFov) = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual scalar			getFOV() const = 0;
	};

	

}//namespace Blade



#endif // __Blade_ICameraController_h__