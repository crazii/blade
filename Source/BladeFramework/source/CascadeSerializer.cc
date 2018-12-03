/********************************************************************
	created:	2013/04/15
	filename: 	CascadeSerializer.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <CascadeSerializer.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	CascadeSerializer::CascadeSerializer()
		:mSyncLoading(false)
	{
		
	}

	//////////////////////////////////////////////////////////////////////////
	CascadeSerializer::~CascadeSerializer()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	CascadeSerializer::SubResource*		CascadeSerializer::addSubResource(const TString& resourcePath, 
		const TString& resourceType/* = TString::EMPTY*/, const TString& serializerType/* = TString::EMPTY*/)
	{
		SubResource* state = BLADE_NEW SubResource();
		state->setResourceInfo(resourceType, resourcePath, serializerType);
		mGroup.addSubState(state);
		return state;
	}

	//////////////////////////////////////////////////////////////////////////
	CascadeSerializer::SubResourceGroup*	CascadeSerializer::addSubGroup()
	{
		SubResourceGroup* group = BLADE_NEW SubResourceGroup();
		mGroup.addSubState(group);
		return group;
	}

	//////////////////////////////////////////////////////////////////////////
	void		CascadeSerializer::loadSubResources()
	{
		if( mSyncLoading )
		{
			mGroup.loadSync();
		}
		else
			//if not sync loading, current context probably be IO task
			//some resources only can be created(post process) on main sync state
			//so we use async mode to make it properly loaded
			mGroup.load();
	}
	
}//namespace Blade