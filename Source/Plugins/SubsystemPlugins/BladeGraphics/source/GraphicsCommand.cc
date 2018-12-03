/********************************************************************
	created:	2017/12/05
	filename: 	GraphicsCommand.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <GraphicsCommand.h>
#include <Element/GraphicsElement.h>
#include <interface/ISpace.h>
#include <interface/ISpaceContent.h>

namespace Blade
{

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void GraphicsElementCommand::update(SSTATE)
	{
		if (this->getSlot() != NULL)
		{
			this->execute();
			this->getTarget()->finishCommand(this);
		} //else element deleted, do nothing
		BLADE_DELETE this;
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void AddContentToSpaceCommand::execute()
	{
		mSpace->addContent(mContent);
	}

	//////////////////////////////////////////////////////////////////////////
	void RemoveContentFromSpaceCommand::execute()
	{
		mSpace->removeContent(mContent);
	}

}//namespace Blade