/********************************************************************
	created:	2013/11/17
	filename: 	IViewportWindow.h
	author:		Crazii
	purpose:	interface for UI window with view ports
*********************************************************************/
#ifndef __Blade_IViewportWindow_h__
#define __Blade_IViewportWindow_h__
#include <interface/public/ui/IUIToolBox.h>
#include <interface/public/ui/UIWindowBase.h>
#include <interface/public/ui/IViewport.h>

namespace Blade
{
	/// window with multiple view ports
	class BLADE_FRAMEWORK_API IViewportWindow : public UIWindowBase
	{
	public:
		static const TString VIEWPORT_WINDOW_TYPE;
	public:
		IViewportWindow() :UIWindowBase(IViewportWindow::VIEWPORT_WINDOW_TYPE) {}
		virtual ~IViewportWindow() {}

		/**
		@describe
		@param
		@return
		*/
		virtual index_t		getCurrentViewportLayout() const = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual bool		setCurrentViewportLayout(index_t index) = 0;

		/**
		@describe get window's view port set, DO NOT holding this pointer since it may change
		@param
		@return
		*/
		virtual IViewportSet*	getViewportSet() const = 0;

		/**
		@describe maximize/restore the active view port
		@param
		@return
		*/
		virtual bool		maximizeViewport() = 0;

		/**
		@describe whether active view port is maximized
		@param
		@return
		*/
		virtual bool		isViewportMaximized(index_t index) const = 0;

		/**
		@describe get mouse for specific view port
		@param
		@return
		*/
		virtual IMouse*		getViewportMouse(index_t index) const = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual IKeyboard*	getViewportKeyboard(index_t index) const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void		updateViewportLayout() = 0;

		/** @brief get the view port camera */
		inline virtual CameraActor*	getCamera() const
		{
			return this->getViewportSet()->getActiveViewport()->getCameraActor();
		}

		/** @brief get the view port view */
		inline virtual IGraphicsView*getView() const
		{
			return this->getViewportSet()->getActiveViewport()->getView();
		}
	};

	///real toolbox of default window, for view ports
	class IViewportToolboxSet : public IUIToolBox
	{
	public:
		/** @brief  */
		virtual ~IViewportToolboxSet() {}

		/**
		@describe
		@param
		@return
		*/
		virtual IUIToolBox*	getViewportToolbox(index_t index) const = 0;
	};
	
}//namespace Blade

#endif //  __Blade_IViewportWindow_h__