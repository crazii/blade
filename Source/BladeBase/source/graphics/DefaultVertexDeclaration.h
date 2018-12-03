/********************************************************************
	created:	2010/04/22
	filename: 	DefaultVertexDeclaration.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_DefaultVertexDeclaration_h__
#define __Blade_DefaultVertexDeclaration_h__
#include <interface/public/graphics/IVertexDeclaration.h>
#include <utility/BladeContainer.h>

namespace Blade
{
	class DefaultVertexDeclaration : public IVertexDeclaration, public Allocatable
	{
	public:
		DefaultVertexDeclaration();
		~DefaultVertexDeclaration();

		/*
		@describe 
		@param 
		@return 
		*/
		virtual	size_t					getElementCount() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const VertexElement&	getElement(index_t index) const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const VertexElement&	addElement(uint16 source,uint16 offset,EVertexElementType type,EVertexUsage usage,uint8 usage_index);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const VertexElement&	insertElement(index_t insertPos,uint16 source,uint16 offset,EVertexElementType type,EVertexUsage usage,uint8 usage_index);			

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void					removeElement(index_t removePos);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const VertexElement&	modifyElement(index_t position,uint16 source,uint16 offset,EVertexElementType type,EVertexUsage usage,uint8 usage_index);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual	size_t					getVertexSize(uint16 source) const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const VertexElement*	getElementByUsage(EVertexUsage usage,uint8 usage_index) const;

		/*
		@describe 
		@param 
		@return 
		@remark DONOT store the return value,as it is temporary allocated
		*/
		virtual size_t					getElementsBySource(uint16 source, ElementList& outList) const;

		/*
		@describe 
		@param
		@return
		*/
		virtual const VertexElement*	getFirstElementsBySource(uint16 source) const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual HVDECL					clone() const;

	protected:
		typedef		Vector<VertexElement>	VertexElementList;

		VertexElementList		mElementList;
	};
	
}//namespace Blade


#endif //__Blade_DefaultVertexDeclaration_h__