/********************************************************************
	created:	2011/05/13
	filename: 	EditorLogic.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "EditorLogic.h"
#include <interface/ITimeService.h>
#include <interface/IEventManager.h>
#include <interface/public/geometry/IGeometryService.h>
#include <interface/public/ui/uiwindows/IViewportWindow.h>
#include <interface/public/ui/IMenuManager.h>
#include <interface/public/ui/UIEvents.h>
#include <interface/EditorEvents.h>
#include <BladeFramework_blang.h>

#include <BladeEditor_blang.h>
#include <interface/IEditorFramework.h>
#include <interface/IEditorToolManager.h>
#include <interface/public/ui/IViewport.h>
#include <interface/public/ui/IViewportManager.h>
#include <interface/public/EditorToolTypes.h>

namespace Blade
{
	const TString EditorLogic::DEFAULT_TYPE = IViewportWindow::VIEWPORT_WINDOW_TYPE;

	//////////////////////////////////////////////////////////////////////////
	EditorLogic::EditorLogic()
		:mName(DEFAULT_TYPE)
		,mTargetFile(NULL)
		,mToolNeedUpdate(false)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	EditorLogic::~EditorLogic()
	{

	}

	/************************************************************************/
	/* ILogic interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void	EditorLogic::onAddToScene(IStage* stage,ILogicScene* scene)
	{
		assert( stage == mTargetFile->getStage() );
		assert( scene == mTargetFile->getLogicScene() );
		BLADE_UNREFERENCED(stage);
		BLADE_UNREFERENCED(scene);

		IViewportWindow* window = static_cast<IViewportWindow*>(mTargetFile->getEditorWindow());
		IViewportSet* viewportSet = window->getViewportSet();

		size_t viewportCount = viewportSet->getViewportCount();
		for(size_t i = 0; i < viewportCount; ++i)
		{
			IMouse* mouse = window->getViewportMouse(i);
			IUIService::getSingleton().addMouseListener(this, mouse);
		}
		IEventManager::getSingleton().addEventHandler( ViewportCameraConfigEvent::NAME, EventDelegate(this, &EditorLogic::onCameraStateChange) );
	}
	
	//////////////////////////////////////////////////////////////////////////
	void	EditorLogic::onRemove()
	{
		mTargetFile = NULL;
		IEventManager::getSingleton().removeEventHandlers(ViewportCameraConfigEvent::NAME, this);
		IUIService::getSingleton().removeMouseListener(this);
	}

	//////////////////////////////////////////////////////////////////////////
	void	EditorLogic::onRun()
	{
		const TimeSource& time = ITimeService::getSingleton().getTimeSource();
		scalar frametime = time.getTimeThisLoop();

		//for debugging, if a breakpoint is set and activated,
		//the time will be large, so ignore it.
		if( frametime > 0.5f )
			return;

		IViewportWindow* window = static_cast<IViewportWindow*>(mTargetFile->getEditorWindow());

		CameraActor* camera = window->getCamera();
		INode* camerNode = camera->getGeometryNode();
		IMouse* mouse = window->getMouse();
		IKeyboard* keyboard = window->getKeyboard();

		scalar speed = window->getCamera()->getMoveSpeed();
		scalar rotateSpeed = window->getCamera()->getRotateSpeed();
		 
		//if( keyboard->isKeyPressed(KC_NUM_ADD) )
		//	camera->setMoveSpeed( speed*2 );
		//else if( keyboard->isKeyPressed(KC_NUM_SUBTRACT) )
		//	camera->setMoveSpeed( std::max(Math::LOW_EPSILON, speed*0.5f) );

		POINT3I absMovement = mouse->getMouseMovement();
		POINT3 movement = mouse->getNormalizedMovement();
		if( mouse->isButtonDown(MSB_MIDDLE) )
			camerNode->translate(-absMovement.x*frametime*speed*1.0f, absMovement.y*frametime*speed*1.0f, 0);
		else if( absMovement.z != 0)
			camerNode->translate( 0, 0, -absMovement.z*frametime*speed*1.0f );
		else if( (mouse->isButtonDown(MSB_RIGHT) || ( mouse->isButtonDown(MSB_LEFT) && !mToolNeedUpdate)) )
		{
			IViewportSet* viewportSet = window->getViewportSet();

			if( viewportSet->getActiveViewport()->getViewportType() == VT_PERSPECTIVE )
			{
				camerNode->yaw( -movement.x*rotateSpeed, GS_WORLD);
				float curPitch = camerNode->getWorldRotation().getPitch();
				float dPitch = -movement.y*rotateSpeed;
				if(std::fabs(curPitch + dPitch) < Math::HALF_PI)
					camerNode->pitch(dPitch, GS_OBJECT);
			}

			if( mouse->isButtonDown(MSB_RIGHT) )
			{
				//now this can be done on main loop
				if( keyboard->isKeyDown(KC_W) )
					camerNode->translate(0,0,-1.0f*frametime*speed);
				else if( keyboard->isKeyDown(KC_S) )
					camerNode->translate(0,0,1.0f*frametime*speed);
				else if( keyboard->isKeyDown(KC_A) )
					camerNode->translate(-1.0f*frametime*speed,0,0);
				else if( keyboard->isKeyDown(KC_D) )
					camerNode->translate(1.0f*frametime*speed,0,0);
				else if( keyboard->isKeyDown(KC_Q) )
					camerNode->translate(0,-1.0f*frametime*speed,0);
				else if( keyboard->isKeyDown(KC_E) )
					camerNode->translate(0,1.0f*frametime*speed,0);
			}

			if( mToolNeedUpdate )
			{
				IEditorTool* tool = IEditorToolManager::getSingleton().getSelecctedTool();
				POINT2 pt = mouse->getNormalizedMousePosition();
				tool->update(pt.x, pt.y, movement, keyboard->getKeyModifier(),frametime,mTargetFile);
			}
		}
		else
		{
			IEditorTool* tool = IEditorToolManager::getSingleton().getSelecctedTool();
			POINT2 pt = mouse->getNormalizedMousePosition();
			tool->update(pt.x, pt.y, movement, keyboard->getKeyModifier(),frametime,mTargetFile);
			//ILog::DebugOutput << TEXT("Mouse RButton is Up") << ILog::endLog;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void	EditorLogic::onUpdate()
	{
		IGeometryService::getSingleton().updateScenes();
	}

	//////////////////////////////////////////////////////////////////////////
	void	EditorLogic::onMainLoopUpdate()
	{
		IEditorFileUpdater* updater = mTargetFile->getFileUpdater();
		IMouse* mouse = mTargetFile->getEditorWindow()->getMouse();
		IKeyboard* keyboard = mTargetFile->getEditorWindow()->getKeyboard();

		if( updater != NULL )
			updater->update(mTargetFile);

		const TimeSource& time = ITimeService::getSingleton().getTimeSource();
		scalar frametime = time.getTimeThisLoop();

		IEditorTool* tool = IEditorToolManager::getSingleton().getSelecctedTool();

		POINT2 pt = mouse->getNormalizedMousePosition();
		tool->mainUpdate(pt.x,pt.y,keyboard->getKeyModifier(),frametime,mTargetFile);
	}

	/************************************************************************/
	/* IEditorLogic interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool	EditorLogic::initalize(IEditorFile* file, IUIWindow* window)
	{
		if (mTargetFile == NULL)
			mTargetFile = file;
		else
		{
			assert(false);
			return false;
		}

		//setup default toolbar button
		IUIToolBox* toolbox = window->createToolBox( BTString(BLANG_VIEWPORT), IS_16);
		IMenu* menu = IMenuManager::getSingleton().findMenu( BTString(BLANG_VIEWPORT) );
		toolbox->addButton(menu, IUIToolBox::TBS_DROPDOWN);
		menu = IMenuManager::getSingleton().findMenu( BXLang(BLANG_VIEW_VIEWMAX) );
		toolbox->addButton(menu);

		IconIndex icon = IIconManager::getSingleton().loadSingleIconImage( BTString("camera.png") );
		for(size_t i = 0; i < MAX_VIEWPORT_COUNT; ++i)
		{
			const HCONFIG& viewportConfig = IViewportManager::getSingleton().getViewportConfig(i);
			static_cast<IViewportToolboxSet*>(toolbox)->getViewportToolbox(i)->addToolSet(viewportConfig, icon);
		}

		menu = IMenuManager::getSingleton().findMenu( BTString(BLANG_VIEW_VIEWMODE) );
		toolbox->addButton(menu, IUIToolBox::TBS_DROPDOWN);
		menu = IMenuManager::getSingleton().findMenu( BTString(BLANG_VIEW_VIEWREALTIME) );
		toolbox->addButton(menu);

		IViewportWindow* win = static_cast<IViewportWindow*>(window);
		//setup view port
		IViewportSet* viewports = win->getViewportSet();
		viewports->initialize(file->getWorld(), IEditorFramework::EDITOR_ENTITY_PREFIX);
		//update view port
		win->updateViewportLayout();
		return true;
	}

	/************************************************************************/
	/* IMouse::Listener interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool	EditorLogic::onMouseButtonDown(MSBUTTON button)
	{
		IEditorTool* tool = IEditorToolManager::getSingleton().getSelecctedTool();
		IMouse* mouse = mTargetFile->getEditorWindow()->getMouse();
		IKeyboard* keyboard = mTargetFile->getEditorWindow()->getKeyboard();

		if( button == MSB_LEFT && tool != NULL )
		{
			POINT2 pt = mouse->getNormalizedMousePosition();
			mToolNeedUpdate = tool->onMouseLeftDown(pt.x,pt.y,keyboard->getKeyModifier(),mTargetFile);

			//?
			if( mToolNeedUpdate/*tool->getType() == EditorToolTypes::BRUSH_EDIT_TYPE */)
				return true;

		}
		mouse->hide();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	EditorLogic::onMouseButtonUp(MSBUTTON button)
	{
		IEditorTool* tool = IEditorToolManager::getSingleton().getSelecctedTool();
		IMouse* mouse = mTargetFile->getEditorWindow()->getMouse();
		IKeyboard* keyboard = mTargetFile->getEditorWindow()->getKeyboard();

		if( button == MSB_LEFT && tool != NULL )
		{
			POINT2 pt = mouse->getNormalizedMousePosition();

			mToolNeedUpdate = tool->onMouseLeftUp(pt.x,pt.y,keyboard->getKeyModifier(),mTargetFile);
			if( tool->getType() == EditorToolTypes::BRUSH_EDIT_TYPE )
				return true;
		}

		if( (button == MSB_RIGHT && mouse->isButtonUp(MSB_LEFT)) ||
			(button == MSB_LEFT && mouse->isButtonUp(MSB_RIGHT)) ||
			(button == MSB_MIDDLE && mouse->isButtonUp(MSB_RIGHT) && mouse->isButtonUp(MSB_LEFT)) )
			mouse->show();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void	EditorLogic::onMouseButtonClick(MSBUTTON button)
	{
		IEditorTool* tool = IEditorToolManager::getSingleton().getSelecctedTool();
		IMouse* mouse = mTargetFile->getEditorWindow()->getMouse();
		IKeyboard* keyboard = mTargetFile->getEditorWindow()->getKeyboard();

		if( button == MSB_LEFT && tool != NULL )
		{
			POINT2 pt = mouse->getNormalizedMousePosition();

			tool->onMouseLeftClick(pt.x,pt.y,keyboard->getKeyModifier(),mTargetFile);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void	EditorLogic::onMouseButtonDBClick(MSBUTTON /*button*/)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	void	EditorLogic::onCameraStateChange(const Event& data)
	{
		if( mTargetFile == NULL )
		{
			assert(false);
			return;
		}
		assert( data.mName == ViewportCameraConfigEvent::NAME );
		const ViewportCameraConfigEvent& state = static_cast<const ViewportCameraConfigEvent&>(data);

		IViewportWindow* window = static_cast<IViewportWindow*>(mTargetFile->getEditorWindow());
		IViewportSet* viewportSet = window->getViewportSet();
		IViewport* viewport = viewportSet->getViewport( (index_t)state.mViewportIndex );
		viewport->getCameraActor()->setMoveSpeed( state.mCameraSpeed );
		viewport->getCameraActor()->setRotateSpeed( state.mCameraRotateSpeed );
	}

}//namespace Blade
