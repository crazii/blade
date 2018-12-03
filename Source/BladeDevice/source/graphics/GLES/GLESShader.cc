/********************************************************************
	created:	2014/12/26
	filename: 	GLESShader.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <graphics/GLES/GLESHeader.h>
#if BLADE_USE_GLES
#include <graphics/GLES/GLESShader.h>
#include <graphics/GLES/GLESShaderIntermediate.h>
#include <interface/public/graphics/ShaderInfoTable.h>
#include <interface/public/graphics/IGraphicsResourceManager.h>
#include <graphics/GLES/public/GLESAttributeNames.h>

namespace Blade
{
	static_assert(sizeof(GLenum) == 4, "size inconsistent");

	//////////////////////////////////////////////////////////////////////////
	GLESShader::GLESShader(GLESShaderIntermediate* intermediate)
		:Shader(intermediate)
		,mProgram(0)
		,mLastDecl(NULL)
	{
		if( !intermediate->hasLinkedProgram()  )
		{
			mProgram = ::glCreateProgram();
			assert( ::glGetError() == GL_NO_ERROR );
			GLenum* data = (GLenum*)intermediate->getData();
			assert( data != NULL );
			::glProgramBinary(mProgram, data[0], &data[1], intermediate->getDataSize() - sizeof(GLenum) );
			assert( ::glGetError() == GL_NO_ERROR );

			GLint status = 0;
			::glGetProgramiv(mProgram, GL_LINK_STATUS, &status);
			assert( ::glGetError() == GL_NO_ERROR );
			if( status != GL_TRUE )
			{
				GLchar buffer[4096];
				GLsizei infoLength = 0;
				::glGetProgramInfoLog(mProgram, 4096, &infoLength, buffer);
				assert( ::glGetError() == GL_NO_ERROR );
				if( infoLength != 0 )
					BLADE_LOG(Error, String(buffer, infoLength) );
				assert(false);
			}
		}
		else
			mProgram = intermediate->getProgram();

		//constant table: glGetActiveUniform
		bool ret = this->updateConstants();
		assert(ret);
		BLADE_UNREFERENCED(ret);
	}

	//////////////////////////////////////////////////////////////////////////
	GLESShader::~GLESShader()
	{
		::glDeleteProgram(mProgram);
		mProgram = 0;
	}

	/************************************************************************/
	/* IShader interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void			GLESShader::getCustomOptions(EShaderType /*type*/, ParamList& /*option*/)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	bool			GLESShader::getData(Buffer& data) const
	{
		if( mProgram != 0 )
		{
			GLint length = 0;
			::glGetProgramiv(mProgram, GL_PROGRAM_BINARY_LENGTH, &length);
			assert( ::glGetError() == GL_NO_ERROR );

			data.reserve( length + sizeof(GLenum) );
			data.setSize( length + sizeof(GLenum) );

			GLenum* pData = (GLenum*)data.getData();
			if( pData == NULL )
				return false;

			::glGetProgramBinary(mProgram, length, NULL, &pData[0], &pData[1]);
			assert( ::glGetError() == GL_NO_ERROR );
			return true;
		}
		else
			return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			GLESShader::updateConstants()
	{
		assert(mProgram != 0);

		static const GLsizei MAX_NAME = 512;
		GLchar name[MAX_NAME];

		//note: render device has program id cache, don't mess it up
		GLuint old = 0;
		::glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*)&old);
		assert(::glGetError() == GL_NO_ERROR);

		//bind program object to get constants
		::glUseProgram(mProgram);
		assert( ::glGetError() == GL_NO_ERROR );
		GLint textureIndex = 0;

		//TODO: get uniform blocks for UBO

		GLint uniformCount = 0;
		::glGetProgramiv(mProgram, GL_ACTIVE_UNIFORMS, &uniformCount);
		assert( ::glGetError() == GL_NO_ERROR );
		for(GLint i = 0; i < uniformCount; ++i)
		{
			GLint size;
			GLenum type;
			GLsizei nameLen;
			::glGetActiveUniform(mProgram, i, MAX_NAME, &nameLen, &size, &type, name);
			assert( ::glGetError() == GL_NO_ERROR );
			GLint index = ::glGetUniformLocation(mProgram, name);
			assert( ::glGetError() == GL_NO_ERROR );

			/*
			https://www.khronos.org/opengles/sdk/docs/man3/html/glGetActiveUniform.xhtml
			If one or more elements of an array are active, the name of the array is returned in name, 
			the type is returned in type, and the size parameter returns the highest array element index used, plus one, as determined by the compiler and/or linker. 
			
			Only one active uniform variable will be reported for a uniform array. If the active uniform is an array, 
			the uniform name returned in name will always be the name of the uniform array appended with "[0]".

			Uniform variables that are declared as structures or arrays of structures will not be returned directly by this function. 
			Instead, each of these uniform variables will be reduced to its fundamental components containing the "." and "[]" operators such that 
			each of the names is valid as an argument to glGetUniformLocation. 
			Each of these reduced uniform variables is counted as one active uniform variable and is assigned an index. 
			A valid name cannot be a structure, an array of structures, or a subcomponent of a vector or matrix.
			
			The size of the uniform variable will be returned in size. Uniform variables other than arrays will have a size of 1. 
			Structures and arrays of structures will be reduced as described earlier, such that each of the names returned will be a data type in the earlier list. 
			If this reduction results in an array, the size returned will be as described for uniform arrays; otherwise, the size returned will be 1.

			The list of active uniform variables may include both built-in uniform variables 
			(which begin with the prefix "gl_") as well as user-defined uniform variable names.
			This function will return as much information as it can about the specified active uniform variable. 
			If no information is available, length will be 0, and name will be an empty string.
			This situation could occur if this function is called after a link operation that failed.
			If an error occurs, the return values length, size, type, and name will be unmodified.
			*/

			IShader::CONSTANT_INFO info;
			TString tName = StringConverter::StringToTString(name, nameLen);
			index_t leftBracket = tName.find(TEXT('[') );
			index_t dot = tName.find(TEXT('.') ); //array of structures: only this name could be reduplicated
			//uint arrayIndex = 0;
			if( leftBracket != INVALID_INDEX && dot == INVALID_INDEX )
			{
				info.mName = tName.substr(0, leftBracket);
				index_t rightBracket = tName.find(TEXT(']'));
				assert(rightBracket != INVALID_INDEX); BLADE_UNREFERENCED(rightBracket);
				//arrayIndex = TStringHelper::toUInt( tName.substr(leftBracket+1, rightBracket-leftBracket-1) );
			}
			else
				info.mName = tName;

			info.mType = GLESShader::getConstantType(type);
			info.mElementSize = IGraphicsResourceManager::getSingleton().getShaderConstantSize( info.mType );
			info.mIndex = index;
			info.mElementCount = size;
			info.mBytes = info.mElementSize*info.mElementCount;
			info.mRegisterCount = info.mBytes / 4;	//align to one float/int
			info.mShaderType = SHADER_ANY;			//linked program doesn't have type
			//bind sampler to fixed index
			if( info.mType == SCT_SAMPLER )
			{
				::glUniform1iv(index, 1, &textureIndex);
				assert( ::glGetError() == GL_NO_ERROR );
				info.mIndex = textureIndex++;
			}
			
			bool ret = mInfoTable.addConstantInfo( info );
			assert(ret); BLADE_UNREFERENCED(ret);
		}//for

		for(int i = 0; i < VU_COUNT; ++i)
		{
			for(int j = 0; j < 8; ++j)
			{
				const String& builtinName = GLES_Attributes[i][j];
				mVSAttributeLocation[i][j] = ::glGetAttribLocation(mProgram, builtinName.c_str());	//maybe -1 if attribute is not active
			}
		}

		::glUseProgram(old);
		assert(::glGetError() == GL_NO_ERROR);

		return true;
	}


	//////////////////////////////////////////////////////////////////////////
	SHADER_CONSTANT_TYPE	GLESShader::getConstantType(GLenum glType)
	{
		switch(glType)
		{
		case GL_FLOAT_VEC2:
			return SCT_FLOAT2;
		case GL_FLOAT_VEC3:
			return SCT_FLOAT3;
		case GL_FLOAT_VEC4:
			return SCT_FLOAT4;
		case GL_INT:
			return SCT_INT1;
		case GL_INT_VEC2:
			return SCT_INT2;
		case GL_INT_VEC3:
			return SCT_INT3;
		case GL_INT_VEC4:
			return SCT_INT4;
		case GL_BOOL:
			return SCT_BOOL1;
		case GL_BOOL_VEC2:
			return SCT_BOOL2;
		case GL_BOOL_VEC3:
			return SCT_BOOL3;
		case GL_BOOL_VEC4:
			return SCT_BOOL4;
		case GL_FLOAT_MAT2:
			return SCT_MATRIX22;
		case GL_FLOAT_MAT3:
			return SCT_MATRIX33;
		case GL_FLOAT_MAT4:
			return SCT_MATRIX44;
		case GL_SAMPLER_2D:
		case GL_SAMPLER_3D:
		case GL_SAMPLER_CUBE:
		case GL_SAMPLER_2D_SHADOW:
			return SCT_SAMPLER;
		case GL_FLOAT:
			return SCT_FLOAT1;
		case GL_UNSIGNED_INT:
			return SCT_UINT1;
		case GL_UNSIGNED_INT_VEC2:
			return SCT_UINT2;
		case GL_UNSIGNED_INT_VEC3:
			return SCT_UINT3;
		case GL_UNSIGNED_INT_VEC4:
			return SCT_UINT4;
		case GL_FLOAT_MAT2x3:
			return SCT_MATRIX32;
		case GL_FLOAT_MAT2x4:
			return SCT_MATRIX42;
		case GL_FLOAT_MAT3x2:
			return SCT_MATRIX23;
		case GL_FLOAT_MAT3x4:
			return SCT_MATRIX43;
		case GL_FLOAT_MAT4x2:
			return SCT_MATRIX24;
		case GL_FLOAT_MAT4x3:
			return SCT_MATRIX34;
		default:
			assert( false && "unsupported" );
		}//switch
		return SCT_FLOAT1;
	}
	
}//namespace Blade

#endif//BLADE_USE_GLES