/********************************************************************
	created:	2017/07/12
	filename: 	RenderSchemeConfig.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "RenderSchemeConfig.h"
#include <utility/StringHelper.h>
#include <ConfigTypes.h>
#include <databinding/IDataSourceDataBase.h>
#include <databinding/ConfigDataHandler.h>
#include <interface/IGraphicsSystem.h>
#include <interface/public/IImageEffect.h>
#include <interface/IRenderSchemeManager.h>
#include <BladeGraphics_blang.h>
#include "RenderScheme.h"
#include "ImageEffectOutput.h"

namespace Blade
{
	static const TString RENDER_OPERATIONS = BTString("Rendering Operations");
	static const TString IMAGE_EFFECTS = BTString("Image Effects");

	namespace Impl
	{
		class GlobalDelegatHandler : public DelegateOnlyHandler
		{
		public:
			struct DelegateData
			{
				TString typeName;
				TString	configName;
				const void* binary;
				size_t bytes;
				CONFIG_ACCESS verify;
				index_t index;
			};

		public:
			GlobalDelegatHandler(const TString& typeName, HCDHANDLER original)
				:mTypeName(typeName)
				,mOriginal(original)
			{}

			/** @brief config's value is changed, maybe update handler's value from config */
			virtual bool	setData(const TString& name, const void* binary, size_t bytes, CONFIG_ACCESS verify, index_t index = INVALID_INDEX) const
			{
				if (mOriginal != NULL && mOriginal->getTarget() != NULL)
					mOriginal->setData(name, binary, bytes, verify, index);
				else
				{
					DelegateData data = { mTypeName, name, binary, bytes, verify, index };
					mDelegates.call(&data);
				}
				return true;
			}

			/** @brief get sub element count */
			virtual size_t	getCount() const { return mOriginal != NULL ? mOriginal->getCount() : 0; }

			/** @brief  */
			virtual void	bindTarget(Bindable* target) { if (mOriginal != NULL) mOriginal->bindTarget(target); }

			/** @brief  */
			virtual Bindable* getTarget() const { return (mOriginal != NULL && mOriginal->getTarget() != NULL) ? mOriginal->getTarget() : DelegateOnlyHandler::getTarget(); }

			/** @brief  */
			virtual HCDHANDLER clone() const { return HCDHANDLER(BLADE_NEW GlobalDelegatHandler(*this)); }

		protected:
			mutable TString mTypeName;
			HCDHANDLER		mOriginal;
		};
	}//namespace Impl

	//////////////////////////////////////////////////////////////////////////
	void RenderSchemeConfig::initConfig()
	{
		struct HandlerChanger : public IConfig::IVisitor
		{
			TString mTypeName;
			Delegate mDelegate;
			virtual void visit(IConfig* config)
			{
				HCDHANDLER newHandler(BLADE_NEW Impl::GlobalDelegatHandler(mTypeName, config->getDataHandler()));
				newHandler->addDelegate(mDelegate);
				config->setDataHandler(newHandler);
			}
		};

		ConfigGroup* operatorConfig = NULL;
		for (size_t i = 0; i < Factory<IRenderSchemeOperation>::getSingleton().getNumRegisteredClasses(); ++i)
		{
			const TString& operatorType = Factory<IRenderSchemeOperation>::getSingleton().getRegisteredClass(i);
			DataBinder binder(operatorType, NULL);
			if (binder.isBound())
			{
				HandlerChanger handlerChanger;
				handlerChanger.mTypeName = operatorType;
				handlerChanger.mDelegate = Delegate(this, &RenderSchemeConfig::onOperationConfigChanged);

				if (operatorConfig == NULL)
					operatorConfig = BLADE_NEW ConfigGroup(RENDER_OPERATIONS, CAF_NORMAL);

				HCONFIG cfg = binder.getConfig()->clone();
				cfg->traverse(&handlerChanger);
				operatorConfig->addSubConfig(cfg);
			}
		}
		if (operatorConfig != NULL)
			IGraphicsSystem::getSingleton().addAdvanceConfig(HCONFIG(operatorConfig));

		//TODO: add per-scene proxy entity/element to store image effect config on per scene basis
		ConfigGroup* ifxConfig = NULL;
		for (size_t i = 0; i < Factory<IImageEffect>::getSingleton().getNumRegisteredClasses(); ++i)
		{
			const TString& effectType = Factory<IImageEffect>::getSingleton().getRegisteredClass(i);
			DataBinder binder(effectType, NULL);
			if (binder.isBound())
			{
				HandlerChanger handlerChanger;
				handlerChanger.mTypeName = effectType;
				handlerChanger.mDelegate = Delegate(this, &RenderSchemeConfig::onIFXConfigChanged);

				if (ifxConfig == NULL)
					ifxConfig = BLADE_NEW ConfigGroup(IMAGE_EFFECTS, CAF_NORMAL_DEVMODE);

				HCONFIG cfg = binder.getConfig()->clone();
				cfg->traverse(&handlerChanger);
				ifxConfig->addSubConfig(cfg);
			}
		}
		if (ifxConfig != NULL)
			IGraphicsSystem::getSingleton().addAdvanceConfig(HCONFIG(ifxConfig));
	}

	//////////////////////////////////////////////////////////////////////////
	const HCONFIG&	RenderSchemeConfig::getOperationConfig(const TString& typeName) const
	{
		HCONFIG cfg = IGraphicsSystem::getSingleton().getAdvanceConfig(RENDER_OPERATIONS);
		return cfg != NULL ? cfg->getSubConfigByName(typeName) : HCONFIG::EMPTY;
	}

	//////////////////////////////////////////////////////////////////////////
	const HCONFIG& RenderSchemeConfig::getIFXConfig(const TString& typeName) const
	{
		HCONFIG cfg = IGraphicsSystem::getSingleton().getAdvanceConfig(IMAGE_EFFECTS);
		return cfg != NULL ? cfg->getSubConfigByName(typeName) : HCONFIG::EMPTY;
	}

	//////////////////////////////////////////////////////////////////////////
	void RenderSchemeConfig::onOperationConfigChanged(void* data)
	{
		//@see GlobalIFXDelegatHandler::setData
		const Impl::GlobalDelegatHandler::DelegateData* dataImpl = static_cast<const Impl::GlobalDelegatHandler::DelegateData*>(data);
		const TString& typeName = dataImpl->typeName;

		TPointerParam<IRenderScheme> schemeList;
		size_t count = IRenderSchemeManager::getSingleton().getActiveSchemes(schemeList);

		for (size_t i = 0; i < count; ++i)
		{
			RenderScheme* scheme = static_cast<RenderScheme*>(schemeList[i]);

			IRenderSchemeOperation* op = scheme->getOperation(typeName);
			if (op != NULL)
			{
				DataBinder binder(typeName, op);
				assert(binder.isBound());

				HCONFIG targetCfg;
				if ((targetCfg = binder.getConfig()->findConfig(dataImpl->configName)) != NULL && targetCfg->getDataHandler() != NULL)
					targetCfg->getDataHandler()->setData(dataImpl->configName, dataImpl->binary, dataImpl->bytes, dataImpl->verify, dataImpl->index);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void RenderSchemeConfig::onIFXConfigChanged(void* data)
	{
		//@see GlobalIFXDelegatHandler::setData
		const Impl::GlobalDelegatHandler::DelegateData* dataImpl = static_cast<const Impl::GlobalDelegatHandler::DelegateData*>(data);
		const TString& typeName = dataImpl->typeName;

		TPointerParam<IRenderScheme> schemeList;
		size_t count = IRenderSchemeManager::getSingleton().getActiveSchemes(schemeList);

		for (size_t i = 0; i < count; ++i)
		{
			RenderScheme* scheme = static_cast<RenderScheme*>(schemeList[i]);
			ImageEffectHelper* ifxHelper = scheme->getIFXHelper();

			ImageEffectOutput*const* outputs;
			size_t effectCount = ifxHelper->getEffectOutputs(outputs);
			for (size_t j = 0; j < effectCount; ++j)
			{
				IImageEffect* effect = outputs[j]->getEffect();
				if (effect->getName() == typeName)
				{
					DataBinder binder(typeName, static_cast<ImageEffectBase*>(effect));
					assert(binder.isBound());

					HCONFIG targetCfg;
					if((targetCfg=binder.getConfig()->findConfig(dataImpl->configName)) != NULL && targetCfg->getDataHandler() != NULL)
						targetCfg->getDataHandler()->setData(dataImpl->configName, dataImpl->binary, dataImpl->bytes, dataImpl->verify, dataImpl->index);
				}
			}
		}
	}
	
}//namespace Blade