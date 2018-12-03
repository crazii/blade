/********************************************************************
	created:	2009/03/09
	filename: 	IConfigManager.h
	author:		Crazii
	
	purpose:	the config manger saves the configerations for the Framework,subsystems,and application
	remark:		currently there are 2 sytle of configs:\n
				1.tree config for IConfig itnerface using XMLFile.
				2.simple config as INI using ConfigFile, called common config here
*********************************************************************/
#ifndef __Blade_IConfigManager_h__
#define __Blade_IConfigManager_h__
#include <BladeFramework.h>
#include <interface/InterfaceSingleton.h>
#include <utility/TList.h>
#include <interface/IConfig.h>
#include <utility/ConfigFile.h>
#include <interface/public/ui/IConfigDialog.h>

namespace Blade
{
	class IConfigManager : public InterfaceSingleton<IConfigManager>
	{
	public:
		virtual ~IConfigManager()	{}

		/************************************************************************/
		/* config support (tree hierarchy using XML)                                                                      */
		/************************************************************************/
		/**
		@describe 
		@note the stream should be opened in binary mode
		@param 
		@return 
		*/
		virtual bool					loadConfig(const HSTREAM& streamXml ) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void					saveConfig(const HSTREAM& streamXml ) = 0;

		/**
		@describe set the config dialog of the system
		@param [in] dialog new dialog to set
		@return old dialog pointer
		@note the dialog need to have almost a whole app life time, or need to set the dialog here to NULL bebfore delete the object
		*/
		virtual IConfigDialog*			setConfigDialog(IConfigDialog* dialog) = 0;

		/**
		@describe show config dialog for configs managed by manager
		@param 
		@return 
		*/
		virtual bool					showConfigDialog(const TString& caption = BTString("Blade Config")) = 0;

		/**
		@describe show config dialog for external configs
		@param 
		@return 
		*/
		virtual bool					showConfigDialog(const HCONFIG* configs, size_t count, const TString& caption = BTString("Blade Config")) = 0;

		/**
		@describe add IConfig object
		@param [in] config: the config object holding config info
		@return true if succeed.
		@remark this function will add the IConfig name add the object into registry \n
		* if an object of the same name exist,it will fail
		*/
		virtual bool					addConfig(const HCONFIG& config) = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual bool					removeConfig(const HCONFIG& config) = 0;

		/**
		@describe some other plugin may add configurable into config manager,those configs should be cleared before plugin uninstalling.
		@param
		@return
		*/
		virtual void					clearAllConfigs() = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void					getAllConfigs(TPointerParam<IConfig>& outList) const = 0;

		/************************************************************************/
		/* common config support (ini style)                                                                    */
		/************************************************************************/
		/**
		@brief get one section of the config
		@describe if a section with the name exist,then return a pointer to it;if not found ,return NULL
		@param [in] sectionName the name of the section specified to read
		@return a section pointer (or NULL)
		*/
		virtual const CFGFILE_SECTION*	getSection(const TString& sectionName) const = 0;

		/**
		@describe get one config entry
		@param [in] key key name
		@param [in] section section to search from
		@return value, NULL if section not exist or key not exist
		@remark param section could be NULL(""),which means default section(a section without name)
		*/
		virtual const TString&			getEntry(const TString& key,const TString& section = TString::EMPTY ) const = 0;

		/**
		@describe add a config entry
		@param [in] key		the key of the config
		@param [in] value	the value of the config
		@param [in] section	the section which the entry will be add into
		@return true if succeed
		@retval false if key already exist
		@remark if the section not exist,then the section will be added too,\n
		then add the entry to the new section
		*/
		virtual bool					addEntry(const TString& key,const TString& value,const TString& section = TString::EMPTY ) = 0;

		/**
		@describe add entry if entry not exist, or get existing entry
		@param 
		@return 
		*/
		virtual const TString&			addOrGetEntry(const TString& key ,const TString& defaultVal, const TString& section = TString::EMPTY) = 0;

		/**
		@describe set the value of a config entry
		@param [in] key		the specified key of the config to find
		@param [in] value	the value of the config to change
		@param [in] section	the section which the entry be in
		@return true if succeed
		*/
		virtual bool					setEntry(const TString& key,const TString& value,const TString& section = TString::EMPTY ) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void					loadCommonConfig(const HSTREAM& configStream) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void					saveCommonConfig(const HSTREAM& configStream) = 0;

	};//class IConfigManager


	extern template class BLADE_FRAMEWORK_API Factory<IConfigManager>;
	
}//namespace Blade


#endif // __Blade_IConfigManager_h__