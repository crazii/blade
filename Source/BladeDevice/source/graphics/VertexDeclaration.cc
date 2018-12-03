/********************************************************************
	created:	2014/12/20
	filename: 	VertexDeclaration.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <graphics/VertexDeclaration.h>

namespace Blade
{
	
	//////////////////////////////////////////////////////////////////////////
	VertexDeclaration::VertexDeclaration()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	VertexDeclaration::~VertexDeclaration()
	{
	}

	/************************************************************************/
	/* IVertexDeclaration interface                                                                     */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	size_t					VertexDeclaration::getElementCount() const
	{
		return mElementList.size();
	}

	//////////////////////////////////////////////////////////////////////////
	const VertexElement&	VertexDeclaration::getElement(index_t index) const
	{
		if( index < mElementList.size() )
		{
			VertexElementList::const_iterator it = mElementList.begin();
			for( index_t i = 0; i < index; ++i)
				++it;
			return *it;			
		}
		else
			BLADE_EXCEPT(EXC_OUT_OF_RANGE,BTString("index out of range.") );
	}

	//////////////////////////////////////////////////////////////////////////
	const VertexElement&	
		VertexDeclaration::addElement(uint16 source,uint16 offset,EVertexElementType type,EVertexUsage usage,uint8 usage_index)
	{
		mElementList.push_back( VertexElement(source,offset,type,usage,usage_index) );
		const VertexElement& elem = *mElementList.rbegin();

		mSourceSizeMap[source] += (int)elem.getSize();

		mElementList.sort();	//sort by source
		this->invalidate();
		return elem;
	}

	//////////////////////////////////////////////////////////////////////////
	const VertexElement&	
		VertexDeclaration::insertElement(index_t insertPos,uint16 source,uint16 offset,EVertexElementType type,EVertexUsage usage,uint8 usage_index)
	{
		VertexElementList::iterator it;
		if( insertPos >= mElementList.size() )
			it = mElementList.end();
		else
		{
			it = mElementList.begin();
			for( size_t i = 0; i < insertPos; ++i)
				++it;
		}
		VertexElementList::iterator n = mElementList.insert(it,VertexElement(source,offset,type,usage,usage_index) );
		const VertexElement& elem = *n;
		mSourceSizeMap[source] += (int)elem.getSize();
		mElementList.sort();	//sort by source
		this->invalidate();
		return elem;
	}

	//////////////////////////////////////////////////////////////////////////
	void					VertexDeclaration::removeElement(index_t removePos)
	{
		if( removePos < mElementList.size() )
		{
			VertexElementList::iterator it = mElementList.begin();
			for( index_t i = 0; i < removePos; ++i)
				++it;
			const VertexElement& elem = *it;
			mSourceSizeMap[elem.getSource()] -= (int)elem.getSize();
			mElementList.erase(it);
			this->invalidate();	
		}
		else
			BLADE_EXCEPT(EXC_OUT_OF_RANGE,BTString("index out of range.") );
	}

	//////////////////////////////////////////////////////////////////////////
	const VertexElement&	
		VertexDeclaration::modifyElement(index_t position,uint16 source,uint16 offset,EVertexElementType type,EVertexUsage usage,uint8 usage_index)
	{
		if( position < mElementList.size() )
		{
			VertexElementList::iterator it = mElementList.begin();
			for( index_t i = 0; i < position; ++i)
				++it;
			VertexElement& elem = *it;
			int oldSize = (int)elem.getSize();
			elem = VertexElement(source,offset,type,usage,usage_index);
			int size_diff = (int)elem.getSize() - oldSize;
			mSourceSizeMap[source] += size_diff;
			mElementList.sort();	//sort by source
			this->invalidate();
			return elem;			
		}
		else
			BLADE_EXCEPT(EXC_OUT_OF_RANGE,BTString("index out of range.") );
	}

	//////////////////////////////////////////////////////////////////////////
	size_t					VertexDeclaration::getVertexSize(uint16 source) const
	{
		VertexElementSizeMap::const_iterator i = mSourceSizeMap.find(source);
		if( i == mSourceSizeMap.end() )
			return 0;
		else
			return (size_t)i->second;
	}

	//////////////////////////////////////////////////////////////////////////
	const VertexElement*	VertexDeclaration::getElementByUsage(EVertexUsage usage,uint8 usage_index) const
	{
		for(VertexElementList::const_iterator iter = mElementList.begin(); iter != mElementList.end(); ++iter)
		{
			const VertexElement& elem = *iter;

			if( elem.getUsage() == usage && elem.getIndex() == usage_index)
				return &elem;
		}
		return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t					VertexDeclaration::getElementsBySource(uint16 source,IVertexDeclaration::ElementList& outList) const
	{
		for(VertexElementList::const_iterator iter = mElementList.begin(); iter != mElementList.end(); ++iter)
		{
			const VertexElement& elem = *iter;
			if( elem.getSource() == source )
				outList.push_back( elem );
		}
		return outList.size();
	}

	//////////////////////////////////////////////////////////////////////////
	const VertexElement*	VertexDeclaration::getFirstElementsBySource(uint16 source) const
	{
		for(VertexElementList::const_iterator iter = mElementList.begin(); iter != mElementList.end(); ++iter)
		{
			const VertexElement& elem = *iter;
			if( elem.getSource() == source )
				return &elem;
		}
		return NULL;
	}

}//namespace Blade