/********************************************************************
	created:	2014/12/27
	filename: 	SoftwareShader.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_SoftwareShader_h__
#define __Blade_SoftwareShader_h__
#include <BladePlatform.h>
#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
#include <interface/public/graphics/IShader.h>
#include <interface/public/graphics/ShaderInfoTable.h>
#include <utility/Buffer.h>

#include <graphics/Shader.h>
#include <graphics/windows/D3DShaderIntermediate.h>


namespace Blade
{
	class SoftwareShader : public Shader, public Allocatable
	{
	public:
		SoftwareShader(D3DShaderIntermediate* intermediate)
			:Shader(intermediate)
		{
			mInfoTable = intermediate->getConstantTable();
			mBufferSize = intermediate->getDataSize();
			if( mBufferSize != 0 )
			{
				mBuffer = BLADE_RES_ALLOC(mBufferSize);
				std::memcpy(mBuffer, intermediate->getData(), mBufferSize);
			}
			else
			{
				mBuffer = NULL;
				assert(false);
			}
		}
		virtual ~SoftwareShader()
		{
			BLADE_RES_FREE(mBuffer);
		}

		/*
		@describe
		@param
		@return
		*/
		virtual const INFO_TABLE& getConstantTable() const
		{
			return mInfoTable;
		}

		/*
		@describe
		@param
		@return
		*/
		virtual const TString& getProfileString(EShaderType type) const
		{
			assert( type >= SHADER_TYPE_BEGIN && type < SHADER_TYPE_COUNT );
			return mProfile[type];
		}

		/*
		@describe
		@param
		@return
		*/
		virtual void			getCustomOptions(EShaderType /*type*/, ParamList& /*option*/)	{}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			getData(Buffer& data) const
		{
			data.clear();
			data.reserve( mBufferSize );
			data.setSize( mBufferSize );
			std::memcpy(data.getData(), mBuffer, mBufferSize);
			return true;
		}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			isReady() const {return true;}

	protected:
		TString					mProfile[SHADER_TYPE_COUNT];
		IShader::INFO_TABLE		mInfoTable;
		void*					mBuffer;
		size_t					mBufferSize;
	};
	
}//namespace Blade

#endif//BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
#endif //  __Blade_SoftwareShader_h__