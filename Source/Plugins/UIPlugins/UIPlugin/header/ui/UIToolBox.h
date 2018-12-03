/********************************************************************
	created:	2011/05/07
	filename: 	UIToolBox.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_UIToolBox_h__
#define __Blade_UIToolBox_h__
#include <interface/public/ui/IUIToolBox.h>
#include <ui/private/UIToolBar.h>

namespace Blade
{
	class UIToolBox : public IUIToolBox , public Allocatable
	{
	public:
		UIToolBox(const TString& name, ICONSIZE is, UINT nID, CFrameWnd* pParent, size_t existingWidth, 
			bool autoDock = true, bool gripper = true, CFrameWnd* Owner = NULL);
		~UIToolBox();

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
		@param
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
		virtual ICONSIZE		getIconSize()	const	{return mIconSize;}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			setIconSize(ICONSIZE is);

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		CToolBar*	getToolbar()	{return &mToolbar;}

		/** @brief  */
		const TString& getName() const {return mToolbar.getName();}

		/** @brief  */
		size_t		getWidth() const;

		/** @brief  */
		CToolBarCtrl* getToolbarCtrl() {return &(mToolbar.GetToolBarCtrl());}

		/** @brief  */
		const CToolBarCtrl* getToolbarCtrl() const {return &(mToolbar.GetToolBarCtrl());}

	protected:
		::CFrameWnd*			mParent;	//outer link
		::CUIToolBar			mToolbar;
		ICONSIZE				mIconSize;
		Set<CUIToolBar::Button>	mButtons;
		bool					mAutoDock;
	};

}//namespace Blade





#endif // __Blade_UIToolBox_h__