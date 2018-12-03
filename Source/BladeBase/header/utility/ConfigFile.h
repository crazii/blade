/********************************************************************
	created:	2009/04/06
	filename: 	ConfigFile.h
	author:		Crazii
	
	purpose:	configuration parse and save. like ".ini" file
	
	note:		like TextFile , config file is also temporary.
				open it,read data into a persistent place,then soon close it,
				the content will exist , do NOT open it for long term usage.
*********************************************************************/
#ifndef __Blade_ConfigFile_h__
#define __Blade_ConfigFile_h__
#include "String.h"
#include "TextFile.h"

namespace Blade
{
	/** @brief one config entry string pair name:value */
	typedef struct IConfigEntry
	{
		virtual ~IConfigEntry()		{}
		/** @brief  */
		virtual const TString& getName() const = 0;
		/** @brief  */
		virtual const TString& getValue() const = 0;
		/** @brief  */
		virtual void setValue(const TString& value) const = 0;
	}CFGFILE_ENTRY;

	/** @brief CFGFILE_ENTRY set,in one section */
	typedef class BLADE_BASE_API IConfigSection
	{
	public:
		virtual ~IConfigSection()		{}

		/** @brief get section name */
		virtual const TString&		getName() const = 0;

		/** @brief  */
		virtual const TString&		getValue(const TString& key) const = 0;

		/** @brief [optional] [recommended] prepare memory for key-value count, optimizing memory*/
		virtual void				prepare(size_t count) = 0;

		/** @brief  add new or set to existing */
		virtual void				addOrSetValue(const TString& key, const TString& val) = 0;
		
		/** @brief add new or get existing */
		virtual const TString&		addOrGetValue(const TString& key, const TString& val) = 0;

		/** @brief  */
		virtual bool				setValue(const TString& key, const TString& val) = 0;

		/** @brief add new */
		virtual bool				addValue(const TString& key, const TString& val) = 0;

		struct Helper : public NonAssignable
		{
		public:
			const TString& KeyRef;
			IConfigSection* Section;
			Helper(const TString& keyRef, IConfigSection* section) :KeyRef(keyRef),Section(section) {}
			inline void operator=(const TString& val) { Section->addOrSetValue(KeyRef, val); }
		};
		inline Helper				operator[](const TString& key)	{return Helper(key,this);}

		/** @brief query for modification */
		virtual const TString*		findValue(const TString& key) const = 0;

		/** @brief  */
		virtual size_t				getEntryCount() const = 0;

		/** @brief  */
		virtual const CFGFILE_ENTRY&getEntry(index_t index) const = 0;

	}CFGFILE_SECTION;//class CFGFILE_ENTRY

	/** @brief config setting ,a pair of section name and section set */
	typedef class BLADE_BASE_API IConfigSetting
	{
	public:
		virtual ~IConfigSetting()		{}

		/**
		@remark there is a default section(global section) without name.
		i.e. a config file contains only entries, no sections
		then those entries are in the default section
		*/

		/** @brief read only query */
		virtual const CFGFILE_SECTION*	getSection(const TString& SectionName) const = 0;

		/** @brief query for modification */
		virtual CFGFILE_SECTION*		getSection(const TString& SectionName) = 0;

		/** @brief  */
		virtual size_t					getSectionCount() const = 0;

		/** @brief  */
		virtual const CFGFILE_SECTION&	getSection(index_t index) const = 0;

		/** @brief  */
		virtual CFGFILE_SECTION&		getSection(index_t index) = 0;

		/** @brief insertion */
		virtual CFGFILE_SECTION&		operator[](const TString& SectionName) = 0;

		/** @brief  */
		virtual void					clear() = 0;

	}CFGFILE_SETTING;//class ConfigSetting

	typedef Handle<CFGFILE_SETTING> HCONFIGSETTING;

	class BLADE_BASE_API ConfigFile : protected TextFile
	{
	private:
		ConfigFile(const ConfigFile&);
	public:
		///ctor & dector
		ConfigFile();
		~ConfigFile();

		/**
		@describe open the config file on local disk and parse the content
		@param [in] pathname filepath of the config file
		@param [in] mode access mode of the file , may be read/write
		@param [in] seperator 
		@return true if load and parse successfully
		@exception EXC_FILE_READ:invalid config file
		@note the file is to be opened & parsed in binary file mode,that means the mode is auto shifted with the IStream::AM_BINARY flag
		*/
		bool	open(const TString& pathname,IStream::EAccessMode mode,const TString& seperator = BTString("="));


		/**
		@describe open the config file on specific file system
		@param 
		@return 
		@note the stream should be opened in binary mode, or it will fail.
		*/
		bool	open(const HSTREAM& stream,const TString& seperator = BTString("=") );

		/**
		@describe close the file
		@param none
		@return none
		*/
		void	close();

		/**
		@describe get the config file's content
		@param none
		@return all sections
		*/
		const CFGFILE_SETTING*	getAllConfigs() const;

		/**
		@describe clone the file's content for persistent usage
		@param 
		@return 
		*/
		HCONFIGSETTING			cloneConfigs() const;

		/**
		@brief get one section of the config
		@describe if a section with the name exist,then return a pointer to it;if not found ,return NULL
		@param [in] sectionName the name of the section specified to read
		@return a section pointer (or NULL)
		*/
		const CFGFILE_SECTION*	getSection(const TString& sectionName) const;


		/**
		@describe get one config entry
		@param [in] key key name
		@param [in] section section to search from
		@return value, NULL if section not exist or key not exist
		@remark param section could be NULL(""),which means default section(a section without name)
		*/
		const TString&			getEntry(const TString& key, const TString& section = TString::EMPTY) const;


		/**
		@describe save a config setting into the file content
		@param [in] content to save
		@return true if succeed
		*/
		bool	saveConfig(const IConfigSetting& content);

		/**
		@describe
		@param
		@return
		*/
		bool	parseString(const TString& content,const TString& seperator = BTString("=") );

	private:

		/**
		@describe parse the file content
		@param none
		@return true if succeed
		@note the file must be already opened
		*/
		bool	parseConfig(const TString& seperator);

		//to avoid dll-client memory layout difference ,using pointer only 8/10/2010
		IConfigSetting* mConfigSettings;
	};//class ConfigFile
	
}//namespace Blade

#endif // __Blade_ConfigFile_h__