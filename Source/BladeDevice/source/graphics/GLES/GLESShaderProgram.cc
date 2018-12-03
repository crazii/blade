#/********************************************************************
	created:	2014/12/22
	filename: 	GLESShaderProgram.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <graphics/GLES/GLESHeader.h>
#if BLADE_USE_GLES
#include <graphics/GLES/GLESShaderProgram.h>
#include <graphics/GLES/GLESVertexShader.h>
#include <graphics/GLES/GLESFragmentShader.h>

namespace Blade
{
	/************************************************************************/
	/* IShaderProgram interface                                                                     */
	/************************************************************************/
	bool			GLESShaderProgram::link()
	{
		if( !mLinked && this->isCompelete() )
		{
			GLESVertexShader* vs = static_cast<GLESVertexShader*>(this->getShader(SHADER_VERTEX));
			vs->compileShader();
			assert( vs->isValid() );
			::glAttachShader(mGLProgram, vs->getShader() );
			assert( ::glGetError() == GL_NO_ERROR );

			GLESFragmentShader* fs = static_cast<GLESFragmentShader*>(this->getShader(SHADER_FRAGMENT));
			fs->compileShader();
			assert( fs->isValid() );
			::glAttachShader(mGLProgram, fs->getShader() );
			assert( ::glGetError() == GL_NO_ERROR );

			//TODO: glGetAttribLocation/glBindAttribLocation

			::glLinkProgram(mGLProgram);
			assert( ::glGetError() == GL_NO_ERROR );

			GLint result;
			::glGetProgramiv( mGLProgram, GL_LINK_STATUS, &result); 
			assert( ::glGetError() == GL_NO_ERROR );
			mLinked = result == GL_TRUE;

			::glDetachShader(mGLProgram, vs->getShader() );
			assert( ::glGetError() == GL_NO_ERROR );

			::glDetachShader(mGLProgram, fs->getShader() );
			assert( ::glGetError() == GL_NO_ERROR );
			//TODO: delete compiled shader after linking?
			//but compiled shader may be shared across programs, delete them will cause a re-compile. 
		}
		return mLinked;
	}
}//namespace Blade

#endif//BLADE_USE_GLES