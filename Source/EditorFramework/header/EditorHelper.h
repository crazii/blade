/********************************************************************
	created:	2015/11/23
	filename: 	EditorHelper.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_EditorHelper_h__
#define __Blade_EditorHelper_h__
#include <BladeEditor.h>
#include <interface/public/IEditable.h>
#include <interface/public/IEditorFile.h>
#include <interface/EditorElement.h>

namespace Blade
{

	namespace EditorHelper
	{
		/** @brief create a dummy entity & editable with unite scale, you set the scale after it created */
		BLADE_EDITOR_API IEditable* createDummyEditable(IStage* stage, IGraphicsScene* graphicsScene, IGeometryScene* geomScene,
			const TString& editableName, const TString& editableElement = EditorElement::EDITOR_ELEMENT_TYPE);

		inline IEditable* createDummyEditable(IEditorFile* file,
			const TString& editableName, const TString& editableElement = EditorElement::EDITOR_ELEMENT_TYPE)
		{
			return createDummyEditable(file->getStage(), file->getGraphicsScene(), file->getGeometryScene(), editableName, editableElement);
		}


		/** @brief  */
		BLADE_EDITOR_API IEntity* createLightEntity(IStage* stage, IGraphicsScene* graphicsScene, IGeometryScene* geomScene, const TString& entityName);

		inline IEntity* createLightEntity(IEditorFile* file, const TString& entityName)
		{
			return createLightEntity(file->getStage(), file->getGraphicsScene(), file->getGeometryScene(), entityName);
		}

		/** @brief create a light entity with editable */
		BLADE_EDITOR_API IEditable* createLightEditable(IStage* stage, IGraphicsScene* graphicsScene, IGeometryScene* geomScene, const TString& entityName);

		inline IEditable* createLightEditable(IEditorFile* file, const TString& entityName)
		{
			return createLightEditable(file->getStage(), file->getGraphicsScene(), file->getGeometryScene(), entityName);
		}
	}

}//namespace Blade


#endif // __Blade_EditorHelper_h__
