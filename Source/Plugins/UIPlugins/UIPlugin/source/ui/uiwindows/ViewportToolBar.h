/********************************************************************
	created:	2013/04/25
	filename: 	ViewportToolBar.h
	author:		Crazii
	purpose:	tool bars set for view ports of a view port set
*********************************************************************/
#ifndef __Blade_ViewportToolBar_h__
#define __Blade_ViewportToolBar_h__
#include <interface/public/ui/uiwindows/IViewportWindow.h>

namespace Blade
{
	class UIToolBox;

	class ViewportToolBar : public IViewportToolboxSet, public Allocatable
	{
	public:
		ViewportToolBar(const TString& name, size_t viewportCount, size_t existingWidth, ::CFrameWnd* viewPort[]);
		~ViewportToolBar();

		/************************************************************************/
		/* IUIToolBox interface                                                                     */
		/************************************************************************/
		/**
		@describe
		@param
		@return
		*/
		virtual bool			addButton(IUICommand* cmd, const TString& name, IconIndex icon, int style = TBS_NONE);

		/*
		@describe 
		@param [in] groupStyle set the style for none-embedded grouped menu
		@return 
		*/
		virtual bool			addButton(IMenu* menu, int style = TBS_CHECKGROUP);

		/*
		@describe 
		@param [in] width: width of the tool, -1 means using default
		@return 
		*/
		virtual bool			addTool(IConfig* config, int width = -1);

		/*
		@describe
		@param [in] config: group config with specified config list
		@return
		*/
		virtual bool			addToolSet(IConfig* config, IconIndex icon);

		/*
		@describe update tool data from config
		@param
		@return
		*/
		virtual void			updateTools();

		/*
		@describe 
		@param 
		@return 
		*/
		virtual size_t			getButtonCount() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual ICONSIZE		getIconSize()	const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			setIconSize(ICONSIZE is);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual IUIToolBox*	getViewportToolbox(index_t index) const;

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		size_t					getWidth() const;
		/** @brief  */
		const TString&			getName() const;

		//view port tool bars
		UIToolBox*	mViewportToolBox[MAX_VIEWPORT_COUNT];
		size_t		mViewportCount;
	};//class ViewportToolBar
	
}//namespace Blade


#endif //  __Blade_ViewportToolBar_h__