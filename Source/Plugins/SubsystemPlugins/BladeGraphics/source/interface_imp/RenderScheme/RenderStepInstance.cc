/********************************************************************
	created:	2013/04/29
	filename: 	RenderStepInstance.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <interface/IRenderTypeManager.h>
#include "RenderStepInstance.h"

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	RenderStepInstance::RenderStepInstance(RenderStep* step)
		:mStep(step)
		,mFillMode(GFM_DEFAULT)
		,mEnable(true)
	{
		mFixedFillMode = mStep->getType()->isFixedFillMode();
	}

	//////////////////////////////////////////////////////////////////////////
	RenderStepInstance::~RenderStepInstance()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	RenderStep*			RenderStepInstance::getRenderStep() const
	{
		return mStep;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				RenderStepInstance::isEnabled() const
	{
		return mEnable && mStep->isEnabled();
	}

	//////////////////////////////////////////////////////////////////////////
	void				RenderStepInstance::setEnable(bool enabled)
	{
		if( mEnable != enabled )
		{
			mEnable = enabled;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void				RenderStepInstance::setFillMode(FILL_MODE mode)
	{
		if( mFillMode != mode && !mFixedFillMode )
		{
			mFillMode = mode;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	FILL_MODE			RenderStepInstance::getFillMode() const
	{
		return mFillMode;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				RenderStepInstance::isFixedFillMode() const
	{
		return mFixedFillMode;
	}

}//namespace Blade