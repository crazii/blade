/********************************************************************
	created:	2010/04/24
	filename: 	IGraphicsView.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_IGraphicsView_h__
#define __Blade_IGraphicsView_h__
#include <interface/public/graphics/IRenderView.h>
#include <interface/public/graphics/IGraphicsChannel.h>
#include <interface/public/graphics/IGraphicsPhase.h>


namespace Blade
{
	class IRenderScheme;

	class IGraphicsView : public IRenderView
	{
	protected:
		///not delete able by client
		///see @IGraphicsService::destroyView
		virtual ~IGraphicsView()	{}
	public:
		

		/**
		@describe
		@param
		@return
		*/
		virtual const TString&		getRenderScheme() const = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual void				setRenderScheme(const TString& schemeName) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual size_t				getChannelCount() const = 0;

		/**
		@describe get the render queues appended
		@param
		@return
		*/
		virtual IGraphicsChannel*	getChannel(index_t index) const = 0;

		/**
		@describe get the render queues appended
		@param
		@return
		*/
		virtual IGraphicsChannel*	getChannel(const TString& channelRenderType) const = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual size_t				getPhaseCount() const = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual IGraphicsPhase*		getPhase(index_t index) const = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual IGraphicsPhase*		getPhase(const TString& phaseName) const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void				setFillMode(FILL_MODE mode) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual FILL_MODE			getFillMode() const = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual void				enableImageEffect(bool enable) = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual bool				isImageEffectEnabled() const = 0;

	};//class IGraphicsView
	
}//namespace Blade


#endif //__Blade_IGraphicsView_h__