/********************************************************************
	created:	2010/05/08
	filename: 	Frustum.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_Frustum_h__
#define __Blade_Frustum_h__
#include <BladeGraphics.h>
#include <math/Matrix44.h>
#include <math/Plane.h>
#include <interface/ICamera.h>
#include <math/Line2.h>
#include <algorithm>

namespace Blade
{
	class Sphere;

	class Frustum : public Allocatable
	{
	public:
		/*
		+----------------+
		|\		1		/|
		| \			   / |
		|  \_________ /	 |
		| 2|	0(5) |	 |
		|  |_________|4	 |
		| /			  \	 |
		|/		3	   \ |
		+----------------+
		*/

		//near	0
		//far	5
		typedef enum EFrustumPlaneIndex
		{
			FPI_NEAR = 0,
			FPI_TOP,
			FPI_LEFT,
			FPI_BOTTOM,
			FPI_RIGHT,
			FPI_FAR,

			FPI_COUNT,
		}PLANE;

		enum EFrustumPlaneMask
		{
			FPM_NEAR	= 1<<FPI_NEAR,
			FPM_TOP		= 1<<FPI_TOP,
			FPM_LEFT	= 1<<FPI_LEFT,
			FPM_BOTTOM	= 1<<FPI_BOTTOM,
			FPM_RIGHT	= 1<<FPI_RIGHT,
			FPM_FAR		= 1<<FPI_FAR,

			FPM_ALL = FPM_NEAR | FPM_TOP | FPM_LEFT | FPM_BOTTOM | FPM_RIGHT | FPM_FAR,
		};

		typedef enum EFrustumCornerIndex
		{
			FCI_NEAR_LEFT_TOP = 0,
			FCI_NEAR_LEFT_BOTTOM,
			FCI_NEAR_RIGHT_BOTTOM,
			FCI_NEAR_RIGHT_TOP,

			FCI_FAR_LEFT_TOP,
			FCI_FAR_LEFT_BOTTOM,
			FCI_FAR_RIGHT_BOTTOM,
			FCI_FAR_RIGHT_TOP,

			FCI_COUNT,
		}CORNER;

		//XZ plane projects
		typedef struct SFrustumProjectionXZ : public Allocatable
		{
			SFrustumProjectionXZ(){}
			Line2		mBoundLine[6];
			size_t		mCount;

			/** @brief  */
			BLADE_GRAPHICS_API bool		isVisible(const Box2& box) const;

			/** @brief  */
			BLADE_GRAPHICS_API bool		isVisible(const Vector2& point) const;

			/** @brief  */
			BLADE_GRAPHICS_API bool		isVisible(const Vector2& boxcenter,const Vector2& halfsize) const;

			/** @brief  */
			BLADE_GRAPHICS_API ICamera::VISIBILITY	getVisibility(const Box2& box) const;

			/** @brief  */
			BLADE_GRAPHICS_API ICamera::VISIBILITY	getVisibility(const Vector2& boxcenter,const Vector2& halfsize) const;

			/** @brief  */
			void		checkOrder();
		}Frustum2;

	public:
		Frustum();
		~Frustum();

		/*
		@describe
		@param
		@return
		*/
		BLADE_GRAPHICS_API void				set(const Matrix44& view,const Matrix44& projection, EProjectionType projType);

		/*
		@describe
		@param
		@return
		*/
		BLADE_GRAPHICS_API void				setViewMatrix(const Matrix44& view);

		/*
		@describe
		@param
		@return
		*/
		BLADE_GRAPHICS_API void				setProjectionMatrix(const Matrix44& projection, EProjectionType projType);

		/*
		@describe
		@param
		@return
		*/
		BLADE_GRAPHICS_API const Plane&		getFrustumPlane(PLANE index) const;

		/*
		@describe
		@param
		@return
		*/
		BLADE_GRAPHICS_API const Plane*		getFrustumPlanes() const;

		/*
		@describe
		@param
		@return
		*/
		BLADE_GRAPHICS_API const Vector4&		getFrustumCorner(CORNER index) const;

		/*
		@describe
		@param
		@return
		*/
		BLADE_GRAPHICS_API  const Vector4*		getFrustumCorners() const;

		inline const Vector4&		getEyePosition() const
		{
			return this->getFrustumCorners()[8];
		}

		/*
		@describe
		@param
		@return
		*/
		BLADE_GRAPHICS_API const Vector3&		getLookAtDirection() const;

		/*
		@describe 
		@param 
		@return 
		*/
		BLADE_GRAPHICS_API scalar				getNearClipDistance() const;

		/*
		@describe 
		@param 
		@return 
		*/
		BLADE_GRAPHICS_API scalar				getFarClipDistance() const;

		/*
		@describe
		@param
		@return
		*/
		BLADE_GRAPHICS_API const Frustum2&		getProjectionXZ() const;

		/*
		@describe
		@param
		@return
		*/
		BLADE_GRAPHICS_API bool				isVisible(const Vector3& point) const;

		/*
		@describe
		@param
		@return
		*/
		BLADE_GRAPHICS_API bool				isVisible(const AABB& box) const;

		/*
		@describe
		@param
		@return
		*/
		BLADE_GRAPHICS_API bool				isVisible(const Sphere& sphere) const;

		/*
		@describe
		@param
		@return
		*/
		BLADE_ALWAYS_INLINE ICamera::VISIBILITY			getVisibility(const AABB& box, unsigned int* inoutMask = NULL) const
		{
			if (box.isInfinite())
				return ICamera::CV_ALL;
			else if (box.isNull())
				return ICamera::CV_NULL;
			const POINT3 center = box.getCenter();
			const Vector3 half = box.getHalfSize();
			return this->getVisibility(center, half, inoutMask);
		}

		/*
		@describe
		@param
		@return
		*/
		BLADE_GRAPHICS_API ICamera::VISIBILITY			getVisibility(const POINT3& boxcenter,const Vector3& halfsize, unsigned int* inoutMask = NULL) const;

		/**
		@describe 
		@param
		@return
		*/
		BLADE_GRAPHICS_API	void			updateFrustumPlanes();

		/** @brief  */
		const Matrix44&						getViewMatrix() const { return mViewMatrix; }

		/** @brief  */
		const Matrix44&						getProjectionMatrix() const { return mProjectionMatrix; }

		/** @brief  */
		BLADE_GRAPHICS_API static const CORNER LINES[12][2];

		/** @brief enable near far plane while doing visibility test. default: FALSE */
		inline void							enableNearFarPlane(bool enable)
		{
			mEnableNearFarPlane = enable;
		}

		/** @brief  */
		inline void							enableExtraCullingPlane(const Plane planes[3] = NULL, size_t count = 0)
		{
			mEnableExtraPlane = (planes != NULL && count > 0);
			if (mEnableExtraPlane)
			{
				mExtraPlaneCount = (int)std::min<size_t>(5u, count);
				std::memcpy(mExtraPlanes, planes, sizeof(Plane)*mExtraPlaneCount);
			}
		}

	protected:
		/*
		@describe
		@param
		@return
		*/
		BLADE_GRAPHICS_API void				calculcateLocalNearCorner() const;

		/** @brief  */
		BLADE_GRAPHICS_API void				calucateXZProjection(const Vector2 corner[8],const Vector2& eye) const;

		mutable Plane		mFrustumPlanes[6];
		mutable Vector4		mFrustumCorners[9];
		mutable scalar		mLocalNearCorners[4];
		Plane				mExtraPlanes[5];	//hard code: hack for shadows, use OR operation
		mutable	Vector3		mLookatDirection;
		Matrix44			mViewMatrix;
		Matrix44			mProjectionMatrix;

		mutable	Frustum2*	m2DProjection;
		EProjectionType		mProjectionType : 2;
		int					mExtraPlaneCount : 10;
		mutable bool		mIsCalcLocalCornerNeeded : 1;
		mutable	bool		mIsCalcCornerNeeded : 1;
		mutable bool		mIsCalc2DProjectionNeeded : 1;
		mutable bool		mIsCalcDirectionNeeded : 1;
		bool				mEnableNearFarPlane : 1;
		bool				mEnableExtraPlane : 1;
	};//class Frustum
	
}//namespace Blade


#endif //__Blade_Frustum_h__