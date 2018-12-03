/********************************************************************
	created:	2013/04/28
	filename: 	IRenderPipeline.h
	author:		Crazii
	purpose:	the render pipeline is a instance interface of a render scheme, used to access instances of render steps/render outputs
				each pipe line is bound to a view instance
*********************************************************************/
#ifndef __Blade_IRenderPipeline_h__
#define __Blade_IRenderPipeline_h__
#include <interface/public/graphics/IGraphicsPhase.h>
#include <interface/public/graphics/IGraphicsChannel.h>

namespace Blade
{
	class IRenderScheme;
	class IRenderPipeline
	{
	public:
		virtual ~IRenderPipeline() {}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual IRenderScheme*		getPrototype() const = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual size_t				getChannelCount() const = 0;

		/*
		@describe get the render queues appended
		@param
		@return
		*/
		virtual IGraphicsChannel*	getChannel(index_t index) const = 0;

		/*
		@describe get the render queues appended
		@param
		@return
		*/
		virtual IGraphicsChannel*	getChannel(const TString& channelRenderType) const = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual size_t				getPhaseCount() const = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual IGraphicsPhase*		getPhase(index_t index) const = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual IGraphicsPhase*		getPhase(const TString& phaseName) const = 0;

	};
	
}//namespace Blade

#endif //  __Blade_IRenderPipeline_h__