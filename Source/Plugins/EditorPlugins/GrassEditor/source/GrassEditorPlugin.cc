/********************************************************************
	created:	2018/05/27
	filename: 	GrassEditorPlugin.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladeGrassEditor.h>
#include <databinding/DataSourceWriter.h>
#include <interface/IWorldEditableManager.h>
#include <BladeGrass_blang.h>
#include <interface/IGrassConfig.h>
#include "GrassEditorPlugin.h"
#include "interface_imp/GrassEditable.h"

namespace Blade
{
	static const TString PLUGIN_NAME = BTString("Editor_GrassEditorPlugin");
	BLADE_DEF_PLUGIN(GrassEditorPlugin);

	//////////////////////////////////////////////////////////////////////////
	GrassEditorPlugin::GrassEditorPlugin()
		:PluginBase(PLUGIN_NAME)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	GrassEditorPlugin::~GrassEditorPlugin()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	void				GrassEditorPlugin::getSupportList(TStringParam& supportList) const
	{
		supportList.push_back(BTString("GrassEditor"));
	}

	//////////////////////////////////////////////////////////////////////////
	void				GrassEditorPlugin::getDependency(TStringParam& dependencyList) const
	{
		dependencyList.push_back(BTString("GraphicsGrass"));
		dependencyList.push_back(BTString("WorldEditor"));
	}

	//////////////////////////////////////////////////////////////////////////
	void				GrassEditorPlugin::install()
	{
		NameRegisterFactory(GrassEditable, IEditable, GrassEditable::GRASS_EDITABLE_TYPE);

		//register Grass editor element data source (no extra data)
		//DataSourceWriter<GrassEditorElement> writer(GrassEditorElement::GRASS_EDITOR_ELEMENT, DS_SIGNATURE(EditorElement));

		//IStageConfig::getSingleton().addLayerElement(IPage::DEFAULT_LAYER, GrassEditorElement::Grass_EDITOR_ELEMENT);
	}

	//////////////////////////////////////////////////////////////////////////
	void				GrassEditorPlugin::uninstall()
	{
		IWorldEditableManager::getInterface().unregisterEditableInfo(BTLang(BLANG_GRASS));
	}

	//////////////////////////////////////////////////////////////////////////
	void				GrassEditorPlugin::initialize()
	{
		//register editable types into world editable manager

		//ConfigGroup* group = BLADE_NEW ConfigGroup(BTString("Grass Config"));
		//mGrassConfig.bind(group);
		//group->addSubConfig(IGrassConfigManager::getInterface().getGrassCreationConfig());
		//group->addSubConfig(IGrassConfigManager::getInterface().getGrassGlobalConfig());

		ED_INFO info;
		info.mClassName = GrassEditable::GRASS_EDITABLE_TYPE;
		//info.mConfigOnCreation = IGrassConfig::getInterface().getGrassCreationConfig();
		info.mConfigOnCreation = NULL;
		info.mModalConfig = true;
		info.mEditableName = BTLang(BLANG_GRASS);
		//info.mGlobalConfig = mGrassGlobalConfig;
		info.mGlobalConfig = NULL;
		info.mInstanceLimit = 1;
		info.mType = IGrassConfig::getInterface().getGraphicsType();
		info.mNeedAppFlag = true;
		info.mNeedMenuCreation = true;
		//info.mGlobalConfigNotify = Delegate(IGrassConfig::getInterfacePtr(), &IGrassConfig::notifyGlobalConfigChanged);
		IWorldEditableManager::getInterface().registerEditableInfo(info);
	}

	//////////////////////////////////////////////////////////////////////////
	void				GrassEditorPlugin::shutdown()
	{
		//mGrassGlobalConfig.clear();
	}
	
}//namespace Blade