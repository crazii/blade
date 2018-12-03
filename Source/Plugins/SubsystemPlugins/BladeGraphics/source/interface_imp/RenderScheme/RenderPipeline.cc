/********************************************************************
	created:	2013/04/28
	filename: 	RenderPipeline.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "RenderPipeline.h"
#include <interface/IRenderTypeManager.h>

namespace Blade
{

	//////////////////////////////////////////////////////////////////////////
	RenderPipeline::RenderPipeline(IRenderScheme* prototype)
		:mPrototype(prototype)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	RenderPipeline::~RenderPipeline()
	{
		for(size_t i = 0; i < mChannels.size(); ++i)
		{
			BLADE_DELETE mChannels[i];
		}

		for(size_t i = 0; i < mPhases.size(); ++i)
		{
			BLADE_DELETE mPhases[i];
		}
	}

	/************************************************************************/
	/* IRenderPipeline interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	IRenderScheme*		RenderPipeline::getPrototype() const
	{
		return mPrototype;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t				RenderPipeline::getChannelCount() const
	{
		return mChannels.size();
	}

	//////////////////////////////////////////////////////////////////////////
	IGraphicsChannel*	RenderPipeline::getChannel(index_t index) const
	{
		if( index >= mChannels.size() )
			return NULL;
		else
			return mChannels[index];
	}

	//////////////////////////////////////////////////////////////////////////
	struct FnChannelFind
	{
		FnChannelFind(const TString& name) :mNameRef(name)	{}

		bool operator()(const RenderChannel* target) const
		{
			return target->getRenderType() == mNameRef;
		}

		const TString& mNameRef;
	private:
		FnChannelFind& operator=(const FnChannelFind&)	{return *this;}
	};
	IGraphicsChannel*	RenderPipeline::getChannel(const TString& channelRenderType) const
	{
		ChannelList::const_iterator i = std::find_if( mChannels.begin(), mChannels.end(), FnChannelFind(channelRenderType) );
		if( i == mChannels.end() )
			return NULL;
		else
			return *i;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t				RenderPipeline::getPhaseCount() const
	{
		return mPhases.size();
	}

	//////////////////////////////////////////////////////////////////////////
	IGraphicsPhase*		RenderPipeline::getPhase(index_t index) const
	{
		if( index >= mPhases.size() )
			return NULL;
		else
			return mPhases[index];
	}

	//////////////////////////////////////////////////////////////////////////
	struct FnPhaseFind
	{
		FnPhaseFind(const TString& name) :mNameRef(name)	{}

		bool operator()(const RenderPhase* target) const
		{
			return target->getName() == mNameRef;
		}

		const TString& mNameRef;
	private:
		FnPhaseFind& operator=(const FnPhaseFind&)	{return *this;}
	};
	IGraphicsPhase*		RenderPipeline::getPhase(const TString& phaseName) const
	{
		PhaseList::const_iterator i = std::find_if( mPhases.begin(), mPhases.end(), FnPhaseFind(phaseName) );
		if( i == mPhases.end() )
			return NULL;
		else
			return *i;
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void				RenderPipeline::initialize(RenderOutput** outputList, size_t count)
	{
		mChannels.resize( IRenderTypeManager::getSingleton().getRenderTypeCount() );
		for(size_t i = 0; i < mChannels.size(); ++i)
			mChannels[i] = BLADE_NEW RenderChannel( IRenderTypeManager::getSingleton().getRenderType(i)->getName() );

		mPhases.resize(count);
		for(size_t i = 0; i < count; ++i)
		{
			RenderOutput* output = outputList[i];
			mPhases[i] = BLADE_NEW RenderPhase( output );

			for(size_t j = 0; j < mChannels.size(); ++j)
				mChannels[j]->gatherRenderSteps( mPhases[i] );
		}
	}

}//namespace Blade