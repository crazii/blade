/********************************************************************
	created:	2010/04/22
	filename: 	IVertexDeclaration.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_IVertexDeclaration_h__
#define __Blade_IVertexDeclaration_h__
#include "VertexElement.h"
#include <Handle.h>
#include <utility/TList.h>

namespace Blade
{
	class IVertexDeclaration
	{
	public:
		/** @brief this is return type for function */
		typedef TList<VertexElement, TempAllocator<VertexElement>, TempAllocatable> ElementList;

		virtual ~IVertexDeclaration()	{}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual	size_t					getElementCount() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const VertexElement&	getElement(index_t index) const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const VertexElement&	addElement(uint16 source,uint16 offset,EVertexElementType type,EVertexUsage usage,uint8 usage_index) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const VertexElement&	insertElement(index_t insertPos,uint16 source,uint16 offset,EVertexElementType type,EVertexUsage usage,uint8 usage_index) = 0;			

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void					removeElement(index_t removePos) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const VertexElement&	modifyElement(index_t position,uint16 source,uint16 offset,EVertexElementType type,EVertexUsage usage,uint8 usage_index) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual	size_t					getVertexSize(uint16 source) const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const VertexElement*	getElementByUsage(EVertexUsage usage,uint8 usage_index) const = 0;

		/**
		@describe 
		@param 
		@return 
		@remark DONOT store the return value,as it is temporary allocated
		*/
		virtual size_t					getElementsBySource(uint16 source, ElementList& outList) const = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual const VertexElement*	getFirstElementsBySource(uint16 source) const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual Handle<IVertexDeclaration>clone() const = 0;

	};//class IVertexDeclaration

	typedef Handle<IVertexDeclaration>	HVDECL;
	
}//namespace Blade


#endif //__Blade_IVertexDeclaration_h__