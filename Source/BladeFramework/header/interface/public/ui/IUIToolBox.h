/********************************************************************
	created:	2010/05/23
	filename: 	IUIToolBox.h
	author:		Crazii
	purpose:	tool box (tool bar) interface to add tools (button, config control etc.)
				there are 2 way to add tools:
				1.create a menu and add tool button bound to the menu
				2.create a config interface and add config tool bound to the config
*********************************************************************/
#ifndef __Blade_IUIToolBox_h__
#define __Blade_IUIToolBox_h__
#include <interface/IConfig.h>
#include "IIconManager.h"

namespace Blade
{
	class IUICommand;
	class IMenu;

	class IUIToolBox
	{
	public:
		typedef enum EToolButtonStyle
		{
			TBS_NONE		= 0x00,	//default button
			TBS_CHECK		= 0x01,	//checkable button
			TBS_CHECKGROUP	= 0x02,	//checkable for non-embedded grouped menu, default button for non-group menu
			TBS_DROPDOWN	= 0x04,	//drop down for grouped menu
			TBS_TEXT		= 0x08,	//show text for grouped menu
		}GROUP_STYLE;
	public:
		virtual ~IUIToolBox() {}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const TString&	getName() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool			addButton(IUICommand* cmd, const TString& name, IconIndex icon, int style = TBS_NONE) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool			addButton(IMenu* menu, int style = TBS_CHECKGROUP) = 0;

		/**
		@describe add button for config
		@param [in] width: width of the tool, -1 means using default
		@return 
		*/
		virtual bool			addTool(IConfig* config, int width = -1) = 0;

		/**
		@describe 
		@param [in] config: group config with specified config list
		@return 
		*/
		virtual bool			addToolSet(IConfig* config, IconIndex icon) = 0;

		/**
		@describe update tool data from config
		@param
		@return
		*/
		virtual void			updateTools() = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual size_t			getButtonCount() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual ICONSIZE		getIconSize()	const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool			setIconSize(ICONSIZE is) = 0;
	};

	
}//namespace Blade


#endif //__Blade_IUIToolBox_h__