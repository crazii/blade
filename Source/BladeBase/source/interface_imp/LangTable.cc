/********************************************************************
	created:	2013/10/28
	filename: 	LangTable.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "LangTable.h"
#include "platform/PlatformData_Private.h"

namespace Blade
{
	template class Factory<ILangTableManager>;

	bool ILangTableManager::msInited = false;
	const TString ILangTableManager::DEFAULT_LANG = BTString("en_US");
	const TString ILangTableManager::EXTENSION = BTString("blang");

	//////////////////////////////////////////////////////////////////////////
	LangTableManager::LangTableManager()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	LangTableManager::~LangTableManager()
	{
		this->shutdown();
	}

	//////////////////////////////////////////////////////////////////////////
	bool	LangTableManager::initialize(const HSTREAM& langTableFile, const TString& lang/* = DEFAULT_LANG*/)
	{
		if( msInited || !mCurrentLang.empty() || mLangTable.isOpen() )
		{
			assert(false);
			return false;
		}

		if(langTableFile == NULL || !langTableFile->isValid())
			return false;

		TString appLang = lang;

		//load app data (read only) for current lang
		if( appLang == DEFAULT_LANG)
		{
			msInited = false;	//don't init
			return true;
		}

		if( !mLangTable.initialize(langTableFile, false) )
			return false;

#if !BLADE_UNICODE
		//MBCS build only support current system language
		//ideally if system locale is utf-8, we can support all languages
		if( CodeConverter::GetCodePage().compare_no_case(BString("UTF-8")) != 0 )
		{
			appLang = this->getSystemLang();
			if( appLang != lang )
				BLADE_LOG(Warning, BTString("current build MBCS doesn't support all languages, "
				"change app language setting from ") + lang + BTString(" to ") + appLang);
		}
#endif

		const LangStringTable* defTable = mLangTable.getTable(DEFAULT_LANG);
		const LangStringTable* curTable = mLangTable.getTable(appLang);
		if( defTable == NULL || curTable == NULL || defTable->size() != curTable->size() )
			return false;

		//create map
		for(size_t i = 0; i < defTable->size(); ++i)
		{
			const TString& defaultString = defTable->at(i);
			const TString& langString = curTable->at(i);
			mLangMap[ defaultString ] = langString;
		}
		mCurrentLang = appLang;
		msInited = true;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	LangTableManager::shutdown()
	{
		mCurrentLang = TString::EMPTY;
		mLangTable.close();
		mLangMap.clear();	
		msInited = false;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	const TString&	LangTableManager::getLangString(const TString& orignal)
	{
		if( orignal.empty() || !msInited)
			return TString::EMPTY;

		std::pair<LocalLangMap::iterator, bool> ret = mLangMap.insert( std::make_pair(orignal, orignal) );
		return ret.first->second;
	}

	//////////////////////////////////////////////////////////////////////////
	const TString&	LangTableManager::getLangString(index_t index) const
	{
		return mLangTable.getString(index);
	}

	//////////////////////////////////////////////////////////////////////////
	const TString&	LangTableManager::getSystemLang() const
	{
		return PlatformData_Private::getSingleton().mLanguage;
	}

	//////////////////////////////////////////////////////////////////////////
	const TString&	LangTableManager::getCurrentLang() const
	{
		return mCurrentLang;
	}

	//////////////////////////////////////////////////////////////////////////
	ILangTable*		LangTableManager::loadLangTable(const HSTREAM& stream, bool creatOnFailure/* = true*/) const
	{
		LangTable* table = BLADE_NEW LangTable();
		if( !table->initialize(stream, true) )
		{
			if( !creatOnFailure )
			{
				BLADE_DELETE table;
				return NULL;
			}
			else
				table->close();
		}
		return table;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			LangTableManager::saveLangTable(ILangTable* langTable, const HSTREAM& stream) const
	{
		if( langTable == NULL || stream == NULL || !stream->isValid())
			return false;

		LangTable* table = static_cast<LangTable*>(langTable);
		return table->save(stream);
	}

	//////////////////////////////////////////////////////////////////////////
	const TString*	LangTableManager::getSupportedLanuages(size_t& count) const
	{
		count = PlatformData_Private::getSingleton().mSupportedLanguages.size();
		if( count == 0 )
			return &TString::EMPTY;
		else
			return &(PlatformData_Private::getSingleton().mSupportedLanguages[0]);
	}
	
}//namespace Blade