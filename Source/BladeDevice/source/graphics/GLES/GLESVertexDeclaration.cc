/********************************************************************
	created:	2014/12/20
	filename: 	GLESVertexDeclaration.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <graphics/GLES/GLESHeader.h>
#if BLADE_USE_GLES
#include <interface/public/graphics/IVertexSource.h>
#include <graphics/GLES/GLESUtil.h>
#include <graphics/GLES/GLESVertexDeclaration.h>
#include <graphics/GLES/GLESShader.h>
#include <graphics/GLES/GLESVertexBuffer.h>

namespace Blade
{
	GLuint GLESVertexDeclaration::msAttribLimit = 16;	//minimal requirement for GLES 3.0
	GLESVertexDeclaration::AttribMask GLESVertexDeclaration::msAttribMask;
	//////////////////////////////////////////////////////////////////////////
	GLESVertexDeclaration::GLESVertexDeclaration()
		:mLastSource(NULL)
		,mLastVertexStart(0)
		,mDirty(true)
	{
		
	}

	//////////////////////////////////////////////////////////////////////////
	GLESVertexDeclaration::~GLESVertexDeclaration()
	{
		
	}

	/************************************************************************/
	/* IVertexDeclaration interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	HVDECL					GLESVertexDeclaration::clone() const
	{
		HVDECL decl(BLADE_NEW GLESVertexDeclaration());
		for( VertexElementList::const_iterator iter = mElementList.begin(); iter != mElementList.end(); ++iter )
		{
			const VertexElement& elem = *iter;
			decl->addElement( elem.getSource(),elem.getOffset(),elem.getType(),elem.getUsage(),elem.getIndex() );
		}
		return decl;
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void				GLESVertexDeclaration::apply(GLESShader* shader, IVertexSource* source, size_t vertexStart) const
	{
		mDirty = mDirty || shader->getDecl() != this;
		if( mDirty )
		{
			shader->bindDecl(this);
			mLastSource = NULL;
			mLastVertexStart = 0;
		}

		//nothing to do.
		if (mLastSource == source && mLastVertexStart == vertexStart)
			return;

		size_t maxSource = source->getBufferSourceCount()-1;
		TempVector<bool>	attribMask(msAttribLimit, false);
		for(VertexElementList::const_iterator i = mElementList.begin(); i != mElementList.end(); ++i)
		{
			const VertexElement& elem = *i;
			size_t sourceIndex = elem.getSource();

			if( elem.getUsage() < VU_BEGIN || elem.getUsage() >= VU_COUNT || elem.getIndex() > 8 )
				BLADE_EXCEPT(EXC_OUT_OF_RANGE, BTString("invalid elemnt usage or index") );

			HVBUFFER buffer;
			GLint location = shader->getVertexAttributeLocation(elem.getUsage(), elem.getIndex());
			if( location == -1 || sourceIndex > maxSource || (buffer=source->getStreamInfo(sourceIndex).buffer) == NULL ) //may have gap
				continue;
			if (location >= msAttribLimit)
			{
				assert(false);
				continue;
			}

			const GLESVertexBuffer* glesBuffer = static_cast<GLESVertexBuffer*>(buffer);
			size_t vertexSize = glesBuffer->GLESVertexBuffer::getVertexSize();	//explicit call to utilize inline. (may broke inheritance/abstraction). TODO: use C++11 final
			size_t vertexOffset = vertexSize*vertexStart;
			//bind VBO
			glesBuffer->bindBuffer();
			//bind attribute using VBO
			GLESVertexAttrib attrib = GLESVertexElementConverter( elem.getType() );
			::glVertexAttribPointer(location, attrib.GLcount, attrib.GLtype, attrib.GLnormalize, (GLsizei)vertexSize,  (const void*)(vertexOffset + elem.getOffset()) );
			assert( ::glGetError() == GL_NO_ERROR );
			//TODO: glVertexAttribDivisor for instancing

			attribMask[location] = true;
		}
		::glBindBuffer(GL_ARRAY_BUFFER, 0);
		assert(::glGetError() == GL_NO_ERROR);

		for(GLuint i = 0; i < msAttribLimit; ++i)
		{
			if(attribMask[i])
				GLESVertexDeclaration::enableAttrib(i);
			else
			{
				GLESVertexDeclaration::disableAttrib(i);
				::glVertexAttribDivisor(i, 0);
				assert( ::glGetError() == GL_NO_ERROR );
			}
		}

		mLastSource = source;
		mLastVertexStart = vertexStart;
		mDirty = false;
	}

	//////////////////////////////////////////////////////////////////////////
	void GLESVertexDeclaration::setAttribLimit(GLuint limit)
	{
		msAttribLimit = limit;
		msAttribMask.resize(limit, false);

		for (size_t i = 0; i < msAttribLimit; ++i)
			::glDisableVertexAttribArray((GLuint)i);
	}

	//////////////////////////////////////////////////////////////////////////
	void GLESVertexDeclaration::enableAttrib(GLuint index)
	{
		assert(index < msAttribLimit);
		if (!msAttribMask[index])
		{
			::glEnableVertexAttribArray(index);
			assert(::glGetError() == GL_NO_ERROR);
			msAttribMask[index] = true;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void GLESVertexDeclaration::disableAttrib(GLuint index)
	{
		assert(index < msAttribLimit);
		if (msAttribMask[index])
		{
			::glDisableVertexAttribArray(index);
			assert(::glGetError() == GL_NO_ERROR);
			msAttribMask[index] = false;
		}
	}
	
}//namespace Blade

#endif//BLADE_USE_GLES