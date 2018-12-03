/********************************************************************
	created:	2011/05/25
	filename: 	TerrainToolBase.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "TerrainToolBase.h"
#include <interface/IWorldEditableManager.h>

#include <ConfigTypes.h>
#include <interface/IResourceManager.h>
#include <interface/public/graphics/IGraphicsService.h>
#include <interface/public/graphics/IGraphicsEffectManager.h>
#include <interface/public/graphics/IGraphicsScene.h>
#include <interface/public/graphics/SpaceQuery.h>
#include <interface/public/graphics/IGraphicsCamera.h>

#include <interface/IEditorFramework.h>


namespace Blade
{
	const TString TerrainCommandBaseTool::CATEGORY = BXLang(BLANG_TERRAIN);
	const TString TerrainBrushBaseTool::CATEGORY = BXLang(BLANG_TERRAIN);

	//////////////////////////////////////////////////////////////////////////
	TerrainCommandBaseTool::TerrainCommandBaseTool(const TString& name,IconIndex icon,HOTKEY hotkey)
		:EditorCommandTool(name,icon,hotkey)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	TerrainCommandBaseTool::~TerrainCommandBaseTool()
	{

	}


	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////
	TerrainBrushRect		TerrainBrushBaseTool::msRect;
	bool					TerrainBrushBaseTool::msInited = false;

	//////////////////////////////////////////////////////////////////////////
	TerrainBrushBaseTool::TerrainBrushBaseTool(const TString& name,IconIndex icon,HOTKEY hotkey,const TString& defaultBrushImage/* = TString::EMPTY*/)
		:EditorBrushTool(name,icon,hotkey,defaultBrushImage,64,64)
		,mTerrainAppFlag(INVALID_APPFLAG)
		,mAccumulateTime(0.0f)
		,mEditing(false)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	TerrainBrushBaseTool::~TerrainBrushBaseTool()
	{
	}


	/************************************************************************/
	/* IEditorTool interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool				TerrainBrushBaseTool::onMouseLeftClick(scalar x,scalar y,uint32 keyModifier,IEditorFile* file)
	{
		BLADE_UNREFERENCED(x);
		BLADE_UNREFERENCED(y);
		BLADE_UNREFERENCED(keyModifier);
		BLADE_UNREFERENCED(file);

		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				TerrainBrushBaseTool::onMouseLeftDown(scalar x,scalar y,uint32 /*keyModifier*/,IEditorFile* file)
	{
		assert(file != NULL);
		mEditing = true;
		mAccumulateTime = 0.0f;

		IGraphicsView* view = file->getView();
		IGraphicsScene* scene = file->getGraphicsScene();
		IGraphicsCamera* camera = file->getDefaultCamera()->getCamera();
		if(view == NULL || scene == NULL || camera == NULL )
			return true;

		if( this->getBrushDecalEffect() == NULL )
			return true;

		Ray ray = camera->getSpaceRayfromViewPoint(x,y);
		RaycastQuery rcq(ray, FLT_MAX, PS_POSITIVE, mTerrainAppFlag);
		IElement* elem = scene->queryNearestElement(rcq);
		if( elem == NULL )
		{
			this->getBrushDecalEffect()->setVisible(false);
			return true;
		}

		IEntity* entity = elem->getEntity();
		IElement* element = entity->getElementBySystemType(EditorElement::EDITOR_ELEMENT_TYPE);
		if(element != NULL)
		{
			EditorElement* edem = static_cast<EditorElement*>(element);
			IEditable* editable = edem->getEditable();
			file->setSelectedEditable( editable );
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				TerrainBrushBaseTool::onMouseLeftUp(scalar x,scalar y,uint32 keyModifier,IEditorFile* file)
	{
		BLADE_UNREFERENCED(x);
		BLADE_UNREFERENCED(y);
		BLADE_UNREFERENCED(keyModifier);
		BLADE_UNREFERENCED(file);

		mEditing = false;
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				TerrainBrushBaseTool::update(scalar x, scalar y, const POINT3& /*deltaMove*/, uint32 keyModifier,scalar deltaTime, IEditorFile* file)
	{
		if( mTerrainAppFlag == INVALID_APPFLAG )
			return false;

		if( x < 0 || y < 0 || x > 1 || y > 1)
			return false;

		assert(file != NULL);
		IGraphicsView* view = file->getView();
		IGraphicsScene* scene = file->getGraphicsScene();
		IGraphicsCamera* camera = file->getDefaultCamera()->getCamera();
		if(view == NULL || scene == NULL || camera == NULL )
			return false;

		if( this->getBrushDecalEffect() == NULL )
			return false;

		Ray ray = camera->getSpaceRayfromViewPoint(x,y);
		RaycastQuery rcq(ray, FLT_MAX, PS_POSITIVE, mTerrainAppFlag);
		rcq.setPrecise(true);
		IElement* elem = scene->queryNearestElement(rcq);
		if( elem == NULL )
		{
			this->getBrushDecalEffect()->setVisible(false);
			return false;
		}

		scalar distance = rcq[0].distance;
		this->getBrushDecalEffect()->setVisible(true);
		POINT3 pt = ray.getRayPoint(distance);
		this->getBrushDecalEffect()->setPosition(pt);

		if (!mEditing)
			return false;

		PointerParam elementList;
		size_t elementCount = this->getBrushDecalEffect()->queryElement(elementList, mTerrainAppFlag);
		if( elementCount == 0)
			return false;

		//mRect
		TerrainEditableList terrainList;
		for(size_t i = 0; i < elementCount; ++i )
		{
			IElement* elem2 = elementList.getAt<IElement>(i);
			IEntity* entity = elem2->getEntity();
			ElementListParam list;
			EditorElement* edem = static_cast<EditorElement*>(entity->getElementBySystemType(EditorElement::EDITOR_ELEMENT_TYPE));

			if(edem != NULL)
			{
				IEditable* editable = edem->getEditable();
				terrainList.push_back( static_cast<TerrainEditable*>(editable) );
			}
		}

		if( mEditing )
		{
			mAccumulateTime += deltaTime;
			bool inverse = (keyModifier&KM_SHIFT) != 0;
			//note that brush size is a radius
			AABB aab = this->getBrushDecalEffect()->getAABB();
			msRect.clear();
			msRect.setup(terrainList, mBrushSize, aab.getMinPoint().x, aab.getMinPoint().z);
			msRect.process(this->getBrushOperation(), deltaTime, inverse);
			msRect.clear();
		}
		return false;
	}


	//////////////////////////////////////////////////////////////////////////
	void		TerrainBrushBaseTool::onEnable(bool bEnabled)
	{
		if(bEnabled && !msInited)
		{
			msInited = true;
		}

		if(bEnabled)
		{
			mTerrainAppFlag = IWorldEditableManager::getInterface().getEditableInfo( BTLang(BLANG_TERRAIN) )->mAppFlag;
		}

		EditorBrushTool::onEnable(bEnabled);
	}

}//namespace Blade
