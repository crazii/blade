//**************************************************************************/
// Copyright (c) 1998-2007 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// DESCRIPTION: Appwizard generated plugin
// AUTHOR: 
//***************************************************************************/

#include "BladeMaxExport.h"
#include <utility/BladeContainer.h>
#include <utility/StringHelper.h>
#include <BladeEditor.h>
#include <ExceptionEx.h>
#include <interface/public/graphics/IGraphicsResourceManager.h>
#include <interface/IFramework.h>
#include <interface/IConfigManager.h>
#include <interface/IPlatformManager.h>
#include <interface/IEnvironmentManager.h>
#include <databinding/IDataSourceDataBase.h>
#include <ConfigTypes.h>

//////////////////////////////////////////////////////////////////////////
const TCHAR* BladeMaxExportGetFileExtension(void)
{
	return Blade::ModelConsts::MODEL_FILE_EXT.c_str();
}

//////////////////////////////////////////////////////////////////////////
const TCHAR* BladeMaxAnimationExportGetFileExtension(void)
{
	return Blade::ModelConsts::SKELETON_FILE_EXT.c_str();
}

//////////////////////////////////////////////////////////////////////////
int BladeMaxExportEntry(const TCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts, DWORD options)
{
	//Init:
	BOOL result = TRUE;

	Blade::initializeFoundation();
	Blade::initializeFramework();
	//leak dump need keep DLL module never unloaded, but we need unload all DLL every time.
	Blade::Memory::enableLeakDump(false);

	TCHAR path[MAX_PATH];
	::GetModuleFileName(GetDLLInstance(), path, MAX_PATH);

	Blade::TString dir, file;
	Blade::TStringHelper::getFilePathInfo(Blade::TString(path), dir, file);

	try
	{
		//To use ITableEditorWindow as animation config,
		//Now we use framework to load all plugins, its pretty heavy weight...
		Blade::IEnvironmentManager::getSingleton().setVariable( Blade::ConstDef::EnvString::WORKING_MODE, BTString("tool") );

		Blade::ParamList plugins;
		plugins[BTString("BladeDevice.blugin")] = true;	//device plugin (window device) needed by window system
		plugins[BTString("BladeWindow.blugin")] = true;	//Window system needed by UIPlugin
		plugins[BTString("BladeImageFI.blugin")] = true;	//for UI to load icon
		plugins[BTString("BladeUI.blugin")] = true;		//UI system needed by UIPlugin
		plugins[BTString("BladeUIPlugin.blugin")] = true;//UI to use ITableEditorWindow
		plugins[BTString("BladeModel.blugin")] = true;
		Blade::IPlatformManager::getSingleton().initialize(plugins, dir);
		HINSTANCE hUIInstance = NULL;
		Blade::IPlugin* UIPlugin = Blade::IPlatformManager::getSingleton().getPlugin( BTString("BladeUIPlugin") );
		if( UIPlugin != NULL )
			hUIInstance = (HINSTANCE)UIPlugin->getAddOnObject(BTString("HINSTANCE"));

		Blade::IResourceManager::getSingleton().initialize(Blade::IPlatformManager::getSingleton().getProcesFilePath(), 
			BTString("."), 
			Blade::IPlatformManager::getSingleton().getCurrentWorkingPath());

		//start Window/UI Service...
		Blade::IFramework::getSingleton().initialize();
		Blade::IFramework::getSingleton().addSubSystem(BTString("BladeWindowSystem"));
		Blade::IFramework::getSingleton().addSubSystem(BTString("BladeUISystem"));
		Blade::IFramework::getSingleton().start();

		//Do export
		BladeMaxExport exp(hUIInstance);
		result = exp.doExport(name, ei, i, suppressPrompts, options);

		//shutdown framework...
		Blade::IFramework::getSingleton().terminate();
		Blade::IFramework::getSingleton().shutdown();
		Blade::IDataSourceDataBase::getSingleton().shutdown();
	}
	catch(Blade::Exception& /*e*/)
	{
		result = FALSE;
	}
	catch(std::exception& /*e*/)
	{
		result = FALSE;
	}
	catch(...)
	{
		result = FALSE;
	}
	Blade::IPlatformManager::getSingleton().unloadAllPlugins();
	return result;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
//////////////////////////////////////////////////////////////////////////
static DWORD WINAPI ExpCallbackFn(LPVOID /*lpThreadParameter*/)
{
	return 0;
}

//////////////////////////////////////////////////////////////////////////
static bool IsBoneNode(INode* node)
{
	bool isBone = false;

	Object* obj = node->EvalWorldState(0).obj;
	Class_ID cid = obj->ClassID();

	//// nodes that have Biped controllers are bones -- ignore the ones that are dummies
	//if (cid == Class_ID(DUMMY_CLASS_ID, 0))
	//	return TREE_CONTINUE;

	//NOTE: pre-collect bones - bone index will be used on mesh export later
	//NOTE: all bones will be exported, even bones that are not referenced by any skinning mesh - this is useful to change/add attachments to an empty bone.
	//check bones: skin/physique modifier
	Control* c = node->GetTMController();
	if ( c->ClassID() == BIPSLAVE_CONTROL_CLASS_ID
		|| c->ClassID() == BIPBODY_CONTROL_CLASS_ID
		|| c->ClassID() == FOOTPRINT_CLASS_ID
		)
	{
		isBone = true;
	}

	if( cid == Class_ID(BONE_CLASS_ID, 0) || 
		cid == Class_ID(DUMMY_CLASS_ID, 0) || 
		cid == BONE_OBJ_CLASSID 
		)
	{
		isBone = true;
	}
	return isBone;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
class MaxExportCallback : public Blade::IProgressCallback 
{
public:
	MaxExportCallback(Interface* i):mInterface(i)	{ mPercent = 0; }
	virtual ~MaxExportCallback()	{}

	/* @brief  */
	virtual void onNotify(Blade::scalar percent)
	{
		int p = (int)(percent*100);
		if( mPercent != p )
		{
			mPercent = p;
			mInterface->ProgressUpdate( mPercent, TRUE, TEXT("Exporting...") );
		}
	}
protected:
	Interface* mInterface;
	int mPercent;
};


//////////////////////////////////////////////////////////////////////////
BladeMaxExport::BladeMaxExport(HINSTANCE hUIPlugin)
{
	mModelConfig.bind( BLADE_NEW Blade::ConfigGroup( BTString("Model") ) );
	mAnimationConfig.bind( BLADE_NEW Blade::ConfigGroup( BTString("Animation") ) );
	mUIInstance = hUIPlugin;
}

//////////////////////////////////////////////////////////////////////////
BladeMaxExport::~BladeMaxExport() 
{
	Blade::IConfigManager::getSingleton().clearAllConfigs();
}

//////////////////////////////////////////////////////////////////////////
//Note: this function cames from part of the OGRE source codes
//and some modification is made
int BladeMaxExport::callback(INode* node)
{
	//Interface* i = ::GetCOREInterface();
	TimeValue time = 0;//i->GetTime();
	Object* obj = node->EvalWorldState(time).obj;

	//skip animation info collection
	//TODO: do animation check on the export pass?
	bool isBone = mConfig.mExportAnimation && IsBoneNode(node);

	// if the node cannot be converted to a TriObject (mesh), ignore it
	if ( !obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0)) 
		&& !obj->CanConvertToType(Class_ID(LIGHT_CLASS_ID,0))
		&& !obj->CanConvertToType(Class_ID(CAMERA_CLASS_ID,0))
		&& !(isBone && node->IsRootNode() )
		)
	{
		return TREE_CONTINUE;
	}
	mNodeList.Append(1, &node);
	return TREE_CONTINUE;
}

