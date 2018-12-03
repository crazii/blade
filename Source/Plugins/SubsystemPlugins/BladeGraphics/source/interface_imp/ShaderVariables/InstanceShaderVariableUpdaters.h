/********************************************************************
	created:	2010/09/05
	filename: 	InstanceShaderVariableUpdaters.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_InstanceShaderVariableUpdaters_h__
#define __Blade_InstanceShaderVariableUpdaters_h__
#include "CameraViewShaderVariableUpaters.h"
#include "GlobalLightShaderVariableUpdaters.h"
#include <GraphicsShaderShared.inl>
#include <utility/FunctorsExt.h>
#include <math/Matrix44.h>
#include <math/Vector4.h>
#include <MaterialInstance.h>
#include <interface/ISpace.h>
#include <interface/IRenderTypeManager.h>
#include <Element/LightElement.h>
#include "../RenderableSchemeData.h"
#include "../Light.h"

namespace Blade
{
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	class WorldMatrixUpdater : public InstanceVariableUpdater
	{
	public:
		WorldMatrixUpdater()
			:InstanceVariableUpdater(SCT_MATRIX44,1)
			,mPrevData(NULL)
		{

		}

		/** @brief  */
		virtual bool	needCommit() const
		{
			return this->needCommitData();
		}

		/** @brief  */
		virtual const void*	updateData() const
		{
			const IRenderable* renderable = this->getRenderable();
			return &renderable->getWorldTransform();
		}

		/** @brief  */
		virtual void		onShaderSwitch(bool dynamicSwithing)
		{
			InstanceVariableUpdater::onShaderSwitch(dynamicSwithing);
			mPrevData = NULL;
		}

	protected:
		mutable const void* mPrevData;
	};

	//////////////////////////////////////////////////////////////////////////
	class WorldPositionUpdater : public InstanceVariableUpdater
	{
	public:
		WorldPositionUpdater(WorldMatrixUpdater* worldUpdater)
			:InstanceVariableUpdater(SCT_FLOAT4,1)
			,mWorldUpdater(worldUpdater)
		{

		}

		/** @brief  */
		virtual const void*	updateData() const
		{
			const Matrix44& world = *(const Matrix44*)mWorldUpdater->getVariable()->getData();
			return &world.getTranslation();
		}

		/** @brief  */
		virtual bool	needCommit() const
		{
			return this->needCommitData();
		}
	protected:
		WorldMatrixUpdater* mWorldUpdater;
	};

	//////////////////////////////////////////////////////////////////////////
	class WorldViewMatrixUpdater : public InstanceVariableUpdater
	{
	public:
		WorldViewMatrixUpdater(WorldMatrixUpdater* worldUpdater,ViewMatrixUpdater* viewUpdater)
			:InstanceVariableUpdater(SCT_MATRIX44,1)
			,mWorldUpdater(worldUpdater)
			,mViewUpdater(viewUpdater)
		{

		}

		/** @brief  */
		virtual const void*	updateData() const
		{
			const Matrix44& world = *(const Matrix44*)mWorldUpdater->getVariable()->getData();
			const Matrix44& view = *(const Matrix44*)mViewUpdater->getVariable()->getData();
			mWorldView = world * view;
			return &mWorldView;
		}

		/** @brief  */
		virtual bool needCommit() const
		{
			return mViewUpdater->getVariable()->isDirty() || this->needCommitData();
		}

	protected:
		WorldMatrixUpdater* mWorldUpdater;
		ViewMatrixUpdater*	mViewUpdater;
		mutable Matrix44	mWorldView;
	};

	//////////////////////////////////////////////////////////////////////////
	class WorldViewProjectionMatrixUpdater : public InstanceVariableUpdater
	{
	public:
		WorldViewProjectionMatrixUpdater(WorldMatrixUpdater* worldUpdater,ViewProjectionMatrixUpdater* vpUpdater)
			:InstanceVariableUpdater(SCT_MATRIX44,1)
			,mWorldUpdater(worldUpdater)
			,mViewProjectionUpdater(vpUpdater)
		{

		}

		/** @brief  */
		virtual const void*	updateData() const
		{
			const Matrix44& world = *(const Matrix44*)mWorldUpdater->getVariable()->getData();
			const Matrix44& viewProj = *(const Matrix44*)mViewProjectionUpdater->getVariable()->getData();
			mWorldViewProjection = world * viewProj;
			return &mWorldViewProjection;
		}

		/** @brief  */
		virtual bool needCommit() const
		{
			return mViewProjectionUpdater->getVariable()->isDirty() || this->needCommitData();
		}

	protected:
		WorldMatrixUpdater*				mWorldUpdater;
		ViewProjectionMatrixUpdater*	mViewProjectionUpdater;
		mutable Matrix44				mWorldViewProjection;
	};

	//////////////////////////////////////////////////////////////////////////
	class InverseWorldMatrixUpdater : public InstanceVariableUpdater
	{
	public:
		InverseWorldMatrixUpdater(WorldMatrixUpdater* worldUpdater)
			:InstanceVariableUpdater(SCT_MATRIX44,1)
			,mWorldUpdater(worldUpdater)
		{

		}

		/** @brief  */
		virtual const void*	updateData() const
		{
			const Matrix44& world = *(const Matrix44*)mWorldUpdater->getVariable()->getData();
			mInverseWorld = world.getInverse();
			return &mInverseWorld;
		}

		/** @brief  */
		virtual bool	needCommit() const
		{
			return this->needCommitData();
		}

	protected:
		WorldMatrixUpdater* mWorldUpdater;
		mutable Matrix44	mInverseWorld;
	};

	//////////////////////////////////////////////////////////////////////////
	class InverseWorldViewMatrixUpdater : public InstanceVariableUpdater
	{
	public:
		InverseWorldViewMatrixUpdater(WorldViewMatrixUpdater* WorldView)
			:InstanceVariableUpdater(SCT_MATRIX44,1)
			,mWorldViewUpdater(WorldView)
		{

		}

		/** @brief  */
		virtual const void*	updateData() const
		{
			const Matrix44& WorldView = *(const Matrix44*)mWorldViewUpdater->getVariable()->getData();
			mInverseWorldView = WorldView.getInverse();
			return &mInverseWorldView;
		}

		/** @brief  */
		virtual bool	needCommit() const
		{
			return mWorldViewUpdater->needCommit();
		}

	protected:
		WorldViewMatrixUpdater*	mWorldViewUpdater;
		mutable Matrix44		mInverseWorldView;
	};


	//////////////////////////////////////////////////////////////////////////
	class InverseWorldViewProjectionMatrixUpdater : public InstanceVariableUpdater
	{
	public:
		InverseWorldViewProjectionMatrixUpdater(WorldViewProjectionMatrixUpdater* WorldViewProj)
			:InstanceVariableUpdater(SCT_MATRIX44,1)
			,mWorldViewProjectionUpdater(WorldViewProj)
		{

		}

		/** @brief  */
		virtual const void*	updateData() const
		{
			const Matrix44& WorldViewProejction = *(const Matrix44*)mWorldViewProjectionUpdater->getVariable()->getData();
			mInverseWorldViewProjection = WorldViewProejction.getInverse();
			return &mInverseWorldViewProjection;
		}

		/** @brief  */
		virtual bool	needCommit() const
		{
			return this->needCommitData();
		}

	protected:
		WorldViewProjectionMatrixUpdater*	mWorldViewProjectionUpdater;
		mutable Matrix44					mInverseWorldViewProjection;
	};


	//////////////////////////////////////////////////////////////////////////
	class TransposeWorldMatrixUpdater : public InstanceVariableUpdater
	{
	public:
		TransposeWorldMatrixUpdater(WorldMatrixUpdater* worldUpdater)
			:InstanceVariableUpdater(SCT_MATRIX44,1)
			,mWorldUpdater(worldUpdater)
		{

		}

		/** @brief  */
		virtual const void*	updateData() const
		{
			const IRenderable* renderable = this->getRenderable();
			if( renderable == NULL )
				return &Matrix44::IDENTITY;
			else
			{
				const Matrix44& world = *(const Matrix44*)mWorldUpdater->getVariable()->getData();
				mTransposeWorld = world.getTranspose();
				return &mTransposeWorld;
			}
		}
		
		/** @brief  */
		virtual bool	needCommit() const
		{
			return this->needCommitData();
		}

	protected:
		WorldMatrixUpdater* mWorldUpdater;
		mutable Matrix44	mTransposeWorld;
	};

	//////////////////////////////////////////////////////////////////////////
	class TransposeWorldViewMatrixUpdater : public InstanceVariableUpdater
	{
	public:
		TransposeWorldViewMatrixUpdater(WorldViewMatrixUpdater* worldView)
			:InstanceVariableUpdater(SCT_MATRIX44,1)
			,mWroldViewUpdater(worldView)
		{

		}

		/** @brief  */
		virtual const void*	updateData() const
		{
			const Matrix44& WorldView = *(const Matrix44*)mWroldViewUpdater->getVariable()->getData();
			mTransposeWorldView = WorldView.getTranspose();
			return &mTransposeWorldView;
		}

		/** @brief  */
		virtual bool	needCommit() const
		{
			return this->needCommitData();
		}

	protected:
		WorldViewMatrixUpdater*	mWroldViewUpdater;
		mutable Matrix44	mTransposeWorldView;
	};


	//////////////////////////////////////////////////////////////////////////
	class TransposeWorldViewProjectionMatrixUpdater : public InstanceVariableUpdater
	{
	public:
		TransposeWorldViewProjectionMatrixUpdater(WorldViewProjectionMatrixUpdater* wpv)
			:InstanceVariableUpdater(SCT_MATRIX44,1)
			,mWorldViewProjectionUpdater(wpv)
		{

		}

		/** @brief  */
		virtual const void*	updateData() const
		{
			const Matrix44& WorldViewProejction = *(const Matrix44*)mWorldViewProjectionUpdater->getVariable()->getData();
			mTransposeWorldViewProjection = WorldViewProejction.getTranspose();
			return &mTransposeWorldViewProjection;
		}

		/** @brief  */
		virtual bool	needCommit() const
		{
			return this->needCommitData();
		}

	protected:
		WorldViewProjectionMatrixUpdater*	mWorldViewProjectionUpdater;
		mutable Matrix44	mTransposeWorldViewProjection;
	};


	//////////////////////////////////////////////////////////////////////////
	class InvTransposeWorldMatrixUpdater : public InstanceVariableUpdater
	{
	public:
		InvTransposeWorldMatrixUpdater(InverseWorldMatrixUpdater* invWorld)
			:InstanceVariableUpdater(SCT_MATRIX44,1)
			,mInverseWorldUpdater(invWorld)
		{

		}

		/** @brief  */
		virtual const void*	updateData() const
		{
			const Matrix44& invWorld = *(const Matrix44*)mInverseWorldUpdater->getVariable()->getData();
			mInverseTransposeWorld = invWorld.getTranspose();
			return &mInverseTransposeWorld;
		}

		/** @brief  */
		virtual bool needCommit() const
		{
			return this->needCommitData();
		}
	protected:
		InverseWorldMatrixUpdater*	mInverseWorldUpdater;
		mutable Matrix44	mInverseTransposeWorld;
	};

	//////////////////////////////////////////////////////////////////////////
	class InvTransposeWorldViewMatrixUpdater : public InstanceVariableUpdater
	{
	public:
		InvTransposeWorldViewMatrixUpdater(InverseWorldViewMatrixUpdater* invWorldView)
			:InstanceVariableUpdater(SCT_MATRIX44,1)
			,mInverseWorldViewUpdater(invWorldView)
		{

		}

		/** @brief  */
		virtual const void*	updateData() const
		{
			const Matrix44& invWorldView = *(const Matrix44*)mInverseWorldViewUpdater->getVariable()->getData();
			mInverseTransposeWorldView = invWorldView.getTranspose();
			return &mInverseTransposeWorldView;
		}

		/** @brief  */
		virtual bool	needCommit() const
		{
			return this->needCommitData();
		}

	protected:
		InverseWorldViewMatrixUpdater*	mInverseWorldViewUpdater;
		mutable Matrix44	mInverseTransposeWorldView;
	};


	//////////////////////////////////////////////////////////////////////////
	class InvTransposeWorldViewProjectionMatrixUpdater : public InstanceVariableUpdater
	{
	public:
		InvTransposeWorldViewProjectionMatrixUpdater(InverseWorldViewProjectionMatrixUpdater* invWorldViewProejction)
			:InstanceVariableUpdater(SCT_MATRIX44,1)
			,mInverseWorldViewProjectionUpdater(invWorldViewProejction)
		{

		}

		/** @brief  */
		virtual const void*	updateData() const
		{
			const Matrix44& invWorldViewProj = *(const Matrix44*)mInverseWorldViewProjectionUpdater->getVariable()->getData();
			mInverseTransposeWorldViewProjection = invWorldViewProj.getTranspose();
			return &mInverseTransposeWorldViewProjection;
		}

		/** @brief  */
		virtual bool	needCommit() const
		{
			return this->needCommitData();
		}

	protected:
		InverseWorldViewProjectionMatrixUpdater*	mInverseWorldViewProjectionUpdater;
		mutable Matrix44	mInverseTransposeWorldViewProjection;
	};

	//////////////////////////////////////////////////////////////////////////
	//object space camera position
	class InstanceCameraPositionUpdater : public InstanceVariableUpdater
	{
	public:
		InstanceCameraPositionUpdater( CameraPositionUpdater* cameraPosUppdater, InverseWorldMatrixUpdater* invWorldMatrixUpdater)
			:InstanceVariableUpdater(SCT_FLOAT4, 1)
			,mCameraPosUpdater(cameraPosUppdater)
			,mInvWorldMatrixUpdater(invWorldMatrixUpdater)
		{

		}

		/** @brief  */
		virtual const void* updateData() const
		{
			const Matrix44& invWorldMatrix = *(const Matrix44*)mInvWorldMatrixUpdater->getVariable()->getData();
			const Vector4& cameraPos = *(const Vector4*)mCameraPosUpdater->getVariable()->getData();
			mObjectEyePos = cameraPos * invWorldMatrix;
			return &mObjectEyePos;
		}

		/** @brief  */
		virtual bool	needCommit() const
		{
			return mCameraPosUpdater->getVariable()->isDirty() || this->needCommitData();
		}
	protected:
		CameraPositionUpdater*		mCameraPosUpdater;
		InverseWorldMatrixUpdater*	mInvWorldMatrixUpdater;
		mutable Vector4				mObjectEyePos;
	};

	//////////////////////////////////////////////////////////////////////////
	//object space light vectors
	class ForwardLocalLightUpdater : public Singleton<ForwardLocalLightUpdater>
	{
		class LocalLightQeuryCallback : public SpaceQuery::ICallback, public TempMap<scalar, Light*, FnScalarLess<Math::LOW_EPSILON> >
		{
		public:
			virtual ~LocalLightQeuryCallback() {}
			/** @brief return value indicates whether to continue querying */
			virtual bool	onQuery(IElement* element, scalar distance, size_t /*total*/)
			{
				//we need to get the nearest lights, so query all local lights and sort
				//if (total >= BLADE_MAX_LIGHT_COUNT)
				//	return false;
				(*this)[distance] = static_cast<Light*>(static_cast<LightElement*>(element)->getLight());
				return false;	//don't keep in result, keep it here
			}

			/** @brief return value indicates element should be added to result or not */
			virtual bool	validate(IElement* element, scalar /*distance*/)
			{
				if (static_cast<LightElement*>(element)->getLight()->getType() == LT_DIRECTIONAL)
					return false;
				return true;
			}
		};

	public:
		ForwardLocalLightUpdater()
		{
		}

		/** @brief  */
		inline void	reset()
		{
			mRenderable = NULL;
			mLightCount[0] = 0;
		}

		/** @brief  */
		bool	update(const IRenderable* renderable)
		{
			if (mRenderable == renderable)
				return false;

			if (renderable != NULL)
			{
				RenderSchemeData* schemeData = Impl::ensureRenderData(renderable);
				if (!schemeData->mFixedLocalLightCalculated ||
					(!schemeData->mLocalLightCaclulated && schemeData->mFixedLocalLights.size() < BLADE_MAX_LIGHT_COUNT) )
				{
					TempVector<ILight*> lightList;
					lightList.reserve(BLADE_MAX_LIGHT_COUNT);

					TempVector<ILight*> fixedLightList;
					if(!schemeData->mLocalLightCaclulated)
						fixedLightList.reserve(BLADE_MAX_LIGHT_COUNT);

					AABB aab = renderable->getWorldBounding();	//NOTE: batch combined terrain will lost most of local lights
					ISpace* space = renderable->getSpaceContent()->getSpace();
					if (!aab.isNull())
					{
						LocalLightQeuryCallback cb;
						AABBQuery aabQuery(aab, FULL_APPFLAG, NULL, &cb);
						//query lights that are within object's bounding
						scalar limit = FLT_MAX;
						space->queryContents(aabQuery, CSF_LIGHT, limit);

						for (LocalLightQeuryCallback::iterator i = cb.begin(); i != cb.end(); ++i)
						{
							Light* light = i->second;
							assert(light->getType() != LT_DIRECTIONAL);

							if ((light->getSpaceFlags()&CSF_DYNAMIC) || (renderable->getSpaceContent()->getSpaceFlags()&CSF_DYNAMIC))
							{
								if (lightList.size() < BLADE_MAX_LIGHT_COUNT)
									lightList.push_back(light);
							}
							else if (!schemeData->mFixedLocalLightCalculated)
							{
								if (fixedLightList.size() < BLADE_MAX_LIGHT_COUNT)
									fixedLightList.push_back(light);
							}
							if (schemeData->mFixedLocalLights.size() + lightList.size() > BLADE_MAX_LIGHT_COUNT)
								break;
						}
					}
					schemeData->mLocalLightCaclulated = true;

					schemeData->mLocalLights.reserve(lightList.size());
					schemeData->mLocalLights.insert(schemeData->mLocalLights.end(), lightList.begin(), lightList.end());

					if (!schemeData->mFixedLocalLightCalculated)
					{
						schemeData->mFixedLocalLightCalculated = true;
						schemeData->mFixedLocalLights.reserve(fixedLightList.size());
						schemeData->mFixedLocalLights.insert(schemeData->mFixedLocalLights.end(), fixedLightList.begin(), fixedLightList.end());
					}
				}

				size_t lightCount = 0;
				for (size_t i = 0; i < schemeData->mFixedLocalLights.size(); ++i)
				{
					Light* light = static_cast<Light*>(schemeData->mFixedLocalLights[i]);
					mLightPosition[lightCount] = light->getPosition();
					mLightPosition[lightCount].w = (light->getType() == LT_POINT) ? 1.0f : 0.0f;
					mLightDirection[lightCount] = -light->getDirection();
					mLightDiffuse[lightCount] = light->getDiffuse();
					mLightSpecular[lightCount] = light->getSpecular();
					//simple linear attenuation = (distance - range) / (intense - range);
					// = distance * [1/(intense - range)] + [-range/(intense - range)]
					mLightAttenuation[lightCount] = light->getAttenuationParam();
					if (++lightCount >= BLADE_MAX_LIGHT_COUNT)
						break;
				}

				if (lightCount < BLADE_MAX_LIGHT_COUNT)
				{
					for (size_t i = 0; i < schemeData->mLocalLights.size(); ++i)
					{
						Light* light = static_cast<Light*>(schemeData->mLocalLights[i]);
						mLightPosition[lightCount] = light->getPosition();
						mLightPosition[lightCount].w = (light->getType() == LT_POINT) ? 1.0f : 0.0f;
						mLightDirection[lightCount] = -light->getDirection();
						mLightDiffuse[lightCount] = light->getDiffuse();
						mLightSpecular[lightCount] = light->getSpecular();
						//simple linear attenuation = (distance - range) / (intense - range);
						// = distance * [1/(intense - range)] + [-range/(intense - range)]
						mLightAttenuation[lightCount] = light->getAttenuationParam();
						if (++lightCount >= BLADE_MAX_LIGHT_COUNT)
							break;
					}
				}
				mLightCount[0] = (float)lightCount;
			}
			mRenderable = renderable;
			return true;
		}

		Vector4	mLightPosition[BLADE_MAX_LIGHT_COUNT];
		Vector4	mLightDirection[BLADE_MAX_LIGHT_COUNT];
		Vector4	mLightAttenuation[BLADE_MAX_LIGHT_COUNT];	//use simple attenuation
		Color	mLightDiffuse[BLADE_MAX_LIGHT_COUNT];
		Color	mLightSpecular[BLADE_MAX_LIGHT_COUNT];
		Vector4	mLightCount;
	protected:
		const IRenderable* mRenderable;
		size_t	mLightFlag;
		bool	mDirty;
	};

	//////////////////////////////////////////////////////////////////////////
	class ForwardLocalLightCountUpdater : public InstanceVariableUpdater
	{
	public:
		ForwardLocalLightCountUpdater(ForwardLocalLightUpdater* forwardLocalLightUpdater)
			:InstanceVariableUpdater(SCT_FLOAT4, 1)
			,mForwardLocalLightUpdater(forwardLocalLightUpdater)
		{

		}

		/** @brief  */
		virtual const void* updateData() const
		{
			mForwardLocalLightUpdater->update(this->getRenderable());
			return &(mForwardLocalLightUpdater->mLightCount);
		}
	protected:
		ForwardLocalLightUpdater* mForwardLocalLightUpdater;
	};

	//////////////////////////////////////////////////////////////////////////
	class ForwardLocalLightPositionUpdater : public InstanceVariableUpdater
	{
	public:
		ForwardLocalLightPositionUpdater(ForwardLocalLightUpdater* forwardLocalLightUpdater)
			:InstanceVariableUpdater(SCT_FLOAT4, BLADE_MAX_LIGHT_COUNT)
			,mForwardLocalLightUpdater(forwardLocalLightUpdater)
		{

		}

		/** @brief  */
		virtual const void* updateData() const
		{
			mForwardLocalLightUpdater->update(this->getRenderable());
			return mForwardLocalLightUpdater->mLightPosition;
		}
	protected:
		ForwardLocalLightUpdater* mForwardLocalLightUpdater;
	};

	//////////////////////////////////////////////////////////////////////////
	class ForwardLocalLightDirectionUpdater : public InstanceVariableUpdater
	{
	public:
		ForwardLocalLightDirectionUpdater(ForwardLocalLightUpdater* forwardLocalLightUpdater)
			:InstanceVariableUpdater(SCT_FLOAT4, BLADE_MAX_LIGHT_COUNT)
			,mForwardLocalLightUpdater(forwardLocalLightUpdater)
		{

		}

		/** @brief  */
		virtual const void* updateData() const
		{
			mForwardLocalLightUpdater->update(this->getRenderable());
			return mForwardLocalLightUpdater->mLightDirection;
		}
	protected:
		ForwardLocalLightUpdater* mForwardLocalLightUpdater;
	};

	//////////////////////////////////////////////////////////////////////////
	class ForwardLocalLightDiffuseUpdater : public InstanceVariableUpdater
	{
	public:
		ForwardLocalLightDiffuseUpdater(ForwardLocalLightUpdater* forwardLocalLightUpdater)
			:InstanceVariableUpdater(SCT_FLOAT4, BLADE_MAX_LIGHT_COUNT)
			,mForwardLocalLightUpdater(forwardLocalLightUpdater)
		{

		}

		/** @brief  */
		virtual const void* updateData() const
		{
			mForwardLocalLightUpdater->update(this->getRenderable());
			return mForwardLocalLightUpdater->mLightDiffuse;
		}
	protected:
		ForwardLocalLightUpdater* mForwardLocalLightUpdater;
	};

	//////////////////////////////////////////////////////////////////////////
	class ForwardLocalLightSpecularUpdater : public InstanceVariableUpdater
	{
	public:
		ForwardLocalLightSpecularUpdater(ForwardLocalLightUpdater* forwardLocalLightUpdater)
			:InstanceVariableUpdater(SCT_FLOAT4, BLADE_MAX_LIGHT_COUNT)
			,mForwardLocalLightUpdater(forwardLocalLightUpdater)
		{

		}

		/** @brief  */
		virtual const void* updateData() const
		{
			mForwardLocalLightUpdater->update(this->getRenderable());
			return mForwardLocalLightUpdater->mLightSpecular;
		}
	protected:
		ForwardLocalLightUpdater* mForwardLocalLightUpdater;
	};

	//////////////////////////////////////////////////////////////////////////
	class ForwardLocalLightAttenuationUpdater : public InstanceVariableUpdater
	{
	public:
		ForwardLocalLightAttenuationUpdater(ForwardLocalLightUpdater* forwardLocalLightUpdater)
			:InstanceVariableUpdater(SCT_FLOAT4, BLADE_MAX_LIGHT_COUNT)
			,mForwardLocalLightUpdater(forwardLocalLightUpdater)
		{

		}

		/** @brief  */
		virtual const void* updateData() const
		{
			mForwardLocalLightUpdater->update(this->getRenderable());
			return mForwardLocalLightUpdater->mLightAttenuation;
		}
	protected:
		ForwardLocalLightUpdater* mForwardLocalLightUpdater;
	};

	//////////////////////////////////////////////////////////////////////////
	class InstanceTextureProjectionUpdater : public InstanceVariableUpdater
	{
	public:
		InstanceTextureProjectionUpdater()
			:InstanceVariableUpdater(SCT_MATRIX44,1)
			,mIndex(INVALID_INDEX)
		{
		}

		InstanceTextureProjectionUpdater(index_t index)
			:InstanceVariableUpdater(SCT_MATRIX44,1)
			,mIndex(index)
		{
			assert( mIndex < MaterialInstance::TEX_PROJECTION_LIMIT );
		}

		/** @brief  */
		virtual const void*	updateData() const
		{
			ICamera* camera = this->getMaterial()->getTextureProjectionCamera(mIndex);
			mTexProjectVP = camera->getViewMatrix()*camera->getProjectionMatrix();
			return &mTexProjectVP;
		}

		/** @brief  */
		void		setIndex(index_t index)
		{
			mIndex = index;
			if( mIndex >= MaterialInstance::TEX_PROJECTION_LIMIT )
				assert(false);
		}

	protected:
		index_t				mIndex;
		mutable Matrix44	mTexProjectVP;
	};//class PassTextureProjectionUpdater

	//////////////////////////////////////////////////////////////////////////
	class BillboardWorldViewMatrixUpdater : public InstanceVariableUpdater
	{
	public:
		BillboardWorldViewMatrixUpdater(BillboardViewMatrixUpdater* viewUpdater, WorldMatrixUpdater* worldUpdater)
			:InstanceVariableUpdater(SCT_MATRIX44,1)
			,mViewUpdater(viewUpdater)
			,mWorldUpdater(worldUpdater)
		{
			mWorldView = Matrix44::IDENTITY;
		}

		/** @brief  */
		virtual const void*	updateData() const
		{
			const Matrix44& world = *(const Matrix44*)mWorldUpdater->getVariable()->getData();
			const Matrix44& view = *(const Matrix44*)mViewUpdater->getVariable()->getData();
			//note: billboard view matrix only has translation
			//only use world matrix's translation
			mWorldView.setTranslation( world.getTranslation()+view.getTranslation() );
			return &mWorldView;
		}

		/** @brief  */
		virtual bool	needCommit() const
		{
			return mViewUpdater->getVariable()->isDirty()
				|| this->needCommitData();
		}

	protected:
		BillboardViewMatrixUpdater*	mViewUpdater;
		WorldMatrixUpdater*			mWorldUpdater;
		mutable Matrix44			mWorldView;
	};


	//////////////////////////////////////////////////////////////////////////
	class BillboardWorldViewProjectionMatrixUpdater : public InstanceVariableUpdater
	{
	public:
		BillboardWorldViewProjectionMatrixUpdater(BillboardWorldViewMatrixUpdater* bwvUpdater, ProjectionMatrixUpdater* projectionUpdater)
			:InstanceVariableUpdater(SCT_MATRIX44,1)
			,mWorldViewUpdater(bwvUpdater)
			,mProjectionUpdater(projectionUpdater)
		{

		}

		/** @brief  */
		virtual const void*	updateData() const
		{
			const Matrix44& worldview = *(const Matrix44*)mWorldViewUpdater->getVariable()->getData();
			const Matrix44& projection = *(const Matrix44*)mProjectionUpdater->getVariable()->getData();
			mWorldViewProjection = worldview*projection;
			return &mWorldViewProjection;
		}

		/** @brief  */
		virtual bool	needCommit() const
		{
			return mProjectionUpdater->getVariable()->isDirty() || this->needCommitData();
		}

	protected:
		BillboardWorldViewMatrixUpdater*	mWorldViewUpdater;
		ProjectionMatrixUpdater*			mProjectionUpdater;
		mutable Matrix44			mWorldViewProjection;
	};

	//////////////////////////////////////////////////////////////////////////
	class BonePaletteUpdater : public InstanceVariableUpdater
	{
	public:
		BonePaletteUpdater()
			:InstanceVariableUpdater(SCT_FLOAT4, BLADE_BONE_PALETTE_SIZE*2)
		{
		}

		/** @brief  */
		virtual const void*	updateData() const
		{
			mPrevData = this->getData();
			return mPrevData;
		}

		/** @brief  */
		virtual bool	needCommit() const
		{
			return mDynamicShader || mPrevData != this->getData();
		}

		/** @brief  */
		virtual void		onShaderSwitch(bool dynamicSwithing)
		{
			InstanceVariableUpdater::onShaderSwitch(dynamicSwithing);
			mPrevData = NULL;
		}

		virtual const void*	getData() const
		{
			const IRenderable* renderable = this->getRenderable();
			//change real data updating size
			const DualQuaternion* palette = NULL;
			size_t boneCount = renderable->getBonePalette(palette);
			assert( boneCount <= BLADE_BONE_PALETTE_SIZE );
			mVariable.setRealSize( (uint32)(std::min<size_t>(boneCount, BLADE_BONE_PALETTE_SIZE) * 2 * mVariable.getSize()) );
			return palette;
		}
	};

	//////////////////////////////////////////////////////////////////////////
	class ProjectedPosUpdater : public InstanceVariableUpdater
	{
	public:
		ProjectedPosUpdater(WorldViewProjectionMatrixUpdater* worldViewProjectionUpdater)
			:InstanceVariableUpdater(SCT_FLOAT4, 1)
			,mWorldViewProjectionUpdater(worldViewProjectionUpdater)
		{

		}

		/** @brief  */
		virtual const void* updateData() const
		{
			const Matrix44& m = *(const Matrix44*)mWorldViewProjectionUpdater->getVariable()->getData();
			mProjectedPos = m[3];
			mProjectedPos /= mProjectedPos.w;
			return &mProjectedPos;
		}

		/** @brief  */
		virtual bool	needCommit() const
		{
			return this->needCommitData();
		}

	protected:
		WorldViewProjectionMatrixUpdater* mWorldViewProjectionUpdater;
		mutable Vector4 mProjectedPos;
	};

	//////////////////////////////////////////////////////////////////////////
	class ViewSpacePositionUpdater : public InstanceVariableUpdater
	{
	public:
		ViewSpacePositionUpdater(ViewMatrixUpdater* viewUpdater, WorldMatrixUpdater* worldMatrixUpdater)
			:InstanceVariableUpdater(SCT_FLOAT4, 1)
			,mWorldMatrixUpdater(worldMatrixUpdater)
			,mViewUpdater(viewUpdater)
		{

		}

		/** @brief  */
		virtual const void*	updateData() const
		{
			const Matrix44& view = *(const Matrix44*)mViewUpdater->getVariable()->getData();
			mViewPos = (*(const Matrix44*)mWorldMatrixUpdater->getVariable()->getData()).getTranslation()*view;
			return &mViewPos;
		}

		/** @brief  */
		virtual bool	needCommit() const
		{
			return this->needCommitData();
		}
	protected:
		mutable Vector4		mViewPos;
		WorldMatrixUpdater* mWorldMatrixUpdater;
		ViewMatrixUpdater*	mViewUpdater;
	};

	//////////////////////////////////////////////////////////////////////////
	class ViewSpaceBoundingnUpdater : public InstanceVariableUpdater
	{
	public:
		ViewSpaceBoundingnUpdater(ViewMatrixUpdater* viewUpdater)
			:InstanceVariableUpdater(SCT_FLOAT4, 2)
			, mCenter(Vector4::ZERO)
			, mHalfSize(Vector4::ZERO)
			, mViewUpdater(viewUpdater)
		{

		}

		/** @brief  */
		virtual const void* updateData() const
		{
			const IRenderable* renderable = this->getRenderable();
			const Matrix44& viewMatrix = *(Matrix44*)mViewUpdater->getVariable()->getData();
			AABB aab = renderable->getWorldBounding();
			aab *= viewMatrix;
			mCenter = aab.getCenter();
			mHalfSize = Vector4(aab.getHalfSize(),0);
			return &mCenter;
		}
	protected:
		mutable Vector4 mCenter;
		mutable Vector4	mHalfSize;
		ViewMatrixUpdater*	mViewUpdater;
	};

	//////////////////////////////////////////////////////////////////////////
	class SamplerCountUpdater : public InstanceVariableUpdater
	{
	public:
		SamplerCountUpdater()
			:InstanceVariableUpdater(SCT_FLOAT4, 1)
			,mData(Vector4::ZERO)
		{

		}

		/** @brief  */
		virtual const void* updateData() const
		{
			const IRenderable* renderable = this->getRenderable();
			if( renderable == NULL )
			{
				assert(false);
				return &Vector4::ZERO;
			}
			const MaterialInstance* mat = this->getMaterial();
			const TextureState* textures = mat->getTextureState();
			size_t count = 0;
			for(size_t i = 0; i < textures->getTextureStageCount(); ++i)
			{
				if( textures->getTextureStage(i)->getResource() != NULL )
					++count;
			}
			mData[0] = (scalar)count;
			return &mData;
		}
	protected:
		mutable Vector4	mData;
	};

	//////////////////////////////////////////////////////////////////////////
	class DeferredLightPositionUpdater : public InstanceVariableUpdater
	{
	public:
		DeferredLightPositionUpdater()
			:InstanceVariableUpdater(SCT_FLOAT4, 1)
			, mLightPositionData(Vector4::ZERO)
		{

		}

		/** @brief  */
		virtual const void* updateData() const
		{
			const IRenderable* renderable = this->getRenderable();
			const Light* light = static_cast<const Light*>(renderable);
			mLightPositionData = light->getPosition();
			mLightPositionData.w = light->getRange();
			return &mLightPositionData;
		}
	protected:
		mutable Vector4	mLightPositionData;
	};

	//////////////////////////////////////////////////////////////////////////
	class DeferredLightViewSpacePositionUpdater : public InstanceVariableUpdater
	{
	public:
		DeferredLightViewSpacePositionUpdater(ViewSpacePositionUpdater* viewUpdater)
			:InstanceVariableUpdater(SCT_FLOAT4, 1)
			,mViewSpacePosUpdater(viewUpdater)
		{

		}

		/** @brief  */
		virtual const void*	updateData() const
		{
			const IRenderable* renderable = this->getRenderable();
			const Vector4& viewPos = *(const Vector4*)mViewSpacePosUpdater->getVariable()->getData();
			mViewPos = viewPos;
			mViewPos.w = static_cast<const Light*>(renderable)->getRange();
			return &mViewPos;
		}

		/** @brief  */
		virtual bool	needCommit() const
		{
			return this->needCommitData();
		}
	protected:
		mutable Vector4		mViewPos;
		ViewSpacePositionUpdater*	mViewSpacePosUpdater;
	};

	//////////////////////////////////////////////////////////////////////////
	class DeferredLightAttenuationUpdater : public InstanceVariableUpdater
	{
	public:
		DeferredLightAttenuationUpdater()
			:InstanceVariableUpdater(SCT_FLOAT4, 1)
		{

		}

		/** @brief  */
		virtual const void* updateData() const
		{
			const IRenderable* renderable = this->getRenderable();
			const Light* light = static_cast<const Light*>(renderable);
			return &light->getAttenuationParam();
		}
	};

	//////////////////////////////////////////////////////////////////////////
	class DeferredLightSpotDirectionUpdater : public InstanceVariableUpdater
	{
	public:
		DeferredLightSpotDirectionUpdater()
			:InstanceVariableUpdater(SCT_FLOAT4, 1)
		{

		}

		/** @brief  */
		virtual const void* updateData() const
		{
			const IRenderable* renderable = this->getRenderable();
			const Light* light = static_cast<const Light*>(renderable);
			return &reinterpret_cast<const Vector4&>(light->getDirection());
		}
	};

	//////////////////////////////////////////////////////////////////////////
	class DeferredLightDiffuseUpdater : public InstanceVariableUpdater
	{
	public:
		DeferredLightDiffuseUpdater()
			:InstanceVariableUpdater(SCT_FLOAT4, 1)
		{

		}

		/** @brief  */
		virtual const void* updateData() const
		{
			const IRenderable* renderable = this->getRenderable();
			const Light* light = static_cast<const Light*>(renderable);
			return &light->getDiffuse();
		}
	};

	//////////////////////////////////////////////////////////////////////////
	class DeferredLightSpecularUpdater : public InstanceVariableUpdater
	{
	public:
		DeferredLightSpecularUpdater()
			:InstanceVariableUpdater(SCT_FLOAT4, 1)
		{

		}

		/** @brief  */
		virtual const void* updateData() const
		{
			const IRenderable* renderable = this->getRenderable();
			const Light* light = static_cast<const Light*>(renderable);
			return &light->getSpecular();
		}
	};

}//namespace Blade


#endif //__Blade_InstanceShaderVariableUpdaters_h__