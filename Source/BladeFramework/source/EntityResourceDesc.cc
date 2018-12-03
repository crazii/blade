/********************************************************************
	created:	2013/04/12
	filename: 	EntityResourceDesc.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <EntityResourceDesc.h>

namespace Blade
{
	namespace Impl
	{
		struct SElementResourceInfo
		{
			TString mPath;
			bool	mOverride;
		};
		class EntityResourceDescImpl : public TempTStringMap<SElementResourceInfo>, public Allocatable	{};
	}//namespace Impl
	using namespace Impl;

	//////////////////////////////////////////////////////////////////////////
	EntityResourceDesc::EntityResourceDesc()
		:mImpl(BLADE_NEW Impl::EntityResourceDescImpl())
	{

	}

	//////////////////////////////////////////////////////////////////////////
	EntityResourceDesc::EntityResourceDesc(const TString& resourcePath)
		:mResourcePath(resourcePath)
		,mImpl(BLADE_NEW Impl::EntityResourceDescImpl())
	{

	}

	//////////////////////////////////////////////////////////////////////////
	EntityResourceDesc::~EntityResourceDesc()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	EntityResourceDesc& EntityResourceDesc::operator=(const EntityResourceDesc& rhs)
	{
		mResourcePath = rhs.mResourcePath;
		*mImpl = *(rhs.mImpl);
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	void		EntityResourceDesc::setPath(const TString& path)
	{
		mResourcePath = path;
	}

	//////////////////////////////////////////////////////////////////////////
	const TString&	EntityResourceDesc::getPath() const
	{
		return mResourcePath;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		EntityResourceDesc::addElementResource(const TString& elementName, const TString& elementReosurcePath, bool force/* = true*/)
	{
		SElementResourceInfo info;
		info.mPath = elementReosurcePath;
		info.mOverride = force;
		return mImpl->insert( std::make_pair(elementName, info) ).second;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		EntityResourceDesc::getElementResource(const TString& elementName, TString& outPath, bool& outForce) const
	{
		EntityResourceDescImpl::const_iterator i = mImpl->find(elementName);
		if( i == mImpl->end() )
			return false;
		else
		{
			outPath = i->second.mPath;
			outForce = i->second.mOverride;
			return true;
		}
	}
	
	//////////////////////////////////////////////////////////////////////////
	size_t		EntityResourceDesc::getElementResourceCount() const
	{
		return mImpl->size();
	}
	
	//////////////////////////////////////////////////////////////////////////
	bool		EntityResourceDesc::getElementResourceInfo(index_t index, TString& outElementName,TString& outPath, bool& outForce) const
	{
		if( index >= mImpl->size() )
			return false;

		EntityResourceDescImpl::const_iterator i = mImpl->begin();
		std::advance(i, index);
		
		outElementName = i->first;
		outPath = i->second.mPath;
		outForce = i->second.mOverride;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	const TString* EntityResourceDesc::getElementName(index_t index) const
	{
		if (index >= mImpl->size())
			return NULL;

		EntityResourceDescImpl::const_iterator i = mImpl->begin();
		std::advance(i, index);
		return &i->first;
	}
	
}//namespace Blade