//////////////////////////////////////////////////////////////////////////
void	BladeMaxExport::collectBones(ExpInterface *ei)
{
	//IGameScene* game = ::GetIGameInterface();

	struct BoneCollector : public ITreeEnumProc, public Blade::NonAssignable
	{
		Blade::BoneList& listRef;
		BoneCollector(Blade::BoneList& list) :listRef(list){}

		virtual int callback(INode* node)
		{
			if( IsBoneNode(node) )
			{
				IGameScene* game = ::GetIGameInterface();
				IGameNode* iGameNode = game->GetIGameNode(node);
				if( iGameNode != NULL )
					listRef.push_back(iGameNode);
			}
			return TREE_CONTINUE;
		}
	}collector(mCollector.mBoneList);
	ei->theScene->EnumTree(&collector);

	mCollector.initBoneData();
}


//////////////////////////////////////////////////////////////////////////
int	BladeMaxExport::doExport(const TCHAR *name, ExpInterface *ei, Interface* i, BOOL suppressPrompts, DWORD options)
{
	Interval range =  i->GetAnimRange();
	int startFrame =  range.Start() / ::GetTicksPerFrame();
	int endFrame = range.End() / ::GetTicksPerFrame();

	Blade::AnimationDefList& animations = mCollector.mAnimations;

	//max uses uppercase for file extensions, convert it to lowercase.
	Blade::TString path(name);
	Blade::TString dir,fileName,fileBase,fileExt;
	Blade::TStringHelper::getFilePathInfo(path, dir, fileName, fileBase, fileExt);
	Blade::TStringHelper::toLowerCase(fileExt);

	bool exportModel = fileExt == Blade::ModelConsts::MODEL_FILE_EXT;
	bool exportAnimation = (fileExt == Blade::ModelConsts::SKELETON_FILE_EXT);// || (startFrame != endFrame);//auto export animation?
	const MSTR& maxFile = GetCOREInterface()->GetCurFilePath();
	this->InitliazeConfig(exportModel, exportAnimation, options == SCENE_EXPORT_SELECTED, startFrame, endFrame, maxFile.data());
	//ignore config for XML file, use runtime spec
	mConfig.mSkeltonRef = fileBase;

	if( !suppressPrompts )
	{
		try
		{
			do
			{
				if( !Blade::IConfigManager::getSingleton().showConfigDialog( BTString("Export Options") ) )
				{
					//::MessageBox( ::GetActiveWindow(), TEXT("Blade model exporting is canceled."), TEXT("Canceled"), MB_ICONEXCLAMATION );
					i->SetCancel(TRUE);
					return TRUE;
				}
				else if( this->CheckAnimationList(startFrame, endFrame) )
					break;
			}while(true);
		}

		catch(Blade::Exception& e)
		{
			//BLADE_LOG(ILog::Error, e.getFullDescription());
			Blade::IPlatformManager::getSingleton().showMessageBox(e.getFullDescription(),e.getTypeName() );
			return FALSE;
		}
		catch(...)
		{
			return FALSE;
		}

		Blade::HSTREAM stream = Blade::IResourceManager::getSingleton().openStream( this->getConfigFile(), false);
		Blade::IConfigManager::getSingleton().saveConfig( stream );
	}
	else if( exportAnimation )
	{
		Blade::AnimationDef def;
		def.Name = BTString("default");
		def.StartFrame = startFrame;
		def.EndFrame = endFrame;
		animations.push_back( def );
	}

	//we don't walk the IGame node tree recursively to enumerate all nodes
	//instead, first enumerate(recursively) the max scene with ITreeEnumProc, and collect useful INodes
	//then process the corresponding IGameNode list in one linear pass without recursion
	mNodeList.Init();
	//collect data
	ei->theScene->EnumTree(this);
	
	//info:export starting
	i->ProgressStart( TEXT("Start exporting model..."), TRUE, ExpCallbackFn, NULL);

	//IGame interface
	IGameScene* game = ::GetIGameInterface();

	//coordinates
	IGameConversionManager* cm = ::GetConversionManager();
	//Blade uses right handed coordinates, like OGL
	//now Blade use OGL uv(right handed tangent space) too.
	cm->SetCoordSystem( IGameConversionManager::IGAME_OGL );

	bool ret = game->InitialiseIGame( mConfig.mExportSelected );
	//bool ret = game->InitialiseIGame(mNodeList, true);
	assert(ret);
	
	int count = game->GetTopLevelNodeCount();
	if( count <= 0 )
	{
		i->ProgressEnd();
		return TRUE;
	}

	Blade::ParamList params;
	Blade::HRESOURCE hModel;
	Blade::HRESOURCE hSkeleton;

	//collect bones, this should be done before collecting any mesh
	this->collectBones(ei);

	//create resources
	{
		if( mConfig.mExportModel )
			hModel = Blade::IResourceManager::getSingleton().createResource(dir + BTString("\\") + fileBase + BTString(".") + fileExt, params);

		if( mConfig.mExportAnimation && mCollector.mBoneList.size() > 0 )
			hSkeleton = Blade::IResourceManager::getSingleton().createResource( dir + BTString("\\") + fileBase + BTString(".") + fileExt, params);

		//create resource failed?
		if( mConfig.mExportModel && hModel == NULL 
			|| mConfig.mExportAnimation && mCollector.mBoneList.size() != 0 && hSkeleton == NULL )
		{
			Blade::IResourceManager::getSingleton().unloadResource(hModel);
			Blade::IResourceManager::getSingleton().unloadResource(hSkeleton);

			i->ProgressEnd();
			return FALSE;
		}
	}

	Blade::IModelResource* exportingModel = static_cast<Blade::IModelResource*>(hModel);
	Blade::ISkeletonResource* exportingSkeleton = static_cast<Blade::ISkeletonResource*>(hSkeleton);

	MaxExportCallback expCallback(i);
	Blade::ProgressNotifier notifier;
	notifier = &expCallback;

	//collect model data
	notifier.beginStep( 0.1 );
	for(int n = 0; n < count; n++)
	{
		notifier.onStep((size_t)n, (size_t)count);

		IGameNode * gameNode = game->GetTopLevelNode(n);

		if( options == SCENE_EXPORT_SELECTED && !gameNode->GetMaxNode()->Selected() )		
			continue;

		IGameObject * obj = gameNode->GetIGameObject();
		switch(obj->GetIGameType())
		{
		//NOTE: in-model light won't be supported
		//lights will be supported at blade scene editor & blade effect editor level.
		//case IGameObject::IGAME_LIGHT:
		//	break;
		case IGameObject::IGAME_CAMERA:
			if( mConfig.mExportModel )
			{
				//we may need portrait camera to be exported
				this->CollectCamera( (IGameCamera*)obj, exportingModel);
			}
			break;
		case IGameObject::IGAME_MESH:
			{
				IGameMesh* mesh = (IGameMesh*)obj;
				if(mesh->InitializeData() && mConfig.mExportModel)
					this->CollectMesh(gameNode->GetName(), mesh, gameNode->GetNodeMaterial());
			}
			break;
		default:
			break;
		}//switch
	}

	//export model
	notifier.beginStep(0.9);

	//export skeleton
	bool skeletonExport = mConfig.mExportAnimation && mCollector.mBoneList.size() != 0;	

	assert(mConfig.mExportModel || skeletonExport);

	if( mConfig.mExportModel )
		this->ExportModel(exportingModel, notifier);


	if( skeletonExport )
	{
		assert( exportingSkeleton != NULL );
		this->ExportSkeleton(exportingSkeleton, notifier);
	}

	game->ReleaseIGame();

	if( hModel != NULL )
	{
		if( mCollector.mBoneList.size() != 0 )
			exportingModel->setSkeletonRefernce( mConfig.mSkeltonRef + BTString(".") + Blade::ModelConsts::SKELETON_FILE_EXT);
		Blade::IResourceManager::getSingleton().saveResourceSync( hModel, false );
		Blade::IResourceManager::getSingleton().unloadResource(hModel);
	}

	if( exportingSkeleton != NULL )
	{
		Blade::IResourceManager::getSingleton().saveResourceSync( hSkeleton, false);
		Blade::IResourceManager::getSingleton().unloadResource(hSkeleton);
	}

	i->ProgressEnd();
	return TRUE;
}