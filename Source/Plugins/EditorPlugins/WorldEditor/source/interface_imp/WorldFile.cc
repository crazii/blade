/********************************************************************
	created:	2010/06/14
	filename: 	WorldFile.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <utility/Profiling.h>
#include <ConfigTypes.h>
#include <interface/IFramework.h>
#include <interface/IEditorFramework.h>
#include <interface/IResourceManager.h>
#include <interface/IStageConfig.h>
#include <interface/IPage.h>
#include <interface/public/graphics/IGraphicsService.h>
#include <interface/public/logic/ILogicWorldManager.h>
#include <interface/public/logic/ILogicService.h>
#include <databinding/IDataSourceDataBase.h>

#include "WorldFile.h"
#include "WorldEditableManager.h"

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	WorldFile::WorldFile()
		:EditorFileBase(WORLD_FILE_TYPE)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	WorldFile::~WorldFile()
	{

	}

	/************************************************************************/
	/* IEditorFile interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void				WorldFile::openFile(const TString& dir,const TString& name)
	{
		if( this->isFileOpened() || this->getEditorWindow() == NULL )
		{
			assert(false);
			return;
		}
		assert(this->getWorld() == NULL);
		mFile = TStringHelper::standardizePath(dir + TEXT('/') + name);

		TString base, ext, root;
		ILogicWorld* world;
		{
			BLADE_LW_PROFILING(PRE_INIT);
			TStringHelper::getFileInfo(name, base, ext);
			IResourceManager::getSingleton().registerScheme(base, dir, false);

			root = base + BTString(":");
			
			//create world first
			world = ILogicWorldManager::getSingleton().createWorld( base );
			this->setWorld(world);
			WorldEditableManager::getSingleton().setEditorFile(this);
			this->setEditableManager( WorldEditableManager::getSingletonPtr() );
		}

		IProgressCallback* callback = IEditorFramework::getSingleton().getEditorUI()->showProgress(BXLang("Loading..."));
		//then create stage
		IStage* stage;
		{
			BLADE_LW_PROFILING(CREATE_STAGE);
			stage = IFramework::getSingleton().createStage(base, this->getEditorWindow()->getWindow(), false);
			this->setStage(stage);
		}
		stage->getPagingManager()->setEditMode(true);
		WORLD_INFO winfo(stage);
		winfo.worldResourcePath = root;
		world->initialize(winfo, mFile, callback);

		//init editor data
		this->setOpened();
		this->setView( world->getWorldView() );
		this->setGraphicsScene(world->getGraphicsScene());
		this->setGeometryScene(world->getGeometryScene());
		//this->getGraphicsScene()->setRealtimeDrawing(false);
		this->setLogicScene( world->getLogicScene() );
		this->setCamera( world->getWorldCamera() );
		this->setUpdater(this);
	}

	//////////////////////////////////////////////////////////////////////////
	void				WorldFile::saveFile()
	{
		if( !this->isFileOpened() )
			return;

		if( this->isFileModified() )
		{
			this->setUnModified();
			this->getStage()->saveStageFile(mFile, true);
		}

		//save world editables
		IWorldEditableManager::getSingleton().saveAllEditable();
	}

	//////////////////////////////////////////////////////////////////////////
	void				WorldFile::newFile(const TString& dir,const TString& name)
	{
		if( this->isFileOpened() || this->getEditorWindow() == NULL )
		{
			assert(false);
			return;
		}
		assert( this->getWorld()  == NULL );
		mFile = TStringHelper::standardizePath(dir + TEXT('/') + name);
		TStringHelper::appendFileExtension(mFile, WORLD_FILE_EXT);

		TString base,ext;
		TStringHelper::getFileInfo(name,base,ext);
		IResourceManager::getSingleton().registerScheme(base, dir, true);

		IStage* stage = IFramework::getSingleton().createStage(base, this->getEditorWindow()->getWindow(), false);
		IEditorFramework::getSingleton().initStageConfig(stage, this->getFileConfig());
		stage->setTitle(base);	//set title after initStageConfig: this is not needed since stage title is not writable but only serializable.
		this->setStage(stage);

		ILogicWorld* world = ILogicWorldManager::getSingleton().createWorld( base );
		this->setWorld(world);
		WorldEditableManager::getSingleton().setEditorFile(this);
		this->setEditableManager( WorldEditableManager::getSingletonPtr() );
		stage->getPagingManager()->setEditMode(true);

		WORLD_INFO winfo(stage);
		winfo.worldResourcePath = base + BTString(":");
		world->initialize(winfo, Vector3(0,100,0));
		
		//init editor data
		this->setOpened();
		this->setGraphicsScene(winfo.graphicsScene);
		this->setGeometryScene(winfo.geomScene);
		this->setLogicScene(winfo.logicScene);
		this->setView( world->getWorldView() );
		this->setCamera(world->getWorldCamera());
		this->setUpdater(this);
	}

	//////////////////////////////////////////////////////////////////////////
	void				WorldFile::closeFile()
	{
		if( this->isFileOpened() )
		{
			mFile = TString::EMPTY;
			ILogicWorld* world = this->getWorld();
			assert( world != NULL );

			world->deinitialize();

			IResourceManager::getSingleton().unRegisterScheme( world->getWorldName() );

			ILogicWorldManager::getSingleton().destroyWorld(world);
			this->setWorld(NULL);
			//remove all editable before destroying stage
			WorldEditableManager::getSingleton().setEditorFile(NULL);
			IFramework::getSingleton().destroyStage( mEditorData.mStage );
			this->reset();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void				WorldFile::update(IEditorFile* file)
	{
		assert(file == this);
		BLADE_UNREFERENCED(file);

		if( mEditorData.mWorld != NULL )
			mEditorData.mWorld->update( *mEditorData.mCamera );

		if( !this->isModified() && IWorldEditableManager::getSingleton().isEditablesModified() )
			this->setModified();
	}

	//////////////////////////////////////////////////////////////////////////
	void				WorldFile::setupWindow(IUIWindow* window)
	{
		assert(window == this->getWindow() );
		BLADE_UNREFERENCED(window);
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	static HCONFIG	lsWorldConfig;
	const HCONFIG&		WorldFile::operateFileConfig(const TStringList* sceneList, bool bDelete/* = false*/)
	{
		if( lsWorldConfig == NULL && !bDelete)
		{
			lsWorldConfig.lock();

			ConfigGroup* pConfig = BLADE_NEW ConfigGroup();

			pConfig->addSubConfig(IStageConfig::getSingleton().getStageConfig());

			if( sceneList != NULL && !sceneList->empty() )
			{
				IConfig::IList* list = pConfig->addOrGetSubConfigs(Version::MAX);
				//add scene configs
				for(size_t i = 0; i < sceneList->size(); ++i)
				{
					const TString& sceneType = (*sceneList)[i];
					DataBinder binder(sceneType, NULL);
					if (binder.isBound())
					{
						list->addConfig(binder.getConfig());
						binder.unbindTarget();
					}
				}
			}
			Lock::memoryBarrier();
			lsWorldConfig.bind( pConfig );
			lsWorldConfig.unlock();
		}
		else
		{
			assert(sceneList == NULL);
			if(bDelete)
				lsWorldConfig.clear();
		}
		return lsWorldConfig;
	}
	



}//namespace Blade