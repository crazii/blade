/********************************************************************
	created:	2014/12/26
	filename: 	GLESShader.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_GLESShader_h__
#define __Blade_GLESShader_h__
#include "GLESHeader.h"
#if BLADE_USE_GLES
#include <interface/public/graphics/VertexElement.h>
#include <graphics/Shader.h>

namespace Blade
{
	class GLESShaderIntermediate;
	class GLESVertexDeclaration;

	class GLESShader : public Shader, public Allocatable
	{
	public:
		GLESShader(GLESShaderIntermediate* intermedia);
		~GLESShader();
		/************************************************************************/
		/* IShader interface                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		virtual void			getCustomOptions(EShaderType type, ParamList& option);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			getData(Buffer& data) const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			isReady() const	{return mProgram != 0;}

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		inline GLuint	getProgram() const	{return mProgram;}

		/** @brief  */
		inline const GLESVertexDeclaration*	getDecl() const				{return mLastDecl;}
		/** @brief  */
		inline void		bindDecl(const GLESVertexDeclaration* decl)		{mLastDecl = decl;}
		/** @brief  */
		inline GLint	getVertexAttributeLocation(EVertexUsage usage, int index)
		{
			assert(usage >= VU_BEGIN && usage < VU_COUNT && index >=0 && index < 8);
			return mVSAttributeLocation[usage][index];
		}

	protected:

		/** @brief get program uniform info and fill in constant info table */
		bool			updateConstants();

		/** @brief  */
		static SHADER_CONSTANT_TYPE	getConstantType(GLenum glType);

		GLuint	mProgram;
		const GLESVertexDeclaration* mLastDecl;

		GLint mVSAttributeLocation[VU_COUNT][8];
	};//class GLESShader
	
}//namespace Blade

#endif//BLADE_USE_GLES
#endif // __Blade_GLESShader_h__