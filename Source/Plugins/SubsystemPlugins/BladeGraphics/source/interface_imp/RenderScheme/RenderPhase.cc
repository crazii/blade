/********************************************************************
	created:	2013/04/28
	filename: 	RenderPhase.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "RenderPhase.h"
#include "RenderOutput.h"
#include "RenderStepInstance.h"

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	RenderPhase::RenderPhase(RenderOutput* output)
		:mOutput(output)
		,mEnabled(true)
	{
		mStepInstances.resize( output->getStepCount() );
		for(size_t i = 0; i < output->getStepCount(); ++i)
			mStepInstances[i] = BLADE_NEW RenderStepInstance( output->getRenderStep(i) );
	}

	//////////////////////////////////////////////////////////////////////////
	RenderPhase::~RenderPhase()
	{
		for(size_t i = 0; i < mStepInstances.size(); ++i)
		{
			BLADE_DELETE mStepInstances[i];
		}
	}

	/************************************************************************/
	/* IGraphicsPhase interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	const TString&	RenderPhase::getName() const
	{
		return mOutput->getName();
	}

	//////////////////////////////////////////////////////////////////////////
	bool			RenderPhase::isEnabled() const
	{
		return mEnabled && mOutput->isEnabled();
	}

	//////////////////////////////////////////////////////////////////////////
	void			RenderPhase::setEnable(bool enabled)
	{
		mEnabled = enabled;
		mOutput->setEnable(enabled);
	}

	//////////////////////////////////////////////////////////////////////////
	const TString&	RenderPhase::getCameraName() const
	{
		return (mOutput != NULL && mOutput->getDesc().mCameraDesc.mCameraRef == CR_BYNAME) ? mOutput->getDesc().mCameraDesc.mCameraName : TString::EMPTY;
	}

	//////////////////////////////////////////////////////////////////////////
	IGraphicsView*	RenderPhase::getView() const
	{
		return mOutput != NULL ? mOutput->getView() : NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	IRenderTarget*	RenderPhase::getTarget() const
	{
		return mOutput != NULL ? mOutput->getTarget() : NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t			RenderPhase::getStepCount() const
	{
		return mOutput != NULL ? mOutput->getStepCount() : 0;
	}

	//////////////////////////////////////////////////////////////////////////
	const RenderType* RenderPhase::getStepRenderType(index_t step) const
	{
		if (mOutput == NULL)
			return NULL;
		RenderStep* renderStep = mOutput->getRenderStep(step);
		return renderStep->getType();
	}
	
}//namespace Blade