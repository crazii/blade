/********************************************************************
	created:	2010/04/22
	filename: 	DefaultVertexDeclaration.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "DefaultVertexDeclaration.h"

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	DefaultVertexDeclaration::DefaultVertexDeclaration()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	DefaultVertexDeclaration::~DefaultVertexDeclaration()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	size_t			DefaultVertexDeclaration::getElementCount() const
	{
		return mElementList.size();
	}

	//////////////////////////////////////////////////////////////////////////
	const VertexElement&	DefaultVertexDeclaration::getElement(index_t index) const
	{
		if( index < mElementList.size() )
			return mElementList[index];
		else
			BLADE_EXCEPT(EXC_OUT_OF_RANGE,BTString("index out of range.") );
	}

	//////////////////////////////////////////////////////////////////////////
	const VertexElement&	DefaultVertexDeclaration::addElement(uint16 source,uint16 offset,EVertexElementType type,EVertexUsage usage,uint8 usage_index)
	{
		mElementList.push_back( VertexElement(source,offset,type,usage,usage_index) );
		return *mElementList.rbegin();
	}

	//////////////////////////////////////////////////////////////////////////
	const VertexElement&	DefaultVertexDeclaration::insertElement(index_t insertPos,uint16 source,uint16 offset,EVertexElementType type,EVertexUsage usage,uint8 usage_index)
	{
		VertexElementList::iterator i;
		if( insertPos >= mElementList.size() )
			i = mElementList.end();
		else
		{
			i = mElementList.begin();
			while( insertPos < mElementList.size() )
				++i;
		}

		mElementList.insert(i,1,VertexElement(source,offset,type,usage,usage_index) );
		return mElementList[insertPos];
	}

	//////////////////////////////////////////////////////////////////////////
	void			DefaultVertexDeclaration::removeElement(index_t removePos)
	{
		VertexElementList::iterator i;
		if( removePos >= mElementList.size() )
			return;
		else
		{
			i = mElementList.begin();
			while( removePos < mElementList.size() )
				++i;
		}
		mElementList.erase(i);
	}

	//////////////////////////////////////////////////////////////////////////
	const VertexElement&	DefaultVertexDeclaration::modifyElement(index_t position,uint16 source,uint16 offset,EVertexElementType type,EVertexUsage usage,uint8 usage_index)
	{
		if( position < mElementList.size() )
		{
			VertexElement& elem = mElementList[position];
			elem = VertexElement(source,offset,type,usage,usage_index);
			return elem;
		}
		else
			BLADE_EXCEPT(EXC_OUT_OF_RANGE,BTString("index out of range.") );
	}

	//////////////////////////////////////////////////////////////////////////
	size_t			DefaultVertexDeclaration::getVertexSize(uint16 source) const
	{
		size_t ret = 0;
		for( size_t i = 0; i < mElementList.size(); ++i )
		{
			const VertexElement& elem = mElementList[i];
			if( elem.getSource() == source )
				ret += elem.getSize();
		}
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	const VertexElement*	DefaultVertexDeclaration::getElementByUsage(EVertexUsage usage,uint8 usage_index) const
	{
		for( size_t i = 0; i < mElementList.size(); ++i )
		{
			const VertexElement& elem = mElementList[i];

			if( elem.getUsage() == usage && elem.getIndex() == usage_index)
				return &elem;
		}
		return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t					DefaultVertexDeclaration::getElementsBySource(uint16 source, ElementList& outList) const
	{
		for( size_t i = 0; i < mElementList.size(); ++i )
		{
			const VertexElement& elem = mElementList[i];
			if( elem.getSource() == source )
				outList.push_back( elem );
		}
		return outList.size();
	}

	//////////////////////////////////////////////////////////////////////////
	const VertexElement*	DefaultVertexDeclaration::getFirstElementsBySource(uint16 source) const
	{
		for( size_t i = 0; i < mElementList.size(); ++i )
		{
			const VertexElement& elem = mElementList[i];
			if( elem.getSource() == source )
				return &elem;
		}
		return NULL;
	}


	//////////////////////////////////////////////////////////////////////////
	HVDECL					DefaultVertexDeclaration::clone() const
	{
		HVDECL decl(BLADE_NEW DefaultVertexDeclaration());
		for( size_t i = 0; i < mElementList.size(); ++i )
		{
			const VertexElement& elem = mElementList[i];
			decl->addElement( elem.getSource(),elem.getOffset(),elem.getType(),elem.getUsage(),elem.getIndex() );
		}
		return decl;
	}
	
}//namespace Blade