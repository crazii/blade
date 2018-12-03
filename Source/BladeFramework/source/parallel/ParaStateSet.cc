/********************************************************************
	created:	2011/04/09
	filename: 	ParaStateSet.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <interface/public/IElement.h>
#include <interface/IParaState.h>
#include <parallel/ParaStateSet.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	void ParaStateSet::SOpAdapter::operator=(IParaState& objRef)
	{
		mPtrRef = &objRef;
		mPtrRef->setQueue(mQueue);
		mPtrRef->setOwner(mOwner);
		mPtrRef->setPriority(mPriority);
	}

	/************************************************************************/
	/* ParaStateSet                                                                     */
	/************************************************************************/

}//namespace Blade
