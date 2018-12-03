/********************************************************************
	created:	2010/04/08
	filename: 	IStage.h
	author:		Crazii
	
	purpose:	stage definition, stage contains all entities.
				all methods should be called in main sync state except entity operations
*********************************************************************/
#ifndef __Blade_IStage_h__
#define __Blade_IStage_h__
#include <BladeFramework.h>
#include <utility/String.h>
#include <interface/IEntity.h>
#include <interface/public/IProgressCallback.h>

namespace Blade
{
	class IWindow;
	class IScene;
	class EntityResourceDesc;
	class SerializableMap;
	class IPagingManager;

	class IStage : public ISerializable
	{
	public:
		typedef struct SDescriptor
		{
			IWindow*	mWindow;		///bound window on creation
			TString		mName;			///unique name
			TString		mTitle;			///visible title
			bool		mAutoSceneBinding;	///scenes auto created on stage creation, auto created scene name uses stage name.
											///if true, you can use services to get scene by using stage's name. i.e IGraphicsService::getSingleton().getGraphicsScene(stage->getName());
											///otherwise, you probably need maintain a pointer list to all created scenes.
			bool		mAutoDelete;	///auto delete stage on bound window close
		}DESC;
	public:
		virtual ~IStage()		{}

		/**
		@describe get stage's descriptor
		@param
		@return
		*/
		virtual const DESC&		getDesc() const = 0;

		inline const TString&	getName() const { return this->getDesc().mName; }
		inline const TString&	getTitle() const { return this->getDesc().mTitle; }
		inline bool				isSceneAutoBinded() const { return this->getDesc().mAutoSceneBinding; }
		inline IWindow*			getWindow() const { return this->getDesc().mWindow; }

		/**
		@describe set the stage's tittle
		@param
		@return
		*/
		virtual void			setTitle(const TString& tittle) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool			addScene(IScene* pscene) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool			removeScene(const TString& sceneType) = 0;

		/** @brief  */
		virtual bool			removeScene(IScene* pscene) = 0;

		/**
		@describe get a scene contained in this stage. scene type is implementation defined type.
		@note: even for a specific scene (i.e. IGraphicsScene), it may(or be allowed to), have different implementations, so there is no pre-defined sceneType for it.
		@param 
		@return 
		*/
		virtual IScene*			getScene(const TString& sceneType) const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual size_t			getSceneCount() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual IScene*			getScene(index_t index) const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual IEntity*		createEntity(const TString& entityName) = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual bool			renameEntity(const TString& entityName, const TString& newName) = 0;

		inline bool		renameEntity(IEntity* entity, const TString& newName)
		{
			return this->renameEntity(entity->getName(), newName);
		}

		/**
		@describe reload existing entity from a separate file.
		if this happens in main sync state, the entity will be reloaded directly in place, unless forceAsync is true.
		@param 
		@return 
		*/
		virtual bool			loadEntity(IEntity* entity, const EntityResourceDesc* desc = NULL, bool forceAsync = false) = 0;

		/**
		@describe load entity from file, synchronously in MAIN_SYNC state.
		@param 
		@return 
		*/
		virtual bool			loadEntitySync(IEntity* entity, const EntityResourceDesc* desc = NULL, const ProgressNotifier& notifier = ProgressNotifier::EMPTY) = 0;

		/**
		@describe save entity to a separate file.
		@param elements: types of elements to save. if saveAllelements is true, this parameters is ignored.
		@note
		*/
		virtual bool			saveEntity(IEntity* entity, const TString& resource, 
			bool textFormat = false, bool saveAllelements = false, const TStringParam* elements = NULL) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual IEntity*		getEntity(const TString& entityName) const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void			destroyEntity(const TString& entityName) = 0;

		inline void			destroyEntity(IEntity* entity)
		{
			this->destroyEntity(entity->getName());
		}

		/**
		@describe load stage, pages won't be loaded. (loading pages need a starting position)
		@param 
		@return 
		*/
		virtual bool			loadStageFile(const TString& pathName, const ProgressNotifier& notifier = ProgressNotifier::EMPTY) = 0;

		/**
		@describe save stage
		@param 
		@return 
		*/
		virtual bool			saveStageFile(const TString& pathName, bool textFormat = false, bool savePage = true) = 0;

		/**
		@describe add common serializable objects
		@note the stage don't care about the real internal data of the serializable objects
		it only handles the loading/saving for those objects
		@param 
		@return 
		*/
		virtual SerializableMap&getSerializableMap() = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual IPagingManager*	getPagingManager() const = 0;

	};//class IStage


	class BLADE_FRAMEWORK_API IStageManager
	{
	public:
		/**
		@describe 
		@param [in] autoDelete: auto delete stage on window closing
		@param [in] autoScene: auto create scenes. if true, scenes will be created using stage name, by enumerating subsystems
		@return 
		*/
		virtual IStage*		createStage(const TString& name,IWindow* Window, bool autoDelete = true, bool autoScene = true, bool showWindow = true) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool		destroyStage(const TString& StageName) = 0;

		inline bool		destroyStage(IStage* pstage)
		{
			if(pstage == NULL)
				return false;
			return this->destroyStage(pstage->getName());
		}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual IStage*		getStage(const TString& StageName) const = 0;

	};//class IStageManager
	
}//namespace Blade



#endif //__Blade_IStage_h__