/********************************************************************
	created:	2017/2/27
	filename: 	ModelEditable.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include "ModelEditable.h"
#include <BladeModelEditor.h>
#include <utility/StringList.h>
#include <utility/StringHelper.h>
#include <math/Quaternion.h>
#include <interface/IPage.h>
#include <interface/IEditorFramework.h>
#include <interface/IWorldEditableManager.h>
#include <interface/IStageConfig.h>
#include <BladeModel.h>
#include <interface/public/graphics/IGraphicsInterface.h>
#include <BladeModel_blang.h>


namespace Blade
{
	const TString ModelEditable::MODEL_EDITABLE_CLASS_NAME = BTString("ModelEditable");

	namespace Impl
	{
		//////////////////////////////////////////////////////////////////////////
		TString generateEntityNameBase(const TString& modelResource)
		{
			TString base;
			{
				TString dir, filename, ext;
				TStringHelper::getFilePathInfo(modelResource, dir, filename, base, ext);
			}
			return base;
		}
	}
	

	//////////////////////////////////////////////////////////////////////////
	ModelEditable::ModelEditable()
		:EditableBase(ModelEditorConsts::MODEL_EDITABLE_NAME, MODEL_EDITABLE_CLASS_NAME)
	{
		mFlag = EDF_SELECTABLE | EDF_DELETABLE | EDF_MOVABLE | EDF_ROTATABLE | EDF_SCALABLE | EDF_PAGED;
	}

	//////////////////////////////////////////////////////////////////////////
	ModelEditable::~ModelEditable()
	{

	}

	/************************************************************************/
	/* IEditable interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool			ModelEditable::initialize(const IConfig* /*creationConfig*/, const ParamList* additionalParams/* = NULL*/)
	{
		assert(additionalParams != NULL 
			&& (*additionalParams)[ModelEditorConsts::ENTITY_FILE_PARAM].isValid()
			&& (*additionalParams)[ModelEditorConsts::ENTITY_POS_PARAM].isValid()
			&& (*additionalParams)[ModelEditorConsts::ENTITY_ROTATION_PARAM].isValid()
			&& (*additionalParams)[ModelEditorConsts::ENTITY_SCALE_PARAM].isValid()
			&& (*additionalParams)[ModelEditorConsts::MODEL_RESOURCE_PARAM].isValid());

		IEditorFile* file = static_cast<IEditorFile*>((void*)(*additionalParams)[ModelEditorConsts::ENTITY_FILE_PARAM]);
		const Vector3& pos = (*additionalParams)[ModelEditorConsts::ENTITY_POS_PARAM];
		const Quaternion& rotation = (*additionalParams)[ModelEditorConsts::ENTITY_ROTATION_PARAM];
		const Vector3& scale = (*additionalParams)[ModelEditorConsts::ENTITY_SCALE_PARAM];
		const TString& modelResource = (*additionalParams)[ModelEditorConsts::MODEL_RESOURCE_PARAM];

		ILogicWorld* pWorld = IWorldEditableManager::getInterface().getWorld();
		//add page suffix to avoid paging conflict (unloaded pages may already have the entity of the same name)
		IPage* page = pWorld->getStage()->getPagingManager()->getPageByPosition(pos.x, pos.z);
		assert(page != NULL);
		TString name = file->getValidEntityName(Impl::generateEntityNameBase(modelResource), EditableBase::generatePageSuffix(page));
		IEntity* entity = pWorld->getStage()->createEntity(name);

		IElement* geomElem = pWorld->getGeometryScene()->createElement(GeomConsts::GEOMETRY_ELEMENT);
		//don't save geometry in editor
		entity->addElement(IEditorFramework::EDITOR_ELEMENT_PREFIX + BTString("geometry"), HELEMENT(geomElem));

		HELEMENT model = pWorld->getGraphicsScene()->createGraphicsElement(ModelConsts::MODEL_ELEMENT_TYPE);
		model->setResourcePath(modelResource);
		entity->addElement(BTString("model"), model);

		this->createEditorElement(EditorElement::EDITOR_ELEMENT_TYPE);
		entity->addElement(IStageConfig::ELEMENT_SKIP_PREFIX + EditorElement::EDITOR_ELEMENT_NAME, HELEMENT(mEditorElement));

		//set position
		IGeometry* geom = geomElem->getInterface(IID_GEOMETRY());
		assert(geom != NULL);
		geom->setGlobalPosition(pos);
		geom->setGlobalRotation(rotation);
		geom->setGlobalScale(scale);

		//load entity before adding to page: bounds info only available after loading mesh
		pWorld->getStage()->loadEntity(entity);

		pWorld->getStage()->getPagingManager()->notifyEntity(entity, true, false);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			ModelEditable::initialize(EditorElement* element)
	{
		this->setEditorElement(element);

		ILogicWorld* pWorld = IWorldEditableManager::getInterface().getWorld();
		IScene* scene = pWorld->getGeometryScene();
		//create geometry for editor
		IElement* geomElem = scene->createElement(GeomConsts::GEOMETRY_ELEMENT);

		//in real time playing, there's no geometry element for static models, so graphics element's position/scale/rotation is used
		//this newly added geometry element is used for editor only, it has higher priority but has initial zero position/scale/rotation
		//set geometry data from graphics
		IGraphicsInterface* model = element->getEntity()->getInterface(IID_GRAPHICS());
		IGeometry* geom = geomElem->getInterface(IID_GEOMETRY());
		geom->setGlobalPosition(model->getStaticPosition());
		geom->setGlobalRotation(model->getStaticRotation());
		geom->setGlobalScale(model->getStaticScale());

		//don't save geometry
		element->getEntity()->addElement(IEditorFramework::EDITOR_ELEMENT_PREFIX + BTString("geometry"), HELEMENT(geomElem));

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			ModelEditable::save()
	{
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	const TString	ModelEditable::getBaseName() const
	{
		return Impl::generateEntityNameBase(this->getTargetEntity()->getElementByType(ModelConsts::MODEL_ELEMENT_TYPE)->getResourcePath());
	}

}//namespace Blade