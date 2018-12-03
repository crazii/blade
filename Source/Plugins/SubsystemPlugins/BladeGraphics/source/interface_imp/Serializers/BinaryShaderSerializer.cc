/********************************************************************
	created:	2013/05/19
	filename: 	BinaryShaderSerializer.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <interface/public/graphics/ShaderInfoTable.h>
#include <interface/public/graphics/IGraphicsResourceManager.h>
#include <interface/IResourceManager.h>
#include <interface/IGraphicsSystem.h>
#include "BinaryShaderSerializer.h"
#include "../../SubShaderImpl.h"

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(disable:4127) //conditional expression is constant
#endif

namespace Blade
{
	using namespace Impl;

	static const FourCC BLADE_SHADER_MAGIC = BLADE_FCC('B','L','S','D');
	static const FourCC SUB_SHADER_MAGIC = BLADE_FCC('B','L','S','S');
	static const FourCC SEMANTIC_MAGIC = BLADE_FCC('B', 'L', 'S', 'M');
	static const FourCC CUSTOM_SEMANTIC_MAGIC = BLADE_FCC('B', 'L', 'C', 'M');
	static const FourCC SHADER_CONTENT_MAGIC = BLADE_FCC('B','L','S','C');

	//////////////////////////////////////////////////////////////////////////
	BinaryShaderSerializer::BinaryShaderSerializer()
	{
		mError = false;
	}

	//////////////////////////////////////////////////////////////////////////
	BinaryShaderSerializer::~BinaryShaderSerializer()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	bool	BinaryShaderSerializer::loadResource(IResource* res,const HSTREAM& stream, const ParamList& /*params*/)
	{
		if( res == NULL || stream == NULL)
			BLADE_EXCEPT(EXC_INVALID_PARAM,BTString("resource or stream data corrupted.") );
		else if( res->getType() == ShaderResource::SHADER_RESOURCE_TYPE )
			return (mError=this->createShaderInterMediate(stream));
		else
			BLADE_EXCEPT(EXC_INVALID_PARAM,BTString("resource type mismatch.") );
	}

	//////////////////////////////////////////////////////////////////////////
	void	BinaryShaderSerializer::postProcessResource(IResource* resource)
	{
		if(!BLADE_TS_CHECK_GRAPHICS_CONTEXT())
			BLADE_EXCEPT(EXCE_STATE_ERROR, BTString("cannot post process shader resource without graphics context."));

		ShaderResource* pShaderRes = static_cast<ShaderResource*>(resource);
		SubShader** subShaders = NULL;
		size_t*		sizeList = NULL;
		size_t profileCount = mProfiles.size();
		TStringList profileList;

		if( profileCount > 0 )
		{
			profileList.reserve(profileCount);
			subShaders = BLADE_RES_ALLOCT(SubShader*, profileCount);
			sizeList = BLADE_RES_ALLOCT(size_t, profileCount);
			size_t profileIndex = 0;

			for(ProfileList::iterator iter = mProfiles.begin(); iter != mProfiles.end(); ++iter)
			{
				const ShaderProfile& profile = iter->second;
				const InterMediaList& intermedias = profile.mShaderInterMedia;
				profileList.push_back(iter->first);
				size_t count = intermedias.size();

				typedef TempVector<HSHADER> ShaderList;
				ShaderList shaders;
				shaders.resize( count );
				count = IGraphicsResourceManager::getSingleton().createShaders(&intermedias[0], &shaders[0], count);
				assert( count == shaders.size() );
				subShaders[profileIndex] = BLADE_NEW SubShader[count];
				sizeList[profileIndex] = count;

				for(size_t i = 0; i < count; ++i)
				{
					subShaders[profileIndex][i].setShader( shaders[i] );
					subShaders[profileIndex][i].setName( profile.mShaderNames[i] );

					for(int j = SHADER_TYPE_BEGIN; j < SHADER_TYPE_COUNT; ++j)
					{
						EShaderType type = EShaderType(j);
						assert( profile.mSemantics[type].size() == count );

						for(SemanticMap::const_iterator iter2 = profile.mSemantics[type][i].begin(); iter2 != profile.mSemantics[type][i].end(); ++iter2)
						{
							const TString& var = iter2->first;
							const TString& semantic = iter2->second;
							subShaders[profileIndex][i].linkInternalShaderVariable(type, var, semantic);
						}

						for(CustomSemanticMap::const_iterator iter2 = profile.mCustomSemantics[type][i].begin(); iter2 != profile.mCustomSemantics[type][i].end(); ++iter2)
						{
							const TString& variable = iter2->first;
							EShaderVariableUsage usage = iter2->second;
							const IShader::CONSTANT_INFO* info = shaders[i]->getConstantTable().getConstantInfoByName(type, variable);
							if( info != NULL )
								subShaders[profileIndex][i].linkInternalCustomVariable(type, variable, info->mType, info->mElementCount, usage);
						}
					}
				}//for each sub shader in profile
				++profileIndex;
			}//for each profile
		}

		pShaderRes->mSubShaders = subShaders;
		pShaderRes->mSubShaderCount = sizeList;
		pShaderRes->mProfiles = profileList;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	BinaryShaderSerializer::saveResource(const IResource* res,const HSTREAM& stream)
	{
		if( res->getType() != ShaderResource::SHADER_RESOURCE_TYPE )
		{
			assert(false);
			return false;
		}
		const ShaderResource* pShaderRes = static_cast<const ShaderResource*>(res);
		if( pShaderRes->getProfileCount() == 0 )
		{
			BLADE_LOG(Warning, BTString("skip EMPTY shader resource."));
			return true;
		}
		mBuffer.clear();

		uint32 profileCount = (uint32)pShaderRes->getProfileCount();
		mBuffer.write(&profileCount);

		for(uint32 i = 0; i < profileCount; ++i)
		{
			const TString& profileName = pShaderRes->getProfileName(i);
			mBuffer.writeTString(profileName);

			uint32 subShaderCount = (uint32)pShaderRes->getSubShaderCount(profileName);
			mBuffer.write(&subShaderCount);
			SubShader* subShaders = pShaderRes->getSubShaders(profileName);
			for(size_t j = 0; j < subShaderCount; ++j)
			{
				SubShader* subShader = subShaders+j;
				mBuffer.write(&BLADE_SHADER_MAGIC);
				mBuffer.writeTString( subShader->getName() );

				IOBuffer buffer;
				if (!this->getContentBuffer(subShader, buffer))
				{
					assert(false);
					return false;
				}

				for(int k = SHADER_TYPE_BEGIN; k < SHADER_TYPE_COUNT; ++k)
				{
					EShaderType type = EShaderType(k);

					//magic & type
					mBuffer.write(&SUB_SHADER_MAGIC);
					int32 type32 = type;
					mBuffer.write(&type32);

					//entry
					mBuffer.writeTString( subShader->getShader()->getEntry(type) );

					//semantics
					mBuffer.write(&SEMANTIC_MAGIC);
					subShader->mSemantics[type]->writeSemantics(mBuffer);			

					//custom semantics
					mBuffer.write(&CUSTOM_SEMANTIC_MAGIC);
					subShader->mSemantics[type]->writeCustomSemantics<uint8>(mBuffer);
				}

				//binary
				mBuffer.write(&SHADER_CONTENT_MAGIC);
				uint32 codeSize = (uint32)buffer.getSize();
				mBuffer.write(&codeSize);
				mBuffer.writeData( buffer.getData(), (IStream::Size)buffer.getSize() );
			}			
		}

		mBuffer.saveToStream(stream);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	BinaryShaderSerializer::createResource(IResource* /*res*/,ParamList& /*params*/)
	{
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	BinaryShaderSerializer::reloadResource(IResource* /*resource*/, const HSTREAM& stream, const ParamList& /*params*/)
	{
		assert(mProfiles.size() == 0);
		return (mError=this->createShaderInterMediate(stream));
	}

	//////////////////////////////////////////////////////////////////////////
	bool	BinaryShaderSerializer::reprocessResource(IResource* resource)
	{
		if( mProfiles.size() == 0 || mError)
			return false;
		else
		{
#if BLADE_DEBUG
			for(ProfileList::const_iterator i = mProfiles.begin(); i != mProfiles.end(); ++i)
			{
				const TString& profile = i->first;
				ShaderResource* pShaderRes = static_cast<ShaderResource*>(resource);
				size_t count = pShaderRes->getSubShaderCount(profile);
				SubShader* subShaders = pShaderRes->getSubShaders(profile);
				for(size_t j = 0; j < count; ++j)
					assert( subShaders[j].getShader().refcount() == 1 );
			}
#endif
			this->BinaryShaderSerializer::postProcessResource(resource);
			return true;
		}
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool	BinaryShaderSerializer::getContentBuffer(SubShader* subShader, Buffer& buffer)
	{
		return subShader->getShader()->getData(buffer);
	}

	//////////////////////////////////////////////////////////////////////////
	HSHADERINTMEDIA	BinaryShaderSerializer::loadContentBuffer(const TString& name, const void* data, size_t bytes,
		const TString& profile, const TString entrypoint[SHADER_TYPE_COUNT])
	{
		return IGraphicsResourceManager::getSingleton().createShaderIntermediate(name, data, bytes, profile, entrypoint);
	}

	//////////////////////////////////////////////////////////////////////////
	bool			BinaryShaderSerializer::createShaderInterMediate(const HSTREAM& stream)
	{
		assert(mProfiles.size() == 0 );
		mBuffer.loadFromStream(stream);

		uint32 profileCount = 0;
		if( !mBuffer.read(&profileCount) )
		{
			assert(false);
			return false;
		}

		for(uint32 i = 0; i < profileCount; ++i)
		{
			TString profileName;
			mBuffer.readTString(profileName);
			ShaderProfile& profile = mProfiles[profileName];

			uint32 count = 0;
			mBuffer.read(&count);

			for(uint32 j = 0; j < count; ++j)
			{
				FourCC fcc;
				if( !mBuffer.read(&fcc) || fcc != BLADE_SHADER_MAGIC )
				{
					assert(false);
					return false;
				}

				TString name;
				if( !mBuffer.readTString(name) )
				{
					assert(false);
					return false;
				}

				const void*	data;
				size_t  dataSize;
				TString entryPoints[SHADER_TYPE_COUNT];

				for( int k = SHADER_TYPE_BEGIN; k < SHADER_TYPE_COUNT; ++k)
				{
					profile.mSemantics[k].resize(count);
					profile.mCustomSemantics[k].resize(count);
				}
				
				for(int k = SHADER_TYPE_BEGIN; k < SHADER_TYPE_COUNT; ++k)
				{
					//magic & type
					if( !mBuffer.read(&fcc) || fcc != SUB_SHADER_MAGIC )
					{
						assert(false);
						return false;
					}
					uint32 type32;
					mBuffer.read(&type32);

					EShaderType type = EShaderType(type32);
					assert(type == k);

					//entry
					mBuffer.readTString(entryPoints[k]);

					//semantic
					if( !mBuffer.read(&fcc) || fcc != SEMANTIC_MAGIC )
					{
						assert(false);
						return false;
					}
					uint32 semanticCount = 0;
					if( !SubShaderSemanticDataImpl::readCount(mBuffer, semanticCount) )
					{
						assert(false);
						return false;
					}

					for(size_t n = 0; n < semanticCount; ++n)
					{
						TString variable, semantic;
						bool ret = SubShaderSemanticDataImpl::readSemantic(mBuffer, variable, semantic);
						assert(ret);
						ret = profile.mSemantics[type][j].insert( std::make_pair(variable, semantic) ).second;
						assert(ret);
					}

					//custom semantic
					if( !mBuffer.read(&fcc) || fcc != CUSTOM_SEMANTIC_MAGIC )
					{
						assert(false);
						return false;
					}
					uint32 customSemanticCount = 0;
					if( !SubShaderSemanticDataImpl::readCount(mBuffer, customSemanticCount) )
					{
						assert(false);
						return false;
					}

					for(size_t n = 0; n < customSemanticCount; ++n)
					{
						TString variable;
						uint8 usage;
						bool ret = SubShaderSemanticDataImpl::readCustomSemantic<uint8>(mBuffer, variable, usage);
						assert(ret);
						assert( usage&SVU_CUSTOM_FLAG && (usage&SVU_TYPE_MASK) >= SVU_BEGIN  && (usage&SVU_TYPE_MASK) < SVU_COUNT );

						ret = profile.mCustomSemantics[type][j].insert( std::make_pair(variable, (EShaderVariableUsage)usage) ).second;
						assert(ret);
					}

				}//for all shader types

				//binary
				if( !mBuffer.read(&fcc) || fcc != SHADER_CONTENT_MAGIC )
				{
					assert(false);
					return false;
				}
				uint32 bytes;
				mBuffer.read(&bytes);
				if (bytes != 0)
				{
					data = mBuffer.getCurrentData();
					mBuffer.seekForward(bytes);
				}
				else
					data = NULL;
				dataSize = bytes;
				HSHADERINTMEDIA interMedia = this->loadContentBuffer(stream->getName(), data, dataSize, profileName, entryPoints);
				if( interMedia != NULL )
				{
					profile.mShaderInterMedia.push_back(interMedia);
					profile.mShaderNames.push_back(name);
				}
				else
				{
					assert(false);
					return false;
				}

			}//for each sub shader in profile

		}//for each profile
		return true;
	}

}//namespace Blade