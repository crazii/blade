/********************************************************************
	created:	2013/10/30
	filename: 	LangBuildApplication.cc
	author:		Crazii
	purpose:	BladeLangBuild will scan all C/C++ files specified,
				and looking for string in BTLANG("") format, and save them.
				TODO: use pre-processing to exactly get the raw string.
*********************************************************************/
#include <interface/IPlatformManager.h>
#include <interface/IResourceManager.h>
#include <utility/TextFile.h>
#include <utility/StringHelper.h>
#include <interface/ILog.h>
#include <utility/BladeContainer.h>
#include <LangBuildApplication.h>

#define ENABLE_DUPLICATE_WARNING	1
#define ENABLE_DUPLICATE_WARNING_RC	0

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(disable:4640) //construction of local static object is not thread-safe
#endif

namespace Blade
{
	//file base to file map, sort in alphabet order
	struct FileInfo
	{
		HFILEDEVICE	folder;
		TString		content;
		TextFile*	file;
	};

	struct LineInfo
	{
		TString file;
		int		line;
		bool	isMacro;
	};

	struct MacroInfo
	{
		LineInfo	lineInfo;
		TString		macro;
	};

	typedef TempMap<TString, FileInfo> FileMap;
	typedef TempMap<TString, MacroInfo> MacroMap;
	//typedef TempTStringSet LangStringList;
	typedef TempMap<TString, LineInfo, FnTStringNCLess> LangStringList;	//alphabet order: ignore case

