/********************************************************************
	created:	2010/04/02
	filename: 	BladeGeometryPlugin.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "BladeGeometryPlugin.h"
#include <interface/IPlatformManager.h>
#include <interface/IStageConfig.h>
#include <interface/public/CommonState.h>
#include <databinding/DataSourceWriter.h>

#include "interface_imp/GeometrySystem.h"
#include "interface_imp/GeometryElement.h"
#include "interface_imp/GeometryElement_ReadOnly.h"

namespace Blade
{
	static const TString PLUGIN_NAME = BTString("BladeGeometry");
	BLADE_DEF_PLUGIN(GeometryPlugin);

	//////////////////////////////////////////////////////////////////////////
	GeometryPlugin::GeometryPlugin()
		:PluginBase(PLUGIN_NAME)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	GeometryPlugin::~GeometryPlugin()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void				GeometryPlugin::getSupportList(TStringParam& supportList) const
	{
		supportList.push_back(BTString("GeometryService"));
	}

	//////////////////////////////////////////////////////////////////////////
	void				GeometryPlugin::getDependency(TStringParam& dependencyList) const
	{
		BLADE_UNREFERENCED(dependencyList);
	}

	//////////////////////////////////////////////////////////////////////////
	void				GeometryPlugin::install()
	{
		NameRegisterFactory(GeometryScene,IScene,GeometryScene::GEOMETRY_SCENE_TYPE);

		{
			DataSourceWriter<GeometryElement> writer(GeomConsts::GEOMETRY_ELEMENT);
			//version 0.1
			writer.beginVersion( Version(0,1,'A') );
			{
				writer << CommonState::POSITION	<< CAF_NONE_CONFIG << &GeometryElement::onConfigChange <<	POINT3::ZERO		<<	&GeometryElement::mPositionData;
				writer << CommonState::SCALE	<< CAF_NONE_CONFIG << &GeometryElement::onConfigChange	<<	Vector3::UNIT_ALL	<<	&GeometryElement::mScaleData;
				writer << CommonState::ROTATION << CAF_NONE_CONFIG << &GeometryElement::onConfigChange	<<	Quaternion::IDENTITY<<	&GeometryElement::mRotationData;
			}
			writer.endVersion();
		}

		{
			DataSourceWriter<GeometryElement_ReadOnly> writer(GeomConsts::GEOMETRY_ELEMENT_READONLY);
			//version 0.1
			writer.beginVersion(Version(0, 1, 'A'));
			{
				writer << CommonState::POSITION << CAF_READ <<  POINT3::ZERO << &GeometryElement_ReadOnly::mPositionData;
				writer << CommonState::SCALE << CAF_READ <<  Vector3::UNIT_ALL << &GeometryElement_ReadOnly::mScaleData;
				writer << CommonState::ROTATION << CAF_READ << Quaternion::IDENTITY << &GeometryElement_ReadOnly::mRotationData;
			}
			writer.endVersion();
		}

		NameRegisterSingleton(GeometrySystem, ISubsystem, GeometrySystem::GEOMETRY_SYSTEM_NAME);
		NameRegisterFactory(GeometryElement, GeometryElementBase, GeomConsts::GEOMETRY_ELEMENT);
		NameRegisterFactory(GeometryElement_ReadOnly, GeometryElementBase, GeomConsts::GEOMETRY_ELEMENT_READONLY);
	}

	//////////////////////////////////////////////////////////////////////////
	void				GeometryPlugin::uninstall()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	void				GeometryPlugin::initialize()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void				GeometryPlugin::shutdown()
	{

	}

}//namespace Blade