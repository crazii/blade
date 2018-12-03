/********************************************************************
	created:	2010/04/27
	filename: 	SpaceBase.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <SpaceBase.h>
#include <interface/ISpaceContent.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	SpaceBase::SpaceBase(const TString& type)
		:mType(type)
		,mFlag(SF_DEFAULT)
		,mCoordinator(NULL)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	SpaceBase::~SpaceBase()
	{

	}

	/************************************************************************/
	/*INodeUpdater Interface                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	const TString&		SpaceBase::getSpaceType() const
	{
		return mType;
	}

	//////////////////////////////////////////////////////////////////////////
	ISpaceCoordinator*	SpaceBase::getCoordinator() const
	{
		return mCoordinator;
	}

	//////////////////////////////////////////////////////////////////////////
	uint32				SpaceBase::getSpaceFlag() const
	{
		return mFlag.getMask();
	}

	//////////////////////////////////////////////////////////////////////////
	void				SpaceBase::setSpaceFlag(uint32 flag)
	{
		mFlag = flag;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				SpaceBase::addContent(ISpaceContent* content, uint32 partitionMask/* = ISpace::INVALID_PARTITION*/, bool strictFit/* = false*/)
	{
		//run state will have other subsystem query space data, so disable modification
		BLADE_TS_VERIFY_GRAPHICS_WRITE();

		bool ret = this->addContentImpl(content, partitionMask, strictFit);
		assert(ret);
		if( ret )
			this->adoptContent(content);
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				SpaceBase::removeContent(ISpaceContent* content)
	{
		//run state will have other subsystem query space data, so disable modification
		BLADE_TS_VERIFY_GRAPHICS_WRITE();

		if (content != NULL)
		{
			ISpace* space = content->getSpace();
			if (space == this)
			{
				bool ret = this->removeContentImpl(content);
				ISpace::clearSpace(content, true);
				return ret;
			}
			else if (space == NULL)
				return true;
		}
		return false;
	}

	/************************************************************************/
	/* custom method                                                                     */
	/************************************************************************/


}//namespace Blade