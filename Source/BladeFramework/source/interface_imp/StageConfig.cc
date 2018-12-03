/********************************************************************
	created:	2011/04/29
	filename: 	StageConfig.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <ExceptionEx.h>
#include <interface/IPage.h>
#include <databinding/IDataSourceDataBase.h>
#include "StageConfig.h"
#include "Stage.h"

namespace Blade
{
	const TString IStageConfig::ENTITY_SKIP_PREFIX = BTString("[ENT_SKIP]");
	const TString IStageConfig::ELEMENT_SKIP_PREFIX = BTString("[ELM_SKIP]");
	const TString IStageConfig::SCENE_TYPE_SKIP_PREFIX = BTString("[SCN_SKIP]");

	//////////////////////////////////////////////////////////////////////////
	StageConfig::StageConfig()
	{
		mSkippingEntityNamePrefix = IStageConfig::ENTITY_SKIP_PREFIX;
		mSkippingElementNamePrefix = IStageConfig::ELEMENT_SKIP_PREFIX;
		mSkippingSceneTypePrefix = IStageConfig::SCENE_TYPE_SKIP_PREFIX;

		PageLayerConfig cfg;
		cfg.size = 1;	//not used	
		mLayerConfig[IPage::DEFAULT_LAYER] = cfg;
	}

	//////////////////////////////////////////////////////////////////////////
	StageConfig::~StageConfig()
	{

	}

	/************************************************************************/
	/* IStageConfig interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	const TString&	StageConfig::getEntitySkippingPrefix() const
	{
		return mSkippingEntityNamePrefix;
	}

	//////////////////////////////////////////////////////////////////////////
	void			StageConfig::setEntitySkippingPrefix(const TString& prefix)
	{
		if( prefix == TString::EMPTY )
		{
			BLADE_EXCEPT(EXC_INVALID_PARAM,BTString("invalid prefix.") );
		}

		mSkippingEntityNamePrefix = prefix;
	}

	//////////////////////////////////////////////////////////////////////////
	const TString&	StageConfig::getElementSkippingPrefix() const
	{
		return mSkippingElementNamePrefix;
	}

	//////////////////////////////////////////////////////////////////////////
	void			StageConfig::setElementSkippingPrefix(const TString& prefix)
	{
		if( prefix == TString::EMPTY )
		{
			BLADE_EXCEPT(EXC_INVALID_PARAM,BTString("invalid prefix.") );
		}

		mSkippingElementNamePrefix = prefix;
	}

	//////////////////////////////////////////////////////////////////////////
	const TString&	StageConfig::getSceneTypeSkippingPrefix() const
	{
		return mSkippingSceneTypePrefix;
	}

	//////////////////////////////////////////////////////////////////////////
	void	StageConfig::setSceneTypeSkippingPrefix(const TString& prefix)
	{
		mSkippingSceneTypePrefix = prefix;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	StageConfig::addPageLayer(const TString& name, const TStringParam& elementTypes, size_t subPageSize, scalar range, int priority/* = IPage::DEFAULT_PRIORITY*/)
	{
		if (name.empty() || elementTypes.size() == 0 || subPageSize == 0)
		{
			assert(false);
			return false;
		}

		PageLayerConfig& cfg = mLayerConfig[name];

		if (cfg.size != 0 || cfg.elements.size() != 0)	//already exist?
		{
			assert(false);
			return false;
		}

		cfg.elements = elementTypes;
		cfg.priority = priority;
		cfg.size = subPageSize;
		cfg.range = range;
		//std::sort(cfg.elements.begin(), cfg.elements.end());
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			StageConfig::addLayerElement(const TString& name, const TString& element)
	{
		if (name.empty() || element.empty())
		{
			assert(false);
			return false;
		}

		LayerConfigMap::iterator iter = mLayerConfig.find(name);
		if (iter != mLayerConfig.end())
		{
			PageLayerConfig& cfg = iter->second;

			if (cfg.elements.find(element) == INVALID_INDEX)
			{
				cfg.elements.push_back(element);
				//std::sort(cfg.elements.begin(), cfg.elements.end());
				return true;
			}
		}
		assert(false);
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	const HCONFIG&	StageConfig::getStageConfig()
	{
		DataBinder binder(STAGE_BINDING_CONFIG, NULL);
		if (binder.isBound())
		{
			const HCONFIG& cfg = binder.getConfig();
			binder.unbindTarget();
			return cfg;
		}
		return HCONFIG::EMPTY;
	}

}//namespace Blade
