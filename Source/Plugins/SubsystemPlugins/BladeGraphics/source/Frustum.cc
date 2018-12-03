/********************************************************************
	created:	2010/05/08
	filename: 	Frustum.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <Frustum.h>
#include <math/AxisAlignedBox.h>
#include <math/Sphere.h>
#include <math/Box2.h>

//near plane config

namespace Blade
{
	const Frustum::CORNER Frustum::LINES[12][2] =
	{
		Frustum::FCI_NEAR_LEFT_TOP, Frustum::FCI_NEAR_LEFT_BOTTOM,
		Frustum::FCI_NEAR_LEFT_TOP, Frustum::FCI_NEAR_RIGHT_TOP,
		Frustum::FCI_NEAR_LEFT_BOTTOM, Frustum::FCI_NEAR_RIGHT_BOTTOM,
		Frustum::FCI_NEAR_RIGHT_TOP, Frustum::FCI_NEAR_RIGHT_BOTTOM,

		Frustum::FCI_FAR_LEFT_TOP, Frustum::FCI_FAR_LEFT_BOTTOM,
		Frustum::FCI_FAR_LEFT_TOP, Frustum::FCI_FAR_RIGHT_TOP,
		Frustum::FCI_FAR_LEFT_BOTTOM, Frustum::FCI_FAR_RIGHT_BOTTOM,
		Frustum::FCI_FAR_RIGHT_TOP, Frustum::FCI_FAR_RIGHT_BOTTOM,

		Frustum::FCI_NEAR_LEFT_TOP, Frustum::FCI_FAR_LEFT_TOP,
		Frustum::FCI_NEAR_LEFT_BOTTOM, Frustum::FCI_FAR_LEFT_BOTTOM,
		Frustum::FCI_NEAR_RIGHT_BOTTOM, Frustum::FCI_FAR_RIGHT_BOTTOM,
		Frustum::FCI_NEAR_RIGHT_TOP, Frustum::FCI_FAR_RIGHT_TOP,
	};


	//////////////////////////////////////////////////////////////////////////
	Frustum::Frustum()
		:mViewMatrix( Matrix44::IDENTITY )
		,mProjectionMatrix( Matrix44::IDENTITY )
		,m2DProjection(NULL)
		,mProjectionType(PT_ORTHOGRAPHIC)
		,mExtraPlaneCount(0)
		,mIsCalcLocalCornerNeeded(false)
		,mIsCalcCornerNeeded(false)
		,mEnableNearFarPlane(false)
		,mEnableExtraPlane(false)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	Frustum::~Frustum()
	{
		BLADE_DELETE m2DProjection;
	}

	//////////////////////////////////////////////////////////////////////////
	void				Frustum::set(const Matrix44& view,const Matrix44& projection,EProjectionType projType)
	{
		mProjectionType = projType;
		mViewMatrix = view;
		mProjectionMatrix = projection;
		mIsCalcCornerNeeded = true;
		mIsCalcLocalCornerNeeded = true;
		mIsCalc2DProjectionNeeded = true;
		mIsCalcDirectionNeeded = true;
	}

	//////////////////////////////////////////////////////////////////////////
	void				Frustum::setViewMatrix(const Matrix44& view)
	{
		mViewMatrix = view;
		mIsCalcCornerNeeded = true;
		mIsCalc2DProjectionNeeded = true;
		mIsCalcDirectionNeeded = true;
	}

	//////////////////////////////////////////////////////////////////////////
	void				Frustum::setProjectionMatrix(const Matrix44& projection,EProjectionType projType)
	{
		mProjectionType = projType;
		mProjectionMatrix = projection;
		mIsCalcLocalCornerNeeded = true;
		mIsCalc2DProjectionNeeded = true;
		mIsCalcCornerNeeded = true;
	}

	//////////////////////////////////////////////////////////////////////////
	const Plane&		Frustum::getFrustumPlane(PLANE index) const
	{
		if( index <= FPI_FAR && index >= FPI_NEAR )
			return this->getFrustumPlanes()[index];
		else
			BLADE_EXCEPT(EXC_OUT_OF_RANGE,BTString("plane index out of range.") );
	}

	//////////////////////////////////////////////////////////////////////////
	const Plane*		Frustum::getFrustumPlanes() const
	{
		return mFrustumPlanes;
	}

	//////////////////////////////////////////////////////////////////////////
	const Vector4&		Frustum::getFrustumCorner(CORNER index) const
	{
		if( index < 8 && index >= 0 )
			return this->getFrustumCorners()[index];
		else
			BLADE_EXCEPT(EXC_OUT_OF_RANGE,BTString("plane index out of range.") );
	}

	//////////////////////////////////////////////////////////////////////////
	const Vector4*		Frustum::getFrustumCorners() const
	{
		if( mIsCalcCornerNeeded )
		{
			this->calculcateLocalNearCorner();
			scalar fNear, fFar;
			if( mProjectionType == PT_PERSPECTIVE )
			{
				fNear	= mProjectionMatrix[3][2] / (mProjectionMatrix[2][2] - 1);
				fFar	= mProjectionMatrix[3][2] / (mProjectionMatrix[2][2] + 1);
			}
			else
			{
				fFar	= (mProjectionMatrix[3][2]-1)/mProjectionMatrix[2][2];
				fNear	= (mProjectionMatrix[3][2]+1)/mProjectionMatrix[2][2];
			}

			scalar ratio = mProjectionType == PT_PERSPECTIVE ? fFar / fNear : 1;
			scalar farLeft = mLocalNearCorners[FCI_NEAR_LEFT_TOP] * ratio;
			scalar farTop = mLocalNearCorners[FCI_NEAR_LEFT_BOTTOM] * ratio;
			scalar farRight = mLocalNearCorners[FCI_NEAR_RIGHT_BOTTOM] * ratio;
			scalar farBottom = mLocalNearCorners[FCI_NEAR_RIGHT_TOP] * ratio;

			//local to world
			Matrix44 invView = mViewMatrix.getInverse();

			// near
			mFrustumCorners[FCI_NEAR_LEFT_TOP]		= Vector3(mLocalNearCorners[0],	mLocalNearCorners[1],	-fNear)*invView;
			mFrustumCorners[FCI_NEAR_LEFT_BOTTOM]	= Vector3(mLocalNearCorners[0],	mLocalNearCorners[3],	-fNear)*invView;
			mFrustumCorners[FCI_NEAR_RIGHT_BOTTOM]	= Vector3(mLocalNearCorners[2],	mLocalNearCorners[3],	-fNear)*invView;
			mFrustumCorners[FCI_NEAR_RIGHT_TOP]		= Vector3(mLocalNearCorners[2],	mLocalNearCorners[1],	-fNear)*invView;
			// far
			mFrustumCorners[FCI_FAR_LEFT_TOP] = Vector3(farLeft,	farTop,		-fFar)*invView;
			mFrustumCorners[FCI_FAR_LEFT_BOTTOM] = Vector3(farLeft,	farBottom,	-fFar)*invView;
			mFrustumCorners[FCI_FAR_RIGHT_BOTTOM] = Vector3(farRight,farBottom,	-fFar)*invView;
			mFrustumCorners[FCI_FAR_RIGHT_TOP] = Vector3(farRight,farTop,		-fFar)*invView;

			//add eye pos
			mFrustumCorners[8] = reinterpret_cast<const Vector3&>(invView[3]);
			mIsCalcCornerNeeded = false;
		}

		return mFrustumCorners;
	}

	//////////////////////////////////////////////////////////////////////////
	const Vector3&		Frustum::getLookAtDirection() const
	{
		if( mIsCalcDirectionNeeded )
		{
			Matrix44 invView = mViewMatrix.getInverse();
			//local dir
			POINT3 dir = POINT3::NEGATIVE_UNIT_Z;
			mLookatDirection = dir * static_cast<const Matrix33&>(invView);
		}
		return mLookatDirection;
	}

	//////////////////////////////////////////////////////////////////////////
	scalar				Frustum::getNearClipDistance() const
	{
		if( mProjectionType == PT_PERSPECTIVE )
			return mProjectionMatrix[3][2] / (mProjectionMatrix[2][2] - 1);
		else
			return (mProjectionMatrix[3][2]+1)/mProjectionMatrix[2][2];
	}

	//////////////////////////////////////////////////////////////////////////
	scalar				Frustum::getFarClipDistance() const
	{
		if( mProjectionType == PT_PERSPECTIVE )
			return mProjectionMatrix[3][2] / (mProjectionMatrix[2][2] + 1);
		else
			return (mProjectionMatrix[3][2]-1)/mProjectionMatrix[2][2];
	}

	//////////////////////////////////////////////////////////////////////////
	const Frustum::Frustum2&		Frustum::getProjectionXZ() const
	{
		if( m2DProjection == NULL )
			m2DProjection = BLADE_NEW Frustum2();

		if( !mIsCalc2DProjectionNeeded )
			return *m2DProjection;

		mIsCalc2DProjectionNeeded = false;

		const Vector3& eyePos = this->getEyePosition();
		Vector2 eyePos2d( eyePos.x,eyePos.z);

		const Vector4* corner = this->getFrustumCorners();
		Vector2 corner2d[8];
		for( int i = 0; i < 8 ; ++i)
		{
			corner2d[i].x = corner[i].x;
			corner2d[i].y = corner[i].z;
		}

		this->calucateXZProjection(corner2d,eyePos2d);

		//check if the frustum is upside down
		m2DProjection->checkOrder();

		return *m2DProjection;

	}

	//////////////////////////////////////////////////////////////////////////
	bool				Frustum::isVisible(const Vector3& point) const
	{
		int planeStart = !mEnableNearFarPlane ? FPI_NEAR + 1 : FPI_NEAR;
		int planeEnd = !mEnableNearFarPlane ? FPI_FAR - 1 : FPI_FAR;

		//AND
		for(int i = planeStart; i <= planeEnd; ++i)
		{
			if( mFrustumPlanes[i].getSide(point, Math::LOW_EPSILON) == PS_NEGATIVE )
				return false;
		}

		if (mExtraPlaneCount > 0)
		{
			//OR
			for (int i = 0; i < mExtraPlaneCount; ++i)
			{
				if (mExtraPlanes[i].getSide(point, Math::LOW_EPSILON) == PS_POSITIVE)
					return true;
			}
			return false;
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				Frustum::isVisible(const AABB& box) const
	{
		if( box.isNull() )
			return false;
		else if( box.isInfinite() )
			return true;
		this->getFrustumPlanes();

		const Vector3 center = box.getCenter();
		const Vector3 half = box.getHalfSize();

		int planeStart = !mEnableNearFarPlane ? FPI_NEAR + 1 : FPI_NEAR;
		int planeEnd = !mEnableNearFarPlane ? FPI_FAR - 1 : FPI_FAR;

		//AND
		for(int i = planeStart; i <= planeEnd; ++i)
		{
			if(mFrustumPlanes[i].getSide(center,half) == PS_NEGATIVE )
				return false;
		}

		//OR
		if (mExtraPlaneCount > 0)
		{
			for (int i = 0; i < mExtraPlaneCount; ++i)
			{
				if (mExtraPlanes[i].getSide(center, half) == PS_POSITIVE)
					return true;
			}
			return false;
		}

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				Frustum::isVisible(const Sphere& sphere) const
	{
		int planeStart = !mEnableNearFarPlane ? FPI_NEAR + 1 : FPI_NEAR;
		int planeEnd = !mEnableNearFarPlane ? FPI_FAR - 1 : FPI_FAR;

		//AND
		for(int i = planeStart; i <= planeEnd; ++i)
		{
			scalar distance = mFrustumPlanes[i].getDistance( sphere.getCenter() );
			if( distance < -sphere.getRadius() )
				return false;
		}

		//OR
		if (mExtraPlaneCount)
		{
			for (int i = 0; i < mExtraPlaneCount; ++i)
			{
				scalar distance = mExtraPlanes[i].getDistance(sphere.getCenter());
				if (distance >= -sphere.getRadius())
					return true;
			}
			return  false;
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	ICamera::VISIBILITY			Frustum::getVisibility(const POINT3& boxcenter, const Vector3& halfsize, unsigned int* inoutMask/* = NULL*/) const
	{
		ICamera::VISIBILITY vis = ICamera::CV_ALL;

		unsigned int defaultMask = FPM_ALL;
		unsigned int& planeMask = inoutMask == NULL ? defaultMask : *inoutMask;

		if (!mEnableNearFarPlane)
		{
			planeMask &= ~FPM_NEAR;
			planeMask &= ~FPM_FAR;
		}
		int planeStart = !mEnableNearFarPlane ? FPI_NEAR + 1 : FPI_NEAR;
		int planeEnd = !mEnableNearFarPlane ? FPI_FAR - 1 : FPI_FAR;

		//AND
		for(int i = planeStart; i <= planeEnd; ++i)
		{
			if( !( (planeMask &(1<<i)) ) )
				continue;

			POS_SIDE side = mFrustumPlanes[i].getSide(boxcenter, halfsize);
			if( side == PS_NEGATIVE )
				return ICamera::CV_NULL;
			else if( side == PS_BOTH )
				vis = ICamera::CV_PARTIAL;
			else
				planeMask &= ~(1<<i);
		}
		assert(vis != ICamera::CV_ALL || planeMask == 0);

		if (mExtraPlaneCount > 0)
		{
			//OR
			for (int i = 0; i < mExtraPlaneCount; ++i)
			{
				if (!((planeMask &(1 << (i + 6)))))
					return vis;

				POS_SIDE side = mExtraPlanes[i].getSide(boxcenter, halfsize);
				if (side == PS_POSITIVE)
				{
					planeMask &= ~(1 << (i + 6));
					return vis;
				}
				else if (side == PS_BOTH)
					return vis;
			}
			return ICamera::CV_NULL;
		}

		return vis;
	}

	//////////////////////////////////////////////////////////////////////////
	void				Frustum::updateFrustumPlanes()
	{
		Matrix44 VP = mViewMatrix * mProjectionMatrix;

		mFrustumPlanes[FPI_LEFT].mNormal.x = VP[0][3] + VP[0][0];
		mFrustumPlanes[FPI_LEFT].mNormal.y = VP[1][3] + VP[1][0];
		mFrustumPlanes[FPI_LEFT].mNormal.z = VP[2][3] + VP[2][0];
		mFrustumPlanes[FPI_LEFT].mD = VP[3][3] + VP[3][0];

		mFrustumPlanes[FPI_RIGHT].mNormal.x = VP[0][3] - VP[0][0];
		mFrustumPlanes[FPI_RIGHT].mNormal.y = VP[1][3] - VP[1][0];
		mFrustumPlanes[FPI_RIGHT].mNormal.z = VP[2][3] - VP[2][0];
		mFrustumPlanes[FPI_RIGHT].mD = VP[3][3] - VP[3][0];

		mFrustumPlanes[FPI_TOP].mNormal.x = VP[0][3] - VP[0][1];
		mFrustumPlanes[FPI_TOP].mNormal.y = VP[1][3] - VP[1][1];
		mFrustumPlanes[FPI_TOP].mNormal.z = VP[2][3] - VP[2][1];
		mFrustumPlanes[FPI_TOP].mD = VP[3][3] - VP[3][1];

		mFrustumPlanes[FPI_BOTTOM].mNormal.x = VP[0][3] + VP[0][1];
		mFrustumPlanes[FPI_BOTTOM].mNormal.y = VP[1][3] + VP[1][1];
		mFrustumPlanes[FPI_BOTTOM].mNormal.z = VP[2][3] + VP[2][1];
		mFrustumPlanes[FPI_BOTTOM].mD = VP[3][3] + VP[3][1];

		mFrustumPlanes[FPI_NEAR].mNormal.x = VP[0][3] + VP[0][2];
		mFrustumPlanes[FPI_NEAR].mNormal.y = VP[1][3] + VP[1][2];
		mFrustumPlanes[FPI_NEAR].mNormal.z = VP[2][3] + VP[2][2];
		mFrustumPlanes[FPI_NEAR].mD = VP[3][3] + VP[3][2];

		mFrustumPlanes[FPI_FAR].mNormal.x = VP[0][3] - VP[0][2];
		mFrustumPlanes[FPI_FAR].mNormal.y = VP[1][3] - VP[1][2];
		mFrustumPlanes[FPI_FAR].mNormal.z = VP[2][3] - VP[2][2];
		mFrustumPlanes[FPI_FAR].mD = VP[3][3] - VP[3][2];

		// re-normalize any normals which were not unit length
		for (int i = 0; i < 6; i++)
			mFrustumPlanes[i].normalize();
	}

	//////////////////////////////////////////////////////////////////////////
	void				Frustum::calculcateLocalNearCorner() const
	{
		if( mIsCalcLocalCornerNeeded )
		{
			//left
			mLocalNearCorners[0] = scalar(-1) / mProjectionMatrix[0][0];
			//right
			mLocalNearCorners[2] = -mLocalNearCorners[0];
			//top
			mLocalNearCorners[1] = scalar(1) / mProjectionMatrix[1][1];
			//bottom
			mLocalNearCorners[3] = -mLocalNearCorners[1];

			if( mProjectionType == PT_PERSPECTIVE )
			{
				scalar fNear	= mProjectionMatrix[3][2] / (mProjectionMatrix[2][2] - 1);
				mLocalNearCorners[0] *= fNear;
				mLocalNearCorners[1] *= fNear;
				mLocalNearCorners[2] *= fNear;
				mLocalNearCorners[3] *= fNear;
			}

			mIsCalcLocalCornerNeeded = false;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void				Frustum::calucateXZProjection(const Vector2 corner2d[8],const Vector2& eyePos2d) const
	{
		/*
		+----------------+
		|\				/|
		| \			   / | 
		 \ +----------+ /
		  \|__________|/
		*/
		
		CORNER indexes[8] =
		{
			//normal
			FCI_FAR_RIGHT_BOTTOM,
			FCI_FAR_RIGHT_TOP,
			FCI_FAR_LEFT_TOP,
			FCI_FAR_LEFT_BOTTOM,

			FCI_NEAR_RIGHT_BOTTOM,
			FCI_NEAR_RIGHT_TOP,
			FCI_NEAR_LEFT_TOP,
			FCI_NEAR_LEFT_BOTTOM,
		};

		const Vector2 vec4[4] = 
		{
			corner2d[FCI_NEAR_LEFT_TOP] - eyePos2d,
			corner2d[FCI_NEAR_LEFT_BOTTOM] - eyePos2d,
			corner2d[FCI_NEAR_RIGHT_TOP] - eyePos2d,
			corner2d[FCI_NEAR_RIGHT_BOTTOM] - eyePos2d,
		};

		const Vector2* lefttop		= &vec4[0];
		const Vector2* leftbottom	= &vec4[1];
		const Vector2* righttop		= &vec4[2];
		const Vector2* rightbottom	= &vec4[3];

		Frustum2& Projection2D = *m2DProjection;

		scalar topcross		= (*righttop).crossProduct((*lefttop));
		bool leftside_right = topcross > 0;
		if( leftside_right )
		{
			std::swap(lefttop,righttop);
			std::swap(leftbottom,rightbottom);
			std::swap(indexes[0],indexes[3]);
			std::swap(indexes[1],indexes[2]);
			std::swap(indexes[4],indexes[7]);
			std::swap(indexes[5],indexes[6]);
		}

		scalar righttopcross = (*rightbottom-*righttop).crossProduct(*lefttop-*righttop);
		bool upsidedown = righttopcross < 0;

		if( upsidedown )
		{
			std::swap(lefttop,leftbottom);
			std::swap(righttop,rightbottom);
			std::swap(indexes[0],indexes[1]);
			std::swap(indexes[2],indexes[3]);
			std::swap(indexes[4],indexes[5]);
			std::swap(indexes[6],indexes[7]);
		}

		{
			//if we need rotation
			scalar cross0 = (*righttop).crossProduct(*rightbottom);
			scalar cross1 = (*righttop).crossProduct(*leftbottom);
			if( cross0 > 0 && cross1 > 0 )
			{
				scalar cross2 = (*lefttop).crossProduct(*leftbottom);
				scalar cross3 = (*lefttop).crossProduct(*rightbottom);
				if( cross2 > 0 && cross3 > 0 )
				{
					const Vector2* old_rightbottom = rightbottom;
					rightbottom = leftbottom;
					leftbottom = lefttop;
					lefttop = righttop;
					righttop = old_rightbottom;

					CORNER c = indexes[3];
					indexes[3] = indexes[2];	indexes[2] = indexes[1];	indexes[1] = indexes[0];
					indexes[0] = c;
					c = indexes[7];
					indexes[7] = indexes[6];	indexes[6] = indexes[5];	indexes[5] = indexes[4];
					indexes[4] = c;
				}

			}
			else if(cross0 < 0 && cross1 < 0 )
			{
				scalar cross2 = (*lefttop).crossProduct(*leftbottom);
				scalar cross3 = (*lefttop).crossProduct(*rightbottom);

				if( cross2 < 0 && cross3 < 0 )
				{
					const Vector2* old_rightbottom = rightbottom;
					rightbottom = righttop;
					righttop = lefttop;
					lefttop = leftbottom;
					leftbottom = old_rightbottom;

					CORNER c = indexes[0];
					indexes[0] = indexes[1];	indexes[1] = indexes[2];	indexes[2] = indexes[3];
					indexes[3] = c;

					c = indexes[4];
					indexes[4] = indexes[5];	indexes[5] = indexes[6];	indexes[6] = indexes[7];
					indexes[7] = c;
				}
			}
		}

		//topcross		= (*righttop).crossProduct((*lefttop));
		//assert( topcross < 0 );
		//righttopcross = (*rightbottom-*righttop).crossProduct(*lefttop-*righttop);
		//assert( righttopcross > 0 );

		//check projection: near plane in far plane,using far plane's projection
		{
			scalar bottomcross	= (*rightbottom).crossProduct((*leftbottom));
			
			if( bottomcross > 0 )
			{
				Projection2D.mCount = 4;
				Projection2D.mBoundLine[0].set( corner2d[ FCI_FAR_RIGHT_BOTTOM ],corner2d[ FCI_FAR_RIGHT_TOP ] );
				Projection2D.mBoundLine[1].set( corner2d[ FCI_FAR_RIGHT_TOP ],corner2d[ FCI_FAR_LEFT_TOP ] );
				Projection2D.mBoundLine[2].set( corner2d[ FCI_FAR_LEFT_TOP ],corner2d[ FCI_FAR_LEFT_BOTTOM ] );
				Projection2D.mBoundLine[3].set( corner2d[ FCI_FAR_LEFT_BOTTOM ],corner2d[ FCI_FAR_RIGHT_BOTTOM ] );
				return;
			}
		}

		scalar rightcross = (*rightbottom).crossProduct((*righttop));
		scalar leftcross = (*leftbottom).crossProduct((*lefttop));
#ifdef FRUSTUM_CULL_DISABLE_NEAR_PLANE
		if( (rightcross < 0 && leftcross > 0) || (rightcross > 0 && leftcross < 0) )
		{
			Projection2D.mCount = 5;
			Projection2D.mBoundLine[0].set( eyePos2d,corner2d[ indexes[0] ] );
			Projection2D.mBoundLine[1].set( corner2d[ indexes[0] ],corner2d[ indexes[1] ] );
			Projection2D.mBoundLine[2].set( corner2d[ indexes[1] ],corner2d[ indexes[2] ] );
			Projection2D.mBoundLine[3].set( corner2d[ indexes[2] ],corner2d[ indexes[3] ] );
			Projection2D.mBoundLine[4].set( corner2d[ indexes[3] ],eyePos2d );
		}
		else
		{
			Projection2D.mCount = 4;
			if( rightcross < 0 )
			{
				Projection2D.mBoundLine[0].set( eyePos2d,corner2d[ indexes[0] ] );
				Projection2D.mBoundLine[1].set( corner2d[ indexes[0] ],corner2d[ indexes[1] ] );
				Projection2D.mBoundLine[2].set( corner2d[ indexes[1] ],corner2d[ indexes[2] ] );
				Projection2D.mBoundLine[3].set( corner2d[ indexes[2] ],eyePos2d );
			}
			else
			{
				Projection2D.mBoundLine[0].set( eyePos2d,corner2d[ indexes[1] ] );
				Projection2D.mBoundLine[1].set( corner2d[ indexes[1] ],corner2d[ indexes[2] ] );
				Projection2D.mBoundLine[2].set( corner2d[ indexes[2] ],corner2d[ indexes[3] ] );
				Projection2D.mBoundLine[3].set( corner2d[ indexes[3] ],eyePos2d );
			}
		}
#else
		if( (rightcross < 0 && leftcross > 0)  || (rightcross > 0 && leftcross < 0) )
		{
			Projection2D.mCount = 6;
			Projection2D.mBoundLine[0].set( corner2d[ indexes[7] ],corner2d[ indexes[4] ] );
			Projection2D.mBoundLine[0].set( corner2d[ indexes[4] ],corner2d[ indexes[0] ] );
			Projection2D.mBoundLine[1].set( corner2d[ indexes[0] ],corner2d[ indexes[1] ] );
			Projection2D.mBoundLine[2].set( corner2d[ indexes[1] ],corner2d[ indexes[2] ] );
			Projection2D.mBoundLine[3].set( corner2d[ indexes[2] ],corner2d[ indexes[3] ] );
			Projection2D.mBoundLine[4].set( corner2d[ indexes[3] ],corner2d[ indexes[7] ] );
		}
		else
		{
			Projection2D.mCount = 5;
			if( rightcross < 0 )
			{
				Projection2D.mBoundLine[0].set( corner2d[ indexes[7] ],corner2d[ indexes[4] ] );
				Projection2D.mBoundLine[0].set( corner2d[ indexes[4] ],corner2d[ indexes[0] ] );
				Projection2D.mBoundLine[1].set( corner2d[ indexes[0] ],corner2d[ indexes[1] ] );
				Projection2D.mBoundLine[2].set( corner2d[ indexes[1] ],corner2d[ indexes[2] ] );
				Projection2D.mBoundLine[3].set( corner2d[ indexes[2] ],corner2d[ indexes[7] ] );
			}
			else
			{
				Projection2D.mBoundLine[0].set( corner2d[ indexes[7] ],corner2d[ indexes[4] ] );
				Projection2D.mBoundLine[0].set( eyePos2d,corner2d[ indexes[1] ] );
				Projection2D.mBoundLine[1].set( corner2d[ indexes[1] ],corner2d[ indexes[2] ] );
				Projection2D.mBoundLine[2].set( corner2d[ indexes[2] ],corner2d[ indexes[3] ] );
				Projection2D.mBoundLine[3].set( corner2d[ indexes[3] ],corner2d[ indexes[7] ] );
			}
		}
#endif
	}


	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool				Frustum::Frustum2::isVisible(const Blade::Vector2 &point) const
	{
		for( size_t i = 0; i < mCount; ++i)
		{
			if( mBoundLine[i].getSide(point) == PS_NEGATIVE )
				return false;
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				Frustum::Frustum2::isVisible(const Blade::Box2 &box) const
	{
		Vector2 center = box.getCenter();
		Vector2 half = box.getHalfSize();

		for( size_t i = 0; i < mCount; ++i)
		{
			if( mBoundLine[i].getSide(center,half) == PS_NEGATIVE )
				return false;
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		Frustum::Frustum2::isVisible(const Vector2& boxcenter,const Vector2& halfsize) const
	{
		for( size_t i = 0; i < mCount; ++i)
		{
			if( mBoundLine[i].getSide(boxcenter,halfsize) == PS_NEGATIVE )
				return false;
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	ICamera::VISIBILITY	Frustum::Frustum2::getVisibility(const Box2& box) const
	{
		ICamera::VISIBILITY vis = ICamera::CV_ALL;

		Vector2 center = box.getCenter();
		Vector2 half = box.getHalfSize();

		for( size_t i = 0; i < mCount; ++i)
		{
			if( mBoundLine[i].getSide(center,half) == PS_NEGATIVE )
				return ICamera::CV_NULL;
			else if(mBoundLine[i].getSide(center,half) == PS_BOTH )
				vis = ICamera::CV_PARTIAL;
		}
		return vis;
	}

	//////////////////////////////////////////////////////////////////////////
	ICamera::VISIBILITY	Frustum::Frustum2::getVisibility(const Vector2& boxcenter,const Vector2& halfsize) const
	{
		ICamera::VISIBILITY vis = ICamera::CV_ALL;

		for( size_t i = 0; i < mCount; ++i)
		{
			POS_SIDE side = mBoundLine[i].getSide(boxcenter,halfsize);
			if( side == PS_NEGATIVE )
				return ICamera::CV_NULL;
			else if(side == PS_BOTH )
				vis = ICamera::CV_PARTIAL;
		}
		return vis;
	}

	//////////////////////////////////////////////////////////////////////////
	void		Frustum::Frustum2::checkOrder()
	{
		assert( mCount > 2);

		//assume all point are already in CW or CCW order ,
		//just to make the right winding

		//note that 2d x-z plane,the positive x axis is pointing to left
		//		^ +z
		//		|
		//+x	|
		//<-----+-- -x
		//so the y axis is pointing inside (y < 0) , the common cross value is minus

		if( mBoundLine[0].getNormal().crossProduct( mBoundLine[1].getNormal() ) > 0 )
		{
			for( size_t i = 0; i < mCount ; ++i)
			{
				mBoundLine[i].mNormal = -mBoundLine[i].mNormal;
				mBoundLine[i].mD = -mBoundLine[i].mD;
			}
		}
	}

	
}//namespace Blade