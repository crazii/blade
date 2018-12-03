/********************************************************************
	created:	2010/09/02
	filename: 	ShaderResourceSerializer.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <interface/public/graphics/ShaderInfoTable.h>
#include <interface/public/graphics/IGraphicsResourceManager.h>
#include <interface/IResourceManager.h>
#include <interface/IGraphicsSystem.h>
#include "ShaderKeywords.h"
#include "ShaderResourceSerializer.h"

#define BUILD_SAMPLER_SEMANTIC 0
#define BUILD_DEFAULT_VARIABLE 1	

#if BUILD_SAMPLER_SEMANTIC || BUILD_DEFAULT_VARIABLE
#	include "../../SubShaderImpl.h"
#endif

namespace Blade
{
	namespace Impl
	{
		static bool checkBuiltinSemantic(const TString& semantic, const TString& builtin, bool _case)
		{
			if (TStringHelper::isStartWith(semantic, builtin, _case))
			{
				TString suffix = semantic.substr_nocopy(builtin.size());
				if (suffix.empty() || (suffix[0] >= TEXT('0') && suffix[0] <= TEXT('9')) )
					return true;
			}
			return false;
		}

		static bool isBultinSemantic(const TString& semantic)
		{
			return checkBuiltinSemantic(semantic, BTString("TEXCOORD"), true)
				|| checkBuiltinSemantic(semantic, BTString("POSITION"), true)
				|| checkBuiltinSemantic(semantic, BTString("DEPTH"), true)
				|| checkBuiltinSemantic(semantic, BTString("COLOR"), true)
				|| checkBuiltinSemantic(semantic, BTString("NORMAL"), true)
				|| checkBuiltinSemantic(semantic, BTString("TANGENT"), true)
				|| checkBuiltinSemantic(semantic, BTString("BINORMAL"), true)
				|| checkBuiltinSemantic(semantic, BTString("VFACE"), true)
				|| checkBuiltinSemantic(semantic, BTString("VPOS"), true)
				|| checkBuiltinSemantic(semantic, BTString("BLENDINDICES"), true)
				|| checkBuiltinSemantic(semantic, BTString("BLENDWEIGHT"), true)
				|| checkBuiltinSemantic(semantic, BTString("POSITIONT"), true)
				|| checkBuiltinSemantic(semantic, BTString("PSIZE"), true)
				|| checkBuiltinSemantic(semantic, BTString("FOG"), true)
				|| checkBuiltinSemantic(semantic, BTString("TESSFACTOR"), true)
				|| checkBuiltinSemantic(semantic, BTString("SV_Position"), false)
				|| checkBuiltinSemantic(semantic, BTString("SV_Target"), false)
				|| checkBuiltinSemantic(semantic, BTString("SV_Depth"), false)
				|| checkBuiltinSemantic(semantic, BTString("SV_VertexID"), false);
		}
		
	}//namespace Impl

	//////////////////////////////////////////////////////////////////////////
	ShaderResourceSerializer::StringToUsageMap	ShaderResourceSerializer::msStringToUsage;
	ShaderResourceSerializer::ValidProfileList	ShaderResourceSerializer::msValidProfiles;
	StaticLock									ShaderResourceSerializer::msInitLock;

	//////////////////////////////////////////////////////////////////////////
	EShaderVariableUsage	ShaderResourceSerializer::getUsage(const TString& usageDef)
	{
		assert( msStringToUsage.size() != 0 );	//must already inited
		StringToUsageMap::const_iterator i = msStringToUsage.find(usageDef);
		if( i != msStringToUsage.end() )
			return i->second;
		else
			return SVU_COUNT;
	}

	//////////////////////////////////////////////////////////////////////////
	ShaderResourceSerializer::ShaderResourceSerializer()
	{
		//fast spin lock
		ScopedLock lock(msInitLock);

		if( msStringToUsage.size() == 0 )
		{
			msStringToUsage[BTString("_GLOBAL_")] = SVU_GLOBAL_CUSTOM;
			msStringToUsage[BTString("_SHADER_")] = SVU_SHADER_CUSTOM;
			msStringToUsage[BTString("_PASS_")] = SVU_PASS_CUSTOM;
			msStringToUsage[BTString("_INSTANCE_")] = SVU_INSTANCE_CUSTOM;
		}
		if( msValidProfiles.size() == 0 )
		{
			msValidProfiles.insert(BTString("2_0"));
			msValidProfiles.insert(BTString("2_0"));
			msValidProfiles.insert(BTString("2_a"));
			msValidProfiles.insert(BTString("2_b"));
			msValidProfiles.insert(BTString("3_0"));
			msValidProfiles.insert(BTString("4_0"));
			msValidProfiles.insert(BTString("4_1"));
			msValidProfiles.insert(BTString("5_0"));
		}
	}

	//////////////////////////////////////////////////////////////////////////
	ShaderResourceSerializer::~ShaderResourceSerializer()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	bool	ShaderResourceSerializer::loadResource(IResource* res,const HSTREAM& stream, const ParamList& params)
	{
		if( res == NULL || stream == NULL)
			BLADE_EXCEPT(EXC_INVALID_PARAM,BTString("resource or stream data corrupted.") );
		else if( res->getType() == ShaderResource::SHADER_RESOURCE_TYPE )
			return (mError=this->createShaderInterMediate(stream, params));
		else
			BLADE_EXCEPT(EXC_INVALID_PARAM,BTString("resource type mismatch.") );
	}

	//////////////////////////////////////////////////////////////////////////
	void	ShaderResourceSerializer::postProcessResource(IResource* resource)
	{
		if(!BLADE_TS_CHECK_GRAPHICS_CONTEXT())
			BLADE_EXCEPT(EXCE_STATE_ERROR, BTString("cannot post process shader resource without graphics context."));

		if( mShaderProfiles.size() == 0 )
			BLADE_EXCEPT(EXCE_STATE_ERROR,BTString("invalid shader intermeda to create shader.") );

		ShaderResource* pShaderRes = static_cast<ShaderResource*>(resource);
		SubShader** subShaders = NULL;
		size_t* sizeList = NULL;
		index_t profileIndex = 0;
		TStringList profileList;

		size_t count = mShaderProfiles.size();
		if(count != 0)
		{
			profileList.reserve(count);
			subShaders = BLADE_RES_ALLOCT(SubShader*, count);
			sizeList = BLADE_RES_ALLOCT(size_t, count);

			for(ProfileList::iterator iter = mShaderProfiles.begin(); iter != mShaderProfiles.end(); ++iter)
			{
				const ShaderProfile& profile = iter->second;
				const InterMediaList& intermedias = profile.subIntermedia;
				profileList.push_back(iter->first);

				typedef TempVector<HSHADER> ShaderList;
				ShaderList shaders;
				shaders.resize( intermedias.size() );

				size_t subShaderCount = 0;
				if(intermedias.size() > 0 )
					subShaderCount = IGraphicsResourceManager::getSingleton().createShaders(&intermedias[0], &shaders[0], intermedias.size());
				else
					subShaderCount = 0;
				assert( subShaderCount == shaders.size() );
				subShaders[profileIndex] = BLADE_NEW SubShader[subShaderCount];
				sizeList[profileIndex] = subShaderCount;

				for(size_t i = 0; i < subShaderCount; ++i)
				{
					subShaders[profileIndex][i].setShader(shaders[i]);
					subShaders[profileIndex][i].setName( profile.subShaderNames[i] );
					subShaders[profileIndex][i].mTextContent = profile.subShaderContent[i];
					//Note: mShaderContents uses const hack, now we need persistent data. this is kind of dirty..
					subShaders[profileIndex][i].mTextContent[0] = subShaders[profileIndex][i].mTextContent[0];

					for(int j = SHADER_TYPE_BEGIN; j < SHADER_TYPE_COUNT; ++j)
					{
						EShaderType type = EShaderType(j);
						for(SemanticMap::const_iterator iter2 = mSemantics.begin(); iter2 != mSemantics.end(); ++iter2)
						{
							const TString& var = iter2->first;
							const SemanticList& list = iter2->second;
							//try all candidates, there should be only one valid
							for( size_t k = 0; k < list.size(); ++k)
							{
								const TString& semantic = list[k];
								subShaders[profileIndex][i].linkInternalShaderVariable(type, var, semantic);
							}
						}

						for(CustomSemanticMap::const_iterator iter2 = mCustomSemantics.begin(); iter2 != mCustomSemantics.end(); ++iter2)
						{
							const TString& variable = iter2->first;
							EShaderVariableUsage usage = iter2->second;
							const IShader::CONSTANT_INFO* info = shaders[i]->getConstantTable().getConstantInfoByName(type, variable);
							if( info != NULL )
								subShaders[profileIndex][i].linkInternalCustomVariable(type, variable, info->mType, info->mElementCount, usage);
						}

#if BUILD_SAMPLER_SEMANTIC
						//add sampler index(for GLSL uniform sampler binding)
						const ShaderInfoTable& table = shaders[i]->getConstantTable();
						for(size_t k = 0; k < table.getCount(); ++k)
						{
							const IShader::CONSTANT_INFO* info = table[k];
							if( info->mType != SCT_SAMPLER )
								continue;
							subShaders[profileIndex][i].mSemantics[type]->addSemantic(BTString("SAMPLER") + TStringHelper::fromUInt(info->mIndex), info->mName);
						}
#endif
					}
					this->addDefaultVariables(&subShaders[profileIndex][i]);
				}

				++profileIndex;
			}//for

		}//if
		pShaderRes->mSubShaders = subShaders;
		pShaderRes->mSubShaderCount = sizeList;
		pShaderRes->mProfiles = profileList;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	ShaderResourceSerializer::saveResource(const IResource* /*res*/,const HSTREAM& /*stream*/)
	{
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	ShaderResourceSerializer::createResource(IResource* res, ParamList& params)
	{
		if( res == NULL )
			BLADE_EXCEPT(EXC_INVALID_PARAM,BTString("resource or stream data corrupted.") );

		else if( res->getType() == ShaderResource::SHADER_RESOURCE_TYPE )
		{
			Variant vContent = params[BTString("content")];
			if( !vContent.isValid() )
				return false;

			TString base,ext;
			TStringHelper::getFileInfo(res->getSource(), base, ext);
			HSTREAM stream = IResourceManager::getSingleton().openStream( BTString("memory:/") + base + BTString(".cache"), false);
			if( stream == NULL )
				return false;

			String utfContent = StringConverter::TStringToUTF8String(vContent);
			stream->writeData( utfContent.c_str(), (IStream::Size)utfContent.size() );
			if( !this->createShaderInterMediate(stream, params) )
			{
				mError = true;
				return false;
			}
			this->postProcessResource(res);
			return true;
		}
		else
			BLADE_EXCEPT(EXC_INVALID_PARAM,BTString("resource type mismatch.") );
	}

	//////////////////////////////////////////////////////////////////////////
	bool	ShaderResourceSerializer::reloadResource(IResource* /*resource*/, const HSTREAM& stream, const ParamList& params)
	{
		assert(mShaderProfiles.size() == 0);
		return (mError=this->createShaderInterMediate(stream, params));
	}

	//////////////////////////////////////////////////////////////////////////
	bool	ShaderResourceSerializer::reprocessResource(IResource* resource)
	{
		if( mError )
			return false;
		else
		{
#if BLADE_DEBUG
			for(ProfileList::const_iterator i = mShaderProfiles.begin(); i != mShaderProfiles.end(); ++i)
			{
				const TString& profile = i->first;
				ShaderResource* pShaderRes = static_cast<ShaderResource*>(resource);
				size_t count = pShaderRes->getSubShaderCount(profile);
				SubShader* subShaders = pShaderRes->getSubShaders(profile);
				for(size_t j = 0; j < count; ++j)
					assert( subShaders[j].getShader().refcount() == 1 );
			}
#endif
			this->ShaderResourceSerializer::postProcessResource(resource);
			return true;
		}
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	bool			ShaderResourceSerializer::createShaderInterMediate(const HSTREAM& stream, const ParamList& complieParams)
	{
		assert(mShaderProfiles.size() == 0 );
		mBuffer.loadFromStream(stream);
		mShaderProfiles.clear();

		ConfigFile virtualheader;
		{
			TString header;
			this->parsingShaderInfomation(stream, header, true);
			if( !virtualheader.parseString(header) )
			{
				BLADE_LOG(Error, BTString("failed parsing shader header") );
				return false;
			}
		}

		ParamList params = complieParams;
		BLADE_LOG(Information, TEXT("parsing ") << TEXT(" shader file header \"")+stream->getName()+TEXT("\"."));
		
		TString shaderTypes[SHADER_TYPE_COUNT] =
		{
			BTString("VS"), BTString("FS"), BTString("GS"),
		};

		const CFGFILE_SECTION* section = virtualheader.getSection(ShaderKeywords::SHADER_SECTION);	//shader section
		const TString* profileTString;
		//since now we build shader offline, so the profile MUST be defined, using host profile is not an option any more.
		if( section == NULL || *(profileTString=&section->getValue(ShaderKeywords::SHADER_PROFILE)) == TString::EMPTY )
		{
			BLADE_LOG(Error, BTString("shader header does not have [")
				<< ShaderKeywords::SHADER_SECTION
				<< BTString("] or ") << ShaderKeywords::SHADER_PROFILE
				<< BTString(" defined.") );
			return false;
		}

		section = virtualheader.getSection( ShaderKeywords::SHADER_SECTION );
		ParamList macros;

		bool hasEntryDef = true;

		TString entryKeys[SHADER_TYPE_COUNT];
		TString defaultEntries[SHADER_TYPE_COUNT];

		for(int i = SHADER_TYPE_BEGIN; i < SHADER_TYPE_COUNT-1; ++i)
		{
			EShaderType type = EShaderType(i);
			const TString entryKey = shaderTypes[i] + ShaderKeywords::ENTRY_POINT;
			if( section->getValue(entryKey) == TString::EMPTY )
			{
				hasEntryDef = false;
				params[entryKey] = BTString("Blade")+shaderTypes[type]+BTString("Main");
			}
			if(!hasEntryDef)
				BLADE_LOG(Information, TEXT("shader does not define an entry point,using default.") );

			for(size_t j = 0; j < section->getEntryCount(); ++j )
			{
				const CFGFILE_ENTRY& entry = section->getEntry(j);
				params[entry.getName()] = entry.getValue();
			}

			entryKeys[i] = entryKey;
			defaultEntries[i] = params[entryKey];
		}//for each shader type
		BLADE_LOG(Information, TEXT("shader file header \"") + stream->getName() + TEXT("\" parsing finished.") );

		//add user defined macros
		for(size_t i = 0; i < complieParams.size(); ++i)
		{
			const NamedVariant& param = complieParams.at(i);
			const TString& paramName = param.getName();
			if( TStringHelper::isStartWith(paramName, BTString("-D") ) )
			{
				TString macro = paramName.substr_nocopy(2);
				const TString& macroVal = param.getValue();
				macros[macro] = macroVal;
			}
		}

		SubShaderDefinitionList subShaderDefines;
		section = virtualheader.getSection(ShaderKeywords::SUBSHADER_SECTION);
		if( section == NULL || section->getEntryCount() == 0 )
			subShaderDefines[TString::EMPTY] = TString::EMPTY;
		else
		{
			size_t subShaderCount = section->getEntryCount();
			for(size_t i = 0; i < subShaderCount; ++i)
			{
				const CFGFILE_ENTRY& entry = section->getEntry(i);
				const TString& name = entry.getName();
				const TString& defines = entry.getValue();
				subShaderDefines[name] = defines;
			}
		}
		
		String profileString = StringConverter::TStringToString( *profileTString);
		StringTokenizer profileList;
		profileList.tokenize(profileString, ",");
		if( profileList.size() == 0 )
		{
			assert(false);
			return false;
		}

		size_t intermediaCount = 0;

		for(size_t i = 0; i < profileList.size(); ++i)
		{
			const String& profileName = profileList[i];
			//validate profile name
			TString tprofileName = StringConverter::StringToTString(profileName);
			if( msValidProfiles.find(tprofileName) == msValidProfiles.end() )
			{
				BLADE_LOG(Error, BTString("Invalid profile: ") + tprofileName);
				return false;
			}

			ShaderProfile& profile = mShaderProfiles[tprofileName];

			SubShaderDefinitionList def;
			section = virtualheader.getSection(ShaderKeywords::SUBSHADER_SECTION + BTString(" ") + tprofileName);
			if( section != NULL && section->getEntryCount() > 0 )
			{
				size_t subShaderCount = section->getEntryCount();
				for(size_t j = 0; j < subShaderCount; ++j)
				{
					const CFGFILE_ENTRY& entry = section->getEntry(j);
					const TString& name = entry.getName();
					const TString& defines = entry.getValue();
					def[name] = defines;
				}
			}
			else
				def = subShaderDefines;

			profile.subShaderContent.reserve( def.size() );
			profile.subShaderNames.reserve( def.size() );
			profile.subIntermedia.reserve( def.size() );

			//sub shader information
			for(SubShaderDefinitionList::const_iterator iter = def.begin(); iter != def.end(); ++iter)
			{
				const TString& name = iter->first;
				const TString& defines = iter->second;

				TStringTokenizer tokenizer;
				tokenizer.tokenize( defines, TEXT(",") );
				ParamList subMacros = macros;

				subMacros[ShaderKeywords::MACRO_PROFILE] = BString("profile_")+profileName;
				params[ShaderKeywords::MACRO_PROFILE] = tprofileName;

				//reset entry
				bool entryOverridden = false;
				for(int j = SHADER_TYPE_BEGIN; j < SHADER_TYPE_COUNT-1; ++j)
					params[entryKeys[j]] = defaultEntries[j];

				for(size_t j = 0; j < tokenizer.size(); ++j)
				{
					TString define = tokenizer[j];
					define = TStringHelper::trim(define);
					if(define.empty())
						continue;

					//entry override
					if(define[0] == TEXT('['))
					{
						if(entryOverridden)
						{
							BLADE_LOG(Warning, BTString("subshader entry already overiden - ") << define << BTString(" for subshader ") << name);
							continue;
						}
						//mark overridden even if following fails
						entryOverridden = true;

						TStringTokenizer entryLine;
						entryLine.tokenize(define, TEXT("[]"));

						bool valid = entryLine.size() == 1;
						TStringTokenizer entries;
						if(valid)
							entries.tokenize(entryLine[0], TEXT(" \t"));
						valid = valid && entries.size() == 2;
						if(!valid)
						{
							BLADE_LOG(Warning, BTString("invalid subshader entry override - ") << define << BTString(" for subshader ") << name);
							continue;
						}
						size_t entryCount = std::min<size_t>(entries.size(), SHADER_TYPE_COUNT-1);
						for(size_t k = SHADER_TYPE_BEGIN; k < entryCount; ++k)
							params[entryKeys[k]] = entries[k];

						continue;
					}
					else
					{
						if (define[0] != '-')	//add '-' prefix to undef. it equals removing the define, but '-' allows fast switching, usually for debug purpose.
							subMacros[define] = BTString("1");
					}
				}

				const void* buffer = mBuffer.getData();
				size_t length = mBuffer.getSize();
				HSHADERINTMEDIA interMedia = IGraphicsResourceManager::getSingleton().compileShader(mBuffer.getName(), params, buffer, length, &subMacros);
				if( interMedia != NULL )
				{
					profile.subIntermedia.push_back(interMedia);
					profile.subShaderNames.push_back(name);
					++intermediaCount;
				}
				else
				{
					assert(false);
					BLADE_LOG(Error, BTString("compile shader failed.") + TEXT(" shader profile: ") 
						<< BString("profile_") + profileName 
						<< BTString(" subshader: ") + name);
					return false;
				}

				//add #define manually for ease of later use.
				String textContent;
				for(size_t j = 0; j < subMacros.size(); ++j)
				{
					const NamedVariant& define = subMacros.at(j);
					textContent += BString("#define ") + 
						StringConverter::TStringToString(define.getName()) + BString(" ") + 
						StringConverter::TStringToString(define.getValue().toTString()) + BString("\n");
				}
				textContent += String( (const char*)mBuffer.getData(), mBuffer.getSize(), String::CONST_HINT);
				profile.subShaderContent.push_back(textContent);
			}
		}
		return intermediaCount > 0;
	}

	//////////////////////////////////////////////////////////////////////////
	void			ShaderResourceSerializer::parsingShaderInfomation(const HSTREAM& stream, TString& outVHeader, bool root)
	{
		TextFile file;
		if( !file.open(stream) )
			BLADE_EXCEPT(EXC_FILE_READ,BTString("failed reading text from stream.") );

		const TString content = file.getText();
		if( root &&!TStringHelper::isStartWith(content,ShaderKeywords::HEADER_MARK+ShaderKeywords::HEADER_SIGNATURE ) )
			BLADE_EXCEPT(EXC_INVALID_PARAM,BTString("not a valid shader format,make sure file has a header - \"")+stream->getName()+BTString("\"."));

		//split up lines
		TStringTokenizer tokenizer;
		tokenizer.tokenize(content,TEXT("\n\r"));

		TStringConcat proxy(TString::EMPTY);
		const size_t start = root ? 1u : 0;
		for( size_t i = start; i < tokenizer.size(); ++i )
		{
			const TString& line = tokenizer[i];
			if( root && TStringHelper::isStartWith(line,ShaderKeywords::HEADER_MARK ) )
			{
				proxy += line.substr_nocopy(3) + TEXT("\n");
				//DataOffset += line.size() + 1;
			}
			else
			{
				TString trimed = TStringHelper::trim(line);

				 if( TStringHelper::isStartWith(trimed,BTString("#include") ) )
				 {
					 //get the directory info
					 TString Dir,FileName;
					 TStringHelper::getFilePathInfo(stream->getName(),Dir,FileName);

					 //get the included file
					 TStringTokenizer double_tokenizer;
					 double_tokenizer.tokenize(trimed,TEXT("\"\""),TEXT("\n\r \t") );
					 assert( double_tokenizer.size() == 2 
						 || (double_tokenizer.size() > 2 && TStringHelper::isStartWith(double_tokenizer[2], BTString("//"))) );
					 TString headerFile = Dir + TEXT("/") + double_tokenizer[1];

					 HSTREAM headerStream = IResourceManager::getSingleton().loadStream( headerFile );
					 if(headerStream != NULL )
						 this->parsingShaderInfomation(headerStream, outVHeader, false);
					 else
						 BLADE_LOG(Error, TEXT("open shader include file failed -\"") << headerFile << TEXT("\"") );
				 }
			}
		}
		outVHeader += proxy;

		//remove comments first
		TString newContent; //content without comments
		{
			TStringConcat contentProxy(TString::EMPTY);
			index_t cursor = 0;
			while( cursor < content.size() )
			{
				index_t last = cursor;
				index_t blockCursor = content.find(BTString("/*"), cursor);
				index_t lineCursor = content.find(BTString("//"), cursor);
				if( blockCursor != TString::npos || lineCursor != TString::npos )
				{
					if( blockCursor < lineCursor )
					{
						contentProxy += content.substr_nocopy(last, blockCursor - last);
						blockCursor = content.find(BTString("*/"), blockCursor + 2);
						//no end /* found, comment to the end
						if( blockCursor == TString::npos )
							break;
						else
							++blockCursor;
						cursor = blockCursor + 2;
					}
					else
					{
						contentProxy += content.substr_nocopy(last, lineCursor - last);
						lineCursor = content.find_first_of(BTString("\n\r"), lineCursor + 2);
						if( lineCursor != TString::npos )
							lineCursor = content.find_first_not_of(BTString("\n\r"), lineCursor);
						else
							break;
						cursor = lineCursor;
					}
				}
				else
				{
					contentProxy += content.substr_nocopy(last);
					break;
				}
			}
			newContent = contentProxy;
		}

		//split up statements/expressions first
		TStringTokenizer statementTokenizer;
		statementTokenizer.tokenize(newContent, TEXT("#\n#\r"), TEXT(";,{}") );
		for(size_t i = 0; i < statementTokenizer.size(); ++i)
		{
			if(statementTokenizer[i].size() == 0 || statementTokenizer.isDoubleToken(i) )
				continue;

			const TString& statement = statementTokenizer[i];
			if( statement.find(TEXT(':')) == TString::npos 
				|| statement.find(TEXT('?')) != TString::npos		// ? :
				|| statement.find(TEXT('=')) != TString::npos )
				continue;

			TStringTokenizer semanticTokenizer;
			semanticTokenizer.tokenize(statement, TEXT(":"));
			//if( semanticTokenizer.size() != 2 )
			//	continue;

			const TString variableLine = semanticTokenizer[0];
			TString semantic = semanticTokenizer[1];
			semanticTokenizer.clear();
			semanticTokenizer.tokenize(semantic, TEXT("()"), TEXT("\n\r \t"));

			//skip fragment shader output semantic
			{
				TString line = TStringHelper::trim(variableLine);
				if(line.empty()		// :COLOR ? at line begin?
					|| (line.size() > 0 && line[line.size()-1] == TEXT(')')))	//) : COLOR
					continue;
			}

			//last semantic including )
			if( semanticTokenizer.size() == 2 && TStringHelper::trim(semanticTokenizer[1]) == BTString(")") )
			{
				TString semantic0 = semanticTokenizer[0];
				semanticTokenizer.clear();
				semanticTokenizer.push_back(semantic0);
			}

			if( semanticTokenizer.size() != 1 )
				continue;

			//last semantic including ), without white spaces
			if (TStringHelper::isEndWith(semanticTokenizer[0], BTString(")")))
			{
				const TString& str = semanticTokenizer[0];
				TString sema = str.substr(0, str.size()-1);
				semanticTokenizer.clear();
				semanticTokenizer.push_back(sema);
			}

			semantic = semanticTokenizer[0];
			
			//probably a semantic declaration - type val : SEMANTIC ,/;
			if(//semantic.find_first_of(BTString("+-*/~!@#$%^&.`")) != TString::npos ||
				semantic.size() > 0 && semantic[0] >= TEXT('0') && semantic[0] <= TEXT('9') )
				continue;
			if (Impl::isBultinSemantic(semantic) )
				continue;
			
			TString variable;

			index_t brackets = variableLine.find(TEXT('['));
			if( brackets != TString::npos)
				variable = variableLine.substr_nocopy(0, brackets);
			else
				variable = variableLine.substr_nocopy(0);

			TStringTokenizer variableTokenizer;

			//we don't care how many words before, just need the last one
			//prefix type variableName[N] : SEMANTIC
			variableTokenizer.tokenize(variable, TEXT("()"), TEXT("() \t"));	//remove "()" or single "(",")"
			if( variableTokenizer.size() == 0 )
				continue;

			variable = variableTokenizer[ variableTokenizer.size() - 1 ];
			//custom semantics: _GLOBAL_, _SHADER_, _PASS_, _INSTANCE_
			if( semantic == BTString("_GLOBAL_") || semantic == BTString("_SHADER_") 
				|| semantic == BTString("_PASS_") || semantic == BTString("_INSTANCE_") )
			{
				mCustomSemantics[variable] = ShaderResourceSerializer::getUsage(semantic);
			}
			else
				mSemantics[variable].push_back( semantic );
		}//for

	}

	//////////////////////////////////////////////////////////////////////////
	void			ShaderResourceSerializer::addDefaultVariables(SubShader* subShader)
	{
		//treat default variable (variable without semantics) as custom 'shader' variable
#if BUILD_DEFAULT_VARIABLE
		using namespace Impl;

		//default variable without semantic
		TempVector<const IShader::CONSTANT_INFO*> defualtVariables[SHADER_TYPE_COUNT];
		const IShader::INFO_TABLE& table = subShader->getShader()->getConstantTable();
		for (size_t i = 0; i < table.getCount(); ++i)
		{
			const IShader::CONSTANT_INFO* info = table[i];
			if (info->mType == SCT_SAMPLER)
				continue;
			if (info->mName.find_first_of(BTString("[]")) != TString::npos)	//skip array elements
				continue;
			//skip if already has semantics
			if (mSemantics.find(info->mName) != mSemantics.end())
				continue;
			if (mCustomSemantics.find(info->mName) != mCustomSemantics.end())
				continue;

			//skip special uniform bound to sampler
			if (TStringHelper::isEndWith(info->mName, SAMPLER_SIZE_SUFFIX))
			{
				TString samplerName = info->mName.substr_nocopy(0, info->mName.size() - SAMPLER_SIZE_SUFFIX.size());
				const IShader::CONSTANT_INFO* samplerInfo = table.getConstantInfoByName(info->mShaderType, samplerName);
				if (samplerInfo != NULL && samplerInfo->mType == SCT_SAMPLER)
					continue;
			}

			if (TStringHelper::isEndWith(info->mName, SAMPLER_UVSPACE_SUFFIX))
			{
				TString samplerName = info->mName.substr_nocopy(0, info->mName.size() - SAMPLER_UVSPACE_SUFFIX.size());
				const IShader::CONSTANT_INFO* samplerInfo = table.getConstantInfoByName(info->mShaderType, samplerName);
				if (samplerInfo != NULL && samplerInfo->mType == SCT_SAMPLER)
					continue;
			}
			//any more skipping?

			defualtVariables[info->mShaderType].push_back(info);
		}

		for (int i = SHADER_TYPE_BEGIN; i < SHADER_TYPE_COUNT; ++i)
		{
			for (size_t j = 0; j < defualtVariables[i].size(); ++j)
			{
				const IShader::CONSTANT_INFO* info = defualtVariables[i][j];
				subShader->linkInternalCustomVariable(EShaderType(i), info->mName, info->mType, info->mElementCount, SVU_SHADER_CUSTOM);
			}
		}
#endif
	}

}//namespace Blade