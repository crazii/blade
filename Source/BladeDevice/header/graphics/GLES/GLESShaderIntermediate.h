/********************************************************************
	created:	2014/12/26
	filename: 	GLESShaderIntermediate.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_GLESShaderIntermediate_h__
#define __Blade_GLESShaderIntermediate_h__
#include "GLESHeader.h"
#if BLADE_USE_GLES
#include <graphics/ShaderIntermediate.h>

namespace Blade
{
	class GLESShaderIntermediate : public ShaderIntermediate
	{
		static const TString	GLES_SHADER_ID;
	public:
		GLESShaderIntermediate(const TString& name, GLuint program = 0);
		~GLESShaderIntermediate();

		/************************************************************************/
		/* IShaderIntermediate interface                                                                     */
		/************************************************************************/
		/** @brief safe id through different shader framework, such as HLSL & CG,etc */
		virtual const TString&		getShaderID() const	{return GLES_SHADER_ID;}

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		static const TString&	getGLESShaderID()		{return GLES_SHADER_ID;}

		/** @brief  */
		GLuint	getProgram() const			{return mProgram;}

		/** @brief  */
		bool	hasLinkedProgram() const	{return mProgram != 0;}

	protected:
		GLuint mProgram;	//runtime-compiling result
	};
	
}//namespace Blade

#endif//BLADE_USE_GLES
#endif // __Blade_GLESShaderIntermediate_h__