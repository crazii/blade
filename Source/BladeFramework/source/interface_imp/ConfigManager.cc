/********************************************************************
	created:	2009/03/09
	filename: 	ConfigManager.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "ConfigManager.h"
#include <BladeFramework.h>
#include <ExceptionEx.h>
#include <ConfigSerializer.h>


namespace Blade
{
	static const TString XML_ROOT_NAME = BTString("bladeconfig");
	static const TString XML_ENTRY_NAME = BTString("configentry");

	//////////////////////////////////////////////////////////////////////////
	//ctor & dector
	ConfigManager::ConfigManager()
		:mCfgDialog(NULL)
		,mConfigLoaded(false)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	ConfigManager::~ConfigManager()
	{

	}


	/************************************************************************/
	/* IConfigManager interfaces                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool			ConfigManager::loadConfig(const HSTREAM& streamXml)
	{
		if( !mConfigLoaded )
		{
			mConfigLoaded = true;

			XmlFile xmlFile;
			if( !xmlFile.open(streamXml) )
			{
				BLADE_LOG(Error, BTString("Invalid xml file for config settings.") );
				return false;
			}

			IXmlNode* root = xmlFile.getRootNode();
			if( root == NULL || root->getName() != XML_ROOT_NAME )
			{
				BLADE_LOG(Error, BTString("Invalid config file for config settings.") );
				return false;
			}

			for( size_t i = 0;i < root->getChildCount();++i)
			{
				IXmlNode* child = root->getChild(i);
				const TString* name = child->getAttributeValue( BTString("name") );
				if( name == NULL || child->getName() != XML_ENTRY_NAME )
					continue;

				IConfig* config = this->getConfig(*name);
				if( config != NULL )
				{
					config->updateData(true,true);
					if( !ConfigSerializer::readConfigXml(*config,*child) )
						continue;
				}
			}
			return true;
		}
		else
		{
			//BLADE_EXCEPT(EXCE_STATE_ERROR,BTString("Config xml file has already been loaded."));
			BLADE_LOG(Error, BTString("Config xml file has already been loaded.") );
			return false;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void			ConfigManager::saveConfig(const HSTREAM& streamXml )
	{
		XmlFile xmlFile;
		if( !xmlFile.open(streamXml) )
			BLADE_EXCEPT(EXC_INVALID_PARAM,BTString("Invalid xml file") );

		IXmlNode* root = IXmlNode::createNode();
		root->setName( XML_ROOT_NAME );

		for(StaticConfigList::const_iterator i = mConfigList.begin(); i != mConfigList.end(); ++i)
		{
			IConfig* config = *i;
			config->updateData(true);
			IXmlNode* child = root->addChild( XML_ENTRY_NAME );
			ConfigSerializer::writeConfigXml( *config,*child );
		}

		xmlFile.saveXml(root);

		BLADE_DELETE root;
	}

	//////////////////////////////////////////////////////////////////////////
	IConfigDialog*	ConfigManager::setConfigDialog(IConfigDialog* dialog)
	{
		IConfigDialog* preDialog = mCfgDialog;

		//NULL is acceptable
		mCfgDialog = dialog;

		return preDialog;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			ConfigManager::showConfigDialog(const TString& caption/* = BTString("Blade Config")*/)
	{
		if( mCfgDialog != NULL && mConfigList.size() > 0 )
		{
			TempVector<HCONFIG> configList;
			configList.reserve( mConfigList.size() );
			for(StaticConfigList::iterator i = mConfigList.begin(); i != mConfigList.end(); ++i)
				configList.push_back(*i);

			return mCfgDialog->show( IConfigDialog::INFO(), &configList[0], configList.size(), caption );
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	ConfigManager::showConfigDialog(const HCONFIG* configs, size_t count, const TString& caption/* = BTString("Blade Config")*/)
	{
		if( mCfgDialog != NULL )
		{
			return mCfgDialog->show( IConfigDialog::INFO(), configs, count, caption );
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			ConfigManager::addConfig(const HCONFIG& config)
	{
		if( config == NULL )
			return false;

		const TString& name = config->getName();

		HCONFIG& empty_config = mConfigMap[name];

		if( empty_config != NULL )
		{
			ILog::Warning
				<< BLADE_TFUNCTION 
				<< TEXT(": add configurable failed.")
				<< TEXT("reason : name conflict - ")
				<< name
				<< ILog::endLog;
			//configurable already exist
			return false;
		}

		ILog::Information
			<< TEXT("ConfigManager: add configurable \"")
			<< name << TEXT("\"")
			<< ILog::endLog;

		empty_config = config;
		mConfigList.push_back(config);

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			ConfigManager::removeConfig(const HCONFIG& config)
	{
		if( config == NULL )
			return false;

		bool ret = mConfigMap.erase( config->getName() ) == 1;
		if( ret )
		{
			mConfigList.erase( std::find( mConfigList.begin(),mConfigList.end(),config) );
		}
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	void			ConfigManager::clearAllConfigs()
	{
		mConfigMap.clear();
		mConfigList.clear();
		//well,set unloaded.
		mConfigLoaded = false;
	}

	//////////////////////////////////////////////////////////////////////////
	void			ConfigManager::getAllConfigs(TPointerParam<IConfig>& outList) const
	{
		outList.reserve(mConfigList.size());
		for (StaticConfigList::const_iterator i = mConfigList.begin(); i != mConfigList.end(); ++i)
			outList.push_back(*i);
	}

	//////////////////////////////////////////////////////////////////////////
	const CFGFILE_SECTION*	ConfigManager::getSection(const TString& sectionName) const
	{
		return mConfigSettings->getSection(sectionName);
	}

	//////////////////////////////////////////////////////////////////////////
	const TString&			ConfigManager::getEntry(const TString& key,const TString& section/* = NULL*/) const
	{
		if( mConfigSettings == NULL )
			return TString::EMPTY;

		const CFGFILE_SECTION* psection = mConfigSettings->getSection(section);
		if( psection == NULL )
			return TString::EMPTY;
		else
			return psection->getValue(key);
	}

	//////////////////////////////////////////////////////////////////////////
	bool			ConfigManager::addEntry(const TString& key,const TString& value,const TString& section/* = NULL*/)
	{
		//check valid value
		if( value == TString::EMPTY || mConfigSettings == NULL)
			return false;

		CFGFILE_SECTION& cfgSection = (*mConfigSettings)[section];

		return cfgSection.addValue(key, value);
	}

	//////////////////////////////////////////////////////////////////////////
	const TString&	ConfigManager::addOrGetEntry(const TString& key ,const TString& defaultVal, const TString& section/* = TString::EMPTY*/)
	{
		if( defaultVal == TString::EMPTY )
			BLADE_EXCEPT(EXC_INVALID_PARAM, BTString("input value cannot be empty."));
		if(mConfigSettings == NULL)
			BLADE_EXCEPT(EXCE_STATE_ERROR, BTString("no config file loaded."));

		CFGFILE_SECTION& cfgSection = (*mConfigSettings)[section];

		return cfgSection.addOrGetValue(key, defaultVal);
	}

	//////////////////////////////////////////////////////////////////////////
	bool			ConfigManager::setEntry(const TString& key,const TString& value,const TString& section/* = NULL*/)
	{
		//check valid value
		if( value == TString::EMPTY || mConfigSettings == NULL)
			return false;


		CFGFILE_SECTION* psection = mConfigSettings->getSection(section);

		if( psection == NULL )
			//section not found
			return false;

		CFGFILE_SECTION& cfgSection = *psection;
		return cfgSection.setValue(key, value);
	}

	//////////////////////////////////////////////////////////////////////////
	void			ConfigManager::loadCommonConfig(const HSTREAM& configStream)
	{
		ConfigFile cfgFile;
		if( configStream == NULL || !(configStream->getAccesMode()&IStream::AM_READ) || !cfgFile.open(configStream) )
		{
			BLADE_EXCEPT(EXC_FILE_READ,BTString("unable to load config,ensure that the stream is opened in read mode.")  );
		}
		mConfigSettings = cfgFile.cloneConfigs();
	}

	//////////////////////////////////////////////////////////////////////////
	void			ConfigManager::saveCommonConfig(const HSTREAM& configStream)
	{
		if( mConfigSettings == NULL )
			return;

		ConfigFile cfgFile;
		if( !(configStream->getAccesMode()&IStream::AM_WRITE) || !cfgFile.open(configStream) )
			BLADE_EXCEPT(EXC_FILE_READ,BTString("unable to save config,ensure that the stream is opened in write mode.")  );

		cfgFile.saveConfig(*mConfigSettings);
	}


	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	IConfig*	ConfigManager::getConfig(const TString& name) const
	{
		ConfigMap::const_iterator i = mConfigMap.find(name);
		if( i == mConfigMap.end() )
			return NULL;
		else
			return i->second;
	}
	
}//namespace Blade