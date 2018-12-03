/********************************************************************
	created:	2013/04/28
	filename: 	RenderPipeline.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_RenderPipeline_h__
#define __Blade_RenderPipeline_h__
#include <interface/IRenderPipeline.h>
#include <utility/BladeContainer.h>
#include "RenderPhase.h"
#include "RenderChannel.h"

namespace Blade
{
	class RenderOutput;

	class RenderPipeline : public IRenderPipeline, public Allocatable
	{
	public:
		typedef Vector<RenderPhase*>		PhaseList;
	public:
		RenderPipeline(IRenderScheme* prototype);
		~RenderPipeline();

		/************************************************************************/
		/* IRenderPipeline interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual IRenderScheme*		getPrototype() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual size_t				getChannelCount() const;

		/*
		@describe get the render queues appended
		@param
		@return
		*/
		virtual IGraphicsChannel*	getChannel(index_t index) const;

		/*
		@describe get the render queues appended
		@param
		@return
		*/
		virtual IGraphicsChannel*	getChannel(const TString& channelRenderType) const;

		/*
		@describe
		@param
		@return
		*/
		virtual size_t				getPhaseCount() const;

		/*
		@describe
		@param
		@return
		*/
		virtual IGraphicsPhase*		getPhase(index_t index) const;

		/*
		@describe
		@param
		@return
		*/
		virtual IGraphicsPhase*		getPhase(const TString& phaseName) const;

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		void				initialize(RenderOutput** outputList, size_t count);
		/** @brief  */
		const PhaseList&	getPhaseList() const	{return mPhases;}

	protected:
		typedef Vector<RenderChannel*>		ChannelList;

		ChannelList	mChannels;
		PhaseList	mPhases;
		IRenderScheme* mPrototype;
	};//class RenderPipeline
	
}//namespace Blade

#endif //  __Blade_RenderPipeline_h__