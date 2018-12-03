/********************************************************************
	created:	2011/06/05
	filename: 	BiRef.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <utility/BiRef.h>

namespace Blade
{

	//////////////////////////////////////////////////////////////////////////
	BiRef::BiRef(BiRef* that/* = NULL*/)
		:mRef(that)
	{
		if( mRef != NULL )
		{
			assert( mRef->getRef() == NULL );
			mRef->setRef(this);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	BiRef::~BiRef()
	{
		this->detach();
	}

	//////////////////////////////////////////////////////////////////////////
	BiRef::BiRef(BiRef& src)
		:mRef(src.mRef)
	{
		src.detach();
		if( mRef != NULL )
			mRef->setRef(this);
	}

	//////////////////////////////////////////////////////////////////////////
	BiRef&	BiRef::operator=(BiRef& rhs)
	{
		if(mRef == rhs.mRef)
			return *this;

		if( mRef != NULL )
			mRef->detach();

		mRef = rhs.mRef;
		rhs.detach();
		if( mRef != NULL )
			mRef->setRef(this);
		return *this;
	}
	
	//////////////////////////////////////////////////////////////////////////
	void				BiRef::detach()
	{
		if( mRef != NULL )
		{
			assert( mRef->getRef() == this );
			mRef->setRef(NULL);
			mRef = NULL;
		}
	}

}//namespace Blade
