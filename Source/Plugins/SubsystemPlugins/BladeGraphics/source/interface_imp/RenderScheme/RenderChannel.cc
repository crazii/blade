/********************************************************************
	created:	2012/03/28
	filename: 	RenderChannel.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "RenderPhase.h"
#include "RenderChannel.h"

namespace Blade
{

	//////////////////////////////////////////////////////////////////////////
	RenderChannel::RenderChannel(const TString& renderTypeName)
		:mRenderType(renderTypeName)
		,mFillMode(GFM_SOLID)
		,mEnable(true)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	RenderChannel::~RenderChannel()
	{

	}

	/************************************************************************/
	/* IGraphicsChannel interface                                                                     */
	/************************************************************************/


	//////////////////////////////////////////////////////////////////////////
	const TString&		RenderChannel::getRenderType() const
	{
		return mRenderType;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				RenderChannel::isEnabled() const
	{
		return mEnable;
	}

	//////////////////////////////////////////////////////////////////////////
	void				RenderChannel::setEnable(bool enabled)
	{
		if( mEnable != enabled )
		{
			mEnable = enabled;
			for(StepInstanceList::iterator i = mRenderSteps.begin(); i != mRenderSteps.end(); ++i)
			{
				RenderStepInstance* step = *i;
				step->setEnable(enabled);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void				RenderChannel::setFillMode(FILL_MODE mode)
	{
		if( mFillMode != mode )
		{
			mFillMode = mode;
			for(StepInstanceList::iterator i = mRenderSteps.begin(); i != mRenderSteps.end(); ++i)
			{
				RenderStepInstance* step = *i;
				step->setFillMode(mode);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	FILL_MODE			RenderChannel::getFillMode() const
	{
		return mFillMode;
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void				RenderChannel::gatherRenderSteps(RenderPhase* phase)
	{
		if( phase != NULL )
		{
			for(size_t j = 0; j < phase->getStepInstanceCount(); ++j)
			{
				RenderStepInstance* stepInstance = phase->getStepInstance(j);
				if( stepInstance->getRenderStep()->getTypeName() == mRenderType )
				{
					mRenderSteps.push_back(stepInstance);
					break;
				}
			}
		}
	}

}//namespace Blade