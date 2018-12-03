/********************************************************************
	created:	2011/04/22
	filename: 	EditorCommand.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_EditorCommand_h__
#define __Blade_EditorCommand_h__
#include <interface/public/ui/IUICommand.h>
#include <Singleton.h>
#include <utility/FixedArray.h>
#include <utility/Version.h>

namespace Blade
{

	enum EUICommand
	{
		UIC_NEW = 0,
		UIC_OPEN,
		UIC_SAVE,
		UIC_SAVE_ALL,
		UIC_EXIT,
		UIC_EDIT_ENGINECFG,
		UIC_VIEWMODE,		//solid, wire frame...
		UIC_VIEWMAX,		//maximize view port
		UIC_VIEW_REALTIME,	//real-time switch
		UIC_VIEW_CHANNEL,	//per-render type setting
		UIC_VIEW_PHASE,		//output selection
		UIC_VIEW_WIDGET,	//toggle widget
		UIC_VIEW_RESETLAYOUT, //reset layout
		
		UIC_HELP_ABOUT,

		UIC_COUNT,
	};

	class IEditorTool;

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class EditorCommandManager : public Singleton<EditorCommandManager>
	{
	public:
		EditorCommandManager();
		~EditorCommandManager();

		/** @brief  */
		void		initialize(Version ver);

		/** @brief  */
		void		shutdown();

		/** @brief  */
		IUICommand*	getEditorCommand(EUICommand eCmd) const;

		/** @brief command tools */
		IUICommand*	addToolCommand(IEditorTool* tool);

		/** @brief for IConfig menu command. see @IMenu::addSubMenu(IConfig* config, IUICommand* subCmd, ...) */
		IUICommand*	addConfigCommand(IConfig* cfg);

	protected:
		typedef FixedArray<IUICommand*,UIC_COUNT>	EditorCmdList;
		typedef List<IUICommand*>	ToolCmdList;

		EditorCmdList	mCmdList;
		ToolCmdList		mDynamicCmdList;
	};

}//namespace Blade



#endif // __Blade_EditorCommand_h__