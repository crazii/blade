/********************************************************************
	created:	2014/12/29
	filename: 	HybridShaderSerializer.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "HybridShaderSerializer.h"
#include "ShaderKeywords.h"

#include <interface/public/graphics/ShaderInfoTable.h>
#include <interface/public/graphics/IGraphicsResourceManager.h>
#include <interface/IResourceManager.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	HybridShaderSerializer::HybridShaderSerializer()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	HybridShaderSerializer::~HybridShaderSerializer()
	{

	}

	/************************************************************************/
	/* BinaryShaderSerializer overrides                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool	HybridShaderSerializer::getContentBuffer(SubShader* subShader, Buffer& buffer)
	{
		//get text to save
		const String& content = subShader->mTextContent;
		assert( content != String::EMPTY );
		buffer.reserve( content.size() + 1 );
		buffer.setSize( content.size() + 1 );
		std::memcpy(buffer.getData(), content.c_str(), content.size() );
		((char*)buffer.getData())[content.size()] = '\0';
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	HSHADERINTMEDIA	HybridShaderSerializer::loadContentBuffer(const TString& name, const void* data, size_t bytes,
		const TString& profile, const TString entrypoint[SHADER_TYPE_COUNT])
	{
		TString shaderTypes[SHADER_TYPE_COUNT] =
		{
			BTString("VS"), BTString("FS"), BTString("GS"),
		};

		//compile text source
		ParamList option;

		const TString profileKey =  ShaderKeywords::SHADER_PROFILE;
		option[profileKey] = profile;

		for(int i = SHADER_TYPE_BEGIN; i < SHADER_TYPE_COUNT; ++i)
		{
			const TString entryKey = shaderTypes[i] + ShaderKeywords::ENTRY_POINT;
			
			
			option[entryKey] = entrypoint[i];
		}

		HSHADERINTMEDIA hIntermediate = IGraphicsResourceManager::getSingleton().compileShader(name, option, data, bytes);
		assert( hIntermediate != HSHADERINTMEDIA::EMPTY );
		return hIntermediate;
	}

}//namespace Blade