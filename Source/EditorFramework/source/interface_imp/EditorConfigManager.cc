/********************************************************************
	created:	2013/11/11
	filename: 	EditorConfigManager.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "EditorConfigManager.h"
#include <ConfigSerializer.h>
#include <interface/public/geometry/GeometryInterface_blang.h>
#include <interface/public/ui/UIEvents.h>
#include <interface/IResourceManager.h>
#include <interface/IEventManager.h>
#include <databinding/DataSourceWriter.h>
#include <ConfigTypes.h>
#include <interface/EditorEvents.h>

namespace Blade
{
	template class Factory<IEditorConfigManager>;

	static const TString EDITOR_CONFIG = BTString("EditorConfig");
	static const TString EDITOR_GLOBALCONFIG = BTString("EditorGlobalConfig");

	//////////////////////////////////////////////////////////////////////////
	EditorConfigManager::EditorConfigManager()
	{
		mData.mTransformSpace = GS_WORLD;

		{
			DataSourceWriter<EditorConfigData> writer(EDITOR_GLOBALCONFIG);
			//version 0.1
			writer.beginVersion(Version(0,1));

			TStringParam list;
			//note: order is important: same to the enum
			list.push_back( BXLang(BLANG_GEOMETRY_SPACE_OBJECT) );
			list.push_back( BXLang(BLANG_GEOMETRY_SPACE_LOCAL) );
			list.push_back( BXLang(BLANG_GEOMETRY_SPACE_WORLD) );
			writer << BXLang(BLANG_GEOMETRY_TRANSFORM_SPACE) << list << DS_USE_INDEX(2u)
				<< Delegate(this, &EditorConfigManager::onConfigChange)
				<< &EditorConfigData::mTransformSpace;
			writer.endVersion();
		}

		{
			DataSourceWriter<EditorConfigManager> writer(EDITOR_CONFIG);
			writer.beginVersion(Version(0,1));
			//struct/class type of member binding
			writer << EDITOR_GLOBALCONFIG << &EditorConfigManager::mData;

			writer.endVersion();
		}

		//bind this permanently
		DataBinder binder(EDITOR_CONFIG, this, true);
		mEditorConfig = binder.getConfig();
		mGlobalConfig = mEditorConfig->getSubConfigByName(EDITOR_GLOBALCONFIG);
	}

	//////////////////////////////////////////////////////////////////////////
	EditorConfigManager::~EditorConfigManager()
	{
		
	}

	/************************************************************************/
	/* IEditorConfigManager interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void	EditorConfigManager::shutdown()
	{
		//note: config may hold data from another library, clear them
		mEditorConfig.clear();
		mGlobalConfig.clear();
	}

	//////////////////////////////////////////////////////////////////////////
	bool	EditorConfigManager::loadConfig(const TString& config)
	{
		HSTREAM stream = IResourceManager::getSingleton().loadStream(config);
		return ConfigSerializer::readConfigXml(*mEditorConfig, stream);
	}

	//////////////////////////////////////////////////////////////////////////
	bool	EditorConfigManager::saveConfig(const TString& config)
	{
		HSTREAM stream = IResourceManager::getSingleton().openStream(config, false);
		return ConfigSerializer::writeConfigXml( *mEditorConfig, stream );
	}

	//////////////////////////////////////////////////////////////////////////
	bool	EditorConfigManager::addConfig(const HCONFIG& config)
	{
		if(config == NULL )
			return false;
		return mEditorConfig->addSubConfig(config);
	}

	//////////////////////////////////////////////////////////////////////////
	const HCONFIG&	EditorConfigManager::getConfig(const TString& name) const
	{
		return mEditorConfig->getSubConfigByName(name);
	}

	//////////////////////////////////////////////////////////////////////////
	GEOM_SPACE	EditorConfigManager::getTransformSpace() const
	{
		return (GEOM_SPACE)mData.mTransformSpace;
	}

	//////////////////////////////////////////////////////////////////////////
	void		EditorConfigManager::onConfigChange(const uint32& data)
	{
		if( &data == &mData.mTransformSpace )
		{
			//dispatch state
			TransformSpaceEvent state( (GEOM_SPACE)mData.mTransformSpace );
			IEventManager::getSingleton().dispatchEvent(state);
		}
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	const HCONFIG&	EditorConfigManager::getTransformConfig() const
	{
		return mGlobalConfig->getSubConfigByName( BTString(BLANG_GEOMETRY_TRANSFORM_SPACE) );
	}
	
}//namespace Blade