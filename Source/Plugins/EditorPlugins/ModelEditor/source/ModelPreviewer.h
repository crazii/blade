/********************************************************************
	created:	2017/2/27
	filename: 	ModelPreviewer.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_ModelPreviewer_h__
#define __Blade_ModelPreviewer_h__
#include <BladeModelEditor.h>
#include <interface/IMediaLibrary.h>
#include <interface/IFramework.h>
#include <interface/IResourceManager.h>
#include <interface/public/window/IWindowService.h>
#include <interface/public/graphics/IGraphicsService.h>
#include <interface/public/geometry/IGeometryService.h>
#include <interface/public/graphics/IGraphicsInterface.h>
#include <interface/public/graphics/IGraphicsCamera.h>
#include <EditorHelper.h>
#include <databinding/DataBindingHelper.h>
#include <BladeBase_blang.h>

namespace Blade
{
	static const TString PREVIEW_WINDOW_NAME = BTString("Hidden_Preview_Window");
	static const TString PREVIEW_STAGE_NAME = BTString("Hidden_Preview_Stage");
	static const TString PREVIEW_VIEW_NAME = BTString("Hidden_Preview_View");
	static const TString PREVIEW_ENTITY_NAME = BTString("Hidden_Preview_Entity");

	class ModelPreviewer : public IMediaLibrary::IMediaPreviewer, public Allocatable
	{
	public:
		ModelPreviewer()
		{
			mPreviewStage = NULL;
			mPreviewScene = NULL;
			mPreviewGeomScene = NULL;
			mPreviewEntity = NULL;
		}

		~ModelPreviewer()
		{
			IGraphicsService::getSingleton().destroyView(PREVIEW_VIEW_NAME);
			IWindowService::getSingleton().destroyWindow(PREVIEW_WINDOW_NAME);
		}

		/** @brief called for generating icon, or large image preview if it is not playable*/
		/** @brief if size == -1, then there is no scale, will use the original size */
		virtual HIMAGE	generatePreview(const TString& mediaFile, size_t size, IMAGE_POOL pool)
		{
			if (mWindow == NULL)
			{
				mWindow = IWindowService::getSingleton().createWindow(PREVIEW_WINDOW_NAME);
				mWindow->setVisibility(false);
				mPreviewStage = IFramework::getSingleton().createStage(PREVIEW_STAGE_NAME, mWindow, true, true, false);
				mPreviewEntity = mPreviewStage->createEntity(PREVIEW_ENTITY_NAME);
				mPreviewScene = IGraphicsService::getSingleton().getGraphicsScene(PREVIEW_STAGE_NAME);
				mPreviewGeomScene = IGeometryService::getSingleton().getGeometryScene(PREVIEW_STAGE_NAME);

				IGraphicsView* view = IGraphicsService::getSingleton().createView(PREVIEW_VIEW_NAME);
				view->setRenderScheme(BTString("Forward Shading"));
				view->enableImageEffect(false);
				mPreviewScene->initialize(view);
				mPreviewScene->setRealtimeDrawing(false);
				HELEMENT hElem = mPreviewScene->createGraphicsElement(ModelConsts::MODEL_ELEMENT_TYPE);
				mPreviewEntity->addElement(BTString("model"), hElem);
				mPreviewModel = hElem;
				mPreviewStage->loadEntitySync(mPreviewEntity);

				IEntity* light = EditorHelper::createLightEntity(mPreviewStage, mPreviewScene, mPreviewGeomScene, BTString("PreviewLight"));
				DataBindingHelper::setElementData(*light, BTString(BLANG_LIGHT_ELEMENT), BTString(BLANG_TYPE), (uint32)LT_DIRECTIONAL);
				light->getInterface(IID_GEOMETRY())->setGlobalDirection(Vector3::NEGATIVE_UNIT_ALL.getNormalizedVector());
				mPreviewStage->loadEntitySync(light);
			}
			assert(mPreviewStage != NULL && mPreviewEntity != NULL && mPreviewModel != NULL);

			//model doesn't have original size
			if (size == size_t(-1))
				size = 128;
			
			//load model	
			assert(mPreviewModel->getBoundResource() == NULL);
			mPreviewModel->unloadResource(true);
			mPreviewModel->setResourcePath(mediaFile);
			mPreviewModel->loadResource(false);

			if (mPreviewModel->getBoundResource() != NULL)
			{
				//the initial model has no transforms, use local AABB as world space
				const AABB& aabb = mPreviewModel->getInterface(IID_GRAPHICS())->getStaticLocalBounds();
				Vector3 center = aabb.getCenter();
				scalar radius = aabb.getHalfSize().getLength();

				IGraphicsCamera* camera = mPreviewScene->getMainCamera();

				scalar fov = camera->getFieldOfView();
				//use the larger dimension
				scalar aspect = 1;
				scalar distance = radius*aspect / ::tan(fov / 2);
				Vector3 lookdir = Vector3(-1,0,-1);
				Quaternion rotation;
				Quaternion::generateRotation(rotation, Vector3::NEGATIVE_UNIT_Z, lookdir);
				Vector3 cameraPos = center - lookdir*distance;
				camera->setEyePosition(cameraPos);
				camera->setOrientation(rotation);
				camera->setNearClipDistance(distance - radius + 1.0f);
				camera->setFarClipDistance(distance + radius);

				//render model to RTT and save to image
				HIMAGE img = IImageManager::getSingleton().createImage(size, size, pool, PF_RGB, IMGO_DEFAULT, 1);
				//sync draw: camera culling will use loop flag, update to next loop
				IFramework::getSingleton().update();
				mPreviewGeomScene->update();
				mWindow->setSize(size, size);
				mPreviewScene->drawScene(static_cast<IImage*>(img));
				mPreviewModel->unloadResource();	//unload resource
				return img;
			}
			mPreviewGeomScene->update();
			return HIMAGE::EMPTY;
		}

		/** @brief start play the preview if it is playable */
		virtual void	play(IWindow* /*targetWindow*/, const TString& /*mediaFile*/)
		{

		}

		/** @brief  */
		virtual void	updatePlay()
		{

		}

		/** @brief  */
		virtual void	stop()
		{

		}

	protected:
		IStage*	mPreviewStage;
		IGraphicsScene* mPreviewScene;
		IGeometryScene* mPreviewGeomScene;
		IEntity* mPreviewEntity;
		IElement* mPreviewModel;
		HWINDOW	mWindow;
	};
	
}//namespace Blade

#endif//__Blade_ModelPreviewer_h__