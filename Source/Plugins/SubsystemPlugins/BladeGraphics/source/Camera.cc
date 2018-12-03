/********************************************************************
	created:	2010/04/27
	filename: 	Camera.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "Camera.h"
#include <interface/IRenderQueue.h>
#include <interface/ISpaceCoordinator.h>
#include "Element/GraphicsElement.h"

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(disable:4355) // 'this': used in base member initializer list
#endif

namespace Blade
{
#define BLADE_CS_VERIFY(_state) ( assert(!mStateCheck || BLADE_TS_CHECK(_state)) )
#define BLADE_CS_VERIFY_EXCLUDE(_state) ( assert(!mStateCheck || BLADE_TS_EXCLUDE(_state)) )
#define UPDATE_CAMERA_BOUNDING 1	//set this to enable picking camera

	//////////////////////////////////////////////////////////////////////////
	Camera::Camera()
		:mCullingData(NULL)
		,mMainCamera(this)
		,mUpdateMask(CUF_FORCEWORD)
		,mNear(scalar(0.1))
		,mFar(scalar(1500.0))
		,mFeildOfView(Math::HALF_PI)
		,mAspectRatio(scalar(1.33333))
		,mOrthoWidth(scalar(0.0))
		,mOrthoHeight(scalar(0.0))
		,mProjectionType(PT_PERSPECTIVE)
		,mFixedView(false)
		,mFixedProjection(false)
		,mIsViewDirty(true)
		,mIsProjectionDirty(true)
		,mStateCheck(false)
	{
		mSpaceFlags = CSF_CAMERA | CSF_DYNAMIC | CSF_INDEPENDENT;
		this->updateProjection();
	}

	//////////////////////////////////////////////////////////////////////////
	Camera::Camera(const Camera& src, int flag)
		:mCullingData(NULL)
		,mMainCamera(this)
		,mNear(scalar(0.1))
		,mFar(scalar(1500.0))
		,mFeildOfView(Math::HALF_PI)
		,mAspectRatio(scalar(1.33333))
		,mOrthoWidth(scalar(0.0))
		,mOrthoHeight(scalar(0.0))
		,mProjectionType(PT_PERSPECTIVE)
		,mIsViewDirty(true)
		,mIsProjectionDirty(true)
		,mStateCheck(false)
	{
		this->Camera::copyFrom(src, flag);
		this->updateProjection();
	}

	//////////////////////////////////////////////////////////////////////////
	Camera::~Camera()
	{
		this->clear();
	}

	/************************************************************************/
	/* ICamera Interface                                                                    */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	IEntity*		Camera::getEntity() const
	{
		if(this->getElement() != NULL)
			return this->getElement()->getEntity();
		return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	Ray				Camera::getSpaceRayfromViewPoint(const POINT2& pos) const
	{
		scalar x = pos.x;
		scalar y = pos.y;
		//note:
		//use x,y to construct 2 post-projection point
		//the use the camera's view-project(inverse) matrix to transform the point the world space

		//inverse VP, 
		Matrix44 inverseVP = (this->getViewMatrix()*this->getProjectionMatrix()).getInverse();

		//note that in the post-projection space, z value ranges from -1 to 1
		//although some render device(DirectX) uses (0,1) to render, but the camera's projection matrix is not like that

		//convert x,y from screen space(0,1) to (-1,1)
		//in the screen space, the origin is left-top corner
		//the post-project space's origin is the center
		scalar px = x*2 - 1;
		scalar py = 1 - y*2;

		POINT3 p3near(px,py,-1);
		POINT3 p3far(px,py,1);

		POINT3 p3spaceNear = p3near*inverseVP;
		POINT3 p3spaceFar = p3far*inverseVP;
		Vector3 dir = (p3spaceFar-p3spaceNear).getNormalizedVector();
		return Ray(p3spaceNear,dir);
	}

	//////////////////////////////////////////////////////////////////////////
	POINT3			Camera::getProjectedPosition(const POINT3& pos) const
	{
		Matrix44 ViewProjection = this->getViewMatrix()*this->getProjectionMatrix();
		POINT4 projectedPos = POINT4(pos,1)*ViewProjection;
		if( projectedPos.w < FLT_MIN )
		{
			projectedPos.z = -2;
			return projectedPos;
		}
		projectedPos /= projectedPos.w;
		//post-projection space to screen space
		projectedPos.x = (projectedPos.x+1)/2;
		projectedPos.y = (1-projectedPos.y)/2;
		return projectedPos;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			Camera::isVisible(const AABB& AAB) const
	{
		if( mCullingData != NULL )
			return mCullingData->mFrustum.isVisible( AAB );
		else
		{
			//TODO: use scene data to avoid frequent calculation
			BLADE_LOG(Warning, BTString("expensive operation if null scene data.") );
			Frustum frustum;
			frustum.setViewMatrix( this->getViewMatrix() );
			frustum.setProjectionMatrix( this->getProjectionMatrix(), (EProjectionType)mProjectionType);
			return frustum.isVisible( AAB );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool			Camera::isVisible(const Sphere& sphere) const
	{
		if( mCullingData != NULL )
			return mCullingData->mFrustum.isVisible( sphere );
		else
		{
			//TODO: use scene data to avoid frequent calculation
			Frustum frustum;
			frustum.setViewMatrix( this->getViewMatrix() );
			frustum.setProjectionMatrix( this->getProjectionMatrix(), (EProjectionType)mProjectionType);
			return frustum.isVisible( sphere );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void			Camera::setProjectionType(EProjectionType ProjType)
	{
		//during ASYNC_RUN state, this object maybe accessed by other systems
		//so do not set value internally
		BLADE_CS_VERIFY_EXCLUDE( TS_ASYNC_RUN );

		if( mProjectionType != ProjType )
		{
			mProjectionType = (uint8)ProjType;
			this->needProjectionUpdate();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void			Camera::setNearClipDistance(scalar fNear)
	{
		BLADE_CS_VERIFY_EXCLUDE( TS_ASYNC_RUN );

		mNear = fNear;
		this->needProjectionUpdate();
	}

	//////////////////////////////////////////////////////////////////////////
	void			Camera::setFarClipDistance(scalar fFar)
	{
		BLADE_CS_VERIFY_EXCLUDE( TS_ASYNC_RUN );

		mFar = fFar;
		this->needProjectionUpdate();
	}

	//////////////////////////////////////////////////////////////////////////
	void			Camera::setFieldOfView(scalar fFieldofView)
	{
		BLADE_CS_VERIFY_EXCLUDE( TS_ASYNC_RUN );

		mFeildOfView = fFieldofView;
		this->needProjectionUpdate();
	}

	//////////////////////////////////////////////////////////////////////////
	void			Camera::setAspectRatio(scalar fAspectRatio)
	{
		BLADE_CS_VERIFY_EXCLUDE( TS_ASYNC_RUN );

		if( !Math::Equal(mAspectRatio, fAspectRatio, Math::LOW_EPSILON) )
			this->needProjectionUpdate();

		mAspectRatio = fAspectRatio;
	}

	//////////////////////////////////////////////////////////////////////////
	void			Camera::setAspectRatio(scalar w,scalar h)
	{
		BLADE_CS_VERIFY_EXCLUDE( TS_ASYNC_RUN );

		if (mProjectionType == PT_ORTHOGRAPHIC)
			this->setOrthoMode(w, h);
		else
		{
			scalar fAspectRatio = w / h;
			if( !Math::Equal(mAspectRatio,fAspectRatio, /*Math::LOW_EPSILON*/1e-2f) )
				this->needProjectionUpdate();
			mAspectRatio = fAspectRatio;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void			Camera::setOrthoMode(scalar w,scalar h)
	{
		BLADE_CS_VERIFY_EXCLUDE( TS_ASYNC_RUN );

		scalar fAspectRatio = w / h;
		if(mProjectionType != PT_ORTHOGRAPHIC 
			|| !Math::Equal(mAspectRatio, fAspectRatio, Math::LOW_EPSILON)
			|| !Math::Equal(mOrthoWidth, w, Math::LOW_EPSILON)
			|| !Math::Equal(mOrthoHeight, h, Math::LOW_EPSILON))
		{
			mOrthoWidth = w;
			mOrthoHeight = h;
			mProjectionType = PT_ORTHOGRAPHIC;
			this->needProjectionUpdate();
		}
		mAspectRatio = fAspectRatio;
	}

	//////////////////////////////////////////////////////////////////////////
	void			Camera::setEyePosition(const Vector3& pos)
	{
		BLADE_CS_VERIFY_EXCLUDE( TS_ASYNC_RUN );

		this->SpaceContent::setPosition(pos);
	}

	//////////////////////////////////////////////////////////////////////////
	void			Camera::setOrientation(const Quaternion& orientation)
	{
		BLADE_CS_VERIFY_EXCLUDE( TS_ASYNC_RUN );

		this->SpaceContent::setRotation(orientation);
	}

	//////////////////////////////////////////////////////////////////////////
	void			Camera::setViewMatrix(const Matrix44& view, bool fixedView/* = false*/)
	{
		BLADE_CS_VERIFY_EXCLUDE( TS_ASYNC_RUN );

		mViewMatrix = view;
		mIsViewDirty = !fixedView;
		mFixedView = fixedView;
	}

	//////////////////////////////////////////////////////////////////////////
	void			Camera::setProjectionMatrix(const Matrix44& projection, bool fixedProjection/* = false*/)
	{
		BLADE_CS_VERIFY_EXCLUDE( TS_ASYNC_RUN );

		mProjectionMatrix = projection;
		mIsProjectionDirty = !fixedProjection;
		mFixedProjection = fixedProjection;
	}

	//////////////////////////////////////////////////////////////////////////
	PROJECTION		Camera::getProjectionType() const
	{
		return (EProjectionType)mProjectionType;
	}

	//////////////////////////////////////////////////////////////////////////
	scalar			Camera::getNearClipDistance() const
	{
		return mNear;
	}

	//////////////////////////////////////////////////////////////////////////
	scalar			Camera::getFarClipDistance() const
	{
		return mFar;
	}

	//////////////////////////////////////////////////////////////////////////
	scalar			Camera::getFieldOfView() const
	{
		return mFeildOfView;
	}

	//////////////////////////////////////////////////////////////////////////
	scalar			Camera::getAspectRatio() const
	{
		return mAspectRatio;
	}

	//////////////////////////////////////////////////////////////////////////
	scalar			Camera::getOrthoModeWidth() const
	{
		return mOrthoWidth;
	}

	//////////////////////////////////////////////////////////////////////////
	scalar			Camera::getOrthoModeHeight() const
	{
		return mOrthoHeight;
	}

	//////////////////////////////////////////////////////////////////////////
	const Matrix44&	Camera::getViewMatrix() const
	{
		this->updateView();
		return mViewMatrix;
	}

	//////////////////////////////////////////////////////////////////////////
	const Matrix44&	Camera::getProjectionMatrix() const
	{
		this->updateProjection();
		return mProjectionMatrix;
	}

	//////////////////////////////////////////////////////////////////////////
	const Vector3&	Camera::getEyePosition() const
	{
		return this->SpaceContent::getPosition();
	}

	//////////////////////////////////////////////////////////////////////////
	const Quaternion&	Camera::getOrientation() const
	{
		return this->SpaceContent::getRotation();
	}

	//////////////////////////////////////////////////////////////////////////
	IRenderQueue*	Camera::getRenderQueue(const RenderType* renderType)
	{
		//get the render group copy for view update
		if( mCullingData != NULL )
		{
			IRenderQueue* queue = mCullingData->mRenderBuffer.getRenderQueue( renderType );
			return queue;
		}
		else
			return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			Camera::addCameraData(ISpaceData* data)
	{
		if( data != NULL && mCullingData != NULL)
			return mCullingData->mSpaceDataSet.insert( std::make_pair(data->getSpaceTypeName(), data) ).second;
		else
			return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			Camera::removeCameraData(const TString& dataName)
	{
		return (mCullingData != NULL) ? (mCullingData->mSpaceDataSet.erase(dataName) == 1) : false;
	}

	//////////////////////////////////////////////////////////////////////////
	void			Camera::updateMatrix()
	{
		this->updateView();
		this->updateProjection();
	}

	//////////////////////////////////////////////////////////////////////////
	void			Camera::update()
	{
		this->ensureCullingData(false);

		//in case view/project not updated
		this->updateView();
		this->updateProjection();

		if( !mCullingData->mUpdated )
		{
			mCullingData->mVisibleBounding.setNull();
			//mCullingData->mVisibleContent.reserve(1024);

			mCullingData->mFrustum.updateFrustumPlanes();
			this->getSpace()->getCoordinator()->findVisibleContent(*this->getFrustum(), *this);

			mCullingData->mVisibleBounding = mCullingData->mRenderBuffer.processRenderQueues(this->getPosition(), this->getLookAtDirection());
			mCullingData->mUpdated = true;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	const Frustum*	Camera::getFrustum() const
	{
		if (mCullingData != NULL)
			return &(mCullingData->mFrustum);
		else
			return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	void			Camera::getFrustum(Frustum& outFrustum)
	{
		this->updateMatrix();
		outFrustum.setViewMatrix(mViewMatrix);
		outFrustum.setProjectionMatrix(mProjectionMatrix, (EProjectionType)mProjectionType);
		outFrustum.updateFrustumPlanes();
	}

	//////////////////////////////////////////////////////////////////////////
	ICamera*		Camera::clone(int flag/* = CF_DEFAULT*/) const
	{
		return BLADE_NEW Camera(*this, flag);
	}

	//////////////////////////////////////////////////////////////////////////
	void	Camera::copyFrom(const ICamera& camera, int flag/* = CF_DEFAULT*/)
	{
		this->clear();
		const Camera& src = static_cast<const Camera&>(camera);

		if( (flag&(CF_CULLING| CF_SPACE)) && src.mCullingData != NULL)
		{
			mCullingData = BLADE_NEW CULLING_DATA();
			//mCullingData->mFrustum = src.mCullingData->mFrustum;
			*mCullingData = *(src.mCullingData);
		}
		else
			mCullingData = NULL;

		if( (flag&CF_SPACE) )
		{
			this->copySpace(src);
			mCullingData->mSpaceDataSet = src.mCullingData->mSpaceDataSet;
		}

		if( (flag&CF_GEOMETRY) )
		{
			mViewMatrix = src.mViewMatrix;
			mPosition = src.mPosition;
			mRotation = src.mRotation;
			//mScale = src.mScale;
		}

		if( (flag&CF_DEFAULT) )
		{
			mProjectionMatrix = src.mProjectionMatrix;

			mIsViewDirty = src.mIsViewDirty;
			mIsProjectionDirty = src.mIsProjectionDirty;

			mNear = src.mNear;
			mFar = src.mFar;
			mAspectRatio = src.mAspectRatio;
			mFeildOfView = src.mFeildOfView;
			mProjectionType = src.mProjectionType;

			mOrthoWidth = src.mOrthoWidth;
			mOrthoHeight = src.mOrthoHeight;
		}
	}

	/************************************************************************/
	/* IVisibleContentSet interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void			Camera::addVisibleContent(ISpaceContent* content, ICamera::VISIBILITY _visibility)
	{
		Mask flag = content->getUpdateFlags();

		//set current camera & main camera
		//i.e. shadow camera: the content should update from view camera, not shadow camera itself
		if(flag.checkBits(CUF_VISIBLE_UPDATE) && (mUpdateMask&CUF_VISIBLE_UPDATE) )
			content->visibleUpdate(this, mMainCamera, _visibility);

		{
#if MULTITHREAD_CULLING
			//ScopedLock sl(mCullingData->mLock);
#endif
			//mCullingData->mVisibleContent.push_back(content);
		}

		content->updateRender(&mCullingData->mRenderBuffer);
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/


	/************************************************************************/
	/* SpaceContent override                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void			Camera::notifyAttached()
	{
		this->needViewUpdate();
	}

	//////////////////////////////////////////////////////////////////////////
	void			Camera::notifyDetached()
	{
		this->needViewUpdate();
	}

	//////////////////////////////////////////////////////////////////////////
	void			Camera::notifyPositionChange()
	{
		this->needViewUpdate();
		SpaceContent::notifyPositionChange();
	}

	//////////////////////////////////////////////////////////////////////////
	void			Camera::notifyRotationChange()
	{
		this->needViewUpdate();
		SpaceContent::notifyRotationChange();
	}

	//////////////////////////////////////////////////////////////////////////
	void			Camera::updateView() const
	{
		if(!mFixedView && mIsViewDirty)
		{
			//Matrix44::generateViewMatrix(mViewMatrix,mPosition,mRotation);
			//note: use axes instead of quaternion to improve precision, especially for TAA's reprojection
			Matrix44::generateViewMatrix(mViewMatrix, mPosition, mPosition + this->getLookAtDirection(), this->getUpDirection());	//look at
			if( mCullingData != NULL)
				mCullingData->mFrustum.setViewMatrix(mViewMatrix);
		}
		mIsViewDirty = false;
	}

	//////////////////////////////////////////////////////////////////////////
	void			Camera::updateProjection() const
	{
		if(!mFixedProjection && mIsProjectionDirty)
		{
			if( mProjectionType == PT_ORTHOGRAPHIC )
				Matrix44::generateOrthoProjectionMatrix(mProjectionMatrix,mOrthoWidth,mOrthoHeight,mNear,mFar);
			else
				Matrix44::generatePerspectiveFovProjectionMatrix(mProjectionMatrix, mFeildOfView, mAspectRatio, mNear, mFar);

			if( mCullingData != NULL )
				mCullingData->mFrustum.setProjectionMatrix(mProjectionMatrix, (EProjectionType)mProjectionType);

#if UPDATE_CAMERA_BOUNDING
			scalar x, y;
			scalar z = mNear;
			//scalar z = 1;
			if (mProjectionType == PT_ORTHOGRAPHIC)
			{
				x = mOrthoWidth / 2;
				y = mOrthoHeight / 2;
			}
			else
			{
				x = z * std::tan(mFeildOfView / 2);
				y = x / mAspectRatio;
			}
			mLocalAABB.set(-x, -y, -z, x, y, z);
#endif
		}
		mIsProjectionDirty = false;
	}

	//////////////////////////////////////////////////////////////////////////
	void			Camera::needViewUpdate()
	{
		mIsViewDirty = true;
	}

	//////////////////////////////////////////////////////////////////////////
	void			Camera::needProjectionUpdate()
	{
		mIsProjectionDirty = true;
	}

	//////////////////////////////////////////////////////////////////////////
	void			Camera::clear()
	{
		if (mCullingData != NULL)
		{
			BLADE_DELETE mCullingData;
			mCullingData = NULL;
		}
	}
	
}//namespace Blade