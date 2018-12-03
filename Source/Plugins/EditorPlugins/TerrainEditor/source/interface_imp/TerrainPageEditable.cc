/********************************************************************
	created:	2011/04/15
	filename: 	TerrainPageEditable.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "TerrainPageEditable.h"
#include <ExceptionEx.h>

#include <interface/public/logic/ILogicWorld.h>
#include <interface/IStageConfig.h>
#include <interface/IPage.h>

#include <interface/IEditorFramework.h>
#include <interface/IWorldEditableManager.h>
#include <interface/ITerrainConfigManager.h>
#include <interface/ITerrainInterface.h>
#include "TerrainEditable.h"
#include "TerrainToolManager.h"
#include <BladeTerrainEditor_blang.h>

namespace Blade
{
	const TString TerrainPageEditable::TERRAIN_PAGE_EDITABLE_NAME = BTString("TerrainPageEditable");

	//////////////////////////////////////////////////////////////////////////
	TerrainPageEditable::TerrainPageEditable()
		:EditableBase(BTLang(BLANG_TERRAIN), TERRAIN_PAGE_EDITABLE_NAME)
		,mPageEntity(NULL)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	TerrainPageEditable::~TerrainPageEditable()
	{
		TerrainToolManager::getSingleton().clearTerrainEditables();
	}

	/************************************************************************/
	/* IEditable interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool			TerrainPageEditable::initialize(const IConfig* creationConfig,const ParamList* /*additionalParams*//* = NULL*/)
	{
		//editor creation
		if( creationConfig == NULL)
			return false;

		ILogicWorld* pWorld = IWorldEditableManager::getInterface().getWorld();
		if( pWorld == NULL )
			return false;

		const IPagingManager::DESC& desc = pWorld->getStage()->getPagingManager()->getDesc();
		ITerrainConfigManager::getInterface().initialize(pWorld->getGraphicsScene(), desc.mPageSize, desc.mPageCount, desc.mVisiblePages, BTString("terrains"), pWorld->getWorldName());

		//create dummy entity
		IEntity* entity = mPageEntity = pWorld->getStage()->createEntity(IStageConfig::ELEMENT_SKIP_PREFIX + BTString("Terrain Helper"));
		assert(mEditorElement == NULL);
		this->createEditorElement(EditorElement::EDITOR_ELEMENT_TYPE, entity, BTString("Terrain Helper"));

		//create terrain
		this->setupTerrainEditables(pWorld);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			TerrainPageEditable::initialize(EditorElement* element)
	{
		//editor loading
		this->setEditorElement(element);
		mPageEntity = element->getEntity();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void				TerrainPageEditable::setupTerrainEditables(ILogicWorld* pWorld)
	{
		const IPagingManager::DESC& desc = pWorld->getStage()->getPagingManager()->getDesc();
		size_t totalPage = desc.mPageCount * desc.mPageCount;

		IScene* geomScene = pWorld->getGeometryScene();
		IScene* graphicsScene = pWorld->getGraphicsScene();
		TString namePrefix = pWorld->getWorldName() + BTString("_Terrain");

		//create entity with full pages
		for(size_t i = 0; i < totalPage; ++i )
		{
			size_t iZ = i / desc.mPageCount;
			size_t iX = i % desc.mPageCount;

			const TString tileEntityName = namePrefix
				+ BTString("_")
				+ TStringHelper::fromUInt(iX)
				+ BTString("_")
				+ TStringHelper::fromUInt(iZ);

			IEntity* terrain = pWorld->getStage()->createEntity(tileEntityName);

			IElement* terrainElem = graphicsScene->createElement(TerrainConsts::TERRAIN_ELEMENT);
			terrain->addElement(BTString("terrain"), HELEMENT(terrainElem));

			//don't save geometry in editor
			IElement* geomElem = geomScene->createElement(GeomConsts::GEOMETRY_ELEMENT_READONLY);
			terrain->addElement(IEditorFramework::EDITOR_ELEMENT_PREFIX + BTString("Geometry"), HELEMENT(geomElem));
			IGeometry* geom = geomElem->getInterface(IID_GEOMETRY());

			//create editable
			TString path = this->generateTilePath(iX, iZ);
			TerrainEditable* te = BLADE_NEW TerrainEditable(terrain, geom, path);
			//initialize interface
			te->getInterface()->initializeTerrain(iX, iZ, path);

			pWorld->getStage()->getPagingManager()->notifyEntity(terrain, true, (iX < desc.mVisiblePages && iZ < desc.mVisiblePages));

			TerrainToolManager::getSingleton().addTerrainEditable(te);
			IWorldEditableManager::getInterface().addEditable(HEDITABLE(te));
		}//for
	}
	
	//////////////////////////////////////////////////////////////////////////
	TString			TerrainPageEditable::generateTilePath(size_t x, size_t z) const
	{
		const TString& ResourceNamePrefix = ITerrainConfigManager::getInterface().getTerrainResourcePrefix();
		const TString& ResourcePathPrefix = ITerrainConfigManager::getInterface().getTerrainPath();

		return ResourceNamePrefix + BTString(":") + ResourcePathPrefix +
			BTString("/") + TStringHelper::fromUInt(x) +
			BTString("_") + TStringHelper::fromUInt(z) + BTString(".blt");
	}

}//namespace Blade