	//////////////////////////////////////////////////////////////////////////
	//treat a string as lang string, only if it contains at least one of alphabet chars
	static bool isValidLangString(const TString& str)
	{
		if( str.empty() )
			return false;
		bool isAlpha = false;
		for(size_t i = 0; i < str.size() && !isAlpha; ++i)
		{
			if( i < str.size()-1 )
			{
				if( str[i] == TEXT('\\') && 
					(str[i+1] == TEXT('n') || str[i+1] == TEXT('r') || str[i+1] == TEXT('t') || str[i+1] == TEXT('\'') || str[i+1] == TEXT('\"') ) || str[i+1] == TEXT('x') )
				{
					++i;
					continue;
				}
			}
			isAlpha = std::isalpha(str[i]) != 0;
		}
		return isAlpha;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	LangBuildApplication::run(bool /*bModal*/)
	{
		const TStringList& folders = ICmdParser::getSingleton().getFileList();
		const TString& lang = ICmdParser::getSingleton().getOption(BTString("lang"))->getValue();
		const TString& output = ICmdParser::getSingleton().getOption(BTString("output"))->getValue();
		const TString& skip = ICmdParser::getSingleton().getOption(BTString("skip"))->getValue();

		HFILEDEVICE outputFolder = IResourceManager::getSingleton().loadFolder(output);
		if( outputFolder == NULL )
		{
			BLADE_LOGU(ToolLog::Error, BTString("unable to open output folder."));
			return false;
		}

		TStringList skippingFolders;
		{
			TStringTokenizer tokennizer;
			tokennizer.tokenize(skip, TEXT(";"));
			skippingFolders.reserve(tokennizer.size());
			for(size_t i = 0; i < tokennizer.size(); ++i)
			{
				if( tokennizer[i].empty() )
					continue;
				const TString subSkip = BTString("/") + tokennizer[i] + BTString("/");
				skippingFolders.push_back(subSkip);
			}
		}
		
		//.rc files for win32
		FileMap RCfiles;

		//find all source files
		//note: treat all command line parameters as folders
		FileMap files;
		for(size_t i = 0; i < folders.size(); ++i)
		{
			HFILEDEVICE hFolder = IResourceManager::getSingleton().loadFolder(folders[i]);
			if( hFolder == NULL )
			{
				BLADE_LOGU(ToolLog::Error, BTString("unable to load folder '") << folders[i] << BTString("'."));
				continue;
			}

			TStringParam fileList;
			hFolder->findFile(fileList, BTString("*.cxx"), IFileDevice::FF_FILE|IFileDevice::FF_RECURSIVE);
			hFolder->findFile(fileList, BTString("*.cpp"), IFileDevice::FF_FILE|IFileDevice::FF_RECURSIVE);
			hFolder->findFile(fileList, BTString("*.cc"), IFileDevice::FF_FILE|IFileDevice::FF_RECURSIVE);
			hFolder->findFile(fileList, BTString("*.h"), IFileDevice::FF_FILE|IFileDevice::FF_RECURSIVE);
			hFolder->findFile(fileList, BTString("*.hpp"), IFileDevice::FF_FILE|IFileDevice::FF_RECURSIVE);

			hFolder->findFile(fileList, BTString("*.rc"), IFileDevice::FF_FILE|IFileDevice::FF_RECURSIVE);
			hFolder->findFile(fileList, BTString("*.rc2"), IFileDevice::FF_FILE|IFileDevice::FF_RECURSIVE);

			for(size_t j = 0; j < fileList.size(); ++j)
			{
				TString filePath = (TString)fileList[j];
				//skip some folder like backup/
				bool doSkip = false;
				for(size_t k = 0; k < skippingFolders.size(); ++k )
				{
					if( (doSkip=(filePath.find(skippingFolders[k]) != TString::npos)) == true )
						break;
				}
				if( doSkip )
					continue;

				TextFile* file = BLADE_NEW TextFile();
				HSTREAM stream = hFolder->openFile(filePath);
				if( !file->open( stream ) )
				{
					TString fileFolder, fileName, fileBase, fileExt;
					TStringHelper::getFilePathInfo(filePath, fileFolder, fileName, fileBase, fileExt);

					BLADE_LOGU(ToolLog::Error, BTString("unable to open source file '") << fileName << TEXT("'."));
					BLADE_DELETE file;
					continue;
				}

				FileInfo info = {hFolder, TString::EMPTY, file};
				//note: this tools may be NOT used in the same project of the source files, we need to use full path
				//so that visual studio can trace them.
				hFolder->getFullPath(filePath, filePath);
				filePath = TStringHelper::standardizePath(filePath);
#if BLADE_IS_WINDOWS_CLASS_SYSTEM
				TStringHelper::replace(filePath, BTString("/"), BTString("\\"));
#endif
				if( TStringHelper::isEndWith(filePath, BTString(".rc")) || TStringHelper::isEndWith(filePath, BTString(".rc2")) )
					RCfiles[filePath] = info;
				else
					files[filePath] = info;
			}
		}

		//a valid macro name must contains
		static const TString VALID_NAME_CHARS = TEXT("ABCDEFGHJIKLMNOPQRSTUVWXYZabcdefghjiklmnopqrstuvwxyz0123456789_");
		static const TString DEFINE = BTString("define");

		//Note: in case of BTLANG(MACRO), 
		//we need to scan all MACROs defined with raw string ""

		//TODO: there maybe macros with same name in different files, we need to process include for each C++ file and parse
		//but this also need pre-processing for conditional including
		MacroMap macroStringMap;
		MacroMap macroMacroMap;	//#define MACROA	MACROB
		for(FileMap::iterator iter = files.begin(); iter != files.end(); ++iter)
		{
			const HFILEDEVICE hFolder = iter->second.folder;
			TextFile* file = iter->second.file;
			TString content = file->getText();
			TStringTokenizer tokenizer;
			tokenizer.splitLines(content);
			tokenizer.removeComments();

			for(size_t i = 0; i < tokenizer.size(); ++i)
			{
				const  TString& line = tokenizer[i];
				if( line.empty() )
					continue;

				//find the '#'
				//SKIP: "#
				index_t defIndex = line.find_first_of(TEXT("#"));

				//" \
				//#"
				if( defIndex == INVALID_INDEX ||
					!TStringHelper::trim(line.substr_nocopy(0, defIndex)).empty() 
					|| i > 0 && TStringHelper::isEndWith(tokenizer[i-1], BTString("\\")) )
					continue;

				TString def = line.substr_nocopy(defIndex+1);
				index_t keyword = def.find_first_not_of(TEXT(" \t"));
				if( keyword != INVALID_INDEX )
					def = def.substr_nocopy(keyword);

				if( !TStringHelper::isStartWith(def, DEFINE) )
					continue;

				def = def.substr_nocopy(DEFINE.size());
				index_t nameIndex = def.find_first_not_of(TEXT(" \t"));
//	#	define			ABCDE			"ABCDE"
				if( nameIndex == INVALID_INDEX )
					continue;

				index_t nameEnd = def.find_first_of(TEXT(" \t("), nameIndex+1);
				if( nameEnd != INVALID_INDEX )
				{
					--nameEnd;
					TString name = def.substr(nameIndex, nameEnd-nameIndex+1);
					if( def[nameEnd+1] == TEXT('(') ) //CAREFUL! def[] makes def a non-const reference
					{
						nameEnd = def.find_first_of(TEXT(')'), nameEnd+1);
						if( nameEnd == INVALID_INDEX )
							continue;
					}
					def = TStringHelper::trim( def.substr_nocopy(nameEnd+1) );
					if( def.size() > 3 && def[0] == TEXT('\"') && def[def.size()-1] == TEXT('\"') )
					{
						TString value = TStringHelper::trim( def.substr_nocopy(1, def.size()-2) );
						if( !value.empty() )
						{
							MacroInfo& oldVal  = macroStringMap[name];
							if( oldVal.macro != TString::EMPTY )
							{
								BLADE_LOGU(ToolLog::Error, ILog::noPrefix << iter->first + BTString("(") + TStringHelper::fromUInt(i+1u) + BTString("):")
									+ BTString(" same macro with different definition.\n")
									+ oldVal.lineInfo.file + BTString("(") + TStringHelper::fromInt(oldVal.lineInfo.line) + BTString("):") 
									+ BTString(" see previous definition."));
								continue;
							}
							oldVal.macro = value;
							oldVal.lineInfo.file = iter->first;
							oldVal.lineInfo.line = (int)i + 1;
						}
					}
					else if( def.size() > 0 && def[0] < TEXT('0') && def[0] > TEXT('9') 
						&& def.find_first_not_of(VALID_NAME_CHARS) == INVALID_INDEX )	//a valid macro name
					{
						MacroInfo& oldVal  = macroStringMap[name];
						if( oldVal.macro != TString::EMPTY )
						{
							BLADE_LOGU(ToolLog::Error, ILog::noPrefix << iter->first + BTString("(") + TStringHelper::fromUInt(i+1u) + BTString("):")
								+ BTString(" same macro with different definition.\n")
								+ oldVal.lineInfo.file + BTString("(") + TStringHelper::fromInt(oldVal.lineInfo.line) + BTString("):") 
								+ BTString(" see previous definition."));
							continue;
						}
						oldVal.macro = def;
						oldVal.lineInfo.file = iter->first;
						oldVal.lineInfo.line = (int)i + 1;
					}
				}
			}

			//close file & fill parsed content
			iter->second.content = TStringConcat(file->getText());
			iter->second.file->close();
			BLADE_DELETE iter->second.file;
			iter->second.file = NULL;
		}//for each file

		//replace all macros to string
		//trace nesting macros to the initial value
		for(MacroMap::const_iterator i = macroMacroMap.begin(); i != macroMacroMap.end(); ++i)
		{
			const TString& name = i->first;
			const TString& macro = i->second.macro;
			const TString* key = &macro;
			MacroMap::const_iterator f = macroMacroMap.find(*key);

			//trace macro definition to the end
			while( f != macroMacroMap.end() )
			{
				key = &(f->second).macro;
				f = macroMacroMap.find(*key);
			}

			//check if the macro defined in string
			f = macroStringMap.find( *key );
			if( f != macroStringMap.end() )
				macroMacroMap[ name ] = f->second;
		}

		//find BTLang("") or BTLang(MACRO)
		LangStringList langStringSet;

		//BTLang
		//(
		//	"text"
		//	);
		//definition break across lines
		enum EIncompleteStat
		{
			IS_NONE,
			IS_LBRACKET,	//(
			IS_STRING,		//"xxx"
			IS_RBRACKET,	//)
		};

		for(FileMap::iterator i = files.begin(); i != files.end(); ++i)
		{
			static const TString KEY_WORD = BTString("BTLang");
			static const TString FORCE_WORD = BTString("BXLang");

			TStringTokenizer lines;
			lines.splitLines(TString(i->second.content.c_str(), i->second.content.size(), TString::CONST_HINT));
			lines.removeComments();

			int incompleteStatus = IS_NONE;
			TString langString;
			for(index_t lineNO = 0; lineNO < lines.size(); ++lineNO)
			{
				const TString line = TStringHelper::trim(lines[lineNO]);
				if( line.empty() )
					continue;

				index_t next = std::min( line.find(KEY_WORD), line.find(FORCE_WORD) );
				//there maybe multiple strings within one line
				while( next != TString::npos || incompleteStatus != IS_NONE )
				{
					//handle broken definition across lines
					if( incompleteStatus != IS_NONE )
					{
						bool testNextLine = false;
						if( incompleteStatus == IS_LBRACKET )
						{
							if( line[0] != TEXT('(') )
							{
								assert(false);	//probably coding error
								incompleteStatus = IS_NONE;
								langString = TString::EMPTY;
								continue;
							}
							incompleteStatus = IS_STRING;
							testNextLine = true;
						}

						index_t quotation = 0;
						if( incompleteStatus == IS_STRING )
						{
							index_t firstQuotation = line.find(TEXT('"'));

							if( firstQuotation != TString::npos )
								quotation = line.find(TEXT('"'), firstQuotation+1);

							//there must be two '"'
							if( line.size() < 3 || firstQuotation != TString::npos && quotation == TString::npos )
							{
								if( testNextLine && firstQuotation == TString::npos)	//can not break "" across lines.
									break;

								assert(false);	//probably coding error
								incompleteStatus = IS_NONE;
								langString = TString::EMPTY;
								continue;
							}
							if(firstQuotation != TString::npos)
							{
								assert( quotation != TString::npos );
								langString = line.substr(firstQuotation, quotation-firstQuotation+1);
							}
							else
							{
								index_t rightBracket = line.find(TEXT(')'), quotation);
								rightBracket = (rightBracket == TString::npos ? line.size()-1 : rightBracket);
								langString = line.substr(firstQuotation, rightBracket-firstQuotation+1);
							}
							incompleteStatus = IS_RBRACKET;
							testNextLine = true;
						}

						if( incompleteStatus == IS_RBRACKET )	//"")
						{
							assert(langString != TString::EMPTY);
							if( line.find(TEXT(')'), quotation) == TString::npos )
							{
								if( testNextLine )
									break;

								assert(false);	//probably coding error
								incompleteStatus = IS_NONE;
								langString = TString::EMPTY;
								continue;
							}
							incompleteStatus = IS_NONE;
						}

						assert( incompleteStatus == IS_NONE );
						assert(langString != TString::EMPTY);
					}
					else
					{
						langString = TString::EMPTY;
						//fresh new start
						const TString prefix = line.substr_nocopy(0, next);
						//move to next BTLang
						//Important: content is read only text using shared buffer. use const to prevent allocations
						const TString text = line.substr_nocopy(next + KEY_WORD.size());
						index_t nextFind = std::min( text.find(KEY_WORD), text.find(FORCE_WORD) );
						next = (nextFind != TString::npos) ? next + KEY_WORD.size() + nextFind : nextFind;

						//check current BTLang
						//any prefix to form another word? i.e. ABTLang is not valid.
						if( prefix.size() > 0 && VALID_NAME_CHARS.find( prefix[prefix.size()-1]) != INVALID_INDEX )
							continue;

						//original definition:
						//#		define BTLang(xxx) 
						//this is acceptable:
						//# define XXX BTLang("STR")
						//we don't know if all BTLang("") are used, like XXX above, we just record all occurrences.
						//BTLang in the conditional block is also recorded.
						//#if 0
						// const TString str = BTLang("");
						//#endif
						//note: if BTLang() appears in a string - " BTLang(\"\") ", it's probably generating codes, so we also need to record it.
						if( prefix.size() > 0 && prefix[0] == TEXT('#') )
						{
							index_t keywordIndex = prefix.find_first_not_of(TEXT(" \t"), 1);
							if( keywordIndex != INVALID_INDEX )
							{
								TString keyword = prefix.substr_nocopy(keywordIndex);
								if( TStringHelper::isStartWith(keyword, DEFINE) && keyword.find_first_not_of(TEXT(" \t"), DEFINE.size()) == INVALID_INDEX )
									continue;
							} 
						}

						//BTLang ( xx );
						index_t leftBracket = text.find_first_not_of(TEXT(" \t"));
						if( leftBracket == INVALID_INDEX || text[leftBracket] != TEXT('(') )
						{
							incompleteStatus = IS_LBRACKET;
							break;
						}
						index_t quotation = text.find(TEXT('"'), leftBracket+1);
						if( quotation == INVALID_INDEX )
							incompleteStatus = IS_STRING;
						else
						{
							assert(incompleteStatus == IS_NONE);
							//cannot break "" across lines.
							quotation = text.find(TEXT('"'), quotation+1);
							if( quotation == INVALID_INDEX )
								break;
						}

						index_t rightBracket = text.find(TEXT(')'), leftBracket+1);
						if( rightBracket == INVALID_INDEX )
						{
							assert(incompleteStatus == IS_STRING || incompleteStatus == IS_NONE);
							if(incompleteStatus == IS_NONE)
								incompleteStatus = IS_RBRACKET;
							break;
						}
						else if( quotation != INVALID_INDEX )
						{
							//("" "" ) ? need support this?
							quotation = text.find(TEXT('"'), quotation+1);
							if( quotation < rightBracket )
								break;
						}

						incompleteStatus = IS_NONE;
						langString = text.substr_nocopy(leftBracket+1, rightBracket-leftBracket+1-2);
						langString = TStringHelper::trim(langString);
						if( langString.empty() )
							continue;
					}


					bool isMacro = false;

					//find a string BTLang("")
					if( langString.size() > 2 && langString[0] == TEXT('"') && langString[langString.size()-1] == TEXT('"') )
					{
						langString = TStringHelper::trim( langString.substr_nocopy(1, langString.size()-2) );
						if( langString.empty() )
							continue;
					}
					// " BTLang(\"\") "
					//else if( langString.size() > 4 && TStringHelper::isStartWith(langString, BTString("\\\"")) && TStringHelper::isEndWith(langString, BTString("\\\"")) )
					//{
					//	langString = TStringHelper::trim( langString.substr_nocopy(2, param.size()-4) );
					//	if( langString.empty() )
					//		continue;
					//}
					else
					{
						//macro BTLang(MACRO)
						isMacro = true;
						if( langString.find_first_not_of(VALID_NAME_CHARS) != INVALID_INDEX )
							assert(false);

						MacroMap::const_iterator f = macroStringMap.find(langString);
						if( f != macroStringMap.end() )
							langString = f->second.macro;
						else
						{
							BLADE_LOGU(ToolLog::Error, BTString("\n") + i->first + BTString("(") + TStringHelper::fromUInt(lineNO+1u) +
								BTString("): macro definition not found:") + langString);
							continue;
						}
					}

					//add into string set
					if( Blade::isValidLangString(langString) )
					{
						LineInfo info = {i->first, (int)lineNO+1, isMacro};	//zero based to one based
						std::pair<LangStringList::iterator, bool> ret = langStringSet.insert( std::make_pair(langString,info) );
						if( !ret.second )
						{
#if ENABLE_DUPLICATE_WARNING
							//don't warn on same macro: reduplicate macros are reported already.
							if( !(info.isMacro && ret.first->second.isMacro ) )
							{
								const TString& prevFile = ret.first->second.file;
								index_t prevLine = (index_t)ret.first->second.line;

								//HACK: if files are within the same project/module, the strings are probably have the same meaning/translation,
								//so we don't report it.
								TString dir0, dir1, tmp;
								TStringHelper::getFilePathInfo(prevFile, dir0, tmp);
								TStringHelper::getFilePathInfo(info.file, dir1, tmp);
								tmp = TStringHelper::standardizePath(TStringHelper::getRelativePath(dir0, dir1));
								if( TStringHelper::countSub(tmp, BTString("/"),3) <= 2 )
									continue;

								BLADE_LOGU(ToolLog::Error, ILog::noPrefix << info.file + BTString("(") + TStringHelper::fromInt(info.line) + BTString(")")
									+ BTString(": reduplicated language string: definition already exist: \"") + langString + BTString("\"\n")
									+ prevFile + BTString("(") + TStringHelper::fromUInt(prevLine) + BTString(")")
									+ BTString(": see previous definition.") );
							}
#endif
						}
					}

					langString = TString::EMPTY;
				}//scanning BTLang ended

			}//for each line
		}//for each file

		const TString RESOURCE_SECTIONS[] = 
		{
			BTString("DIALOGEX"),
			BTString("STRINGTABLE"),

			TString::EMPTY
		};

		//parse .rc files for Win32 visual studio projects
		for(FileMap::iterator i = RCfiles.begin(); i != RCfiles.end(); ++i)
		{
			TextFile* file = i->second.file;
			const TString content = file->getText();
			TStringTokenizer tokenizer;
			tokenizer.tokenize(content, TEXT("\n\r"));

			bool isValidSection = false;
			bool isStarted = false;
			for(size_t j = 0; j < tokenizer.size(); ++j)
			{
				const TString& line = tokenizer[j];
				TStringTokenizer words;
				words.tokenize(line, TEXT("\"\""), TEXT(" \t"));

				for(size_t k = 0; !RESOURCE_SECTIONS[k].empty(); ++k)
				{
					if( words.find( RESOURCE_SECTIONS[k] ) != INVALID_INDEX )
					{
						assert(!isValidSection);
						isValidSection = true;
						break;
					}
				}
				if( !isValidSection )
					continue;

				if( TStringHelper::trim(line) == BTString("BEGIN") )
				{
					isStarted = true;
					continue;
				}
				else if( TStringHelper::trim(line) == BTString("END") )
				{
					isValidSection = false;
					isStarted = false;
					continue;
				}

				if( words.size() < 2 || !words.isDoubleToken(1) )
					continue;

				const TString text = words[1];
				//handle escape sequences like \n\r\t
				if( !Blade::isValidLangString(text) )
					continue;

				//special case: word[0] = CAPTION for DIALOGEX
				if( isStarted || isValidSection && (words[0] == BTString("CAPTION")))
				{
					LineInfo info = {i->first, (int)j+1, false};	//zero  based to one based
					std::pair<LangStringList::iterator, bool> ret = langStringSet.insert( std::make_pair(text,info) );
					if( !ret.second )
					{
#if ENABLE_DUPLICATE_WARNING_RC
						BLADE_LOG(ToolLog::Warning, ILog::noPrefix << info.file + BTString("(") + TStringHelper::fromInt(info.line) + BTString(")")
						+ BTString(": reduplicated language string: definition already exist: \"") + text + BTString("\"\n")
						+ ret.first->second.file + BTString("(") + TStringHelper::fromInt(ret.first->second.line) + BTString(")")
						+ BTString(": see previous definition.") );
#endif
					}
				}
			}//for each line

			BLADE_DELETE i->second.file;
			i->second.file = NULL;
		}//for each file

		//final output
		HSTREAM outStream = outputFolder->createFile(lang, IStream::AM_OVERWRITE);
		TextFile outText;
		if( !outText.open(outStream) )
		{
			BLADE_LOGU(ToolLog::Error, BTString("unable to access output file:") + output + BTString("/") + lang );
			return false;
		}
		//better with BOM
		outText.setDestEncoding(TE_UTF8, true);

		outText.write( BTString("[") + ILangTableManager::DEFAULT_LANG + BTString("]\n") );
		for(LangStringList::iterator i = langStringSet.begin(); i != langStringSet.end(); ++i)
			outText.write( i->first + BTString("\n") );

		outText.close();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	LangBuildApplication::doInitialize()
	{
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void	LangBuildApplication::initPlugins()
	{
		ParamList params;
		IPlatformManager::getSingleton().initialize(params);
	}
	
}//namespace Blade