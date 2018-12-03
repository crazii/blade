/********************************************************************
	created:	2010/06/12
	filename: 	EditorFramework.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "EditorFramework.h"
#include <utility/Profiling.h>
#include <interface/IFramework.h>
#include <interface/IResourceManager.h>
#include <interface/IPlatformManager.h>
#include <interface/IStageConfig.h>
#include <interface/IEnvironmentManager.h>
#include <interface/public/window/IWindowService.h>
#include <interface/IMediaLibrary.h>
#include <interface/public/graphics/IGraphicsService.h>
#include <interface/public/ui/IMenu.h>
#include <interface/public/ui/IMenuManager.h>
#include <interface/public/ui/IIconManager.h>
#include <interface/public/ui/IUIWidget.h>
#include <interface/public/ui/widgets/IUIToolPanel.h>
#include <interface/public/ui/IUIMediaLibrary.h>
#include <interface/public/ui/IViewport.h>
#include <interface/public/ui/IViewportManager.h>
#include <ConstDef.h>

#include <interface/IEditorToolManager.h>
#include <interface/public/IEditorFile.h>
#include <interface/public/IEditorLogic.h>
#include <interface/public/IImporter.h>
#include "ImporterManager.h"

#include "EditorCommand.h"
#include "CommonEditorTools.h"
#include "EditorConfigManager.h"

#include <BladeBase_blang.h>
#include <BladeFramework_blang.h>
#include <BladeEditor_blang.h>
#include <interface/public/geometry/GeometryInterface_blang.h>

#include <interface/IEventManager.h>
#include <interface/EditorEvents.h>


#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(disable:4127)		//while(true)
#endif

namespace Blade
{
	template class Factory<IEditorFramework>;

	const TString IEditorFramework::MEDIA_TYPE_BRUSH = BTString("Brush");

	const TString IEditorFramework::EDITOR_RESOURCE_PATH = BTString("media:editor_res");
	const TString IEditorFramework::EDITOR_ICON_PATH = IEditorFramework::EDITOR_RESOURCE_PATH + TEXT("/icons/"); 

	const TString IEditorFramework::EDITOR_ELEMENT_PREFIX = BTString("[ED_ELM_HELPER]");
	const TString IEditorFramework::EDITOR_ENTITY_PREFIX = BTString("[ED_ENT_HELPER]");
	const TString IEditorFramework::EDITOR_SCENE_PREFIX = BTString("[ED_SCN_HELPER]");

	const TString IEditorFramework::EDITOR_WIDGET_TOOL = BTString(BLANG_TOOL);
	const TString IEditorFramework::EDITOR_WIDGET_MEDIAVIEW = BTString(BLANG_MEDIAVIEW);
	const TString IEditorFramework::EDITOR_WIDGET_PROPERTY = BTString(BLANG_PROPERTY);
	const TString IEditorFramework::EDITOR_WIDGET_LOGVIEW = BTString(BLANG_LOG);
	const TString IEditorFramework::EDITOR_WIDGET_TRACKVIEW = BTString(BLANG_TRACKVIEW);

	//////////////////////////////////////////////////////////////////////////
	EditorFramework::EditorFramework()
		:mEditorUI(NULL)
		,mActiveFile(NULL)
		,mCurrentOpeningFile(NULL)
	{
		mOriginalEntityPrefix = IStageConfig::getSingleton().getEntitySkippingPrefix();
		mOriginalElementPrefix = IStageConfig::getSingleton().getElementSkippingPrefix();
		mOriginalSceneTypePrefix = IStageConfig::getSingleton().getSceneTypeSkippingPrefix();

		IStageConfig::getSingleton().setElementSkippingPrefix(EDITOR_ELEMENT_PREFIX);
		IStageConfig::getSingleton().setEntitySkippingPrefix(EDITOR_ENTITY_PREFIX);
		IStageConfig::getSingleton().setSceneTypeSkippingPrefix(EDITOR_SCENE_PREFIX);

		//editor events
		IEventManager::getSingleton().registerEvent(EditableUnLoadEvent::NAME);
		IEventManager::getSingleton().registerEvent(EditorFileSwitchEvent::NAME);
		IEventManager::getSingleton().registerEvent(TransformSpaceEvent::NAME);
		IEventManager::getSingleton().registerEvent(EntityRenameEvent::NAME);
	}

	//////////////////////////////////////////////////////////////////////////
	EditorFramework::~EditorFramework()
	{
		IEventManager::getSingleton().unRegisterEvent(EditableUnLoadEvent::NAME);
		IEventManager::getSingleton().unRegisterEvent(EditorFileSwitchEvent::NAME);
		IEventManager::getSingleton().unRegisterEvent(TransformSpaceEvent::NAME);
		IEventManager::getSingleton().unRegisterEvent(EntityRenameEvent::NAME);
	}

	/************************************************************************/
	/* IEditorUIListener interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void EditorFramework::onWindowDestroy(IWindow* window)
	{
		CHECK_EDITOR();
		UIWindowMap::iterator iter = mUIWindowMap.find(window);
		assert(iter != mUIWindowMap.end());
		IUIWindow* ui = iter->second;

		mUIWindowMap.erase(iter);

		if( ui == mEditorUI )
		{
			assert(mOpenedFiles.size() == 0);
			mEditorUI = NULL;
			std::for_each(mEditors.begin(), mEditors.end(), std::mem_fun(&IEditor::shutdownEditor));
			IEnvironmentManager::getSingleton().setVariable(ConstDef::EnvBool::WORK_TERMINATE, true);
		}
		else
		{
			CHECK_EDITOR();
			IEditorFile* file = IEditorFramework::getFileFromWindow(ui);
			SEditorFileInfoEx* info = this->getFileType( file->getTypeID() );
			assert(file != NULL && file->getEditorWindow() == ui);
			if(file->getFileUpdater() != NULL)
				file->getFileUpdater()->onActivate(file, false);

			//this one is the last
			for (FileNotification::iterator i = mNotifications.begin(); i != mNotifications.end(); ++i)
			{
				IFileUINotify* notify = *i;
				notify->onFileSwitch(file, NULL);
			}

			if (mActiveFile == file)
				mActiveFile = NULL;

			assert(info->mOpenedCount > 0);
			--(info->mOpenedCount);
			size_t n = mOpenedFiles.erase(file->getFileID());
			assert(n == 1); BLADE_UNREFERENCED(n);
			file->closeFile();
			BLADE_DELETE file;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool EditorFramework::onWindowClose(IWindow* window)
	{
		CHECK_EDITOR();
		UIWindowMap::iterator i = mUIWindowMap.find(window);
		assert(i != mUIWindowMap.end());
		IUIWindow* ui = i->second;

		//top window closed, it will close its children
		if (ui == mEditorUI)
		{
			//try close all child
			IUIWindow* child;
			while ((child = mEditorUI->getLayoutManager()->getActiveWindow()) != NULL)
			{
				child->getWindow()->closeWindow();	//this will invoke onWindowClose() to confirm unsaved file
				//closing failed: user canceled a unsaved close
				if (mEditorUI->getLayoutManager()->getActiveWindow() == child && child->getWindow() != NULL)
					return false;
			}
			this->saveLayout();
			return true;
		}

		IEditorFile* file = IEditorFramework::getFileFromWindow(ui);
		assert(file->getEditorWindow() == ui);

		if (!file->isModified())
			return true;

		bool close = true;
		IPlatformManager::MBRESULT confirm =

			IPlatformManager::getSingleton().showMessageBox(
				IPlatformManager::MBT_YESNOCANCEL,
				BTLang(BLANG_TITLE_CLOSE).c_str(),
				BTLang(BLANG_MSG_SAVE).c_str(),
				window->getCaption().c_str());

		if (confirm == IPlatformManager::MBR_YES)
		{
			file->saveFile();
			close = true;
		}
		else if (confirm == IPlatformManager::MBR_NO)
			close = true;
		else//cancel
			close = false;

		return close;
	}

	//////////////////////////////////////////////////////////////////////////
	void EditorFramework::onWindowShow(IWindow* window)
	{
		CHECK_EDITOR();
		UIWindowMap::iterator iter = mUIWindowMap.find(window);
		assert(iter != mUIWindowMap.end());
		IUIWindow* ui = iter->second;
		if (ui == mEditorUI)
			return;
		IEditorFile* file = IEditorFramework::getFileFromWindow(ui);

		if (mActiveFile != file)
		{
			IEditorFile* oldFile = mActiveFile;
			if (mActiveFile != NULL && mActiveFile->getFileUpdater() != NULL)
				mActiveFile->getFileUpdater()->onActivate(mActiveFile, false);
			mActiveFile = file;
			if (mActiveFile != NULL && mActiveFile->getFileUpdater() != NULL)
				mActiveFile->getFileUpdater()->onActivate(mActiveFile, true);

			for (FileNotification::iterator i = mNotifications.begin(); i != mNotifications.end(); ++i)
			{
				IFileUINotify* notify = *i;
				notify->onFileSwitch(oldFile, file);
			}
		}
	}

	/************************************************************************/
	/* IEditorFramework interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void	EditorFramework::initialize(IUIMainWindow* ui, const TString& icon, Version version)
	{
		if( mEditorUI != NULL )
			BLADE_EXCEPT(EXC_REDUPLICATE,BTString("editor framework already initialized."));

		if( ui == NULL )
			BLADE_EXCEPT(EXC_INVALID_PARAM,BTString("invalid data pointer : emtpy(NULL)") );

		IIconManager::getSingleton().initialize(EDITOR_ICON_PATH);
		EditorCommandManager::getSingleton().initialize(version);
		this->setupMenus();

		//add built-in tools : common editor tools
		//select tool
		IconIndex index = IIconManager::getSingleton().loadSingleIconImage(BTString("select.png") );
		SelectTool* selTool = BLADE_NEW SelectTool(index);
		mCommonTools.push_back(selTool);
		IEditorToolManager::getSingleton().addTool(selTool);
		selTool->setSelected(true);
		IEditorToolManager::getSingleton().selectTool(selTool);
		mInternalNotify.addCommonFileTool(selTool);

		//translate tool
		index = IIconManager::getSingleton().loadSingleIconImage( BTString("move.png") );
		TranslateTool* translateTool = BLADE_NEW TranslateTool(index);
		mCommonTools.push_back(translateTool);
		IEditorToolManager::getSingleton().addTool(translateTool);
		mInternalNotify.addCommonFileTool(translateTool);

		//rotate tool
		index = IIconManager::getSingleton().loadSingleIconImage(BTString("rotate.png") );
		RotateTool* rotateTool = BLADE_NEW RotateTool(index);
		mCommonTools.push_back(rotateTool);
		IEditorToolManager::getSingleton().addTool(rotateTool);
		mInternalNotify.addCommonFileTool(rotateTool);

		//scale tool
		index = IIconManager::getSingleton().loadSingleIconImage(BTString("scale.png") );
		ScaleTool* scaleTool = BLADE_NEW ScaleTool(index);
		mCommonTools.push_back(scaleTool);
		IEditorToolManager::getSingleton().addTool(scaleTool);
		mInternalNotify.addCommonFileTool(scaleTool);

		//light creation tool
		index = IIconManager::getSingleton().loadSingleIconImage(BTString("light.png") );
		LightCreationTool* lightTool = BLADE_NEW LightCreationTool(index);
		mCommonTools.push_back(lightTool);
		IEditorToolManager::getSingleton().addTool(lightTool);
		mInternalNotify.addCommonFileTool(lightTool);

		this->addFileNotification(&mInternalNotify);

		//init editor plugins, adding menus, tools
		std::for_each(mEditors.begin(),mEditors.end(),std::mem_fun(&IEditor::initEditor));
		//setup help->about menu : help about menu should be the last
		IMenu* menu = IMenuManager::getSingleton().addRootMenu(BXLang(BLANG_HELP), TEXT('H'));
		index = IIconManager::getSingleton().loadSingleIconImage(BTString("about.png"));
		menu->addItem(BXLang(BLANG_ABOUT), EditorCommandManager::getSingleton().getEditorCommand(UIC_HELP_ABOUT), index, true, TEXT('A'));

		//predefined media types
		IMediaLibrary::MEDIA_TYPE type;
		type.mType = MEDIA_TYPE_BRUSH;
		type.mIndex = 0;
		type.mIsPlayable = false;
		type.mSharedIcon = false;
		type.mExtensions.reserve(2);
		type.mExtensions.push_back( BTString("dds") );
		type.mExtensions.push_back( BTString("png") );
		type.mPreviewer.bind( BLADE_FACTORY_CREATE(IMediaLibrary::IMediaPreviewer, BTString("ImagePreivewer")) );
		IMediaLibrary::getSingleton().registerMediaType(type, EDITOR_RESOURCE_PATH + BTString("/brushes") );

		//show splash info
		IMediaLibrary::getInterface().initialize();
		//build media library resources
		IUIMediaLibrary::getInterface().initialize();

		//add editor system
		//subsystems now can be added through config file, we need to check existence
		if( IFramework::getSingleton().getSubSystem(BTString("BladeEditorSystem")) == NULL)
			IFramework::getSingleton().addSubSystem(BTString("BladeEditorSystem"));

		IViewportManager::getSingleton().initialize();
		IEditorConfigManager::getSingleton().addConfig(IViewportManager::getSingleton().getAllConfig());
		IEditorToolManager::getSingleton().initialize();

		index = IIconManager::getSingleton().loadSingleIconImage(icon);
		ui->initWindow((uintptr_t)NULL, index, BTLang(BLANG_BLADE) + BTString(" ") + BTLang(BLANG_STUDIO));
		ui->getWindow()->addEventHandler(this);
		mEditorUI = ui;
		mUIWindowMap[ui->getWindow()] = ui;

		this->createWidgets();
		this->setupToolbox();
		this->setupUILayout();
		//mEditorUI->getWindow()->setVisibility(true);
	}

	//////////////////////////////////////////////////////////////////////////
	void	EditorFramework::deinitialize()
	{
		IViewportManager::getSingleton().shutdown();

		//close all opened files
		for( OpenedFileMap::iterator i = mOpenedFiles.begin(); i != mOpenedFiles.end(); ++i )
		{
			i->second->closeFile();
			BLADE_DELETE i->second;
		}
		mOpenedFiles.clear();
		mFileTypeList.clear();
		mFileTypeRegistry.clear();

		for(size_t i = 0; i < mCommonTools.size(); ++i)
		{
			IEditorTool* tool = mCommonTools[i];
			BLADE_DELETE tool;
		}
		mCommonTools.clear();

		EditorCommandManager::getSingleton().shutdown();

		IMediaLibrary::getSingleton().shutdown();
		IUIMediaLibrary::getInterface().shutdown();

		//usually mEditorUI is set to NULL on UI close, see @EditorFramework::onUIDestroy,
		//but the UI may not be closed properly on exceptions, so close it manually
		if( mEditorUI != NULL )
			mEditorUI->getWindow()->closeWindow();
	}

	//////////////////////////////////////////////////////////////////////////
	bool		EditorFramework::addEditor(IEditor* editor)
	{
		if( editor != NULL )
			return mEditors.insert(editor).second;
		else
			return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	EditorFramework::removeEditor(IEditor* editor)
	{
		if( editor == NULL )
			return false;

		return mEditors.erase(editor) == 1;
	}

	//////////////////////////////////////////////////////////////////////////
	void	EditorFramework::registerFileType(const SEditorFileInfoEx& fileinfo)
	{
		SEditorFileInfoEx& empty = mFileTypeRegistry[fileinfo.mFileClassName];
		if( empty.mFileClassName != NULL )
		{
			assert( empty.mFileClassName == fileinfo.mFileClassName);
			BLADE_EXCEPT(EXC_REDUPLICATE,BTString("fileinfo with the same class name already added:")+ fileinfo.mFileClassName );
		}
		empty = fileinfo;
		mFileTypeList.push_back(&empty);

		FileTypeID type = FileTypeID(mFileTypeList.size()-1);
		if( INVALID_FILETYPE == type)
			BLADE_EXCEPT(EXC_INTERNAL_ERROR,BTString("cannot add file type.") );

		empty.mTypeID = type;
		empty.mOpenedCount = 0;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	EditorFramework::unregisterFileType(const TString& fileClassName)
	{
		if( mFileTypeRegistry.size() == 0 )
			return true;

		if( mEditorUI != NULL )
			BLADE_EXCEPT(EXCE_STATE_ERROR,BTString("Editor file can only be removed after UI closed.") );

		assert( mOpenedFiles.size() == 0 );

		FileTypeRegistry::iterator i = mFileTypeRegistry.find( fileClassName );
		if( i == mFileTypeRegistry.end() )
			return false;

		EditorFileTypeList::iterator iter = std::find(mFileTypeList.begin(),mFileTypeList.end(), &i->second);
		if( iter == mFileTypeList.end() )
			BLADE_EXCEPT(EXC_NEXIST, BTString("corrupted data.") );

		mFileTypeList.erase(iter);
		mFileTypeRegistry.erase(i);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	const SEditorFileInfoEx* EditorFramework::getFileType(const TString& fileClassName) const
	{
		FileTypeRegistry::const_iterator i = mFileTypeRegistry.find( fileClassName );
		if( i != mFileTypeRegistry.end() )
			return &i->second;
		else
			return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	EditorFramework::addFileNotification(IFileUINotify* notif)
	{
		if( notif == NULL )
			return false;
		return mNotifications.insert(notif).second;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	EditorFramework::removeFileNotification(IFileUINotify* notif)
	{
		return mNotifications.erase(notif) == 1;
	}
	
	//////////////////////////////////////////////////////////////////////////
	IEditorFile*	EditorFramework::getEditorFile(FileInstanceID id) const
	{
		OpenedFileMap::const_iterator i = mOpenedFiles.find(id);
		if( i == mOpenedFiles.end() )
			return NULL;
		else
			return i->second;
	}

	//////////////////////////////////////////////////////////////////////////
	void	EditorFramework::initStageConfig(IStage* stage, const HCONFIG& config) const
	{
		for(size_t i = 0; i < config->getSubConfigCount(); ++i)
		{
			const HCONFIG& cfg = config->getSubConfig(i);
			if (cfg == IStageConfig::getSingleton().getStageConfig())
			{
				cfg->synchronize(stage);
				stage->instantProcess();
				stage->postProcess();
				continue;
			}

			IScene* scene = stage->getScene( cfg->getName() );
			if( scene != NULL )
			{
				cfg->synchronize(scene);
				scene->instantProcess();
				scene->postProcess();
			}
		}
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	IEditorFile*	EditorFramework::getOpenedFile(FileInstanceID fileID)
	{
		OpenedFileMap::iterator i = mOpenedFiles.find(fileID);
		if( i == mOpenedFiles.end() )
			return NULL;
		else
			return i->second;
	}

	//////////////////////////////////////////////////////////////////////////
	void		EditorFramework::newFileInstance(bool bCreate)
	{
		TString dir;
		TString name;
		TString ext;

		if( bCreate )
		{
			size_t fileTypeCount = this->getFileTypeCount();
			TempVector<IUIMainWindow::NEW_FILE_DESC> desc(fileTypeCount);
			for (size_t i = 0; i < fileTypeCount; ++i)
			{
				const SEditorFileInfo* info = IEditorFramework::getSingleton().getFileType((FileTypeID)i);		
				desc[i].mDesc = info->mFileDescription;
				desc[i].mExtension = info->mFileExtension;
				desc[i].mDefaultPath = info->mDefaultPath;
				desc[i].mIcon = info->mIconID;
				desc[i].mNewFileConfig = info->mNewFileConfig;
				desc[i].mEnable = (info->mOperations&EFO_NEW) && info->mOpenedCount < info->mMaxOpenLimit;
			}
			if (desc.size() == 0)
				return;
			std::sort(desc.begin(), desc.end());
			if( !mEditorUI->showCreateFileUI(ext, dir, name, &desc[0], desc.size()) )
				return;
		}
		else
		{
			TStringParam exts;
			TStringParam descs;
			size_t fileTypeCount = this->getFileTypeCount();
			for (size_t i = 0; i < fileTypeCount; ++i)
			{
				const SEditorFileInfo* info = IEditorFramework::getSingleton().getFileType((FileTypeID)i);
				if ((info->mOperations&EFO_OPEN))
				{
					exts.push_back(info->mFileExtension);
					descs.push_back(info->mFileDescription);
				}
			}

			size_t importerCount = IImporterManager::getSingleton().getImporterCount();
			for (size_t i = 0; i < importerCount; ++i)
			{
				const SEditorImporterInfo* info = IImporterManager::getSingleton().getImporterInfo(i);
				for (size_t j = 0; j < info->mExtensions.size(); ++j)
				{
					exts.push_back(info->mExtensions[j]);
					descs.push_back(info->mDescription);
				}
			}

			if( !mEditorUI->showOpenFileUI(ext, dir, name, exts, descs) )
				return;
		}
		BLADE_LW_PROFILING_FUNCTION();

		SEditorFileInfoEx* pInfo = this->getFileTypeByExtension(ext);
		IconIndex icon = INVALID_ICON_INDEX;
		HIMPORT import;
		if (pInfo == NULL)
		{
			if (!bCreate)
			{
				import = ImporterManager::getSingleton().doImport(dir, name, ext);
				if (import != NULL)
				{
					pInfo = import->mTargetInfo;
					icon = import->mInfo->mIconID;
				}
			}
		}
		else
			icon = pInfo->mIconID;

		if (pInfo == NULL)
		{
			assert(false);
			return;
		}

		if( pInfo->mMaxOpenLimit <= pInfo->mOpenedCount )
		{
			IPlatformManager::getSingleton().showMessageBox(BTLang("cannnot open file - file type reaches max limit.").c_str(),
				BTLang(BLANG_ERROR).c_str(), IPlatformManager::MBT_OK);
			return;
		}

		++pInfo->mOpenedCount;

		IEditorFile* file = BLADE_FACTORY_CREATE(IEditorFile, pInfo->mFileClassName );
		FileInstanceID fileID = (FileInstanceID)mFielHandleCounter.increment();

		IUIWindow* editorWindow = NULL;
		if( pInfo->mWindowType != TString::EMPTY )
		{
			BLADE_LW_PROFILING(CREATE_EDITOR_WINDOW);
			const TString& fileType = BTString2Lang(pInfo->mFileClassName);
			TString caption = TEXT("[") + fileType + TEXT("]");
			editorWindow = mEditorUI->getLayoutManager()->createWindow(icon, pInfo->mWindowType, caption);
			editorWindow->setFrameworkData(file);
			editorWindow->getWindow()->addEventHandler(this);
			mUIWindowMap[editorWindow->getWindow()] = editorWindow;
			assert(editorWindow != NULL );
		}
		file->initialize(pInfo, fileID, editorWindow);

		//this one is the first
		IEditorFile* prevFile = mActiveFile;
		if( prevFile != NULL && prevFile->getFileUpdater() != NULL )
			prevFile->getFileUpdater()->onActivate(prevFile, false);

		mActiveFile = file;

		if( bCreate )
		{
			if( pInfo->mNewFolder )
			{
				TString base;
				TStringHelper::getFileInfo(name,base,ext);
				dir = dir + BTString("/") + base;
			}
			file->newFile(dir,name);
		}
		else
		{
			BLADE_LW_PROFILING(OPEN_FILE);
			mCurrentOpeningFile = file;
			file->openFile(dir,name);
			mCurrentOpeningFile = NULL;
		}
		mOpenedFiles[fileID] = file;

		for( FileNotification::iterator i = mNotifications.begin(); i != mNotifications.end(); ++i )
		{
			IFileUINotify* notify = *i;
			notify->onFileSwitch(prevFile, file);
		}

		if (file->getLogicScene() != NULL /*&& EditorLogicFactory::getSingleton().isClassRegistered(pInfo->mWindowType)*/)
		{
			IEditorLogic* logic = BLADE_FACTORY_CREATE(IEditorLogic, pInfo->mWindowType);
			bool ret = logic->initalize(file, editorWindow);
			assert(ret); BLADE_UNREFERENCED(ret);
			file->getLogicScene()->addLogic(HLOGIC(logic));
		}

		//TODO:editorWindow->onFileCreated(file);
		if (file->getFileUpdater() != NULL)
		{
			editorWindow->setUpdater(file->getFileUpdater());
			file->getFileUpdater()->setupWindow(editorWindow);
			file->getFileUpdater()->onActivate(file, true);
		}
		//stage will modify window caption, otherwise we need modify it here
		if(file->getStage() == NULL)
			editorWindow->getWindow()->setCaption(name + BTString(" - ") + editorWindow->getWindow()->getCaption() );
	}
}//namespace Blade