/********************************************************************
	created:	2013/04/09
	filename: 	ModelViewerFile.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <BladeModelViewer.h>
#include "ModelViewerFile.h"
#include <BladeBase_blang.h>

#include <interface/IFramework.h>
#include <interface/IResourceManager.h>
#include <interface/public/logic/ILogicService.h>
#include <interface/public/graphics/IGraphicsService.h>
#include <interface/public/geometry/IGeometryService.h>
#include <interface/IEventManager.h>

#include <interface/IEditorFramework.h>
#include <interface/public/ui/UIEvents.h>
#include <interface/public/ui/widgets/IUITrackView.h>
#include <interface/public/ui/uiwindows/IViewportWindow.h>
#include <interface/public/ui/IUILayoutManager.h>
#include <interface/public/logic/ILogicWorldManager.h>
#include <EditorHelper.h>
#include <interface/public/graphics/GraphicsInterface_blang.h>

#include <BladeModel.h>
#include <interface/IModelResource.h>
#include <interface/IModelConfigManager.h>
#include "ModelViewEditableManager.h"
#include "ModelViewEditable.h"
#include "ModelViewElement.h"

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	ModelViewerFile::ModelViewerFile()
		:EditorFileBase(MODEL_FILE_TYPE)
		,mTrack(NULL)
		,mSkeletonVisible(false)
		,mSubBoundingVisbile(false)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	ModelViewerFile::~ModelViewerFile()
	{
		assert(mTrack == NULL);
	}

	/************************************************************************/
	/* IEditorFile interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void				ModelViewerFile::openFile(const TString& dir,const TString& name)
	{
		if( this->isFileOpened() || mModelViewEditable != NULL || this->getEditorWindow() == NULL )
		{
			assert(false);
			return;
		}

		assert( this->getWorld() == NULL );
		mFile = TStringHelper::standardizePath(dir + TEXT('/') + name);

		IStage* stage = IFramework::getSingleton().createStage(name, this->getEditorWindow()->getWindow(), false);
		stage->instantProcess();
		stage->postProcess();
		stage->setTitle(name);
		this->setStage(stage);

		WORLD_INFO winfo = WORLD_INFO(stage);
		ILogicWorld* world = ILogicWorldManager::getSingleton().createWorld( name );

		world->initialize(winfo, Vector3(0,100,0));
		this->setWorld(world);
		this->setEditableManager( BLADE_NEW ModelViewEditableManager() );

		IGraphicsService::getSingleton().generateGraphicsTypeAppFlag( IModelConfigManager::getInterface().getStaticModelType() );
		IGraphicsService::getSingleton().generateGraphicsTypeAppFlag( IModelConfigManager::getInterface().getSkinnedModelType() );

		//init editor file data
		this->setOpened();
		this->setGraphicsScene(winfo.graphicsScene);
		this->setGeometryScene(winfo.geomScene);
		this->setLogicScene(winfo.logicScene);
		this->setView( world->getWorldView() );
		this->setCamera( world->getWorldCamera() );
		this->setUpdater(this);

		ModelViewEditable* editable = BLADE_NEW ModelViewEditable();
		mModelViewEditable.bind( editable );
		editable->initialize(stage, this->getGraphicsScene(), world->getGeometryScene(), mFile);
		this->getEditableManager()->addEditable(mModelViewEditable);

		//scale the model entity
		IModelResource* modelReosurce = editable->getModelElement()->getBoundResource();
		const AABB& aabb = modelReosurce->getModelAABB();
		scalar radius = aabb.getHalfSize().getLength();
		scalar limit = (scalar)MODEL_VIEW_SIZE_LIMIT / 2;
		if (limit < radius)
		{
			scalar scale = limit / radius;
			editable->getGeometry()->setGlobalScale(Vector3::UNIT_ALL*scale);
		}

		//create light
		HEDITABLE light = HEDITABLE( EditorHelper::createLightEditable(this, BTString("AssistantLight")) );
		light->getEditorElement()->setElementConfig(BTString(BLANG_LIGHT_ELEMENT), BTString(BLANG_TYPE), (uint32)LT_DIRECTIONAL);
		this->getEditableManager()->addEditable(light);
		stage->loadEntitySync(light->getTargetEntity());
		light->getEditorElement()->getGeomtry()->setGlobalDirection(Vector3::NEGATIVE_UNIT_ALL.getNormalizedVector());

		IGeometryService::getSingleton().updateScenes();

		IEntity* entity = editable->getTargetEntity();
		ISyncAnimation* syncAnim = entity->getInterface(IID_SYNCANIM());
		if(syncAnim != NULL)
		{
			mTrack = BLADE_NEW ModelViewTrack( syncAnim );
			IUITrackView* trackView = static_cast<IUITrackView*>( IEditorFramework::getSingleton().getEditorUI()->getLayoutManager()->getWidget(IEditorFramework::EDITOR_WIDGET_TRACKVIEW) );
			trackView->setTrack(mTrack);
		}

		IEventManager::getSingleton().addEventHandler( OptionHighLightEvent::NAME, EventDelegate(this, &ModelViewerFile::onHighLightOption)  );
	}

	//////////////////////////////////////////////////////////////////////////
	void				ModelViewerFile::saveFile()
	{
		//empty
	}

	//////////////////////////////////////////////////////////////////////////
	void				ModelViewerFile::newFile(const TString& /*dir*/,const TString& /*name*/)
	{
		//empty
	}

	//////////////////////////////////////////////////////////////////////////
	void				ModelViewerFile::closeFile()
	{
		if( this->isFileOpened() )
		{
			mModelViewEditable.clear();

			mFile = TString::EMPTY;
			ILogicWorld* world = this->getWorld();
			assert( world != NULL );

			world->deinitialize();

			ILogicWorldManager::getSingleton().destroyWorld(world);
			this->setWorld(NULL);
			IEditableManager* edManager = this->getEditableManager();
			assert(edManager != NULL);
			BLADE_DELETE edManager;
			this->setEditableManager(NULL);

			IFramework::getSingleton().destroyStage(mEditorData.mStage);
			this->reset();

			BLADE_DELETE mTrack;
			mTrack = NULL;

			IEventManager::getSingleton().removeEventHandlers(OptionHighLightEvent::NAME, this);
		}
	}

	/************************************************************************/
	/* IEditorFileUpdater interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void				ModelViewerFile::update(IEditorFile* /*file*/)
	{
		if( mEditorData.mWorld != NULL )
			mEditorData.mWorld->update( *mEditorData.mCamera );
	}

	//////////////////////////////////////////////////////////////////////////
	void				ModelViewerFile::setupWindow(IUIWindow* win)
	{
		assert(win == this->getWindow() && win->getWindowType() == IViewportWindow::VIEWPORT_WINDOW_TYPE );
		assert( mModelViewEditable != NULL);

		//set camera direction to look at the target
		//project the bounding box onto camera view plane and check its radius
		//and then set the proper distance by that radius
		//view-dir is set by bounding box center & camera position
		IViewportWindow* window = static_cast<IViewportWindow*>(win);

		ModelViewEditable* editable = static_cast<ModelViewEditable*>(mModelViewEditable);
		IModelResource* modelReosurce = editable->getModelElement()->getBoundResource();

		//the initial model has no transforms, use local AABB as world space
		const AABB& aabb = modelReosurce->getModelAABB();
		Vector3 center = aabb.getCenter();
		scalar radius = aabb.getHalfSize().getLength();
		scalar scale = editable->getGeometry()->getGlobalScale().x;
		center *= scale;
		radius *= scale;

		size_t viewportCount = window->getViewportSet()->getViewportCount();
		for(size_t i = 0; i < viewportCount; ++i)
		{
			IViewport* vp = window->getViewportSet()->getViewport(i);
			CameraActor* camera = vp->getCameraActor();
			Vector3 lookdir = camera->getGeometry()->getDirection();
			if (vp->getViewportType() == VT_PERSPECTIVE)
			{
				lookdir = Vector3(-1, 0, -1);
				camera->getGeometry()->setDirection(lookdir);
			}

			scalar fov = camera->getController()->getFOV();
			//use the larger dimension
			scalar aspect = camera->getController()->getAspectRatio();
			if( aspect < 1.0f )
				aspect = 1.0f/aspect;
			scalar distance = radius*aspect/::tan(fov/2);
			
			Vector3 cameraPos = center - lookdir*distance;
			camera->getGeometry()->setGlobalPosition(cameraPos);
			if( distance+radius > camera->getController()->getFarDistance() )
				camera->getController()->setFarDistance(distance+radius);
		}

		//update camera geometry instantly
		IGeometryService::getSingleton().updateScenes();

		//refresh viewport
		window->getViewportSet()->drawAllViewport();
	}

	//////////////////////////////////////////////////////////////////////////
	void				ModelViewerFile::onActivate(IEditorFile* file, bool active)
	{
		BLADE_UNREFERENCED(file);
		assert(file == this);
		IUITrackView* trackView = static_cast<IUITrackView*>(IEditorFramework::getSingleton().getEditorUI()->getLayoutManager()->getWidget(IEditorFramework::EDITOR_WIDGET_TRACKVIEW));
		if( active )
		{
			trackView->setTrack( mTrack );
		}
		else
		{
			trackView->setTrack( NULL );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void			ModelViewerFile::onHighLightOption(const Event& data)
	{
		if( data.mName != OptionHighLightEvent::NAME )
		{
			assert(false);
			return;
		}

		const OptionHighLightEvent& state = static_cast<const OptionHighLightEvent&>(data);

		const IEntity* entity = this->getSelectedEntity();
		TPointerParam<Interface> interfaceList;
		if( mModelViewEditable == NULL
			|| entity == NULL 
			|| (state.mUIHint != ModelConsts::MODEL_ANIMATION_ITEM_HINT && state.mUIHint != ModelConsts::MODEL_BONE_ITEM_HINT)
			|| !entity->getInterfaces(IID_MODEL().getName(), interfaceList)  )
			return;

		for(size_t i = 0; i < interfaceList.size(); ++i)
		{
			IModel* imodel = static_cast<IModel*>(interfaceList[i]);
			const TString& optionVal = state.mOptionValue;
			if (state.mUIHint == ModelConsts::MODEL_ANIMATION_ITEM_HINT)
				imodel->setAnimation(optionVal);
			else
				imodel->setSelectedBone(optionVal);
		}
	}

}//namespace Blade