/********************************************************************
	created:	2010/04/14
	filename: 	IGraphicsService.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_IGraphicsService_h__
#define __Blade_IGraphicsService_h__
#include <interface/InterfaceSingleton.h>
#include <BladeFramework.h>
#include <interface/public/graphics/PixelFormat.h>
#include <interface/public/graphics/IGraphicsType.h>
#include <math/BladeMath.h>

namespace Blade
{
	class IWindow;
	class IGraphicsView;
	class IRenderTarget;
	class IRenderWindow;
	class IGraphicsScene;

	class BLADE_FRAMEWORK_API IGraphicsService : public InterfaceSingleton<IGraphicsService>
	{
	public:

		/**
		@describe 
		@param 
		@return 
		*/
		virtual IGraphicsView*	createView(const TString& name) = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual void			destroyView(const TString& name) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void			destroyView(IGraphicsView* view) = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual IGraphicsView*	getView(const TString& name) const = 0;

		/**
		@describe get the created render target
		@note use a window's unique name to get the window's default render target
		@param 
		@return 
		*/
		virtual IRenderTarget*	getRenderTarget(const TString& name) const = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual IRenderWindow*	createRenderWindow(IWindow* pwin) = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual IRenderTarget*	createRenderTarget(const TString& name,size_t width,size_t height,
			bool creatColorBuffer = false,bool createDepthBuffer = false,
			PixelFormat format = PF_UNDEFINED,PixelFormat depthFormat = PF_UNDEFINED) = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual void			destroyRenderTarget(const TString& name) = 0;

		/** @brief  */
		virtual void			destroyRenderTarget(IRenderTarget* target) = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual IGraphicsScene* createGraphicsScene(const TString& name) = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual IGraphicsScene*	getGraphicsScene(const TString& name) const = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual size_t			getGraphicsSceneCount() const = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual IGraphicsScene*	getGraphicsScene(index_t index) const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool			addGraphicsType(IGraphicsType* type) = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual size_t			getGraphicsTypeCount() const = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual IGraphicsType*	getGraphicsType(index_t index) const = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual IGraphicsType*	getGraphicsType(const TString& name) const = 0;

		/**
		@describe
		@param
		@return render type's app flag, if render type exist, otherwise return INVALID_APPFLAG
		@note: the app flag generated should be same for the same render type
		and should be the same for ALL WORLD instances
		*/
		virtual AppFlag			generateGraphicsTypeAppFlag(IGraphicsType& type) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual size_t			getSpaceTypes(TStringParam& outList) const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual size_t			getCoordinatorTypes(TStringParam& outList) const  = 0;

	};//class IGraphicsService

	extern template class BLADE_FRAMEWORK_API Factory<IGraphicsService>;
	
}//namespace Blade


#endif //__Blade_IGraphicsService_h__