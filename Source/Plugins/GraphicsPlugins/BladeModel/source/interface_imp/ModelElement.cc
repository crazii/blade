/********************************************************************
	created:	2013/04/09
	filename: 	ModelElement.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <TextureState.h>
#include "ModelElement.h"
#include <interface/ISpace.h>
#include <ConstDef.h>
#include <interface/IEnvironmentManager.h>
#include "StaticModelResource.h"

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	ModelElement::ModelElement()
		:GraphicsElement(ModelConsts::MODEL_ELEMENT_TYPE)
		,mModel(NULL)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	ModelElement::~ModelElement()
	{
		this->clearEffects();
		BLADE_DELETE mModel;
		mContent = NULL;
	}

	/************************************************************************/
	/* ISerializable interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void			ModelElement::prepareSave()
	{
		GraphicsElement::prepareSave();
		mDesc->mResourcePath = mOriginalResource;

		//strip sub mesh configs if they are identical to initial model setting
		//this will optimize runtime loading & memory usage in shipping/product app
		if (IEnvironmentManager::getSingleton().getVariable(ConstDef::EnvString::WORKING_MODE) == BTString("editor"))
		{
			if (mModel->getModelResource() != NULL)
			{
				SubMeshConfigList old = mMeshConfig;
				this->initMeshConfig(true, true);

				assert(old.size() == mMeshConfig.size());

				for (SubMeshConfigList::iterator oldit = old.begin(); oldit != old.end(); )
				{
					SubMeshConfigList::iterator it = std::find_if(mMeshConfig.begin(), mMeshConfig.end(), FnSubMeshConfigFinder(oldit->mName));
					if (it == mMeshConfig.end() || *it == *oldit)
						oldit = old.erase(oldit);
					else
						++oldit;
				}
				mMeshConfig = old;
			}
			else
				mMeshConfig.clear();	//TODO: clear or not?
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void			ModelElement::postSave()
	{
		GraphicsElement::postSave();
		this->setResourcePath(mOriginalResource);
		this->initMeshConfig(true);
	}

	//////////////////////////////////////////////////////////////////////////
	void			ModelElement::postProcess(const ProgressNotifier& notifier)
	{
		const HRESOURCE& hRes = this->getBoundResource();
		IModelResourceImpl* res = static_cast<IModelResourceImpl*>(hRes);
		if (MODEL_STATIC_BATCHING && !this->isDynamic() && res != NULL)
		{
			//this->setDynamic(0, (res->getSkeletonResource() != NULL));
			
			if (this->isDynamic())
			{
				StaticModelResource* staticRes = static_cast<StaticModelResource*>(res);
				res = static_cast<IModelResourceImpl*>(staticRes->getResource());
			}
		}
		//mBounding = res->getModelAABB(); //will update in GraphicsElement::postProcess
		mModel->setModelResource(res);

		//remove content on AABB change for static object in static space
		//probably on 1.initial loading, 2. in editor mode
		if (mContent->getSpace() != NULL)
		{
			if(!(mModel->getSpaceFlags()&CSF_DYNAMIC) || !(mContent->getSpace()->getSpaceFlag()&CSF_DYNAMIC))
				this->deactivateContent();
			else
				mContent->getSpace()->notifyContentChanged(mContent);
		}

		//apply serialized config to model
		this->applyMeshConfig();

		if (IEnvironmentManager::getSingleton().getVariable(ConstDef::EnvString::WORKING_MODE) == BTString("editor"))
			//initial creation or model resource changed
			this->initMeshConfig(false);

		GraphicsElement::postProcess(notifier);
	}

	/************************************************************************/
	/* IElement interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	Interface*			ModelElement::getInterface(InterfaceName type)
	{
		CHECK_RETURN_INTERFACE(ISyncAnimation, type, mModel);
		CHECK_RETURN_INTERFACE(IModel, type, static_cast<IModel*>(this));
		return GraphicsElement::getInterface(type);
	}

	//////////////////////////////////////////////////////////////////////////
	size_t			ModelElement::initParallelStates()
	{
		mParallelStates[ModelState::CURRENT_ANIMAION] = mPSInAnimation;
		return GraphicsElement::initParallelStates();
	}

	//////////////////////////////////////////////////////////////////////////
	void			ModelElement::onParallelStateChange(const IParaState& data)
	{
		if (data == mPSInAnimation)
		{
			assert(mModel != NULL);
			mModel->setSkinnedAnimation(mPSInAnimation);
		}
		else 
			GraphicsElement::onParallelStateChange(data);
	}

	//////////////////////////////////////////////////////////////////////////
	void			ModelElement::getResourceParams(ParamList& outParams, TString& outResType, TString& /*outSerializer*/)
	{
		if (MODEL_STATIC_BATCHING && !this->isDynamic())
		{
			outResType = StaticModelResource::STATIC_MODEL_RESOURCE_TYPE;
			outParams[StaticModelResource::PARAM_ORIGINAL_MODEL] = mOriginalResource;
			Matrix44 transform = Matrix44::generateTransform(mPosition, mScale, mRotation);
			outParams[StaticModelResource::PARAM_PRE_TRANSFORM] = transform;
		}
	}

	/************************************************************************/
	/* ElementBase interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void ModelElement::onResourceUnload()
	{
		GraphicsElement::onResourceUnload();
		assert(mModel != NULL);
		mModel->setModelResource(NULL);
	}

	/************************************************************************/
	/* GraphicsElement interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void			ModelElement::onInitialize()
	{
		mModel = BLADE_NEW Model();
		mModel->setElement(this);
		mContent = mModel;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			ModelElement::setDynamic(index_t, const bool& _dynamic)
	{
		bool _isDynamic = this->isDynamic();
		bool _setDynamic = _dynamic;

		if (_isDynamic != _setDynamic)
		{
			if (mModel->getModelResource() != NULL && mModel->hasSkinnedAnimation())
				_setDynamic = false;

			if (_isDynamic != _setDynamic)
			{
				if (MODEL_STATIC_BATCHING && mModel->getModelResource() != NULL)
				{
					//TODO: cancel exist loading process
					this->unloadResource(true);
					this->loadResource(false);
				}
			}
		}
		return GraphicsElement::setDynamic(0, _setDynamic);
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void			ModelElement::applyMeshConfig()
	{
		for (SubMeshConfigList::iterator it = mMeshConfig.begin(); it != mMeshConfig.end(); ++it)
		{
			SubMeshConfig& cfg = *it;
			SubMesh* submesh = mModel->findSubmesh(cfg.mName);
			if(submesh == NULL)
				continue;

			mModel->setSubMeshVisible(submesh, cfg.mVisible);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void			ModelElement::initMeshConfig(bool force, bool reset/* = false*/)
	{
		const SubMeshList& subMeshes = mModel->getSubMeshes();
		if (mMeshConfig.size() != subMeshes.size() || force)
		{
			SubMeshConfigList old = mMeshConfig;
			mMeshConfig.clear();
			for (SubMeshList::const_iterator i = subMeshes.begin(); i != subMeshes.end(); ++i)
			{
				SubMesh* submesh = *i;

				SubMeshConfig cfg;
				cfg.mName = submesh->getName();
				if (reset)
				{
					cfg.mVisible = true;
				}
				else
				{
					cfg.mVisible = submesh->isVisible();
				}
				
				SubMeshConfigList::iterator it = std::find_if(old.begin(), old.end(), FnSubMeshConfigFinder(cfg.mName));
				if (it == old.end())
					mMeshConfig.push_back(cfg);
				else
					mMeshConfig.push_back(*it);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void			ModelElement::onSubMeshConfigChanged(void* /*data*/)
	{
		if(mModel->getModelResource() != NULL)
			this->applyMeshConfig();
	}
	
}//namespace Blade