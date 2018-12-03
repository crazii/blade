/********************************************************************
	created:	2010/05/08
	filename: 	GameWorld.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_GameWorld_h__
#define __Blade_GameWorld_h__
#include <BladeGame.h>
#include <interface/public/logic/ILogicWorld.h>
#include <utility/BladeContainer.h>
#include "GameWorldData.h"

namespace Blade
{
	class IStage;
	class IGraphicsScene;
	class IGeometryScene;
	class IGraphicsView;

	//generically, an game app has only one game world, that's enough
	//but considering to set game world not a singleton,
	//so that multiple instance could be created, this may be helpful for a editor
	class GameWorld : public ILogicWorld , public Allocatable
	{
	public:
		GameWorld();
		~GameWorld();

		/************************************************************************/
		/* IGameworld interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			setupWorldName(const TString& name);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const TString&	getWorldName() const	{return mName;}

		/*
		@describe
		@param
		@return
		*/
		virtual void			initialize(const WORLD_INFO& info, const Vector3& startPos);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			initialize(const WORLD_INFO& info, const TString& worldStageFile, IProgressCallback* callback = NULL);

		/*
		@describe reverse of initialize
		@param
		@return
		*/
		virtual void			deinitialize();

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const WORLD_INFO&	getWorldInfo() const	{return mInfo;}

		/*
		@describe 
		@param
		@return
		*/
		virtual const Vector3&		getInitialPosition() const
		{
			assert(mInitialized);
			return mInitialized ? mWorldData->getStartPosition() : Vector3::ZERO;
		}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void				setInitialPosition(const Vector3& pos)
		{
			assert(mInitialized && mWorldData != NULL);
			mWorldData->setStartPosition(pos);
		}

		/*
		@describe
		@param
		@return
		*/
		virtual IGraphicsSpaceCoordinator::IPositionReference*	getPositionRef() const { return mWorldData; }

		/**
		@describe
		@param
		@return
		*/
		virtual LogicActor*		createActor(IEntity* entity, IGeometry* geom = NULL);

		/*
		@describe
		@param
		@return
		*/
		virtual CameraActor*	createCamera(const TString& name);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			destroyActor(LogicActor& actor);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual LogicActor*		getActor(IEntity* entity) const;

		/*
		@describe
		@param
		@return
		*/
		virtual bool			update(const CameraActor& camera);

protected:
		/*
		@describe
		@param
		@return
		*/
		IGraphicsView*			setupWorldView();

		/** @brief  */
		inline	IStage*			getWorldStage() const
		{
			return mInfo.stage;
		}

		/** @brief  */
		inline	IGraphicsScene*	getWorldGraphicsScene() const
		{
			return mInfo.graphicsScene;
		}

		/** @brief  */
		inline IGeometryScene*	getWorldGeometryScene() const
		{
			return mInfo.geomScene;
		}

		/** @brief  */
		inline ILogicScene*		getWorldGameScene() const
		{
			return mInfo.logicScene;
		}

		typedef TStringMap<TString>		SpaceMap;
		typedef Set<LogicActor*>		ActorSet;

		TString				mName;
		WORLD_INFO			mInfo;

		ActorSet			mActors;
		GameWorldData*		mWorldData;
		bool				mInitialized;
		//PagedTerrainControllerSerializer	mTerrainPageSerializer;
	};//class GameWorld
	
}//namespace Blade


#endif //__Blade_GameWorld_h__