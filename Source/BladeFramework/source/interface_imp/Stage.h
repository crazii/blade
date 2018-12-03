/********************************************************************
	created:	2010/04/08
	filename: 	Stage.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_Stage_h__
#define __Blade_Stage_h__
#include <Handle.h>
#include <interface/IConfig.h>
#include <interface/IStage.h>
#include <interface/public/IScene.h>
#include <interface/IPage.h>
#include <interface/public/IResource.h>
#include <interface/public/window/IWindow.h>
#include <interface/public/window/IWindowEventHandler.h>
#include <utility/BladeContainer.h>
#include "Entity.h"
#include "EntityResource.h"

namespace Blade
{
	static const TString STAGE_RESOURCE_TYPE = BTString("StageResource");
	static const TString STAGE_SERIALIZER_TYPE_XML = BTString("Text Stage");
	static const TString STAGE_SERIALIZER_TYPE_BINARY = BTString("Binary Stage");
	static const TString STAGE_BINDING_CONFIG = BXLang("Stage Config");
	class Entity;

	//////////////////////////////////////////////////////////////////////////
	class Stage : public IStage, public IWindowEventHandler, public Allocatable
	{
	public:
		Stage(const TString&name,IWindow* window,IStageManager* manager, bool autoScene, bool autoDelete, bool showWindow);
		~Stage();

		/************************************************************************/
		/* IStage interface                                                                     */
		/************************************************************************/
		/**
		@describe get stage's descriptor
		@param
		@return
		*/
		virtual const DESC&		getDesc() const;

		/**
		@describe set the stage's tittle
		@param
		@return
		*/
		virtual void			setTitle(const TString& tittle);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			addScene(IScene* pscene);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			removeScene(const TString& sceneType);

		/** @brief  */
		virtual bool			removeScene(IScene* pscene)
		{
			if( pscene == NULL )
				return false;
			return this->removeScene( pscene->getSceneType() );
		}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual IScene*			getScene(const TString& sceneType) const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual size_t			getSceneCount() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual IScene*			getScene(index_t index) const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual IEntity*		createEntity(const TString& entityName);

		/*
		@describe reload entity in loading thread
		@param 
		@return 
		*/
		virtual bool			loadEntity(IEntity* entity, const EntityResourceDesc* desc = NULL, bool forceAsync = false);

		/**
		@describe load entity synchronously in MAIN_SYNC state.
		@param 
		@return 
		*/
		virtual bool			loadEntitySync(IEntity* entity, const EntityResourceDesc* desc = NULL, const ProgressNotifier& notifier = ProgressNotifier::EMPTY);

		/*
		@describe 
		@param 
		@return 
		@note this method should be called in main synchronous state
		*/
		virtual bool			saveEntity(IEntity* entity, const TString& resource,
			bool textFormat = false, bool saveAllElements = false, const TStringParam* elements = NULL);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual IEntity*		getEntity(const TString& entityName) const;

		/**
		@describe
		@param
		@return
		*/
		virtual bool			renameEntity(const TString& entityName, const TString& newName);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			destroyEntity(const TString& entityName) { this->destroyEntityImpl(entityName, true); }

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			loadStageFile(const TString& pathName, const ProgressNotifier& notifier = ProgressNotifier::EMPTY);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			saveStageFile(const TString& pathName, bool textFormat = false, bool savePage = true);

		/*
		@describe add common serializable objects
		@note the stage don't care about the real internal data of the serializable objects
		it only handles the loading/saving for those objects
		@param 
		@return 
		*/
		virtual SerializableMap&getSerializableMap()				{return mSerializableMap;}

		/**
		@describe
		@param
		@return
		*/
		virtual IPagingManager*	getPagingManager() const
		{
			return mPageManager;
		}

		/************************************************************************/
		/* IWindowEventHandler interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			onWindowDestroy(IWindow* pwin);

		/************************************************************************/
		/* ISerializable interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual void			postProcess(const ProgressNotifier& notifier);
		/** @brief  */
		virtual void			instantProcess(const ProgressNotifier& callback);

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/*
		@describe 
		@note this method returns STL object, so it cannot be called across DLL boundaries
		@param 
		@return 
		*/
		const EntitySet&		getEntities() const			{return mEntitySet;}

		/** @brief change entity state */
		bool					setEntityState(IEntity* entity, ENTITY_SATUS state, int16 elementLoadingCount = 0, const HRESOURCE* entityResource = NULL);

		/** @brief  */
		void			destroyEntityImpl(const TString& entityName, bool notifyPaging);

		/** @brief  */
		void			destroyPagingEntities(const EntitySet& entities, bool notifyPaging);

		/** @brief  */
		void			update();

	protected:

		/** @brief  */
		IEntity*		createEntityEntry(const TString& name);

		/** @brief  */
		void			clearStage();

		/** @brief  */
		bool			loadEntitySyncImpl(IEntity* entity, const EntityResourceDesc* desc, const ProgressNotifier& notifier = ProgressNotifier::EMPTY);

		typedef			TStringPointerMap<IScene>	SceneMap;
		typedef			List<Entity*>				EntityList;

		IStageManager*	mManager;
		IPagingManager*	mPageManager;

		TString			mWinCaption;

		SceneMap		mSceneMap;
		SceneMap		mElementCreatorMap;

		SerializableMap	mSerializableMap;

		EntitySet		mEntitySet;
		HRESOURCE		mStageResource;
		EntityList		mDeletedEntities;

		DESC			mDesc;

		const uint32&	getPageSize(index_t) const { return mPageManager->getDesc().mPageSize; }
		bool			setPageSize(index_t, const uint32& size);
		const uint32&	getPageCount(index_t) const { return mPageManager->getDesc().mPageCount; }
		bool			setPageCount(index_t, const uint32& count);
		const uint32&	getVisbiblePageCount(index_t) const { return mPageManager->getDesc().mVisiblePages; }
		bool			setVisbiblePageCount(index_t, const uint32& count);
		const TString&	getTitle(index_t) const { return mDesc.mTitle; }
		bool			setTitle(index_t, const TString& title) { this->setTitle(title); return true; }

		friend bool initializeFramework();
	};//class Stage

	typedef Handle<Stage> HSTAGE;

	
}//namespace Blade



#endif //__Blade_Stage_h__