/********************************************************************
	created:	2011/08/30
	filename: 	Light.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <RenderUtility.h>
#include <interface/ISpace.h>
#include "Light.h"
#include "LightManager.h"
#include "LightType.h"
#include "RenderHelper/HelperRenderType.h"

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(disable: 4582)	//union constructor is not implicitly called
#pragma warning(disable: 4355)	//'this': used in base member initializer list
#endif

namespace Blade
{
	namespace Impl
	{
		static LightRenderManager& getLightRenderManager()
		{
			static LightRenderManager& manager = LightRenderManager::getSingleton();
			return manager;
		}

	}//namespace Impl

	//////////////////////////////////////////////////////////////////////////
	Light::Light(ILightManager* lightManager, ELightType type/* = LT_DIRECTIONAL*/, bool notify/* = true*/)
		:mLightManager(lightManager)
		,mWorldTransform(Matrix44::IDENTITY)
		,mDirection(Vector3::NEGATIVE_UNIT_ALL)
		,mPadding(0)
		,mDiffuse(Color::WHITE)
		,mSpecular(Color::WHITE)
		,mRange(1)
		,mSpotInnerAngle(30)
		,mSpotOutAngle(90)
		,mAttenuation(0.5f)
		,mMask(0)
		,mType(LT_DIRECTIONAL)
		,mBoundingHelper(this)
	{
		mUpdateFlags = CUF_DEFAULT_VISIBLE;
		mSpaceFlags = CSF_LIGHT;

		this->setLocalAABB(AABB::INFINITE);
		this->setType(type, false);
		this->setDirection(Vector3::NEGATIVE_UNIT_ALL);

		if (notify)
			static_cast<LightManager*>(mLightManager)->notifyLightCreated(this);
	}

	//////////////////////////////////////////////////////////////////////////
	Light::~Light()
	{
		if( mLightManager != NULL )
			static_cast<LightManager*>(mLightManager)->notifyLightDeleted(this);
	}

	/************************************************************************/
	/* custom interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool					Light::setType(ELightType type, bool notify/* = true*/)
	{
		if( mType != (uint32)type )
		{
			ELightType oldType = (ELightType)mType;
			mType = type;
			if(mType == LT_DIRECTIONAL )
				this->setLocalAABB(AABB(BT_INFINITE));
			else
			{
				if (mType == LT_POINT)
					this->setLocalAABB(AABB(-1, -1, -1, 1, 1, 1));
				else
					this->setLocalAABB(AABB(-0.5, -0.5, -1, 0.5, 0.5, 0));
				
				this->updateAttenuation();
			}

			if (notify)
			{
				static_cast<LightManager*>(mLightManager)->notifyLightTypeChanged(this, oldType, type);

				if ((this->getSpace() != NULL && !(this->getSpace()->getSpaceFlag()&SF_DYNAMIC))
					&& (oldType == LT_DIRECTIONAL || mType == LT_DIRECTIONAL))
				{
					ISpace* space = this->getSpace();
					space->removeContent(this);
					space->addContent(this);
				}
			}
			return true;
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	void					Light::setPosition(const Vector3& pos)
	{
		this->SpaceContent::setPosition(pos);
	}

	//////////////////////////////////////////////////////////////////////////
	void					Light::setDirection(const Vector3& dir)
	{
		Quaternion rotation;
		Quaternion::generateRotation(rotation, Vector3::NEGATIVE_UNIT_Z, dir.getNormalizedVector());
		//Quaternion::generateLookAtRotation(rotation, dir);
		this->setRotation(rotation);
	}

	//////////////////////////////////////////////////////////////////////////
	void			Light::enableVolumeHelper(bool enable, const Color& color)
	{
		const HMATERIALINSTANCE& material = Impl::getLightRenderManager().getLightMaterial((ELightType)mType);
		if (enable)
		{
			mMask.raiseBits(LRHM_VOLUME);
			material->enableColor(true, color);	//shared instance will affect all
		}
		else
		{
			material->enableColor(false, color);
			mMask.clearBits(LRHM_VOLUME);
		}
	}

	/************************************************************************/
	/* SpaceContent overrides                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void			Light::notifyPositionChange()
	{
		mWorldTransform.setTranslation(this->getPosition());
		SpaceContent::notifyPositionChange();
	}

	//////////////////////////////////////////////////////////////////////////
	void			Light::notifyRotationChange()
	{
		mDirection = Vector3::NEGATIVE_UNIT_Z*this->getRotation();
		Matrix44::generateTransform(mWorldTransform, this->getPosition(), this->getScale(), this->getRotation());
		SpaceContent::notifyRotationChange();
	}
	
	//////////////////////////////////////////////////////////////////////////
	void			Light::notifyScaleChange()
	{
		if (mType != LT_DIRECTIONAL)
			this->updateAttenuation();
		Matrix44::generateTransform(mWorldTransform, this->getPosition(), this->getScale(), this->getRotation());
		SpaceContent::notifyScaleChange();
	}

	//////////////////////////////////////////////////////////////////////////
	void			Light::updateRender(IRenderQueue* queue)
	{
		if( mType == LT_DIRECTIONAL )
			Impl::getLightRenderManager().enqueueLightQuad(queue, this);
		else
			queue->addRenderable(this);

		if (mMask.checkBits(LRHM_VOLUME))
			mBoundingHelper.enqueue(queue);
	}


	/************************************************************************/
	/* IRenderable interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	RenderType&				Light::getRenderType() const
	{
		return Impl::getLightRenderManager().getLightRenderType();
	}

	//////////////////////////////////////////////////////////////////////////
	const GraphicsGeometry&	Light::getGeometry() const
	{
		return Impl::getLightRenderManager().getLightQuad();
	}

	//////////////////////////////////////////////////////////////////////////
	const MaterialInstance*	Light::getMaterial() const
	{
		return Impl::getLightRenderManager().getLightMaterial(ELightType(mType));
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void					Light::updateAttenuation()
	{
		if (mType != LT_DIRECTIONAL)
		{
			scalar range = this->getScale().z;
#if 0
			scalar attenuationStart = Math::Clamp(mAttenuationStart, 0.0f, 1.0f);
			//attenuation = 1/(attenuation[0] + attenuation[1] * Distance + attenuation[2] * Distance ^ 2)
			//cutoff = minAttenuation (dist=range)
			//offset attenuation so that attenuation = 0 at dist == range
			//A = (attenuation - cutoff) / (1 / attenuation.x - cutoff);
			scalar outerRange = range - range*attenuationStart;
			scalar minAttenuation = 1 / Vector3(1, outerRange, outerRange*outerRange).dotProduct(mAttenuation);

			float spotRange = this->getScale().x*0.5f;
			scalar outerAngle = std::atan(spotRange / range) * 2;
			mRanges[0] = range;
			mRanges[1] = intensity;
			mRanges[3] = Math::Radian2Degree(outerAngle);
			mRanges[2] = Math::Clamp(mRanges[2], 0.0f, mRanges[3]);

			//shader parameters
			mAttenuationParams[1] = mAttenuation * (1 / mAttenuation.x - minAttenuation);
			mAttenuationParams[1].w = -minAttenuation / (1 / mAttenuation.x - minAttenuation);

			mAttenuationParams[0][0] = range*range;
			mAttenuationParams[0][1] = range*attenuationStart;
			//spot light
			mAttenuationParams[0][2] = std::cos(Math::Degree2Radian(this->getSpotLightInnerAngle()) / 2);
			mAttenuationParams[0][3] = std::cos(outerAngle / 2);
			assert(mAttenuationParams[0][2] >= mAttenuationParams[0][3]);
#else
			//attenuation = 1/(attenuation * Distance)	//linear attenuation
			float radius = this->getScale().x;
			scalar outerAngle = std::atan(radius / range) * 2;
			mRange = range;
			mSpotOutAngle = Math::Radian2Degree(outerAngle);
			mSpotInnerAngle = Math::Clamp(mSpotInnerAngle, 0.0f, mSpotOutAngle);
			//shader parameters
			scalar minAttenuation = 1 / (range*mAttenuation);

			mAttenuationParam[0] = -minAttenuation;
			mAttenuationParam[1] = range * minAttenuation;
			mAttenuationParam[2] = std::cos(Math::Degree2Radian(mSpotInnerAngle) / 2);
			mAttenuationParam[3] = std::cos(outerAngle / 2);
			assert(mAttenuationParam[2] >= mAttenuationParam[3]);
#endif
		}
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	RenderType&				Light::LightHelper::getRenderType() const
	{
		return HelperRenderType::getSingleton();
	}

	//////////////////////////////////////////////////////////////////////////
	const GraphicsGeometry&	Light::LightHelper::getGeometry() const
	{
		switch (mLight->getType())
		{
		case LT_DIRECTIONAL:
		{
			index_t state = mState++;
			if (state == 0)
				return RenderUtility::getUnitArrowHead(GA_NEG_Z);
			else
				return RenderUtility::getUnitArrowTail(GA_NEG_Z);
		}
		case LT_POINT:
			return Impl::getLightRenderManager().getLightSphere();
		case LT_SPOT:
			return Impl::getLightRenderManager().getLightCone();
		}
		BLADE_EXCEPT(EXCE_STATE_ERROR, BTString("light type enum error."));
	}

	//////////////////////////////////////////////////////////////////////////
	void Light::LightHelper::enqueue(IRenderQueue* queue)
	{
		if (mLight->getType() == LT_DIRECTIONAL)
		{
			mState = 0;
			queue->addRenderable(this);
			queue->addRenderable(this);
		}
		else
			queue->addRenderable(this);
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	MaterialInstance* AmbientLight::getMaterial() const
	{
		return Impl::getLightRenderManager().getLightMaterial(LT_COUNT);	//special material index
	}

}//namespace Blade
