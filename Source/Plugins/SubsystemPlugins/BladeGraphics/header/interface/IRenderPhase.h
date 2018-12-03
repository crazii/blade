/********************************************************************
	created:	2017/7/20
	filename: 	IRenderPhase.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_IRenderPhase_h__
#define __Blade_IRenderPhase_h__
#include <interface/public/graphics/IGraphicsPhase.h>

namespace Blade
{
	class RenderType;

	class IRenderPhase : public IGraphicsPhase
	{
	public:
		/**
		@describe custom camera bound to phase, usually could be EMPTY. this is the "camera" def node of "render_output" in render scheme file.
		@param
		@return
		*/
		virtual const TString&	getCameraName() const = 0;

		/**
		@describe custom view bound to phase. this is the "view" def node of "render_output" in render scheme file.
		@param
		@return
		*/
		virtual IGraphicsView*	getView() const = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual IRenderTarget*	getTarget() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual size_t			getStepCount() const = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual const RenderType* getStepRenderType(index_t step) const = 0;
	};
	
}//namespace Blade


#endif//__Blade_IRenderPhase_h__