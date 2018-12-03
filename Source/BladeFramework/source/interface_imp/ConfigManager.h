/********************************************************************
	created:	2009/03/09
	filename: 	ConfigManager.h
	author:		Crazii
	
	purpose:	IConfigManager implementation
*********************************************************************/
#ifndef __Blade_Configmanager_h__
#define __Blade_Configmanager_h__
#include <interface/IConfigManager.h>
#include <utility/BladeContainer.h>

namespace Blade
{
	class ConfigManager : public IConfigManager ,public Singleton<ConfigManager>
	{
	public:
		//ctor & dector
		ConfigManager();
		~ConfigManager();

		/************************************************************************/
		/* IConfigManager interfaces                                                                     */
		/************************************************************************/
		/*
		@describe
		@note the stream should be opened in binary mode
		@param 
		@return 
		*/
		virtual bool			loadConfig(const HSTREAM& streamXml );

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			saveConfig(const HSTREAM& streamXml );
		
		/*
		* @describe set the config dialog of the system
		* @param [in] dialog new dialog to set
		* @return old dialog pointer
		*/
		virtual IConfigDialog*	setConfigDialog(IConfigDialog* dialog);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			showConfigDialog(const TString& caption = BTString("Blade Config"));

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			showConfigDialog(const HCONFIG* configs, size_t count, const TString& caption = BTString("Blade Config"));

		/*
		@describe add IConfig object
		@param [in] config: the config object holding config info
		@return true if succeed.
		@remark this function will add the IConfig name add the object into registry \n
		* if an object of the same name exist,it will fail
		*/
		virtual bool			addConfig(const HCONFIG& config);

		/*
		@describe
		@param
		@return
		*/
		virtual bool			removeConfig(const HCONFIG& config);

		/*
		@describe
		@param
		@return
		*/
		virtual void			clearAllConfigs();

		/**
		@describe
		@param
		@return
		*/
		virtual void			getAllConfigs(TPointerParam<IConfig>& outList) const;

		/*
		* @brief get one section of the config
		* @describe if a section with the name exist,then return a pointer to it;if not found ,return NULL
		* @param [in] sectionName the name of the section specified to read
		* @return a section pointer (or NULL)
		*/
		const CFGFILE_SECTION*	getSection(const TString& sectionName) const;

		/*
		* @describe get one config entry
		* @param [in] key key name
		* @param [in] section section to search from
		* @return value, NULL if section not exist or key not exist
		* @remark param section could be NULL(""),which means default section(a section without name)
		*/
		const TString&			getEntry(const TString& key,const TString& section = TString::EMPTY) const;

		/*
		* @describe add a config entry
		* @param [in] key		the key of the config
		* @param [in] value	the value of the config
		* @param [in] section	the section which the entry will be add into
		* @return true if succeed
		*/
		virtual bool			addEntry(const TString& key,const TString& value,const TString& section = TString::EMPTY);

		/*
		@describe add entry if entry not exist, or get existing entry
		@param 
		@return 
		*/
		virtual const TString&	addOrGetEntry(const TString& key ,const TString& defaultVal, const TString& section = TString::EMPTY);

		/*
		* @describe set the value of a config entry
		* @param [in] key		the specified key of the config to find
		* @param [in] value	the value of the config to change
		* @param [in] section	the section which the entry be in
		* @return true if succeed
		*/
		virtual bool			setEntry(const TString& key,const TString& value,const TString& section = TString::EMPTY);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			loadCommonConfig(const HSTREAM& configStream);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			saveCommonConfig(const HSTREAM& configStream);

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		IConfig*				getConfig(const TString& name) const;

	protected:

		typedef StaticTStringMap<HCONFIG>	ConfigMap;
		typedef	StaticList<HCONFIG>			StaticConfigList;

		HCONFIGSETTING		mConfigSettings;
		ConfigMap			mConfigMap;
		StaticConfigList	mConfigList;
		IConfigDialog*		mCfgDialog;

		bool				mConfigLoaded;
	};//class ConfigManager
	
}//namespace Blade

#endif // __Blade_Configmanager_h__