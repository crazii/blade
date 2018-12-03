/********************************************************************
	created:	2010/04/02
	filename: 	BladeGamePlugin.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "BladeGamePlugin.h"
#include <interface/IPlatformManager.h>
#include <interface/IStageConfig.h>
#include <databinding/DataSourceWriter.h>
#include "interface_imp/CameraController.h"
#include "interface_imp/GameSystem.h"
#include "interface_imp/GameWorld.h"
#include "interface_imp/GameWorldData.h"


namespace Blade
{
	template class Factory<IGameSystem>;
	template class Factory<GameElement>;

	static const TString PLUGIN_NAME = BTString("BladeGame");
	BLADE_DEF_PLUGIN(BladeGamePlugin);

	static const TString GAME_SYSTEM_NAME = BTString("BladeGameSystem");

	//////////////////////////////////////////////////////////////////////////
	BladeGamePlugin::BladeGamePlugin()
		:PluginBase(PLUGIN_NAME)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	BladeGamePlugin::~BladeGamePlugin()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void				BladeGamePlugin::getSupportList(TStringParam& supportList) const
	{
		supportList.push_back(BTString("GameSystem"));
	}

	//////////////////////////////////////////////////////////////////////////
	void				BladeGamePlugin::getDependency(TStringParam& dependencyList) const
	{
		dependencyList.push_back(BTString("WindowService"));
		dependencyList.push_back(BTString("GraphicsService"));
		dependencyList.push_back(BTString("GeometryService"));
		dependencyList.push_back(BTString("UIService"));

		dependencyList.push_back(BTString("ImageManager"));
		dependencyList.push_back(BTString("GraphicsTerrain"));
		dependencyList.push_back(BTString("GraphicsAtmosphere"));
	}

	//////////////////////////////////////////////////////////////////////////
	void				BladeGamePlugin::install()
	{
		RegisterSingleton(GameSystem,IGameSystem);
		NameRegisterFactory(GameWorld,ILogicWorld,BTString("Default"));
		NameRegisterFactory(GameScene,IScene,GameScene::GAME_SCENE_TYPE);
		NameRegisterSingleton(GameSystem,ISubsystem,GAME_SYSTEM_NAME);

		{
			DataSourceWriter<CameraController> writer(BTString("CameraController"));
			//version 0.1
			writer.beginVersion(Version(0,1,'A'));
			{
				writer << CameraState::NEAR_PLANE	<< &CameraController::onConfigChange	<<	&CameraController::mNearData;;
				writer << CameraState::FAR_PLANE	<< &CameraController::onConfigChange	<<	&CameraController::mFarData;
				writer << CameraState::ASPECT		<< &CameraController::onConfigChange	<<	&CameraController::mAspectRatioData;
				writer << CameraState::FIELD_OF_VIEW<< &CameraController::onConfigChange	<<	&CameraController::mFOVData;
			}
			writer.endVersion();
		}

		NameRegisterFactory(CameraController,GameElement,CameraController::CAMERA_CONTROLLER_TYPE);


		{
			DataSourceWriter<GameWorldData> writer(GameWorldData::GAME_WORLD_DATA_TYPE);
			//version 0.1
			writer.beginVersion( Version(0,1,'A') );
			{
				writer << GameWorldData::WORLD_NAME << CUIH_NONE << CAF_NONE_CONFIG << &GameWorldData::mWorldName;
				writer << GameWorldData::WORLD_SART_POS << POINT3::ZERO << &GameWorldData::mStartPostition;
				//Delegate(NULL, &GameWorldData::onConfigChange);
			}
			writer.endVersion();
		}
		NameRegisterFactory(GameWorldData,ISerializable,GameWorldData::GAME_WORLD_DATA_TYPE);
	}

	//////////////////////////////////////////////////////////////////////////
	void				BladeGamePlugin::uninstall()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	void				BladeGamePlugin::initialize()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void				BladeGamePlugin::shutdown()
	{

	}
	
}//namespace Blade