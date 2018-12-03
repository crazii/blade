/********************************************************************
	created:	2010/05/23
	filename: 	BladeEditor.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <interface/public/IEditorFile.h>
#include <interface/public/IEditable.h>

//export template only
#include <interface/EditorElement.h>
#include <BladeEditor_blang.h>
#include <databinding/DataSourceWriter.h>
#include <interface/IEventManager.h>
#include <interface/public/ui/UIEvents.h>
#include <interface/EditorEvents.h>

#include "interface_imp/EditorFramework.h"
#include "interface_imp/EditorToolManager.h"
#include "interface_imp/EditorSystem.h"
#include "interface_imp/EditorConfigManager.h"
#include "interface_imp/EditorLogic.h"
#include "interface_imp/ImporterManager.h"
#include "interface_imp/LightEditable.h"

namespace Blade
{
	/************************************************************************/
	/* template instantiations                                                                     */
	/************************************************************************/
	template class Factory<IEditable>;
	template class Factory<IEditorFile>;
	template class Factory<IEditorLogic>;
	template class Factory<EditorElement>;

	const TString TransformSpaceEvent::NAME = BTString("Editor::TransformSpaceChanged");
	const TString EditorFileSwitchEvent::NAME = BTString("Editor::CurrentFileSwitched");
	const TString EditableUnLoadEvent::NAME = BTString("Editor::EditableUnlaoded");
	const TString EntityRenameEvent::NAME = BTString("Editor::EntityNameChanged");

	const TString LightEditable::LIGHT_EDITABLE_TYPE = BTString("LightEditable");
	const TString LightEditable::LIGHT_EDITABLE_NAME = BTString("Light");
	
	//////////////////////////////////////////////////////////////////////////
	bool initializeEditorFramework()
	{
		RegisterSingleton(EditorFramework,IEditorFramework);
		RegisterSingleton(EditorToolManager,IEditorToolManager);
		RegisterSingleton(EditorConfigManager,IEditorConfigManager);
		RegisterSingleton(ImporterManager,IImporterManager);

		RegisterFactory(EditorElement,EditorElement);
		NameRegisterSingleton(EditorSystem, ISubsystem, EditorSystem::EDITOR_SYSTEM_NAME);
		NameRegisterFactory(EditorLogic, IEditorLogic, EditorLogic::DEFAULT_TYPE);
		NameRegisterFactory(LightEditable, IEditable, LightEditable::LIGHT_EDITABLE_TYPE);

		//editor element config
		DataSourceWriter<EditorElement> writer( EditorElement::EDITOR_ELEMENT_TYPE );
		writer.beginVersion( Version(0,1) );
		writer << BXLang(BLANG_EDITABLE_CLASS) << CAF_SERIALIZE << &EditorElement::mEditableClass << CUIH_NONE;
		writer.endVersion();
		return true;
	}

}//namespace Blade