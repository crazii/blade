/********************************************************************
	created:	2013/02/26
	filename: 	EntityResource.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "EntityResource.h"
#include <ConstDef.h>
#include <interface/IResourceManager.h>

namespace Blade
{

	const TString EntityResource::ENTITY_SERIALIZER_TYPE_XML = BTString("Text Entity");
	const TString EntityResource::ENTITY_SERIALIZER_TYPE_BINARY = BTString("Binary Entity");

	//////////////////////////////////////////////////////////////////////////
	EntityResource::EntityResource()
		:mEntity(NULL)
		,mSaveDesc(NULL)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	EntityResource::~EntityResource()
	{

	}

}//namespace Blade