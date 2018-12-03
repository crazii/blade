/********************************************************************
	created:	2015/11/23
	filename: 	EditorHelper.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <EditorHelper.h>
#include <interface/IStage.h>
#include <interface/IStageConfig.h>
#include <interface/public/graphics/IGraphicsService.h>
#include <interface/public/graphics/IGraphicsScene.h>
#include <interface/public/graphics/IGraphicsInterface.h>
#include <interface/public/graphics/GraphicsInterface_blang.h>
#include <interface/public/geometry/IGeometryService.h>
#include <interface/public/geometry/IGeometryScene.h>

#include <interface/IEditorFramework.h>
#include <interface/public/EditableBase.h>
#include <interface/EditorElement.h>
#include "interface_imp/EditorScene.h"
#include "interface_imp/LightEditable.h"

namespace Blade
{
	class DummyEditable : public EditableBase, public Allocatable
	{
	public:
		DummyEditable(const TString& name, IStage* stage, bool deletable = false)
			:EditableBase(name, BTString("DummyEditable"))
			,mStage(stage)
		{
			mFlag |= EDF_MOVABLE|EDF_ROTATABLE|EDF_SELECTABLE;
			if( deletable )
				mFlag |= EDF_DELETABLE;
		}

		~DummyEditable()
		{
			BLADE_TS_VERIFY(TS_MAIN_SYNC);
			mStage->destroyEntity( mEditorElement->getEntity() );
		}

		/** @copydoc IEditable::initailize  */
		virtual bool			initialize(const IConfig* /*creationConfig*/,const ParamList* /*additionalParams = NULL*/)
		{
			return false;
		}

		/** @copydoc IEditable::initailize  */
		virtual bool			initialize(EditorElement* element)
		{
			mEditorElement = element;
			mEditorElement->setEditable(this);
			return true;
		}

		/** @copydoc IEditable::save  */
		virtual bool			save()
		{
			return false;
		}

	protected:
		IStage*	mStage;	///linkage
	};

	//////////////////////////////////////////////////////////////////////////
	IEditable* EditorHelper::createDummyEditable(IStage* stage, IGraphicsScene* graphicsScene, IGeometryScene* geomScene,
		const TString& editableName, const TString& editableElement/* = EditorElement::EDITOR_ELEMENT_TYPE*/)
	{
		if(stage == NULL)
		{
			assert(false);
			return NULL;
		}

		IScene* editorScene = stage->getScene( EditorScene::EDITOR_SCENE_TYPE );
		if( editorScene == NULL )
		{
			assert(false);
			return NULL;
		}

		HELEMENT editorElement = HELEMENT(editorScene->createElement(editableElement));
		if( editorElement == NULL )
		{
			assert(false);
			return NULL;
		}
		
		IEntity* entity = stage->createEntity(editableName);
		entity->addElement(IEditorFramework::EDITOR_ELEMENT_PREFIX + editableName, editorElement);

		if( graphicsScene != NULL )
		{
			HELEMENT graphicsElement = HELEMENT(graphicsScene->createElement(GraphicsConsts::EMPTY_ELEMENT_TYPE) );
			entity->addElement(IEditorFramework::EDITOR_ELEMENT_PREFIX + editableName + BTString("_Graphics"), graphicsElement);
		}

		if( geomScene != NULL )
		{
			HELEMENT geomElement = HELEMENT(geomScene->createElement(GeomConsts::GEOMETRY_ELEMENT) );
			entity->addElement(IEditorFramework::EDITOR_ELEMENT_PREFIX + editableName + BTString("_Geometry"), geomElement);
		}


		DummyEditable* editable = BLADE_NEW DummyEditable(editableName, stage);
		editable->initialize( static_cast<EditorElement*>(editorElement) );

		stage->loadEntity(entity);
		return editable;
	}

	//////////////////////////////////////////////////////////////////////////
	IEntity* EditorHelper::createLightEntity(IStage* stage, IGraphicsScene* graphicsScene, IGeometryScene* geomScene, const TString& entityName)
	{
		IEntity* entity = stage->createEntity(entityName);

		if (graphicsScene != NULL)
		{
			HELEMENT graphicsElement = HELEMENT(graphicsScene->createElement(BTString(BLANG_LIGHT_ELEMENT)));
			entity->addElement(entityName + BTString("_GraphicsLight"), graphicsElement);
		}

		if (geomScene != NULL)
		{
			HELEMENT geomElement = HELEMENT(geomScene->createElement(GeomConsts::GEOMETRY_ELEMENT));
			entity->addElement(/*IEditorFramework::EDITOR_ELEMENT_PREFIX + */entityName + BTString("_Geometry"), geomElement);
		}
		return entity;
	}

	//////////////////////////////////////////////////////////////////////////
	IEditable* EditorHelper::createLightEditable(IStage* stage, IGraphicsScene* graphicsScene, IGeometryScene* geomScene, const TString& entityName)
	{
		if(stage == NULL)
		{
			assert(false);
			return NULL;
		}

		IScene* editorScene = stage->getScene( EditorScene::EDITOR_SCENE_TYPE );
		if( editorScene == NULL )
		{
			assert(false);
			return NULL;
		}

		IEntity* entity = EditorHelper::createLightEntity(stage, graphicsScene, geomScene, entityName);

		HELEMENT editorElement = HELEMENT(editorScene->createElement(EditorElement::EDITOR_ELEMENT_TYPE));
		entity->addElement(IStageConfig::ELEMENT_SKIP_PREFIX + entityName + BTString("_EditorElement"), editorElement);

		LightEditable* editable = BLADE_NEW LightEditable(/*stage*/);
		editable->initialize( static_cast<EditorElement*>(editorElement) );
		return editable;
	}
	
}//namespace Blade
