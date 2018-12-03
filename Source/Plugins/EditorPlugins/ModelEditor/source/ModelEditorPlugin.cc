/********************************************************************
	created:	2017/2/27
	filename: 	ModelEditorPlugin.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "ModelEditorPlugin.h"
#include <interface/IStageConfig.h>
#include <interface/IModelConfigManager.h>
#include <interface/IEditorFramework.h>
#include <interface/IEditorToolManager.h>
#include <interface/IMediaLibrary.h>
#include <interface/IWorldEditableManager.h>

#include "ModelEditable.h"
#include "ModelEditorElement.h"
#include "tools/ModelPlacementTool.h"
#include "tools/ModelBrushTool.h"
#include "ModelPreviewer.h"

namespace Blade
{
	static const TString PLUGIN_NAME = BTString("Editor_ModelEditorPlugin");
	BLADE_DEF_PLUGIN(ModelEditorPlugin);

	//////////////////////////////////////////////////////////////////////////
	ModelEditorPlugin::ModelEditorPlugin()
		:PluginBase(PLUGIN_NAME)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	ModelEditorPlugin::~ModelEditorPlugin()
	{

	}

	/************************************************************************/
	/* IPlugin interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void				ModelEditorPlugin::getSupportList(TStringParam& supportList) const
	{
		supportList.push_back(BTString("ModelEditor"));
	}

	//////////////////////////////////////////////////////////////////////////
	void				ModelEditorPlugin::getDependency(TStringParam& dependencyList) const
	{
		dependencyList.push_back(BTString("WorldEditor"));
		dependencyList.push_back(BTString("GraphicsModel"));		
	}

	//////////////////////////////////////////////////////////////////////////
	void				ModelEditorPlugin::install()
	{
		//register file class type
	}

	//////////////////////////////////////////////////////////////////////////
	void				ModelEditorPlugin::uninstall()
	{
		IWorldEditableManager::getInterface().unregisterEditableInfo(BTLang(BLANG_MODEL));
	}

	//////////////////////////////////////////////////////////////////////////
	void				ModelEditorPlugin::initialize()
	{
		NameRegisterFactory(ModelPreviewer, IMediaLibrary::IMediaPreviewer, ModelEditorConsts::MEDIA_PREVIEWER_MODEL);
		NameRegisterFactory(ModelEditable, IEditable, ModelEditable::MODEL_EDITABLE_CLASS_NAME);

		IModelConfigManager::getInterface().setEditingMode(true);

		//register media type
		IMediaLibrary::MEDIA_TYPE modelFileMediaType;
		modelFileMediaType.mType = ModelConsts::MODEL_ANIMATION_ITEM_HINT;
		modelFileMediaType.mIndex = 0;
		modelFileMediaType.mIsPlayable = false;
		modelFileMediaType.mSharedIcon = false;
		modelFileMediaType.mPreviewer.bind(BLADE_FACTORY_CREATE(IMediaLibrary::IMediaPreviewer, ModelEditorConsts::MEDIA_PREVIEWER_MODEL));
		//modelFileMediaType.mFilter.bind();
		modelFileMediaType.mExtensions.reserve(2);
		modelFileMediaType.mExtensions.push_back(BTString("blm"));
		index_t index = IMediaLibrary::getSingleton().registerMediaType(modelFileMediaType, BTString("model:/building/static"));
		IMediaLibrary::getSingleton().addMediaFilePath(index, BTString("model:/tree/static"));

		ModelPlacementTool* tool = BLADE_NEW ModelPlacementTool();
		IEditorToolManager::getSingleton().addTool(tool);
		IEditorFramework::getSingleton().getFrameworkNotification()->addFileTypeTool(WORLD_FILE_TYPE, tool);
		ModelBrushTool* btool = BLADE_NEW ModelBrushTool();
		IEditorToolManager::getSingleton().addTool(btool);
		IEditorFramework::getSingleton().getFrameworkNotification()->addFileTypeTool(WORLD_FILE_TYPE, btool);

		//register model info for dynamic creation
		ED_INFO info;
		info.mClassName = ModelEditable::MODEL_EDITABLE_CLASS_NAME;
		info.mConfigOnCreation = NULL;
		info.mModalConfig = true;
		info.mEditableName = ModelEditorConsts::MODEL_EDITABLE_NAME;
		info.mGlobalConfig = NULL;
		info.mInstanceLimit = std::numeric_limits<size_t>::max();
		info.mType = &IModelConfigManager::getInterface().getStaticModelType();
		info.mNeedAppFlag = true;
		info.mNeedMenuCreation = false;
		IWorldEditableManager::getInterface().registerEditableInfo(info);

		IStageConfig::getSingleton().addLayerElement(ModelConsts::MODEL_PAGE_LAYER, EditorElement::EDITOR_ELEMENT_TYPE);
	}

	//////////////////////////////////////////////////////////////////////////
	void				ModelEditorPlugin::shutdown()
	{
		BLADE_DELETE IEditorToolManager::getSingleton().removeTool(ModelEditorConsts::MODEL_EDITOR_CATEGORY, ModelPlacementTool::MODEL_PLACEMENT_TOOL_NAME);
		BLADE_DELETE IEditorToolManager::getSingleton().removeTool(ModelEditorConsts::MODEL_EDITOR_CATEGORY, ModelBrushTool::MODEL_BRUSH_TOOL_NAME);
	}

}//namespace Blade