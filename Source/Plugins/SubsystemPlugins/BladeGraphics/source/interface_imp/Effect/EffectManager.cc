/********************************************************************
	created:	2011/04/02
	filename: 	EffectManager.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <interface/ITimeService.h>

#include <interface/IGraphicsSystem.h>
#include <interface/IRenderTypeManager.h>
#include <interface/ICamera.h>

#include <Element/CameraElement.h>
#include <TextureResource.h>
#include <Material.h>
#include <Technique.h>
#include <Pass.h>
#include <GraphicsCommand.h>
#include "EffectManager.h"
#include "CustomEffect.h"

namespace Blade
{
	EffectRenderType EffectManager::msRenderType;

	//////////////////////////////////////////////////////////////////////////
	EffectManager::EffectManager(GraphicsScene* scene)
		:mScene(scene)
	{
		mUpdateStateMask.raiseBitAtIndex(SS_ASYNC_UPDATE);
	}

	//////////////////////////////////////////////////////////////////////////
	EffectManager::~EffectManager()
	{
		//detach & delete all
		{
			ScopedLock lock(mEffectLock);

			for (PersitentEffectList::iterator i = mEffectList.begin(); i != mEffectList.end(); ++i)
			{
				const HGRAPHICSEFFECT& hEffect = *i;
				this->deactivateEffect(hEffect);
			}
			mEffectList.clear();
		}

		{
			ScopedLock lock(mLifeTimeEffectLock);

			for (TimingEffectList::iterator i = mLifeTimeEffect.begin(); i != mLifeTimeEffect.end(); ++i)
			{
				EFFECT_INFO& info = *i;
				const HGRAPHICSEFFECT& hEffect = info.mEffect;
				this->deactivateEffect(hEffect);
			}
			mLifeTimeEffect.clear();
		}
	}

	/************************************************************************/
	/* IGraphicsEffectManger interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool	EffectManager::addEffect(const HGRAPHICSEFFECT& hEffect, scalar lifeTime)
	{
		BLADE_TS_VERITY_GRAPHICS_PUBLIC_ACCESS();

		if (hEffect == HGRAPHICSEFFECT::EMPTY)
			return false;

		if (lifeTime <= 0)
		{
			ScopedLock lock(mEffectLock);
			return mEffectList.insert(hEffect).second;
		}
		//timing
		else
		{
			EFFECT_INFO info(hEffect, lifeTime);
			ScopedLock lock(mLifeTimeEffectLock);
			if (std::find(mLifeTimeEffect.begin(), mLifeTimeEffect.end(), info) != mLifeTimeEffect.end())
				return false;
			mLifeTimeEffect.push_back(info);
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		EffectManager::removeEffect(const HGRAPHICSEFFECT& hEffect)
	{
		//only should be called in async run state from other sub systems
		//if within graphics system thread, any state is allowed
		BLADE_TS_VERITY_GRAPHICS_PUBLIC_ACCESS();

		if (hEffect == HGRAPHICSEFFECT::EMPTY || hEffect->getManager() != this)
			return false;

		{
			ScopedLock lock(mEffectLock);

			PersitentEffectList::iterator i = mEffectList.find(hEffect);
			if (i != mEffectList.end())
			{
				this->deactivateEffect(hEffect);
				mEffectList.erase(i);
				return true;
			}
		}

		{
			EFFECT_INFO info(hEffect, 0);
			ScopedLock lock(mLifeTimeEffectLock);

			TimingEffectList::iterator i = std::find(mLifeTimeEffect.begin(), mLifeTimeEffect.end(), info);
			if (i != mLifeTimeEffect.end())
			{
				this->deactivateEffect(hEffect);
				mLifeTimeEffect.erase(i);
				return true;
			}
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	HGRAPHICSEFFECT	EffectManager::createEffect(const TString& type)
	{
		BLADE_TS_VERITY_GRAPHICS_PUBLIC_ACCESS();

		IGraphicsEffect* effect = BLADE_FACTORY_CREATE(IGraphicsEffect, type);
		this->manageEffect(effect);
		return HGRAPHICSEFFECT(effect);
	}

	//////////////////////////////////////////////////////////////////////////
	HGRAPHICSEFFECT	EffectManager::createCustomEffect(const Vector3* vertexdata, size_t vertexCount, bool dynamicUpdate, bool independent,
		Material* material, const HTEXTURESTATE& textureState,
		const AABB* worldAABB/* = NULL*/, const uint32* indexdata/* = NULL*/, size_t indexCount/* = 0*/)
	{
		BLADE_TS_VERITY_GRAPHICS_PUBLIC_ACCESS();

		if (material == NULL || vertexdata == NULL || textureState == NULL)
			return HGRAPHICSEFFECT::EMPTY;

		CustomEffect* effect = BLADE_NEW CustomEffect(material, dynamicUpdate, independent);
		HGRAPHICSEFFECT hEffect(effect);

		this->manageEffect(effect);
		if (worldAABB != NULL)
			effect->setWorldAABB(*worldAABB);
		effect->setImmediateBuffer(vertexdata, vertexCount, indexdata, indexCount, true);
		const HMATERIALINSTANCE& matInstance = effect->setCustomMaterial(material);
		matInstance->setSharedTextureState(textureState);
		matInstance->load();
		mScene->getUpdater()->addForUpdateOnce(BLADE_NEW AddContentToSpaceCommand(effect, mScene->getSpaceCoordinatorImpl(), true));
		return hEffect;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	EffectManager::updateCustomEffect(const HGRAPHICSEFFECT& hCustomEffect, const Vector3* vertexdata, size_t vertexCount,
		const AABB* worldAABB/* = NULL*/, const uint32* indexdata/* = NULL*/, size_t indexCount/* = 0*/)
	{
		//only should be called in async run state from other sub systems
		//if within graphics system thread, any state is allowed
		BLADE_TS_VERITY_GRAPHICS_PUBLIC_ACCESS();

		if (hCustomEffect == HGRAPHICSEFFECT::EMPTY || hCustomEffect->getType() != CustomEffect::CUSTOM_EFFECT_TYPE)
			return false;
		CustomEffect* effect = static_cast<CustomEffect*>(hCustomEffect);
		assert(effect->isDynamic());

		if (effect->getSpace() == NULL || !effect->isDynamic())
			return false;

		if (worldAABB != NULL)
		{
			effect->setWorldAABB(*worldAABB);

			ISpaceCoordinator* coord = mScene->getSpaceCoordinatorImpl();
			if (coord != NULL)
				coord->notifyContentChanged(effect);
		}

		effect->setImmediateBuffer(vertexdata, vertexCount, indexdata, indexCount, worldAABB == NULL);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	EffectManager::setCustomShaderVariable(const TString& materialName, const TString& variableName,
		SHADER_CONSTANT_TYPE type, size_t count, const void* pData)
	{
		BLADE_TS_VERITY_GRAPHICS_PUBLIC_ACCESS();

		if (pData == NULL)
			return false;

		Material* mat = IMaterialManager::getSingleton().getMaterial(materialName);
		if (mat == NULL)
			return false;

		return mat->getActiveTechnique()->setShaderVariable(variableName, type, count, pData);
	}

	/************************************************************************/
	/* IEffectManager interface                                                                     */
	/************************************************************************/

	/************************************************************************/
	/* IGraphicsUpdatable interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void			EffectManager::update(SSTATE)
	{
		//update timed custom effect
		ITimeService* timeSvc = ITimeService::getSingletonPtr();

		TimingEffectList::iterator i = mLifeTimeEffect.begin();
		while (i != mLifeTimeEffect.end())
		{
			EFFECT_INFO& info = *i;
			scalar lifetime = info.mLifeTime;
			scalar& timing = info.mTiming;

			//timing only start on material loading finished
			if (info.mEffect->isReady())
			{
				timing += timeSvc->getTimeSource().getTimeThisLoop();
				if (lifetime <= timing)
				{
					GraphicsElement* element = static_cast<GraphicsElement*>(info.mEffect->getHost());
					if (element != NULL)
						element->removeGraphicsEffect(info.mEffect);

					i = mLifeTimeEffect.erase(i);
				}
			}
		}
	}


	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	//static
	EffectRenderType&	EffectManager::getEffectRenderType()
	{
		return msRenderType;
	}

	//////////////////////////////////////////////////////////////////////////
	class RemoveEffectCommand : public RemoveContentFromSpaceCommand
	{
	public:
		HGRAPHICSEFFECT mKeeper;	//keep object alive

		RemoveEffectCommand(const HGRAPHICSEFFECT& effect, ISpaceContent* content, ISpace* space)
			:RemoveContentFromSpaceCommand(content, space, true)
			,mKeeper(effect)
		{

		}

		/** @brief  */
		virtual void execute()
		{
			RemoveContentFromSpaceCommand::execute();
			mKeeper.clear();
		}
	};

	//////////////////////////////////////////////////////////////////////////
	void EffectManager::deactivateEffect(const HGRAPHICSEFFECT& hEffect)
	{
		BLADE_TS_VERITY_GRAPHICS_PUBLIC_ACCESS();

		IGraphicsInterface* element = hEffect->getHost();
		if (element != NULL)
			element->removeEffect(hEffect);
		else if (hEffect->getType() == CustomEffect::CUSTOM_EFFECT_TYPE)
		{
			RemoveEffectCommand* cmd = BLADE_NEW RemoveEffectCommand(hEffect, static_cast<CustomEffect*>(hEffect), mScene->getSpaceCoordinatorImpl());
			mScene->getUpdater()->addForUpdateOnce(cmd);
		}
		this->unmanageEffect(hEffect);
	}

}//namespace Blade
