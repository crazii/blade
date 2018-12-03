/********************************************************************
	created:	2015/09/24
	filename: 	ModelImporterPlugin.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <BladeModelImporter.h>
#include "ModelImporterPlugin.h"

#include <interface/IPlatformManager.h>
#include <interface/IResourceManager.h>
#include <interface/public/ui/IIconManager.h>
#include <interface/IEditorFramework.h>
#include <interface/IImporterManager.h>
#include <interface/IModelConfigManager.h>
#include <BladeModelViewer.h>

#include "interface_imp/FBXImporter.h"

namespace Blade
{
	static const TString PLUGIN_NAME = BTString("Editor_ModelImporterPlugin");
	BLADE_DEF_PLUGIN(ModelImporterPlugin);

	//////////////////////////////////////////////////////////////////////////
	ModelImporterPlugin::ModelImporterPlugin()
		:PluginBase(PLUGIN_NAME)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	ModelImporterPlugin::~ModelImporterPlugin()
	{

	}

	/************************************************************************/
	/* IPlugin interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void				ModelImporterPlugin::getSupportList(TStringParam& supportList) const
	{
		supportList.push_back( BTString("ModelImporter") );
	}

	//////////////////////////////////////////////////////////////////////////
	void				ModelImporterPlugin::getDependency(TStringParam& dependencyList) const
	{
		dependencyList.push_back( BTString("UI") );	//IIconManager implementation
		dependencyList.push_back( BTString("GraphicsModel") );
		//dependencyList.push_back( BTString("ModelViewer") );
	}

	//////////////////////////////////////////////////////////////////////////
	void				ModelImporterPlugin::install()
	{
		//TODO: fbx is cross platform
#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
		NameRegisterFactory(FBXImporter, IImporter, FBXImporter::FBX_IMPORTER);

		SEditorImporterInfo info;
		info.mName = FBXImporter::FBX_IMPORTER;
		info.mTarget = MODEL_FILE_TYPE;
		info.mTargetExt = ModelConsts::MODEL_FILE_EXT;
		info.mDescription = BTString("AutoDesk FBX Model File");
		info.mExtensions.push_back( BTString("fbx") );
		//use system icon, if FBX type not registered to system, icon will be an empty file.
		info.mIconID = IIconManager::getSingleton().loadIconForFileType( BTString("fbx") );
		IImporterManager::getSingleton().registerImporter(info);
		//resource manager can recognize it as model resource after converting/importing to BLM with .fbx extension
		IResourceManager::getSingleton().registerFileExtension(ModelConsts::MODEL_RESOURCE_TYPE, BTString("fbx"));
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	void				ModelImporterPlugin::uninstall()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void				ModelImporterPlugin::initialize()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	void				ModelImporterPlugin::shutdown()
	{

	}
	
}//namespace Blade