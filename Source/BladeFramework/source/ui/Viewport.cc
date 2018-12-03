/********************************************************************
	created:	2013/04/23
	filename: 	Viewport.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "Viewport.h"
#include <interface/public/graphics/IRenderWindow.h>
#include <interface/public/graphics/IGraphicsService.h>
#include <interface/public/geometry/IGeometryService.h>
#include <interface/public/logic/ILogicWorld.h>

namespace Blade
{
	void SViewportData::clear()
	{
		if( mAutoDelete )
		{
			assert( mCamera != NULL );
			assert( mView != NULL );
			assert(mWorld != NULL);
			IGraphicsService::getSingleton().destroyView(mView);
			mWorld->destroyActor(*mCamera);
		}
		mCamera = NULL;
		mView = NULL;
		mWorld = NULL;
		mAutoDelete = false;
	}

	//////////////////////////////////////////////////////////////////////////
	Viewport::Viewport(const VIEWPORT_DESC& desc, IViewportSet* set, index_t index)
		:mDesc(desc)
		,mIndex(index)
		,mSet(set)
	{
		mWindow = NULL;
#if USE_RENDER_WINDOW
		mRenderWindow = NULL;
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	Viewport::~Viewport()
	{
#if USE_RENDER_WINDOW
		if (mRenderWindow != NULL)
			IGraphicsService::getSingleton().destroyRenderTarget(mRenderWindow);
#endif
	}

	/************************************************************************/
	/* IViewport interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void		Viewport::setRealtime(bool realtime)
	{
		if( mDesc.mRealtime == realtime )
			return;

		mDesc.mRealtime = realtime;
		if( !mDesc.mActive )
			return;

		IGraphicsScene* scene = mData.mWorld->getGraphicsScene();

		scene->setRealtimeDrawing(realtime);
		if( realtime )
		{
#if USE_RENDER_WINDOW
			scene->setActiveRenderWindow(mRenderWindow, mData.mCamera->getCamera(), mData.mView);
#else
			scene->getActiveRenderWindow()->setSwapBufferOverride( NULL, NULL );
			scene->setActiveRenderWindow(NULL, mData.mCamera->getCamera(), mData.mView );
			scene->getActiveRenderWindow()->setSwapBufferOverride( mWindow, mData.mView );
#endif
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void		Viewport::setMaximized(bool maximized)
	{
		if( mDesc.mMaximized == maximized )
			return;
		mDesc.mMaximized = maximized;
		//always make runtime of maximized viewport
		if(maximized && mData.mWorld != NULL)
		{
			IGraphicsScene* scene = mData.mWorld->getGraphicsScene();
			scene->setRealtimeDrawing(true);
#if USE_RENDER_WINDOW
			scene->setActiveRenderWindow(mRenderWindow);
#else
			scene->getActiveRenderWindow()->setSwapBufferOverride(NULL, NULL);
			scene->setActiveRenderWindow(NULL, mData.mCamera->getCamera(), mData.mView);
			scene->getActiveRenderWindow()->setSwapBufferOverride(mWindow, mData.mView);
#endif
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void		Viewport::updateViewCamera()
	{
		assert( mData.mView != NULL && mData.mCamera != NULL );
		mData.mView->setSizePosition( mDesc.mLeft, mDesc.mTop, mDesc.mRight, mDesc.mBottom );
		mData.mView->setFillMode( mDesc.mFillMode );

		//IGraphicsView* view = mData.mView;
		//scalar width = scalar(view->getRelativeRight()-view->getRelativeLeft()) * this->getWindow()->getWidth();
		//scalar height = scalar(view->getRelativeBottom()-view->getRelativeTop()) * this->getWindow()->getHeight();
		mData.mCamera->setMoveSpeed( mDesc.mCameraSpeed );

		//set camera orientation according view port type
		IGeometry* geom = mData.mCamera->getGeometry();
		//const Vector3& pos = geom->getGlobalPosition();
		//Vector3	dir = geom->getDirection();
		switch( mDesc.mType )
		{
		case VT_FONT:
			{
				geom->setDirection( Vector3::NEGATIVE_UNIT_Z );
				geom->setGlobalPosition( Vector3::ZERO - Vector3::NEGATIVE_UNIT_Z*100 );
			}
			break;
		case VT_LEFT:
			{
				//use world left directly, not local right
				//Vector3 right = dir.crossProduct(Vector3::UNIT_Y);
				geom->setDirection( Vector3::NEGATIVE_UNIT_X );
				geom->setGlobalPosition( Vector3::ZERO - Vector3::NEGATIVE_UNIT_X*100 );
			}
			break;
		case VT_TOP:
			{
				geom->setDirection( Vector3::NEGATIVE_UNIT_Y );
				geom->setGlobalPosition( Vector3::ZERO - Vector3::NEGATIVE_UNIT_Y*100 );
			}
			break;
		case VT_PERSPECTIVE:
			{
			}
			break;
		case VT_COUNT:
			assert(false);
			break;
		}
		IGeometryService::getSingleton().updateScenes();
	}

	//////////////////////////////////////////////////////////////////////////
	Box2i		Viewport::getViewportRect(IRenderTarget* target) const
	{
		scalar width = (scalar)target->getWidth();
		scalar height = (scalar)target->getHeight();

		POINT2I min( int(width*mDesc.mLeft), int(height*mDesc.mTop) );
		POINT2I max( int(width*mDesc.mRight), int(height*mDesc.mBottom) );
		return Box2i(min, max);
	}
	
	//////////////////////////////////////////////////////////////////////////
	ViewportSet::ViewportSet(const VIEWPORT_LAYOUT& layout)
	{
		mLayout = NULL;
		mWorld = NULL;

		assert( layout.count > 0 );
		mActiveViewport = 0;

		this->resetLayout(&layout);
	}

	//////////////////////////////////////////////////////////////////////////
	ViewportSet::~ViewportSet()
	{
		this->cleanup();
	}

	//////////////////////////////////////////////////////////////////////////
	bool		ViewportSet::initialize(ILogicWorld* world, const TString& cameraNamePrefix)
	{
		if (mWorld != world && world != NULL)
		{
			mWorld = world;
			mCameraPrefix = cameraNamePrefix;

			//setup view & camera
			this->resetLayout();

			//activate viewport
			if( mActiveViewport != INVALID_INDEX )
			{
				index_t activeIndex= mActiveViewport;
				mViewports[mActiveViewport]->setAcitve(false);
				mActiveViewport = INVALID_INDEX;
				this->setActiveViewport(activeIndex);
			}
			
			return true;
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		ViewportSet::setActiveViewport(index_t index)
	{
		if(index >= mViewports.size() )
		{
			assert(false);
			return false;
		}

		if( mActiveViewport != INVALID_INDEX )
			mViewports[mActiveViewport]->setAcitve(false);
		if( index != INVALID_INDEX )
			mViewports[index]->setAcitve(true);

		if(mWorld == NULL )
		{
			mActiveViewport = index;
			return true;
		}

		Viewport* activeViewport = mViewports[index];

		bool realtime = activeViewport->isRealtimeUpdate();
		mWorld->getGraphicsScene()->setRealtimeDrawing( realtime );
#if USE_RENDER_WINDOW
		mWorld->getGraphicsScene()->setActiveRenderWindow(NULL);
#else
		mWorld->getGraphicsScene()->getActiveRenderWindow()->setSwapBufferOverride( NULL, NULL );
#endif

		if(realtime)
		{
			assert(activeViewport->getWindow() != NULL);
#if USE_RENDER_WINDOW
			mWorld->getGraphicsScene()->setActiveRenderWindow(activeViewport->getRenderWindow(), activeViewport->getCamera(), activeViewport->getView());
#else
			mWorld->getGraphicsScene()->setActiveRenderWindow(NULL, activeViewport->getCamera(), activeViewport->getView() );
			//TODO: if IWindow may not be a platform implementation, setSwapBufferOverride will handle it
			//i.e. a pure graphical drawn window. should use render target instead of IWindow
			mWorld->getGraphicsScene()->getActiveRenderWindow()->setSwapBufferOverride( activeViewport->getWindow(), activeViewport->getView() );
#endif
		}
		mActiveViewport = index;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		ViewportSet::resetLayout(const VIEWPORT_LAYOUT* layout/* = NULL*/)
	{
		if (layout != NULL && layout != mLayout)
		{
			this->cleanup();

			index_t activeViewport = INVALID_INDEX;
			mViewports.resize(layout->count);
			for (size_t i = 0; i < layout->count; ++i)
			{
				const VIEWPORT_DESC& desc = layout->layouts[i];
				Viewport* viewport = BLADE_NEW Viewport(desc, this, i);
				mViewports[i] = viewport;

				if (viewport->isActive())
					activeViewport = i;
			}

			mActiveViewport = activeViewport;
			mLayout = layout;
		}

		for(size_t i = 0; i < mViewports.size(); ++i )
		{
			Viewport* viewport = mViewports[i];
			viewport->getViewportDataRef().clear();
			if(mWorld != NULL)
				this->setupCameraView( viewport );
		}
		if(mWorld != NULL)
		{
			for(size_t i = 0; i < mViewports.size(); ++i)
				mViewports[i]->updateViewCamera();
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		ViewportSet::drawViewport(index_t index)
	{
		if( index >= mViewports.size() )
		{
			assert(false);
			return false;
		}
		if(mWorld == NULL )
			return false;

		IViewport* viewport = mViewports[index];
		if( !viewport->isActive() || !viewport->isRealtimeUpdate() )
#if USE_RENDER_WINDOW
			return mWorld->getGraphicsScene()->drawScene(static_cast<Viewport*>(viewport)->getRenderWindow(), viewport->getCamera(), viewport->getView());
#else
			return mWorld->getGraphicsScene()->drawScene(viewport->getWindow(), viewport->getCamera(), viewport->getView());
#endif
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void		ViewportSet::setupCameraView(Viewport* viewport)
	{
		if( viewport->isActive() )
		{
			VIEWPORT_DATA& data = viewport->getViewportDataRef();
			assert( data.empty() );

			data.mAutoDelete = false;

			data.mWorld = mWorld;
			data.mCamera = mWorld->getWorldCamera();
			data.mView = mWorld->getWorldView();
		}
		else
		{
			VIEWPORT_DATA& data = viewport->getViewportDataRef();
			assert( data.empty() );

			data.mAutoDelete = true;
			data.mWorld = mWorld;

			const TString instanceString = TStringHelper::fromUInt( viewport->getIndex() );
			data.mCamera = mWorld->createCamera( mCameraPrefix +
				mWorld->getWorldName()+ BTString("_viewport_camera_") + instanceString );

			data.mCamera->getGeometry()->setGlobalPosition(mWorld->getWorldCamera()->getGeometry()->getGlobalPosition() );
			data.mCamera->getGeometry()->setGlobalRotation(mWorld->getWorldCamera()->getGeometry()->getGlobalRotation() );

			data.mView = IGraphicsService::getSingleton().createView( 
				mWorld->getWorldName()+ BTString("_viewport_view_") + instanceString );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void				ViewportSet::cleanup()
	{
#if USE_RENDER_WINDOW
		//restore render window before current view port render window destroyed
		if (mWorld != NULL)
			mWorld->getGraphicsScene()->setActiveRenderWindow(NULL);
#endif

		for (size_t i = 0; i < mViewports.size(); ++i)
		{
			const VIEWPORT_DESC& desc = mViewports[i]->getDesc();
			//save data
			mLayout->layouts[i].mMaximized = desc.mMaximized;
			mLayout->layouts[i].mRealtime = desc.mRealtime;

			BLADE_DELETE mViewports[i];
		}
	}
	
}//namespace Blade