/********************************************************************
	created:	2010/04/13
	filename: 	IVertexBuffer.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_IVertexBuffer_h__
#define __Blade_IVertexBuffer_h__
#include <Handle.h>
#include "IGraphicsBuffer.h"

namespace Blade
{

	class IVertexBuffer : public IGraphicsBuffer
	{
	public:
		virtual ~IVertexBuffer()	{}

		/**
		@describe get size of one single vertex in bytes
		@param 
		@return 
		*/
		virtual size_t	getVertexSize() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual size_t getVertexCount() const = 0;

		/** @brief  */
		inline size_t	getVertexBytes() const
		{
			return this->getVertexSize() * this->getVertexCount();
		}

	};//class IVertexBuffer

	typedef Handle<IVertexBuffer> HVBUFFER;	
	
}//namespace Blade


#endif //__Blade_IVertexBuffer_h__