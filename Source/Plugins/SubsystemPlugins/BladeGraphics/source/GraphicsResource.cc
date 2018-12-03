/********************************************************************
	created:	2010/05/23
	filename: 	GraphicsResource.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <GraphicsResource.h>
#include "interface_imp/GraphicsTask.h"

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	GraphicsResource::GraphicsResource(const TString& type)
		:mType(type)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	GraphicsResource::~GraphicsResource()
	{

	}

	/************************************************************************/
	/* IResource interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	const TString&	GraphicsResource::getType() const
	{
		return mType;
	}

	//////////////////////////////////////////////////////////////////////////
	const ITask::Type&	GraphicsResource::getGraphicsTaskType()
	{
		//make resource processing in parallel, but be serial with graphics task
		//return EMPTY is OK, but in that case processing will be done in main sync state, not parallel and occupy overall process times
		return GraphicsTask::TYPE;
	}
	
}//namespace Blade