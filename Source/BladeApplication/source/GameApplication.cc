/********************************************************************
	created:	2010/04/03
	filename: 	GameApplication.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <GameApplication.h>
#include <utility/Profiling.h>
#include <interface/IFramework.h>
#include <interface/IConfigManager.h>
#include <interface/ILogManager.h>
#include <interface/IPlatformManager.h>
#include <interface/ITaskManager.h>
#include <interface/IEnvironmentManager.h>
#include <interface/IResourceManager.h>
#include <databinding/IDataSourceDataBase.h>

namespace Blade
{
	const TString GameApplication::TYPE = BTString("Game");

	//////////////////////////////////////////////////////////////////////////
	GameApplication::GameApplication()
		:Application(TYPE)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	GameApplication::~GameApplication()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	bool	GameApplication::preInitialize()
	{
		IEnvironmentManager::getSingleton().setVariable( ConstDef::EnvString::WORKING_MODE,BTString("game") );
		IEnvironmentManager::getSingleton().setVariable(ConstDef::EnvString::STARTUP_IMAGE, BTString("media:splash.png"));
#ifndef SHIPPING
		IEnvironmentManager::getSingleton().setVariable(ConstDef::EnvBool::DEVELOPER_MODE, true);
#endif
		if( !Application::preInitialize() )
			return false;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	GameApplication::doInitialize()
	{
		try
		{
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
	bool	GameApplication::postInitialize()
	{
		bool ret = Application::postInitialize();
		if( ret )
			IFramework::getSingleton().start();
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	void	GameApplication::closeImpl()
	{
		try
		{
			HSTREAM stream = IResourceManager::getSingleton().openStream(this->Application::getCommonConfigFile(), false);
			IConfigManager::getSingleton().saveCommonConfig(stream);
			stream = IResourceManager::getSingleton().openStream(this->Application::getConfigFile(), false);
			IConfigManager::getSingleton().saveConfig( stream );
			IConfigManager::getSingleton().clearAllConfigs();
			IFramework::getSingleton().terminate();
			IResourceManager::getSingleton().removeAllResources();
			ITaskManager::getSingleton().shutdown();
			IPlatformManager::getSingleton().shutdownAllPlugins();
			IDataSourceDataBase::getSingleton().shutdown();
			IPlatformManager::getSingleton().unloadAllPlugins();
		}

		catch(Exception& e)
		{
			BLADE_LOG(Error, e.getFullDescription());
			IPlatformManager::getSingleton().showMessageBox(e.getFullDescription(),e.getTypeName() );
		}
	}
	
}//namespace Blade