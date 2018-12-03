/********************************************************************
	created:	2016/6/8
	filename: 	EditorEvents.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_EditorEvents_h__
#define __Blade_EditorEvents_h__
#include <Event.h>
#include <interface/public/geometry/GeomTypes.h>
#include <BladeEditor.h>

namespace Blade
{
	///(built-in) transform space config change
	class TransformSpaceEvent : public Event
	{
	public:
		static const BLADE_EDITOR_API TString NAME;
		GEOM_SPACE mSpace;

		TransformSpaceEvent(GEOM_SPACE space) :mSpace(space)
		{
			mName = NAME;
		}
	};

	///common state for file switch besides IEditorTool's callback
	class IEditorFile;
	class EditorFileSwitchEvent : public Event
	{
	public:
		static const BLADE_EDITOR_API TString NAME;
		const IEditorFile* preFile;	//maybe NULL
		const IEditorFile* nextFile;	//maybe NULL
		EditorFileSwitchEvent(const IEditorFile* prev, const IEditorFile* next)
			:preFile(prev), nextFile(next)
		{
			mName = NAME;
		}
	};

	//editable unloaded
	class IEditable;
	class EditableUnLoadEvent : public Event
	{
	public:
		static const BLADE_EDITOR_API TString NAME;
		IEditable* editable;
		EditableUnLoadEvent(IEditable* _editable)
			:editable(_editable)
		{
			mName = NAME;
		}
	};

	///event when entity name changes.
	class EntityRenameEvent : public Event
	{
	public:
		static BLADE_EDITOR_API const TString	NAME;
		IEntity* mEntity;

		EntityRenameEvent(IEntity* entity) :Event(NAME), mEntity(entity) {}
	};
	
}//namespace Blade


#endif//__Blade_EditorEvents_h__