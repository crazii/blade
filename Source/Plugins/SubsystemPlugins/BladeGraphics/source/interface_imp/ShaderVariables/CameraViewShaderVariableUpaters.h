/********************************************************************
	created:	2010/09/05
	filename: 	CameraViewShaderVariableUpaters.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_CameraViewShaderVariableUpaters_h__
#define __Blade_CameraViewShaderVariableUpaters_h__
#include "ShaderVariableUpdaters.h"
#include <math/Matrix44.h>
#include <math/Vector4.h>
#include <interface/public/graphics/IGraphicsResourceManager.h>

namespace Blade
{
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class CameraPositionUpdater : public CameraVariableUpdater
	{
	public:
		CameraPositionUpdater()
			:CameraVariableUpdater(SCT_FLOAT4,1)
			,mPosition(0,0,0,1)
		{

		}

		/** @brief  */
		virtual const void*	updateData() const
		{
			const ICamera* camera = this->getCamera();
			if( camera == NULL )
				return &Vector4::ZERO;
			else
			{
				( (Vector3&)mPosition) = camera->getEyePosition();
				return &mPosition;
			}
		}
	protected:
		mutable Vector4 mPosition;
	};

	//////////////////////////////////////////////////////////////////////////
	class ViewMatrixUpdater : public CameraVariableUpdater
	{
	public:
		ViewMatrixUpdater()
			:CameraVariableUpdater(SCT_MATRIX44,1)
		{

		}

		/** @brief  */
		virtual const void*	updateData() const
		{
			const ICamera* camera = this->getCamera();
			if( camera == NULL )
				return &Matrix44::IDENTITY;
			else
				return &camera->getViewMatrix();
		}
	};

	//////////////////////////////////////////////////////////////////////////
	class ViewVectorUpdater : public CameraVariableUpdater
	{
	public:
		ViewVectorUpdater()
			:CameraVariableUpdater(SCT_FLOAT4, 1)
		{

		}
		/** @brief  */
		virtual const void*	updateData() const
		{
			const ICamera* camera = this->getCamera();
			if( camera == NULL )
				return &Vector4::NEGATIVE_UNIT_Z;
			else
			{
				mViewVector = camera->getLookAtDirection();
				return &mViewVector;
			}
		}
	protected:
		mutable Vector4 mViewVector;
	};

	//////////////////////////////////////////////////////////////////////////
	class ProjectionMatrixUpdater : public CameraVariableUpdater
	{
	public:
		ProjectionMatrixUpdater()
			:CameraVariableUpdater(SCT_MATRIX44,1)
		{

		}

		/** @brief  */
		virtual const void*	updateData() const
		{
			const ICamera* camera = this->getCamera();
			if( camera == NULL )
				return &Matrix44::IDENTITY;
			else
			{
				mDeviceRelatedProjectionMatrix = camera->getProjectionMatrix();
				IGraphicsResourceManager::getSingleton().convertProjectionMatrix(mDeviceRelatedProjectionMatrix);
				return &mDeviceRelatedProjectionMatrix;
			}
		}
	protected:
		mutable Matrix44	mDeviceRelatedProjectionMatrix;
	};

	//////////////////////////////////////////////////////////////////////////
	//vector to calculate view space depth from standard depth buffer
	class ViewDepthUpdater : public CameraVariableUpdater
	{
	public:
		ViewDepthUpdater(ProjectionMatrixUpdater* projectionUpdater)
			:CameraVariableUpdater(SCT_FLOAT4, 1)
			,mProjectionUpdater(projectionUpdater)
			,mViewDepth(Vector4::ZERO)
		{

		}

		/** @brief  */
		virtual const void* updateData() const
		{
			const ICamera* camera = this->getCamera();
			if( camera == NULL )
				return &Vector4::ZERO;

			const Matrix44& projection = *(const Matrix44*)mProjectionUpdater->getVariable()->getData();
			//right handed:
			//zfbuffer = (projection[2][2]*viewZ + projection[3][2]) / (-viewZ)
			//viewZ = -projection[3][2] / (zbuffer+projection[2][2])
			//|viewZ| = projection[3][2] / (zbuffer+projection[2][2])
			mViewDepth[0] = projection[2][2];
			mViewDepth[1] = projection[3][2];

			//NDC is API related, on D3D, it ranges[0,1], on OGL, it ranges[-1,1]
			//z near: -1 (OGL), or 0 (D3D)
			scalar NDCZn = ((-camera->getNearClipDistance())*mViewDepth[0] + mViewDepth[1])/(camera->getNearClipDistance());
			scalar NDCZf = 1;
			mViewDepth[2] = NDCZf - NDCZn;
			mViewDepth[3] = NDCZn;
			//in shader: 
			//depth = depth * viewDepth.z + viewDepth.w;
			//z = viewDepth.y / (depth + viewDepth.x);
			return &mViewDepth;
		}

	protected:
		ProjectionMatrixUpdater* mProjectionUpdater;
		mutable Vector4	mViewDepth;
	};


	//////////////////////////////////////////////////////////////////////////
	class ViewSizeUpdater : public ViewVariableUpdater
	{
	public:
		ViewSizeUpdater()
			:ViewVariableUpdater(SCT_FLOAT4, 1)
			, mViewSize(Vector4::ZERO)
		{

		}

		/** @brief  */
		virtual const void* updateData() const
		{
			const IRenderView* view = this->getView();
			if (view == NULL)
				return &Vector4::ZERO;
			mViewSize[0] = (scalar)view->getPixelWidth();
			mViewSize[1] = (scalar)view->getPixelHeight();
			*reinterpret_cast<Vector2*>(&mViewSize[2]) = Vector2::UNIT_ALL / reinterpret_cast<Vector2&>(mViewSize);
			return &mViewSize;
		}
	protected:
		mutable Vector4	mViewSize;
	};

	//////////////////////////////////////////////////////////////////////////
	//updater for view pos: x,y,w,h in relative value [0,1]
	class ViewPosUpdater : public ViewVariableUpdater
	{
	public:
		ViewPosUpdater()
			:ViewVariableUpdater(SCT_FLOAT4, 1)
			, mViewPos(Vector4::ZERO)
		{

		}

		/** @brief  */
		virtual const void* updateData() const
		{
			const IRenderView* view = this->getView();
			const ICamera* camera = IShaderVariableSource::getSingleton().getCamera();
			if (view == NULL || camera == NULL)
				return &Vector4::ZERO;
			mViewPos[0] = (scalar)view->getRelativeLeft();
			mViewPos[1] = (scalar)view->getRelativeTop();
			mViewPos[2] = (scalar)view->getRelativeWidth();
			mViewPos[3] = (scalar)view->getRelativeHeight();
			return &mViewPos;
		}

	protected:
		mutable Vector4	mViewPos;
	};

	//////////////////////////////////////////////////////////////////////////
	class CameraDepthUpdater : public CameraVariableUpdater
	{
	public:
		CameraDepthUpdater() 
			:CameraVariableUpdater(SCT_FLOAT4, 1)
			,mCameraDepth(Vector4::UNIT_ALL)
		{
		}
		/** @brief  */
		virtual const void* updateData() const
		{
			const ICamera* cam = this->getCamera();
			if (cam == NULL)
				return &Vector4::ZERO;
			mCameraDepth[0] = (scalar)cam->getNearClipDistance();
			mCameraDepth[1] = (scalar)cam->getFarClipDistance();
			*reinterpret_cast<Vector2*>(&mCameraDepth[2]) = Vector2::UNIT_ALL / reinterpret_cast<Vector2&>(mCameraDepth);
			return &mCameraDepth;
		}
	protected:
		mutable Vector4 mCameraDepth;
	};

	//////////////////////////////////////////////////////////////////////////
	//view space frustum ray for corners
	class ViewFrustumRayUpdater : public CameraVariableUpdater
	{
	public:
		ViewFrustumRayUpdater(ProjectionMatrixUpdater* projectionUpdater)
			:CameraVariableUpdater(SCT_FLOAT4, 3)
			,mProjectionUpdater(projectionUpdater)
		{

		}

		virtual const void*	updateData() const
		{
			const Matrix44& projectionMatrix = *(const Matrix44*)mProjectionUpdater->getVariable()->getData();
			//near points
			float nears[4];
			//left
			nears[0] = scalar(-1) / projectionMatrix[0][0];
			//top
			nears[1] = scalar(1) / projectionMatrix[1][1];
			//right
			nears[2] = -nears[0];
			//bottom
			nears[3] = -nears[1];

			//use rays that z = -1
			Vector4 rays[2];
			if (IGraphicsResourceManager::getSingleton().getGraphicsConfig().TextureDir == IMGO_BOTTOM_UP)
			{
				rays[0] = Vector4(nears[0], nears[3], -1, 0);
				rays[1] = Vector4(nears[2], nears[1], -1, 0);
			}
			else
			{
				rays[0] = Vector4(nears[0], nears[1], -1, 0);
				rays[1] = Vector4(nears[2], nears[3], -1, 0);
			}
			mFrustumRays[0] = rays[0];
			mFrustumRays[1] = rays[1] - rays[0];
			//in shader: ray = (frustumRay[0] + frustumRay[1] * float3(uv,0)) * viewDepth

			//note: orthographic projection not supported yet. it need remove depth based x y scale.
			//TODO: depthScale, depthOffset
			//ray = (frustumRay[0] + frustumRay[1] * float3(uv,0)) * (viewDepth*depthScale+depthOffest)
			//for perspective: depthScale=(1,1,1), depthOffset=(0,0,0)
			//for orthographic: depthScale=(0,0,1), depthOffset=(1,1,0)

			return mFrustumRays;
		}

	protected:
		mutable Vector4				mFrustumRays[2];
		ProjectionMatrixUpdater*	mProjectionUpdater;
	};

	//////////////////////////////////////////////////////////////////////////
	class ViewProjectionMatrixUpdater : public CameraVariableUpdater
	{
	public:
		ViewProjectionMatrixUpdater(ViewMatrixUpdater* viewUpdater, ProjectionMatrixUpdater* projectionUpdater)
			:CameraVariableUpdater(SCT_MATRIX44,1)
			,mViewUpdater(viewUpdater)
			,mProjectionUpdater(projectionUpdater)
		{

		}

		virtual const void*	updateData() const
		{
			const Matrix44& viewMatrix = *(const Matrix44*)mViewUpdater->getVariable()->getData();
			const Matrix44& projectionMatrix = *(const Matrix44*)mProjectionUpdater->getVariable()->getData();
			mViewProjction = viewMatrix * projectionMatrix;
			return &mViewProjction;
		}
	protected:
		mutable Matrix44	mViewProjction;
		ViewMatrixUpdater*			mViewUpdater;
		ProjectionMatrixUpdater*	mProjectionUpdater;
	};


	//////////////////////////////////////////////////////////////////////////
	class InverseViewMatrixUpdater : public CameraVariableUpdater
	{
	public:
		InverseViewMatrixUpdater()
			:CameraVariableUpdater(SCT_MATRIX44,1)
		{

		}

		virtual const void*	updateData() const
		{
			const ICamera* camera = this->getCamera();
			if( camera == NULL )
				return &Matrix44::IDENTITY;
			else
			{
				mInvView = camera->getViewMatrix().getInverse();
				return &mInvView;
			}
		}
	protected:
		mutable Matrix44	mInvView;
	};


	//////////////////////////////////////////////////////////////////////////
	class InverseProjectionMatrixUpdater : public CameraVariableUpdater
	{
	public:
		InverseProjectionMatrixUpdater()
			:CameraVariableUpdater(SCT_MATRIX44,1)
		{

		}

		virtual const void*	updateData() const
		{
			const ICamera* camera = this->getCamera();
			if( camera == NULL )
				return &Matrix44::IDENTITY;
			else
			{
				mInvProjection = camera->getProjectionMatrix().getInverse();
				return &mInvProjection;
			}
		}
	protected:
		mutable Matrix44	mInvProjection;
	};


	//////////////////////////////////////////////////////////////////////////
	class InverseViewProjectionMatrixUpdater : public CameraVariableUpdater
	{
	public:
		InverseViewProjectionMatrixUpdater(InverseViewMatrixUpdater* invview,InverseProjectionMatrixUpdater* invproj,ViewProjectionMatrixUpdater* vp)
			:CameraVariableUpdater(SCT_MATRIX44,1)
			,mInvViewUpdater(invview)
			,mInvProjUpdater(invproj)
			,mViewProjUpdater(vp)
		{

		}

		virtual const void*	updateData() const
		{
			const ICamera* camera = this->getCamera();
			if( camera == NULL )
				return &Matrix44::IDENTITY;
			else
			{
				//optimization : select ready data
				if( !mInvViewUpdater->getVariable()->isDirty() && !mInvProjUpdater->getVariable()->isDirty() )
				{
					const Matrix44& invView = *(const Matrix44*)mInvViewUpdater->getVariable()->getData();
					const Matrix44& invProj = *(const Matrix44*)mInvProjUpdater->getVariable()->getData();
					mInvViewProjction = invProj * invView;
					return &mInvViewProjction;
				}
				else
				{
					const Matrix44& viewProj = *(const Matrix44*)mViewProjUpdater->getVariable()->getData();
					mInvViewProjction = viewProj.getInverse();
					return &mInvViewProjction;
				}

			}
		}
	protected:
		InverseViewMatrixUpdater*		mInvViewUpdater;
		InverseProjectionMatrixUpdater* mInvProjUpdater;
		ViewProjectionMatrixUpdater*	mViewProjUpdater;
		mutable Matrix44	mInvViewProjction;
	};

	//////////////////////////////////////////////////////////////////////////
	class TransposeViewMatrixUpdater : public CameraVariableUpdater
	{
	public:
		TransposeViewMatrixUpdater()
			:CameraVariableUpdater(SCT_MATRIX44,1)
		{

		}
		
		virtual const void*	updateData() const
		{
			const ICamera* camera = this->getCamera();
			if( camera == NULL )
				return &Matrix44::IDENTITY;
			else
			{
				mTransposeView = camera->getViewMatrix().getTranspose();
				return &mTransposeView;
			}
		}

	protected:
		mutable Matrix44	mTransposeView;
	};

	//////////////////////////////////////////////////////////////////////////
	class TransposeProjectionMatrixUpdater : public CameraVariableUpdater
	{
	public:
		TransposeProjectionMatrixUpdater()
			:CameraVariableUpdater(SCT_MATRIX44,1)
		{

		}

		virtual const void*	updateData() const
		{
			const ICamera* camera = this->getCamera();
			if( camera == NULL )
				return &Matrix44::IDENTITY;
			else
			{
				mTransposeProjection = camera->getProjectionMatrix().getTranspose();
				return &mTransposeProjection;
			}
		}

	protected:
		mutable Matrix44	mTransposeProjection;
	};

	//////////////////////////////////////////////////////////////////////////
	class TransposeViewProjectionMatrixUpdater : public CameraVariableUpdater
	{
	public:
		TransposeViewProjectionMatrixUpdater(ViewProjectionMatrixUpdater* vpUpdater)
			:CameraVariableUpdater(SCT_MATRIX44,1)
			,mViewProjectUpdater(vpUpdater)
		{

		}

		virtual const void*	updateData() const
		{
			const ICamera* camera = this->getCamera();
			if( camera == NULL )
				return &Matrix44::IDENTITY;
			else
			{
				const Matrix44& vp = *(const Matrix44*)mViewProjectUpdater->getVariable()->getData();
				mTransposeViewProjection = vp.getTranspose();
				return &mTransposeViewProjection;
			}
		}

	protected:
		ViewProjectionMatrixUpdater*	mViewProjectUpdater;
		mutable Matrix44	mTransposeViewProjection;
	};

	//////////////////////////////////////////////////////////////////////////
	class InvTransposeViewMatrixUpdater : public CameraVariableUpdater
	{
	public:
		InvTransposeViewMatrixUpdater(InverseViewMatrixUpdater* invView)
			:CameraVariableUpdater(SCT_MATRIX44,1)
			,mInverseViewUpdater(invView)
		{

		}

		virtual const void*	updateData() const
		{
			const ICamera* camera = this->getCamera();
			if( camera == NULL )
				return &Matrix44::IDENTITY;
			else
			{
				const Matrix44& invView = *(const Matrix44*)mInverseViewUpdater->getVariable()->getData();
				mInvTransposeView = invView.getTranspose();
				return &mInvTransposeView;
			}
		}

	protected:
		InverseViewMatrixUpdater* mInverseViewUpdater;
		mutable Matrix44	mInvTransposeView;
	};


	//////////////////////////////////////////////////////////////////////////
	class InvTransposeProjectionMatrixUpdater : public CameraVariableUpdater
	{
	public:
		InvTransposeProjectionMatrixUpdater(InverseProjectionMatrixUpdater* invProj)
			:CameraVariableUpdater(SCT_MATRIX44,1)
			,mInverseProjectionUpdater(invProj)
		{

		}

		virtual const void*	updateData() const
		{
			const ICamera* camera = this->getCamera();
			if( camera == NULL )
				return &Matrix44::IDENTITY;
			else
			{
				const Matrix44& invProj = *(const Matrix44*)mInverseProjectionUpdater->getVariable()->getData();
				mInvTransposeProjection = invProj.getTranspose();
				return &mInvTransposeProjection;
			}
		}

	protected:
		InverseProjectionMatrixUpdater* mInverseProjectionUpdater;
		mutable Matrix44	mInvTransposeProjection;
	};


	//////////////////////////////////////////////////////////////////////////
	class InvTransposeVPMatrixUpdater : public CameraVariableUpdater
	{
	public:
		InvTransposeVPMatrixUpdater(InverseViewProjectionMatrixUpdater* invViewProj)
			:CameraVariableUpdater(SCT_MATRIX44,1)
			,mInverseVPUpdater(invViewProj)
		{

		}

		virtual const void*	updateData() const
		{
			const ICamera* camera = this->getCamera();
			if( camera == NULL )
				return &Matrix44::IDENTITY;
			else
			{
				const Matrix44& invViewProj = *(const Matrix44*)mInverseVPUpdater->getVariable()->getData();
				mInvTransposeViewProjection = invViewProj.getTranspose();
				return &mInvTransposeViewProjection;
			}
		}

	protected:
		InverseViewProjectionMatrixUpdater* mInverseVPUpdater;
		mutable Matrix44	mInvTransposeViewProjection;
	};


	//////////////////////////////////////////////////////////////////////////
	class BillboardViewMatrixUpdater : public CameraVariableUpdater
	{
	public:
		BillboardViewMatrixUpdater(CameraPositionUpdater* viewPosUpdater)
			:CameraVariableUpdater(SCT_MATRIX44,1)
			,mViewPosUpdater(viewPosUpdater)
		{
			mBillboardView = Matrix44::IDENTITY;
		}

		virtual const void*	updateData() const
		{
			const Vector4& viewPos = *(const Vector4*)mViewPosUpdater->getVariable()->getData();
			mBillboardView.setTranslation( -viewPos );
			return &mBillboardView;
		}
	protected:
		mutable Matrix44	mBillboardView;
		CameraPositionUpdater*			mViewPosUpdater;
	};

	//////////////////////////////////////////////////////////////////////////
	class BillboardViewProjectionMatrixUpdater : public CameraVariableUpdater
	{
	public:
		BillboardViewProjectionMatrixUpdater(BillboardViewMatrixUpdater* viewUpdater, ProjectionMatrixUpdater* projectionUpdater)
			:CameraVariableUpdater(SCT_MATRIX44,1)
			,mViewUpdater(viewUpdater)
			,mProjectionUpdater(projectionUpdater)
		{

		}

		virtual const void*	updateData() const
		{
			const Matrix44& viewMatrix = *(const Matrix44*)mViewUpdater->getVariable()->getData();
			const Matrix44& projectionMatrix = *(const Matrix44*)mProjectionUpdater->getVariable()->getData();
			mViewProjction = viewMatrix * projectionMatrix;
			return &mViewProjction;
		}
	protected:
		mutable Matrix44	mViewProjction;
		BillboardViewMatrixUpdater*	mViewUpdater;
		ProjectionMatrixUpdater*	mProjectionUpdater;
	};
	
}//namespace Blade


#endif //__Blade_CameraViewShaderVariableUpaters_h__