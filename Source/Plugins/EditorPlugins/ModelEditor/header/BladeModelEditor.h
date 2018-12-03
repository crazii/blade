/********************************************************************
created:	2013/04/09
filename: 	BladeModelEditor.h
author:		Crazii
purpose:
*********************************************************************/
#ifndef __Blade_BladeModelEditor_h__
#define __Blade_BladeModelEditor_h__
#include <utility/String.h>
#include <BladeModel.h>
#include <interface/IModel.h>
#include <BladeModel_blang.h>

#if !defined(BLADE_STATIC)	//dynamic link lib

#	if BLADE_COMPILER == BLADE_COMPILER_MSVC
#		ifdef BLADE_MODELEDITOR_EXPORTS
#			define BLADE_MODELEDITOR_API __declspec( dllexport )
#		else
#			define BLADE_MODELEDITOR_API __declspec( dllimport )
#		endif
#	elif BLADE_COMPILER_GNU_EXTENSION
#		define BLADE_MODELEDITOR_API __attribute__ ((visibility("default")))
#	endif

#else

#	define BLADE_MODELEDITOR_API	//static link lib

#endif

namespace Blade
{
	namespace ModelEditorConsts
	{
		static const TString MODEL_EDITOR_CATEGORY = BTString(BLANG_MODEL);
		static const TString MODEL_EDITABLE_NAME = BTString(BLANG_MODEL);

		//media previewer
		static const TString MEDIA_PREVIEWER_MODEL = BTString("ModelPreviewer");

		//model creation parameters
		static const TString ENTITY_FILE_PARAM = BTString("FILE");
		static const TString ENTITY_POS_PARAM = BTString("POS");
		static const TString ENTITY_SCALE_PARAM = BTString("SCALE");
		static const TString ENTITY_ROTATION_PARAM = BTString("ROTATION");
		static const TString MODEL_RESOURCE_PARAM = BTString("MODEL");

	}//namespace ModelEditorConsts
	
}//namespace Blade

#endif //  __Blade_BladeModelEditor_h__