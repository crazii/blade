/********************************************************************
	created:	2013/11/21
	filename: 	Delegate.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <utility/Delegate.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	const char Delegate::ZERO[Delegate::MAX_SIZE] = {0};

	//////////////////////////////////////////////////////////////////////////
	namespace Impl
	{
		class DelegateListImpl : public List<Delegate>, public Allocatable	{};
	}//namespace Impl
	using namespace Impl;

	//////////////////////////////////////////////////////////////////////////
	DelegateList::DelegateList()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	DelegateList::~DelegateList()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	DelegateList::DelegateList(const DelegateList& src)
	{
		if(src.mList != NULL)
			*mList = *src.mList;
	}

	//////////////////////////////////////////////////////////////////////////
	DelegateList& DelegateList::operator=(const DelegateList& rhs)
	{
		if (rhs.mList != NULL)
			*mList = *rhs.mList;
		else
			mList.destruct();
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	void			DelegateList::push_back(const Delegate& _delegate)
	{
		mList->push_back(_delegate);
	}

	//////////////////////////////////////////////////////////////////////////
	bool			DelegateList::erase(index_t index)
	{
		if(mList != NULL && index < mList->size() )
		{
			DelegateListImpl::iterator i = mList->begin();
			while( index-- > 0)
				++i;
			mList->erase(i);
			return true;
		}
		else
		{
			assert(false);
			return false;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	const Delegate& DelegateList::at(index_t index) const
	{
		if(mList != NULL && index < mList->size() )
		{
			DelegateListImpl::const_iterator i = mList->begin();
			while( index-- > 0)
				++i;
			return *i;
		}
		else
			BLADE_EXCEPT(EXC_OUT_OF_RANGE, BTString("index out of range"));
	}

	//////////////////////////////////////////////////////////////////////////
	size_t			DelegateList::size() const
	{
		return mList != NULL ? mList->size() : 0;
	}

	//////////////////////////////////////////////////////////////////////////
	void			DelegateList::call(void* data) const
	{
		if (mList == NULL)
			return;

		for (DelegateListImpl::const_iterator i = mList->begin(); i != mList->end(); ++i)
		{
			const Delegate& d = (*i);
			if (d.hasParameter())
				d.call(data);
			else
				d.call();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void			DelegateList::call() const
	{
		if (mList == NULL)
			return;

		for (DelegateListImpl::const_iterator i = mList->begin(); i != mList->end(); ++i)
		{
			const Delegate& d = (*i);
			if (!d.hasParameter())
				d.call();
		}
	}
	
}//namespace Blade