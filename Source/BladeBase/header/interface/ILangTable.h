/********************************************************************
	created:	2013/10/28
	filename: 	ILangTable.h
	author:		Crazii
	purpose:	language table
	
	language pipeline description:
		there will be 2 tools for languages: build tool, and diff-edit tool.
		build tool is used to build pure text file, from program source codes. 
		diff-edit tool is used for translators to translate new added lang strings, using the result of build tool.

		the language table contains multiple sections, each section is a string-list for different locales.
		there's a "default" section, storing built-in strings in source code.
		each string can be mapped into other sections with the same index. i.e. entry[0] in zh_CN section is the translation
		for entry[0] in en_US.

		TODO: the diff-edit tool maybe integrated into IDE studio.
*********************************************************************/
#ifndef __Blade_ILangTable_h__
#define __Blade_ILangTable_h__
#include <interface/InterfaceSingleton.h>
#include <interface/public/file/IStream.h>
#include <utility/Variant.h>
#include <utility/StringList.h>

namespace Blade
{
	//lang table class used by tools
	class ILangTable
	{
	public:
		virtual ~ILangTable()	{}

		/**
		@describe get supported language list
		@param 
		@return 
		*/
		virtual size_t	getLangs(TStringParam* outLangs = NULL) const = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual bool	addLanguage(const TString& newLang, const TString& copyFromLang) = 0;

		/**
		@describe add language strings
		@param langString: original string in default language.
		@param id-translations: language type, and translated string for original string. i.e. [zh_CN,""],[fr,""].
		@return 
		@note if langString already exist and translation is not EMTPY, the existing entry will be updated using translations
		*/
		virtual index_t	addLangString(const TString& langString, const TStringParam& id, const TStringParam& translations) = 0;

		/**
		@describe remove language string by specifying the original string
		@param 
		@return 
		*/
		virtual bool	removeLangString(const TString& langString) = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual bool	removeAll() = 0;

		/**
		@describe get string count for each lang in the table, all of them are the same as this  count.
		@param
		@return
		*/
		virtual size_t	getSize() const = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual bool	getTranslation(index_t index, ParamList& strings) const = 0;

		/**
		@describe get all translation list for a language, the array of string in size of getSize()
		@param
		@return
		*/
		virtual const TString*	getStringList(const TString& lang) const = 0;
	};

	class ILangTableManager : public InterfaceSingleton<ILangTableManager>
	{
	protected:
		BLADE_BASE_API static bool	msInited;
	public:
		virtual ~ILangTableManager()		{}
		static inline bool	isInitialized() {return msInited;}

		BLADE_BASE_API static const TString DEFAULT_LANG;
		BLADE_BASE_API static const TString EXTENSION;			//file extension (without .)

		/**
		@describe 
		@param [in] langTableFile: the language file
		@param [in] lang current app's language setting
		@return 
		@note current language cannot be changed after initialization
		*/
		virtual bool	initialize(const HSTREAM& langTableFile, const TString& lang = DEFAULT_LANG) = 0;

		/**
		@describe 
		@param
		@return 
		*/
		virtual bool	shutdown() = 0;

		/**
		@describe get translated string of current language.
		@param the orignal string in default language.
		@return 
		*/
		virtual const TString&	getLangString(const TString& orignal) = 0;

		/**
		@describe get language string by ID
		@param [in] ID: the index of target string
		@return 
		*/
		virtual const TString&	getLangString(index_t ID) const = 0;

		/**
		@describe get the language settings of the system
		@param
		@return
		*/
		virtual const TString&	getSystemLang() const = 0;

		/**
		@describe get app's current language setting: second param of initialize()
		@param 
		@return 
		*/
		virtual const TString&	getCurrentLang() const = 0;

		/**
		@describe load standalone writable lang table file - used by tools
		@param 
		@return 
		*/
		virtual ILangTable*		loadLangTable(const HSTREAM& stream, bool creatOnFailure = true) const = 0;

		/**
		@describe save lang table - used by tools
		@param 
		@return 
		*/
		virtual bool			saveLangTable(ILangTable* table, const HSTREAM& stream) const = 0;

		/**
		@describe get language list needed to be support - used by tools
		@param
		@return
		*/
		virtual const TString*	getSupportedLanuages(size_t& count) const = 0;

		/** @brief  */
		virtual  size_t	getSupportedLanuages(TStringParam& list) const = 0;

	};//class ILangTableManager

	extern template class BLADE_BASE_API Factory<ILangTableManager>;


	/** @brief designer tool helpers, design data containing text that need translations, should have one lang-map */
	class ILangMap
	{
	public:
		virtual ~ILangMap() {}
		typedef index_t LangID;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const TString&	getCurrentLang() const = 0;

		/**
		@describe get ID in LangTable, from ID in current map
		@param 
		@return 
		*/
		virtual index_t			getGlobalID(LangID ID) const = 0;

		inline const TString&	getLangString(LangID ID) const
		{
			index_t globalID = this->getGlobalID(ID);
			return ILangTableManager::getSingleton().getLangString(globalID);
		}

		/**
		@describe add or get existing lang string for this map - used by tools
		@param 
		@return 
		@note add translations to lang table, then add it to the map  using returned  index as  globalID
		*/
		virtual LangID			addLangString(const TString& langString, index_t globaID) = 0;
	};


#define BLADE_MULTILANG 1

	/*
	Note: for factory class names & data binding names, since they may be stored into resource files,
	using multi lang will break consistency, so DO NOT use BTLang for them.

	if you really need translation for them, use BXLang instead,
	BXLang is BTString() and has no effect, its just indicating the lang build tool to force adding it into translation table.
	meantile, you need hard translation BTString2Lang() for all strings that may be your string to be translated.
	(for Editor/UI module, this already been done.)
	*/

#ifndef BLADE_MULTILANG
#	define BTLang(_str) BTString(_str)
#	define BTString2Lang(_tstring) (_tstring)
#else
#	define BTLang(_str) ( ILangTableManager::isInitialized() ? ILangTableManager::getSingleton().getLangString( BTString(_str) ) : BTString(_str) )
#	define BTString2Lang(_tstring) ( ILangTableManager::isInitialized() ? ILangTableManager::getSingleton().getLangString( _tstring ) : _tstring )
#endif

#define BXLang(_str) BTString(_str)
	
}//namespace Blade

#endif //  __Blade_ILangTable_h__