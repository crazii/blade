/********************************************************************
	created:	2011/05/05
	filename: 	WorldEditorCommand.h
	author:		Crazii
	purpose:	merge two class : WorldEditor & WorldEditableManager
				and move the comamnd def from WorldEditor.cc to here
*********************************************************************/
#ifndef __Blade_WorldEditorCommand_h__
#define __Blade_WorldEditorCommand_h__
#include <interface/public/ui/IUICommand.h>

namespace Blade
{
	class WorldObjectCreateCommand : public IUICommand
	{
	public:
		virtual void execute(ICommandUI* cui);
	};//class WorldObjectCreateCommand

	class WorldObjectGlobalConfigCommand : public IUICommand
	{
	public:
		virtual void execute(ICommandUI* cui);
	};//class WorldObjectGlobalConfigCommand

}//namespace Blade



#endif // __Blade_WorldEditorCommand_h__
