/********************************************************************
	created:	2014/12/20
	filename: 	GLESVertexDeclaration.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_GLESVertexDeclaration_h__
#define __Blade_GLESVertexDeclaration_h__
#include "GLESHeader.h"
#if BLADE_USE_GLES
#include <graphics/VertexDeclaration.h>

namespace Blade
{
	class GLESShader;
	class IVertexSource;

	class GLESVertexDeclaration : public VertexDeclaration, public Allocatable
	{
	public:
		GLESVertexDeclaration();
		~GLESVertexDeclaration();

		/************************************************************************/
		/* IVertexDeclaration interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual HVDECL					clone() const;

		/************************************************************************/
		/* VertexDeclaration interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual void		invalidate()	{mDirty = true;}

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief apply vertex declaration: rebind attribute location if needed, and
		bind vertex buffer to location according to declaration */
		void				apply(GLESShader* shader, IVertexSource* source, size_t vertexStart) const;

	protected:
		typedef Vector<GLint>	AttribMap;
		typedef Vector<bool>	AttribMask;

		mutable IVertexSource*mLastSource;
		mutable size_t		mLastVertexStart;
		mutable bool		mDirty;

		friend class GLESRenderDevice;
		static GLuint		msAttribLimit;
		static AttribMask	msAttribMask;
		static void setAttribLimit(GLuint limit);
		static void enableAttrib(GLuint index);
		static void disableAttrib(GLuint index);
	};//GLESVertexDeclaration

	
}//namespace Blade

#endif//BLADE_USE_GLES
#endif // __Blade_GLESVertexDeclaration_h__