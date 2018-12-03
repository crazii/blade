/********************************************************************
	created:	2012/03/27
	filename: 	ShaderGroup.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <ShaderGroup.h>

namespace Blade
{
	ShaderGroup::ShaderGroup()
	{
	}

	ShaderGroup::ShaderGroup(const TString& name)
		:mName(name)
	{

	}

	ShaderGroup::~ShaderGroup()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	ShaderGroup::SRequireValue
		ShaderGroup::getRequirement(ERequirement eReq) const
	{
		if( eReq < SR_COUNT && eReq >= 0 )
			return mReqList[eReq];
		else
			BLADE_EXCEPT(EXC_OUT_OF_RANGE, BTString("requirement out of range.") );
	}

	//////////////////////////////////////////////////////////////////////////
	bool					ShaderGroup::setRequirement(ERequirement eReq, SRequireValue reqVal)
	{
		if( eReq < SR_COUNT && eReq >= 0)
		{
			mReqList[eReq] = reqVal;
			return true;
		}
		else
			return false;
	}


	/************************************************************************/
	/* ShaderGroupList                                                                     */
	/************************************************************************/
	namespace Impl
	{
		class ShaderGroupListImpl : public Vector<ShaderGroup*>, public Allocatable
		{
		public:
			inline ShaderGroupListImpl& operator=(const ShaderGroupListImpl& rhs)
			{
				this->resize(rhs.size());
				for(size_t i = 0; i < this->size(); ++i)
					(*this)[i] = BLADE_NEW ShaderGroup( *(rhs[i]) );
				return *this;
			}
			inline ~ShaderGroupListImpl()
			{
				for(size_t i = 0; i < this->size(); ++i)
					BLADE_DELETE (*this)[i];
			}
		};
	}//namespace Impl
	using namespace Impl;

	//////////////////////////////////////////////////////////////////////////
	ShaderGroupList::ShaderGroupList()
		:mImpl( BLADE_NEW ShaderGroupListImpl() )
	{

	}

	//////////////////////////////////////////////////////////////////////////
	ShaderGroupList::~ShaderGroupList()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	ShaderGroupList::ShaderGroupList(const ShaderGroupList& src)
		:mImpl( BLADE_NEW ShaderGroupListImpl() )
	{
		*mImpl = *src.mImpl;
	}

	//////////////////////////////////////////////////////////////////////////
	ShaderGroupList& ShaderGroupList::operator=(const ShaderGroupList& rhs)
	{
		*mImpl = *rhs.mImpl;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	void	ShaderGroupList::clear()
	{
		mImpl->clear();
	}

	//////////////////////////////////////////////////////////////////////////
	void	ShaderGroupList::reserve(size_t count)
	{
		mImpl->reserve(count);
	}

	//////////////////////////////////////////////////////////////////////////
	index_t ShaderGroupList::findGroup(const TString& name) const
	{
		for(size_t i = 0; i < mImpl->size(); ++i)
		{
			if( (*mImpl)[i]->getName() == name )
				return i;
		}
		return INVALID_INDEX;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t	ShaderGroupList::size() const
	{
		return mImpl->size();
	}

	//////////////////////////////////////////////////////////////////////////
	void	ShaderGroupList::push_back(const ShaderGroup& group)
	{
		return mImpl->push_back(BLADE_NEW ShaderGroup(group) );
	}

	//////////////////////////////////////////////////////////////////////////
	ShaderGroup&	ShaderGroupList::getAt(index_t index)
	{
		if( index < mImpl->size() )
			return *(*mImpl)[index];
		else
			BLADE_EXCEPT(EXC_OUT_OF_RANGE, BTString("index out of range."));
	}

	//////////////////////////////////////////////////////////////////////////
	const ShaderGroup&	ShaderGroupList::getAt(index_t index) const
	{
		if( index < mImpl->size() )
			return *(*mImpl)[index];
		else
			BLADE_EXCEPT(EXC_OUT_OF_RANGE, BTString("index out of range."));
	}
}//namespace Blade