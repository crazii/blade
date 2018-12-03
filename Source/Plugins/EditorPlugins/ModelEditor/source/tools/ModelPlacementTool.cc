/********************************************************************
	created:	2017/2/27
	filename: 	ModelPlacementTool.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include "ModelPlacementTool.h"
#include <utility/StringHelper.h>
#include <interface/IResourceManager.h>
#include <interface/IEditorFramework.h>
#include <interface/public/ui/IIconManager.h>
#include <interface/public/graphics/SpaceQuery.h>
#include <interface/public/graphics/IGraphicsCamera.h>
#include <interface/public/graphics/IGraphicsInterface.h>

namespace Blade
{
	const TString ModelPlacementTool::MODEL_INDICATOR_ENTITY = IEditorFramework::EDITOR_ENTITY_PREFIX + BTString("Editor_ModelEditorIndicatorHelper");
	const TString ModelPlacementTool::MODEL_PLACEMENT_TOOL_NAME = BTString("Model Placement");

	static const size_t DEF_DISTANCE = 60;

	//////////////////////////////////////////////////////////////////////////
	ModelPlacementTool::ModelPlacementTool()
		:EditorCommonTool(MODEL_PLACEMENT_TOOL_NAME,
			IIconManager::getSingleton().loadSingleIconImage(IEditorFramework::EDITOR_ICON_PATH + BTString("house.png")), KC_UNDEFINED)
		,mStage(NULL)
		,mHelperEntity(NULL)
		,mRandomRotation(false)
	{
		DataSourceWriter<ModelPlacementTool> writer(mToolConfig);
		writer.beginVersion(Version(0, 1));
		{
			writer << ModelConfigString::SELECTED_MODEL << CONFIG_UIHINT(CUIH_IMAGE, ModelConsts::MODEL_ANIMATION_ITEM_HINT) << &ModelPlacementTool::onConfigChange
				<< &ModelPlacementTool::mSelectedModel;

			writer << ModelConfigString::RANDOM_ROTATION << CUIH_CHECK << CAF_NORMAL
				<< &ModelPlacementTool::onConfigChange
				<< &ModelPlacementTool::mRandomRotation;

			writer << ModelConfigString::APP_FLAG << CUIH_CHECK << CAF_NORMAL
				<< &ModelPlacementTool::onConfigChange
				<< ModelToolBase::getIntersectionTypesAddr<ModelPlacementTool>();
		}
		writer.endVersion();
		mToolConfig->bindTarget(this);
	}

	//////////////////////////////////////////////////////////////////////////
	ModelPlacementTool::~ModelPlacementTool()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	bool				ModelPlacementTool::onMouseLeftClick(scalar x, scalar y, uint32 /*keyModifier*/, IEditorFile* file)
	{
		if (mHelperEntity == NULL)
			return false;
		assert(this->isEnable() && this->isSelected());

		Vector3 pos = this->pickPosition(x, y, mAppFlag, (scalar)DEF_DISTANCE, file);

		this->ModelToolBase::createModelEntity(file, mSelectedModel, pos, mRandomRotation);

		file->setModified();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				ModelPlacementTool::update(scalar x, scalar y, const POINT3& /*deltaMove*/, uint32 /*keyModifier*/, scalar /*deltaTime*/, IEditorFile* file)
	{
		if (mHelperEntity == NULL)
			return false;

		IElement* elem = mHelperEntity->getElementByType(GeomConsts::GEOMETRY_ELEMENT);
		if (elem == NULL)
			return false;
		IGeometry* geom = elem->getInterface(IID_GEOMETRY());
		Vector3 pos = this->pickPosition(x, y, mAppFlag, (scalar)DEF_DISTANCE, file);
		geom->setGlobalPosition(pos);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void		ModelPlacementTool::onFileSwitch(const IEditorFile* /*prevFile*/, const IEditorFile* currentFile)
	{
		//note: world editor file is unique.
		//just use the event to create helper
		if (mHelperEntity != NULL)
		{
			IGraphicsInterface* model = mHelperEntity->getInterface(IID_GRAPHICS());
			model->setVisible(false);
		}

		mStage = NULL;
		mHelperEntity = NULL;

		if (currentFile == NULL || currentFile->getType() != WORLD_FILE_TYPE)
			return;

		ILogicWorld* world = currentFile->getWorld();
		if (world == NULL)
			return;

		IStage* stage = currentFile->getStage();
		if (stage == NULL)
			return;

		this->ModelToolBase::updateAppFlag();

		mStage = stage;
		IGraphicsScene* graphicsScene = world->getGraphicsScene();
		if (graphicsScene == NULL)
			return;
		IGeometryScene* geomScene = world->getGeometryScene();
		if (geomScene == NULL)
			return;

		mHelperEntity = stage->getEntity(MODEL_INDICATOR_ENTITY);

		if (mHelperEntity == NULL)
		{
			mHelperEntity = stage->createEntity(MODEL_INDICATOR_ENTITY);
			HELEMENT hElem = graphicsScene->createGraphicsElement(ModelConsts::MODEL_ELEMENT_TYPE);
			hElem->setResourcePath(mSelectedModel);
			mHelperEntity->addElement(BTString("model"), hElem);
			IElement* geomElem = geomScene->createElement(GeomConsts::GEOMETRY_ELEMENT);
			mHelperEntity->addElement(BTString("geometry"), HELEMENT(geomElem));
			stage->loadEntitySync(mHelperEntity);
		}
		else
			this->checkEntityResource();

		IGraphicsInterface* model = mHelperEntity->getInterface(IID_GRAPHICS());
		model->setVisible( this->isSelected() );
		model->enablePicking(false);
	}

	//////////////////////////////////////////////////////////////////////////
	void				ModelPlacementTool::onEnable(bool bEnabled)
	{
		if (mHelperEntity != NULL)
		{
			IGraphicsInterface* model = mHelperEntity->getInterface(IID_GRAPHICS());
			model->setVisible(bEnabled && this->isSelected());
		}
		//delay init app flag on current world
		if (bEnabled)
		{
			this->ModelToolBase::initAppFlag();
		}
	}
	
	//////////////////////////////////////////////////////////////////////////
	void				ModelPlacementTool::onSelection()
	{
		if (mHelperEntity != NULL)
		{
			IGraphicsInterface* model = mHelperEntity->getInterface(IID_GRAPHICS());
			model->setVisible( this->isSelected() );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void	ModelPlacementTool::checkEntityResource()
	{
		if (mHelperEntity == NULL || mStage == NULL)
			return;

		IElement* elem = mHelperEntity->getElementByType(ModelConsts::MODEL_ELEMENT_TYPE);
		assert(elem != NULL);
		if (elem->getResourcePath() != mSelectedModel)
		{
			elem->unloadResource(true);
			elem->setResourcePath(mSelectedModel);
			mStage->loadEntitySync(mHelperEntity);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void	ModelPlacementTool::onConfigChange(void * data)
	{
		if (data == &mSelectedModel)
			this->checkEntityResource();
		else if (data == &mIntersectTypes)
		{
			this->ModelToolBase::updateAppFlag();
		}
		else if (data == &mRandomRotation)
		{

		}
	}

}//namespace Blade