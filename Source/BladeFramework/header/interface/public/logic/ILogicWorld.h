/********************************************************************
	created:	2011/11/02
	filename: 	ILogicWorld.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_ILogicWorld_h__
#define __Blade_ILogicWorld_h__
#include <interface/InterfaceSingleton.h>
#include <interface/IStage.h>

#include <interface/public/graphics/IGraphicsView.h>
//scenes & services
#include <interface/public/geometry/IGeometryScene.h>
#include <interface/public/graphics/IGraphicsScene.h>
#include <interface/public/logic/ILogicScene.h>
#include <interface/public/graphics/IGraphicsSpaceCoordinator.h>

#include <interface/public/graphics/IGraphicsService.h>
#include <interface/public/geometry/IGeometryService.h>
#include <interface/public/logic/ILogicService.h>

#include "CameraActor.h"

namespace Blade
{
	/** @brief world initialization struct, in case that init params are too many */
	struct WORLD_INFO
	{
		//initialization data
		TString			worldResourcePath;	//directory containing a stage file. for flexibility reason,
											//its recommended that register a scheme for the directory, so that the path can be switched dynamically.
		IStage*			stage;

		//setup by world at runtime, manually created scene by user
		mutable IGraphicsScene*	graphicsScene;
		mutable IGeometryScene*	geomScene;
		mutable ILogicScene*	logicScene;
		//setup by world at runtime
		mutable CameraActor*	camera;
		mutable IGraphicsView*	view;

		WORLD_INFO(IStage* _stage)
		{
			stage = _stage;
			if (stage != NULL && stage->isSceneAutoBinded())
			{
				graphicsScene = IGraphicsService::getSingleton().getGraphicsScene(stage->getName());
				geomScene = IGeometryService::getSingleton().getGeometryScene(stage->getName());
				logicScene = ILogicService::getSingleton().getLogicScene(stage->getName());
			}
			else
			{
				graphicsScene = NULL; geomScene = NULL; logicScene = NULL;
			}
			camera = NULL; view = NULL;
		}
	};

	class BLADE_FRAMEWORK_API ILogicWorld
	{
	public:
		virtual ~ILogicWorld()		{}

		/************************************************************************/
		/* note: those methods can only be called in main thread
			mostly at initializing time.										*/
		/************************************************************************/
		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool			setupWorldName(const TString& name) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const TString&	getWorldName() const = 0;

		/**
		@describe init world
		@param
		@return
		*/
		virtual void			initialize(const WORLD_INFO& info, const Vector3& startPos) = 0;

		/**
		@describe init world with resource path, to load stages immediately
		@param 
		@return 
		*/
		virtual void			initialize(const WORLD_INFO& info, const TString& worldStageFile, IProgressCallback* callback = NULL) = 0;

		/**
		@describe reverse of initialize
		@param
		@return
		*/
		virtual void			deinitialize() = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const WORLD_INFO&	getWorldInfo() const = 0;

		inline IStage*			getStage() const
		{
			return this->getWorldInfo().stage;
		}

		inline CameraActor*	getWorldCamera() const
		{
			return this->getWorldInfo().camera;
		}

		inline IGraphicsView*	getWorldView() const
		{
			return this->getWorldInfo().view;
		}

		inline IGraphicsScene*	getGraphicsScene() const
		{
			return this->getWorldInfo().graphicsScene;
		}

		inline IGeometryScene*	getGeometryScene() const
		{
			return this->getWorldInfo().geomScene;
		}

		inline ILogicScene*		getLogicScene() const
		{
			return this->getWorldInfo().logicScene;
		}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const Vector3&	getInitialPosition() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void			setInitialPosition(const Vector3& pos) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual IGraphicsSpaceCoordinator::IPositionReference*	getPositionRef() const = 0;

		inline const Vector3&	getCurrentPosition() const
		{
			return this->getPositionRef()->getPosition();
		}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual LogicActor*	createActor(IEntity* entity, IGeometry* geom = NULL) = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual CameraActor*	createCamera(const TString& name) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool			destroyActor(LogicActor& actor) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual LogicActor*		getActor(IEntity* entity) const = 0;

		inline LogicActor*	getActor(const TString& name) const
		{
			return this->getActor(this->getStage()->getEntity(name));
		}

		/**
		@describe update world position/map data from specific camera
		@param
		@return indicates that world whether will perform a whole loading process ,this means a loading process bar is needed.
		*/
		virtual bool			update(const CameraActor& camera) = 0;

	};//class ILogicWorld

	extern template class BLADE_FRAMEWORK_API Factory<ILogicWorld>;
	

}//namespace Blade



#endif // __Blade_ILogicWorld_h__