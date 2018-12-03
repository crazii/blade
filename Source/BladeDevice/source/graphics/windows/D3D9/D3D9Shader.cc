/********************************************************************
	created:	2014/12/24
	filename: 	D3D9Shader.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
#include <BladeWin32API.h>
#include <graphics/windows/D3D9/D3D9Shader.h>
#include <graphics/windows/D3DShaderIntermediate.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	D3D9Shader::D3D9Shader(IDirect3DVertexShader9* pVS, IDirect3DPixelShader9* pPS, D3DShaderIntermediate* intermediate)
		:Shader(intermediate)
	{
		mInfoTable = intermediate->getConstantTable();
		mVertexShader = pVS;
		mPixelShader = pPS;
		mCompilingFlags[SHADER_VERTEX] = intermediate->getCompilingFlags(SHADER_VERTEX);
		mCompilingFlags[SHADER_FRAGMENT] = intermediate->getCompilingFlags(SHADER_FRAGMENT);
	}

	//////////////////////////////////////////////////////////////////////////
	D3D9Shader::~D3D9Shader()
	{
		if( mVertexShader != NULL )
			mVertexShader->Release();
		if( mPixelShader != NULL )
			mPixelShader->Release();
	}

	/************************************************************************/
	/* IShader interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void			D3D9Shader::getCustomOptions(EShaderType /*type*/, ParamList& /*option*/)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	bool			D3D9Shader::getData(Buffer& data) const
	{
		data.clear();
		UINT vsBytes, psBytes;

		if( mVertexShader != NULL && mPixelShader != NULL )
		{
			if( FAILED(mVertexShader->GetFunction(NULL, &vsBytes) ) )
				return false;
			if( FAILED(mPixelShader->GetFunction(NULL, &psBytes) ) )
				return false;

			TempBuffer vsBuffer,psBuffer;
			vsBuffer.reserve(vsBytes);
			psBuffer.reserve(psBytes);
			
			if( !SUCCEEDED( mVertexShader->GetFunction(vsBuffer.getData(), &vsBytes) ) )
			{
				data.clear();
				return false;
			}

			if( !SUCCEEDED( mPixelShader->GetFunction(psBuffer.getData(), &psBytes) ) )
			{
				data.clear();
				return false;
			}
			return this->writeShaderBinary(data, vsBuffer.getData(), vsBytes, psBuffer.getData(), psBytes);
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	D3D9Shader::readShaderBinary(const void* intermediate, size_t interBytes,
		void*& vsData, size_t& vsBytes, void*& psData, size_t& psBytes)
	{
		const char* data = (const char*)intermediate;
		if( data == NULL )
			return false;

		vsBytes = *((uint32*)data);
		data += sizeof(uint32);

		vsData = (void*)data;
		data += vsBytes;

		psBytes = *((uint32*)data);
		data += sizeof(uint32);

		psData = (void*)data;
		data += psBytes;

		if( vsBytes + psBytes + sizeof(uint32)*2 != interBytes )
		{
			assert(false);
			psData = vsData = NULL;
			vsBytes = psBytes = 0;
			return false;
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	D3D9Shader::writeShaderBinary(Buffer& buffer, const void* vsData, size_t vsBytes, const void* psData, size_t psBytes)
	{
		buffer.clear();
		if( vsData == NULL || vsBytes == 0 || psData == NULL || psBytes == 0 )
		{
			assert(false);
			return false;
		}

		buffer.reserve( vsBytes + psBytes + sizeof(uint32)*2 );
		buffer.setSize( buffer.getCapacity() );
		char* data = (char*)buffer.getData();
		if( data == NULL )
			return false;

		*((uint32*)data) = (uint32)vsBytes;
		data += sizeof(uint32);

		std::memcpy(data, vsData, vsBytes);
		data += vsBytes;


		*((uint32*)data) = (uint32)psBytes;
		data += sizeof(uint32);

		std::memcpy(data, psData, psBytes);
		data += psBytes;


		return true;
	}

}//namespace Blade

#endif //BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS