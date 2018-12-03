/********************************************************************
	created:	2010/06/28
	filename: 	IGraphicsCamera.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_IGraphicsCamera_h__
#define __Blade_IGraphicsCamera_h__
#include <math/Ray.h>
#include <math/Vector2.h>
#include <math/Vector3.h>
#include <math/Quaternion.h>
#include <interface/public/graphics/GraphicsDefs.h>
#include <interface/Interface.h>

namespace Blade
{
	class Matrix44;
	class Sphere;
	class IEntity;

	class IGraphicsCamera : public Interface
	{
	public:
		virtual ~IGraphicsCamera()	{}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual IEntity*getEntity() const = 0;

		/**
		@describe 
		@param [in] x,y the normalized position in view
		@return 
		*/
		virtual Ray		getSpaceRayfromViewPoint(const POINT2& pos) const = 0;
		inline Ray getSpaceRayfromViewPoint(scalar x,scalar y) const
		{
			return this->getSpaceRayfromViewPoint( POINT2(x,y) );
		}

		/**
		@describe project space point to normalized view position
		@param 
		@return 
		*/
		virtual POINT3	getProjectedPosition(const POINT3& pos) const = 0;
		inline POINT3 getProjectedPosition(scalar x, scalar y, scalar z) const
		{
			return this->getProjectedPosition(POINT3(x, y, z));
		}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool	isVisible(const AABB& AAB) const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool	isVisible(const Sphere& sphere) const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void			setProjectionType(PROJECTION ProjType) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void			setNearClipDistance(scalar fNear) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void			setFarClipDistance(scalar fFar) = 0;

		/**
		@describe set horizontal field of view 
		@param 
		@return 
		*/
		virtual void			setFieldOfView(scalar fHorizFov) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void			setAspectRatio(scalar fAspectRatio) = 0;

		/**
		@describe if you don't know whether it is perspective or orthographic
		@param 
		@return 
		*/
		virtual void			setAspectRatio(scalar w,scalar h) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void			setOrthoMode(scalar w,scalar h) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void			setEyePosition(const Vector3& pos) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void			setOrientation(const Quaternion& orientation) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void			setViewMatrix(const Matrix44& view, bool fixedView = false) = 0;

		/**
		@describe 
		@param 
		@return 
		@note: projection matrix is affected by projection type, FOV,aspect etc.
		so if FOV/aspect changes, the projection matrix set here will be overwritten, except that fixedProjection is set
		*/
		virtual void			setProjectionMatrix(const Matrix44& projection, bool fixedProjection = false) = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual void			setCullingMask(AppFlag mask) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual PROJECTION		getProjectionType() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual scalar			getNearClipDistance() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual scalar			getFarClipDistance() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual	scalar			getFieldOfView() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual scalar			getAspectRatio() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual scalar			getOrthoModeWidth() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual scalar			getOrthoModeHeight() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const Matrix44&	getViewMatrix() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const Matrix44&	getProjectionMatrix() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual  const Vector3&	getEyePosition() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const Quaternion&	getOrientation() const = 0;

		/** @brief look at direction (at world space) */
		inline Vector3 getLookAtDirection() const
		{
			return Vector3::NEGATIVE_UNIT_Z*this->getOrientation();
		}

		/** @brief camera's right direction vector */
		inline Vector3 getRightDirection() const
		{
			return Vector3::UNIT_X*this->getOrientation();
		}

		/** @brief up vector */
		inline Vector3 getUpDirection() const
		{
			return Vector3::UNIT_Y*this->getOrientation();
		}

		/**
		@describe
		@param
		@return
		*/
		virtual AppFlag			getCulingMask() const = 0;
	};

	typedef InterfaceID<IGraphicsCamera> IID_CAMERA;
	
}//namespace Blade


#endif //__Blade_IGraphicsCamera_h__