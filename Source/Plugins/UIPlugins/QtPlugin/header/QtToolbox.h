/********************************************************************
	created:	2016/07/23
	filename: 	QtToolbox.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_QtToolbox_h__
#define __Blade_QtToolbox_h__
#include <interface/public/ui/IUIToolBox.h>
#include <interface/public/ui/IUICommand.h>
#include <QtPropertyHelper.h>

namespace Blade
{
	class QtToolButton;
	class QtWindow;

	class QtToolbox : public IUIToolBox, public QToolBar, public Allocatable
	{
	public:
		QtToolbox(const TString& name, ICONSIZE is, QtWindow* parent);
		~QtToolbox();

		/************************************************************************/
		/* IUIToolBox interface                                                                     */
		/************************************************************************/
		/**
		@describe
		@param
		@return
		*/
		virtual const TString&	getName() const { return mName; }

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
		/*                                                                      */
		/************************************************************************/
		/** @brief  */
		void			updateUI();
		/** @brief  */
		virtual bool	eventFilter(QObject *watched, QEvent *evt);

	protected:
		typedef List<QtToolButton*> ButtonList;

		QtWindow*		mParent;
		ICONSIZE		mIconSize;
		TString			mName;
		ButtonList		mButtons;

		QtPropertyHelper	mConfigToolHelper;
	};
	
}//namespace Blade


#endif // __Blade_QtToolbox_h__