/********************************************************************
	created:	2011/08/28
	filename: 	CmdParser.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <BladeStdTypes.h>
#include <interface/IConfigManager.h>
#include <interface/IPlatformManager.h>
#include "CmdParser.h"

namespace Blade
{
	namespace ArgumentString
	{
		static const TString HELP = BTString("help");
		static const TString VERSION = BTString("version");
	}//namespace ArgumentString


	//////////////////////////////////////////////////////////////////////////
	CmdOptionImpl::CmdOptionImpl(const TString& name,const TString& desc,tchar shortcut,int val)
	{
		mName = name;
		mDesc = desc;
		mShorcut = shortcut;
		mType = CAT_INT;
		mValue.reset( val );
		mHidden = false;
	}

	//////////////////////////////////////////////////////////////////////////
	CmdOptionImpl::CmdOptionImpl(const TString& name,const TString& desc,tchar shortcut,float val)
	{
		mName = name;
		mDesc = desc;
		mShorcut = shortcut;
		mType = CAT_FLOAT;
		mValue.reset( val );
		mHidden = false;
	}

	//////////////////////////////////////////////////////////////////////////
	CmdOptionImpl::CmdOptionImpl(const TString& name,const TString& desc,tchar shortcut,bool val)
	{
		mName = name;
		mDesc = desc;
		mShorcut = shortcut;
		mType = CAT_BOOL;
		mValue.reset( val );
		mHidden = false;
	}

	//////////////////////////////////////////////////////////////////////////
	CmdOptionImpl::CmdOptionImpl(const TString& name,const TString& desc,tchar shortcut,const TString& val)
	{
		mName = name;
		mDesc = desc;
		mShorcut = shortcut;
		mType = CAT_STRING;
		mValue.reset( val );
		mHidden = false;
	}

	//////////////////////////////////////////////////////////////////////////
	CmdOptionImpl::CmdOptionImpl(const TString& name,const TString& desc,tchar shortcut,CMD_ARG_TYPE type,const TString& valString)
	{
		mName = name;
		mDesc = desc;
		mShorcut = shortcut;
		mType = type;
		mHidden = false;

		switch(type)
		{
		case CAT_INT:
			mValue.reset( (int)0 );
			break;
		case CAT_BOOL:
			mValue.reset( false );
			break;
		case CAT_FLOAT:
			mValue.reset(0.0f);
			break;
		case CAT_STRING:
			mValue.reset( TString::EMPTY );
			break;
		default:
			break;
		}
		mValue.fromTString(valString);
	}



	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	CmdParser::CmdParser()
	{
		this->CmdParser::addOption( ArgumentString::HELP,BTString("show this help information."),TEXT('h'),CAT_BOOL, BTString("false") );
		this->CmdParser::addOption( ArgumentString::VERSION,BTString("show version number."),TEXT('v'),CAT_BOOL, BTString("false") );
	}

	//////////////////////////////////////////////////////////////////////////
	CmdParser::~CmdParser()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	const CmdOption&	CmdParser::addOption(const TString& _name,const TString& desc, tchar shortcut, CMD_ARG_TYPE type, const TString& defaultVal/* = TString::EMPTY*/)
	{
		TString name = _name;
		bool hidden = false;
		if( name.size() > 0 && name[0] == TEXT('@') )
		{
			name = name.substr(1);
			hidden = true;
		}

		if( name == TString::EMPTY )
			BLADE_EXCEPT(EXC_REDUPLICATE,BTString("the name of the options must not be empty.") );

		std::pair<ShortcutMap::iterator,bool> ret = mShortcutMap.insert( std::make_pair(shortcut,name) );
		if( !ret.second )
			BLADE_EXCEPT(EXC_REDUPLICATE,BTString("the short name of the options in already in use.") );

		CmdOptionImpl option(name,desc,shortcut,type,defaultVal);
		option.mHidden = hidden;
		std::pair<OptionSet::iterator,bool> result = mCmdOptions.insert( option );
		if( !result.second )
			BLADE_EXCEPT(EXC_REDUPLICATE,BTString("the name of the options in already in use.") );

		return *(result.first);
	}

	//////////////////////////////////////////////////////////////////////////
	const CmdOption*	CmdParser::getOption(const TString& name) const
	{
		CmdOptionImpl val(name,BTString(""),TEXT(' '),0);
		OptionSet::const_iterator i = mCmdOptions.find( val );
		if( i == mCmdOptions.end() )
			return NULL;
		else
			return &( *i );
	}

	//////////////////////////////////////////////////////////////////////////
	bool	CmdParser::initialize(IAppConfig& config,int argc, char* argv[])
	{
		String argv_s;

		mCommand = StringConverter::StringToTString(argv[0]);

		StringConcat strings(String::EMPTY);
		for(int i = 1; i < argc; ++i )
		{
			if( ::strchr(argv[i], ' ') != NULL || ::strchr(argv[i], '\t') != NULL)
			{
				strings += "\"";
				strings += argv[i];
				strings += "\"";
			}
			else
				strings += argv[i];
			strings += " ";
		}

		argv_s = strings;

		TString argv_ts = StringConverter::StringToTString( argv_s );
		return this->CmdParser::initialize(config, argc-1, argv_ts);
		
	}

	//////////////////////////////////////////////////////////////////////////
	bool	CmdParser::initialize(IAppConfig& config, int argc, wchar* argv[])
	{
		WString argv_ws;
		mCommand = StringConverter::WStringToTString(argv[0]);

		WStringConcat wstrings(WString::EMPTY);
		for(int i = 1; i < argc; ++i )
		{
			wstrings += argv[i];
			wstrings += L" ";
		}

		argv_ws = wstrings;

		TString argv_ts = StringConverter::WStringToTString( argv_ws );
		return this->CmdParser::initialize(config, argc-1, argv_ts);
	}

	//////////////////////////////////////////////////////////////////////////
	bool	CmdParser::initialize(IAppConfig& config, int /*argc*/, const TString& argv)
	{
		std::tostream& output = std::tcout;

		//add arguments
		config.setupAppCmdArgument();

		TStringTokenizer options;
		options.tokenize(argv,TEXT("\"\""),TEXT(" ") );

		for(size_t i = 0; i < options.size(); ++i)
		{
			//treat it as input list
			if( options[i][0] != TEXT('-') )
			{
				mFileList.push_back(options[i]);
				continue;
			}

			bool bFullName = false;

			//get the option item name
			TString option = options[i].substr_nocopy(1);

			if( option.size() > 0 && option[0] == TEXT('-') )
			{
				option = options[i].substr_nocopy(2);
				bFullName = true;
			}

			if(options.size() == 0)
			{
				output << TEXT("missing option name.") << std::endl;
				continue;
			}

			index_t valuePos = option.find_first_of(BTString("="));
			TString value = valuePos == INVALID_INDEX ? TString::EMPTY : option.substr(valuePos+1);

			const CmdOptionImpl* cmdOption = NULL;

			tchar shortcut = 0;
			const TString& name = option.substr_nocopy(0, valuePos);
			if( !bFullName )
			{
				if(name.size() != 1)
				{
					TString realoption = option.substr(0);
					output << TEXT("invalid option - '") << realoption.c_str() << TEXT("'.") << std::endl;
					continue;
				}

				shortcut = name[0];
				ShortcutMap::iterator iter = mShortcutMap.find( shortcut );
				if( iter != mShortcutMap.end() )
					cmdOption = this->getOptionImpl(iter->second);
			}
			else
				cmdOption = this->getOptionImpl(name);

			if( cmdOption == NULL )
			{
				TString realoption = option.substr(0);
				output << TEXT("option not supported: '") << realoption.c_str() << TEXT("'.") << std::endl;
				continue;
			}

			//name is OK, parsing value
			size_t prefixCount = TStringHelper::countSub(value, BTString("-"), 2);
			if( value.empty() )	//bool option without value: treat as true
			{
				if( cmdOption->getType() == CAT_BOOL )
					cmdOption->getValueRef().reset(true);
				else
				{
					TString realoption = option.substr(0);
					output << TEXT("missing parameter: ") << realoption.c_str() << TEXT(".") << std::endl;
					continue;
				}
			}
			else if( value.find(BTString("=")) == TString::npos
				&& ( cmdOption->getType() != CAT_BOOL || prefixCount == 0 )
				&& ( (cmdOption->getType() != CAT_INT && cmdOption->getType() != CAT_FLOAT) || prefixCount == 0 || (prefixCount == 1 && value[0] == TEXT('-')) )
				)
			{
				if( cmdOption->getType() == CAT_BOOL )
				{
					if( value == BTString("true") )
						cmdOption->getValueRef().fromTString( value );
					else if( value == BTString("false") )
						cmdOption->getValueRef().fromTString( value );
					else
					{
						TString realoption = option.substr(0);
						output << TEXT("invalid parameter: ") << realoption.c_str()
							<< TEXT("\".the valid value should be 'true' or 'false' ") << std::endl;
					}
				}
				else if( cmdOption->getType() == CAT_INT || cmdOption->getType() == CAT_FLOAT )
					cmdOption->getValueRef().fromTString( value );
				else 
				{
					assert(cmdOption->getType() == CAT_STRING );
					cmdOption->getValueRef().fromTString( value );
				}
			}
			else
			{
				TString realoption = option.substr(0);
				output << TEXT("invalid option: ") << realoption.c_str() << TEXT(".") << std::endl;
			}

		}//for

		const CmdOptionImpl* help = this->getOptionImpl(ArgumentString::HELP);
		const CmdOptionImpl* version = this->getOptionImpl(ArgumentString::VERSION);

		if( help->getValue() == true || version->getValue() == true )
		{
			this->showInformation(config);
			return false;
		}
		//else if( mFileList.size() == 0 )
		//	output << TEXT("not input file found, use --help for more information.") << std::endl;

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void			CmdParser::showInformation(IAppConfig& cfg, bool force/* = false*/)
	{
		const CmdOptionImpl* help = this->getOptionImpl(ArgumentString::HELP);
		const CmdOptionImpl* version = this->getOptionImpl(ArgumentString::VERSION);

		std::tostream& output = std::tcout; 

		if( version->getValue() == true )
		{
			output << cfg.getAppName().c_str()
				<< cfg.getVersion().getVersionString().c_str()
				<< std::endl
				<< cfg.getDescription().c_str()
				<< std::endl;
		}

		if( help->getValue() == false && force == false)
			return;

		output << TEXT("available arguments:") << std::endl;

		for(OptionSet::const_iterator i = mCmdOptions.begin(); i != mCmdOptions.end(); ++i)
		{
			const CmdOptionImpl& option = *i;

			if( option.mHidden )
				continue;

			TString formal;
			switch( option.getType() )
			{
			case CAT_INT:
				formal = BTString("=<IntVal>");
				break;
			case CAT_BOOL:
				formal = BTString("[=true|false]");
				break;
			case CAT_FLOAT:
				formal = BTString("=<FloatVal>");
				break;
			case CAT_STRING:
				formal = BTString("=<StringVal>");
				break;
			}

			output << TEXT("--") << option.getName().c_str() << formal.c_str() << TEXT(",-") <<option.getShortcut() << formal.c_str() << std::endl;

			output << TEXT("\t") << option.getDesc().c_str() << std::endl;
			output << TEXT("\tdefault value:") << option.getValue().toTString().c_str() << std::endl << std::endl;
		}

	}


	//////////////////////////////////////////////////////////////////////////
	const CmdOptionImpl*	CmdParser::getOptionImpl(const TString& name)
	{
		CmdOptionImpl val(name,BTString(""),TEXT(' '),0);

		OptionSet::iterator i = mCmdOptions.find( val );
		if( i == mCmdOptions.end() )
			return NULL;
		else
			return &( *i );
	}

}//namespace Blade
