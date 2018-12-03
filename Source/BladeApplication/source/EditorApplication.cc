/********************************************************************
	created:	2010/05/23
	filename: 	EditorEditorApplication.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
#include <utility/Profiling.h>
#include <EditorApplication.h>
#include <interface/IFramework.h>
#include <interface/IConfigManager.h>
#include <interface/ILogManager.h>
#include <interface/IPlatformManager.h>
#include <interface/ITaskManager.h>
#include <interface/IEnvironmentManager.h>
#include <interface/IResourceManager.h>
#include <interface/IEditorFramework.h>
#include <interface/IEditorConfigManager.h>
#include <interface/ITimeService.h>
#include <interface/public/window/IWindowService.h>
#include <interface/public/IStartupOutput.h>
#include <ICmdParser.h>
#include <BladeBase_blang.h>
#include <BladeFramework_blang.h>
#include <BladeEditor_blang.h>
#include <databinding/IDataSourceDataBase.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	const TString EditorApplication::TYPE = BTString("Editor");

	//////////////////////////////////////////////////////////////////////////
	EditorApplication::EditorApplication()
		:Application(TYPE)
		,mEditorConfigFile( BTString("EditorConfig.xml") )
		,mInited(false)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	EditorApplication::~EditorApplication()
	{
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	bool	EditorApplication::doInitialize()
	{
		try
		{
			//after platform manager init, the EditUI plug-in is assumed to be loaded
			if( UIMainWindowFactory::getSingleton().getNumRegisteredClasses() == 0 
				|| Factory<IStartupOutput>::getSingleton().getNumRegisteredClasses() == 0 )
			{
				BLADE_EXCEPT(EXC_NEXIST,BTString("fatal error: no UI module loaded.") );
			}
			this->loadSubSystems();

			this->onSubSystemInstalled();
			return true;
		}
		catch(Exception& e)
		{
			BLADE_LOG(Error, e.getFullDescription() );
			IPlatformManager::getSingleton().showMessageBox(e.getFullDescription(),e.getTypeName() );
			return false;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool	EditorApplication::preInitialize()
	{
		Blade::initializeEditorFramework();

		IEnvironmentManager::getSingleton().setVariable(ConstDef::EnvString::WORKING_MODE, BTString("editor") );
		IEnvironmentManager::getSingleton().setVariable(ConstDef::EnvBool::DEVELOPER_MODE, true );
		IEnvironmentManager::getSingleton().setVariable(ConstDef::EnvString::STARTUP_IMAGE, BTString("media:splash.png"));

		return Application::preInitialize();
	}

	//////////////////////////////////////////////////////////////////////////
	bool	EditorApplication::postInitialize()
	{
		try
		{
			if( Application::postInitialize() == false )
				return false;

			//turn off auto window manually, ignoring configuration
			IWindowService::getSingleton().setAutoCreatingWindow(false);

			//start framework & initialize subsystems
			IFramework::getSingleton().start();
			//ITimeService::getSingleton().setLoopTimeLimit(0.0167f);

			//creat UI
			IUIMainWindow* pEditorUI = this->createEditorUI();
			assert( pEditorUI != NULL );

			IStartupOutput::getSingleton().show(true);
			IStartupOutput::getSingleton().outputProgress( BTLang(BLANG_INIT_UI) );

			//sub systems ready, init editor framework
			IEditorFramework::getSingleton().initialize(pEditorUI, IEditorFramework::EDITOR_RESOURCE_PATH + BTString("/editor.png"), mVersion);

			//load editor config
			IEditorConfigManager::getSingleton().loadConfig( this->getConfigPath() + BTString("/") + mEditorConfigFile );

			IStartupOutput::getSingleton().show(false);

			mInited = true;
			return true;
		}
		catch(Exception& e)
		{
			BLADE_LOG(Error, e.getFullDescription() );
			IPlatformManager::getSingleton().showMessageBox(e.getFullDescription(),e.getTypeName() );
			return false;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void	EditorApplication::closeImpl()
	{
		try
		{
			HSTREAM stream = IResourceManager::getSingleton().openStream( this->Application::getCommonConfigFile(), false );
			IConfigManager::getSingleton().saveCommonConfig( stream );
			stream = IResourceManager::getSingleton().openStream( this->Application::getConfigFile(), false );
			
			//--config mode doesn't need to save editor config, since EditorFramework is NOT initialized.
			//all editor components are NOT initialized, i.e. currently EditorViewportManager will not add config options
			//so saving config will cause config data loss
			if( mInited )
			{
				IEditorConfigManager::getSingleton().saveConfig( this->getConfigPath() + BTString("/") + mEditorConfigFile );
				IEditorConfigManager::getSingleton().shutdown();
				IEditorFramework::getSingleton().deinitialize();
			}

			IConfigManager::getSingleton().saveConfig( stream );
			IConfigManager::getSingleton().clearAllConfigs();
			IPlatformManager::getSingleton().shutdownAllPlugins();
			IFramework::getSingleton().terminate();
			IResourceManager::getSingleton().removeAllResources();
			ITaskManager::getSingleton().shutdown();
			IDataSourceDataBase::getSingleton().shutdown();
			IPlatformManager::getSingleton().unloadAllPlugins();
		}

		catch(Exception& e)
		{
			BLADE_LOG(Error, e.getFullDescription());
			IPlatformManager::getSingleton().showMessageBox(e.getFullDescription(),e.getTypeName() );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	IUIMainWindow*	EditorApplication::createEditorUI()
	{
		//create the first one
		return BLADE_FACTORY_CREATE(IUIMainWindow, UIMainWindowFactory::getSingleton().getRegisteredClass(0) );
	}

}

#endif//BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS