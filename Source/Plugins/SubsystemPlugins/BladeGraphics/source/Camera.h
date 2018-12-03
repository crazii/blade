/********************************************************************
	created:	2010/04/27
	filename: 	Camera.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_Camera_h__
#define __Blade_Camera_h__
#include <utility/BladeContainer.h>
#include <LoopContainers.h>
#include <LoopDataTypes.h>
#include <math/Matrix44.h>

#include <interface/ICamera.h>
#include <SpaceContent.h>
#include <Frustum.h>
#include "CameraRenderBuffer.h"
#include "internal/GraphicsConfig.h"

namespace Blade
{
	class Camera : public ICamera, public SpaceContent, public IVisibleContentSet, public Allocatable
	{
	public:
		Camera();
		Camera(const Camera& src, int flag);
		~Camera();

		/************************************************************************/
		/* ICamera Interface                                                                    */
		/************************************************************************/
		/**
		@describe entity may be NULL
		@param 
		@return 
		*/
		virtual IEntity*		getEntity() const;

		/*
		@describe 
		@param [in] x,y the normalized position in view/window coordinates
		@return 
		*/
		virtual Ray				getSpaceRayfromViewPoint(const POINT2& pos) const;

		/*
		@describe project space point to normalized view position
		@param 
		@return 
		*/
		virtual POINT3			getProjectedPosition(const POINT3& pos) const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			isVisible(const AABB& AAB) const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			isVisible(const Sphere& sphere) const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			setProjectionType(EProjectionType ProjType);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			setNearClipDistance(scalar fNear);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			setFarClipDistance(scalar fFar);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			setFieldOfView(scalar fFieldofView);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			setAspectRatio(scalar fAspectRatio);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			setAspectRatio(scalar w,scalar h);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			setOrthoMode(scalar w,scalar h);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			setEyePosition(const Vector3& pos);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			setOrientation(const Quaternion& orientation);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			setViewMatrix(const Matrix44& view, bool fixedView = false);

		/*
		@describe 
		@param 
		@return 
		@note: projection matrix is affected by projection type, FOV,aspect etc.
		so if FOV/aspect changes, the projection matrix set here will be overwritten
		*/
		virtual void			setProjectionMatrix(const Matrix44& projection, bool fixedProjection = false);

		/**
		@describe
		@param
		@return
		*/
		virtual void			setCullingMask(AppFlag mask)
		{
			mCullingMask = mask;
		}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual PROJECTION		getProjectionType() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual scalar			getNearClipDistance() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual scalar			getFarClipDistance() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual	scalar			getFieldOfView() const;

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
		virtual scalar			getOrthoModeWidth() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual scalar			getOrthoModeHeight() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const Matrix44&	getViewMatrix() const;

		/*
		@describe
		@param
		@return
		*/
		virtual const Matrix44&	getProjectionMatrix() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual  const Vector3&	getEyePosition() const;

		/*
		@describe get the rotation of camera
		@param 
		@return 
		*/
		virtual const Quaternion&	getOrientation() const;

		/**
		@describe
		@param
		@return
		*/
		virtual AppFlag			getCulingMask() const
		{
			return mCullingMask;
		}

		/*
		@describe prepare render queue for rendering,\n
		this action may first obtain visible objects(space contents) in scene(space), \n
		and added the contents' renderables into the queue
		@param
		@return may not be the same as input param
		*/
		virtual	IRenderQueue*	getRenderQueue(const RenderType* renderType);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			addCameraData(ISpaceData* data);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			removeCameraData(const TString& dataName);

		/*
		@describe manually update view & projection matrix
		@param 
		@return 
		@note getViewMatrix/getProjectionMatrix method will auto update the corresponding matrix
		*/
		virtual void			updateMatrix();

		/*
		@describe update the visible objects
		@param 
		@return 
		*/
		virtual void			update();

		/*
		@describe
		@param
		@return
		*/
		const Frustum*			getFrustum() const;

		/**
		  @describe 
		  @param
		  @return
		*/
		void					getFrustum(Frustum& outFrustum);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual ICamera*		clone(int flag = CF_DEFAULT) const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			copyFrom(const ICamera& src, int flag = CF_DEFAULT);

		/************************************************************************/
		/* IVisibleContentSet interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual void			addVisibleContent(ISpaceContent* content, ICamera::VISIBILITY _visibility);

		/************************************************************************/
		/* SpaceContent override                                                                     */
		/************************************************************************/
		/*
		@describe add renderables to render buffer
		@return
		@note  this function will be called if CUF_*_VISIBLE is set
		TODO: add debug switch to show helpers
		*/
		virtual void			updateRender(IRenderQueue* /*buffer*/)	{}

		/*
		@describe
		@param
		@return
		*/
		virtual void			notifyAttached();

		/*
		@describe
		@param
		@return
		*/
		virtual void			notifyDetached();

		/*
		@describe
		@param
		@return
		*/
		virtual	void			notifyPositionChange();

		/*
		@describe
		@param
		@return
		*/
		virtual void			notifyRotationChange();

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void			notifyScaleChange() {}

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/**
		@describe 
		@param
		@return
		*/
		inline void setContentMask(Mask mask)
		{
			mContentMask = mask;
		}

		/**
		@describe
		@param [in] enableFrustumNearFarPlane: by default near/far plane are disable for optimization.
					enable it if the camera has customized near/far plane, especially near is not small and far is not large
		@return
		*/
		inline void		ensureCullingData(bool enableFrustumNearFarPlane = false)
		{
			if (mCullingData == NULL)
			{
				mCullingData = BLADE_NEW CULLING_DATA();
				mCullingData->mUpdated = false;

				//initial state
				mCullingData->mFrustum.enableNearFarPlane(enableFrustumNearFarPlane);
				mCullingData->mFrustum.setViewMatrix(mViewMatrix);
				mCullingData->mFrustum.setProjectionMatrix(mProjectionMatrix, (EProjectionType)mProjectionType);
				mCullingData->mRenderBuffer.initRenderBuffers();
			}
		}

		/**
		@describe mask to enable/disable visible content updates. valid values are CUF_VISIBLE_UPDATE
		@param
		@return
		*/
		inline void setUpdateMask(EContentUpdateFlag mask)
		{
			mUpdateMask = mask;
		}

		/** @brief  */
		inline void setMainCamera(ICamera* mainCamera)
		{
			mMainCamera = mainCamera;
		}

		/** @brief  */
		inline CameraRenderBuffer& getCameraRenderBuffer()
		{
			assert(mCullingData != NULL);
			return (mCullingData->mRenderBuffer);
		}

		/** @brief  */
		inline const AABB&					getVisibleBounding() const
		{
			return (mCullingData != NULL) ? mCullingData->mVisibleBounding : AABB::EMPTY;
		}

		/** @brief  */
		inline bool		enableFrustumExtraPlane(const Plane planes[3] = NULL, size_t count = 0)
		{
			if (mCullingData != NULL)
			{
				mCullingData->mFrustum.enableExtraCullingPlane(planes, count);
				return true;
			}
			return false;
		}

	protected:
		/*
		@describe 
		@param 
		@return 
		*/
		void			updateView() const;

		/*
		@describe 
		@param 
		@return 
		*/
		void			updateProjection() const;

		/*
		@describe 
		@param 
		@return 
		*/
		void			needViewUpdate();

		/*
		@describe 
		@param 
		@return 
		*/
		void			needProjectionUpdate();

		/*
		@describe 
		@param 
		@return 
		*/
		void			clear();

		typedef LoopVector<ISpaceContent*>		VisibleContent;
		typedef TStringMap<ISpaceData*>			SpaceDataSet;
		
		typedef struct SCullingData : public Allocatable
		{
			//VisibleContent		mVisibleContent;
			AABB				mVisibleBounding;
			CameraRenderBuffer	mRenderBuffer;
			SpaceDataSet		mSpaceDataSet;
			Frustum				mFrustum;
			LoopFlag			mUpdated;
#if MULTITHREAD_CULLING
			//Lock				mLock;
#endif

			SCullingData() {}		
			SCullingData(const SCullingData& src)
			{
				*this = src;
			}
			SCullingData& operator=(const SCullingData& rhs)
			{
				//mVisibleContent = rhs.mVisibleContent;
				mVisibleBounding = rhs.mVisibleBounding;
				mFrustum = rhs.mFrustum;
				mUpdated = rhs.mUpdated;
				return *this;
			}

		}CULLING_DATA;

		CULLING_DATA*		mCullingData;
		ICamera*			mMainCamera;

		mutable Matrix44	mViewMatrix;
		mutable Matrix44	mProjectionMatrix;

		EContentUpdateFlag	mUpdateMask;
		scalar			mNear;
		scalar			mFar;
		scalar			mFeildOfView;	//horizontal FOV
		scalar			mAspectRatio;
		scalar			mOrthoWidth;
		scalar			mOrthoHeight;

		uint8			mProjectionType; //EProjectionType
		bool			mFixedView;
		bool			mFixedProjection;
		mutable bool	mIsViewDirty;
		mutable bool	mIsProjectionDirty;
	private:
		bool			mStateCheck;
		/** @brief  */
		inline void		enableStateCheck(bool enable)	{mStateCheck = enable;}
		//wrapper friend
		friend class GraphicsCamera;
	};//class Camera
	
}//namespace Blade


#endif //__Blade_Camera_h__