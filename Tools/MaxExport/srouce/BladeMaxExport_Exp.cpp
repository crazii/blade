/********************************************************************
	created:	2013/04/02
	filename: 	BladeMaxExport_Exp.cpp
	author:		Crazii
	
	purpose:	real export functions definition
*********************************************************************/
#include "AnimDefDialog.h"
#include "BladeMaxExport.h"
#include <utility/BladeContainer.h>
#include <interface/public/graphics/Color.h>

#include <databinding/DataSourceWriter.h>
#include <interface/IConfigManager.h>
#include <utility/StringHelper.h>

using namespace Blade;

//////////////////////////////////////////////////////////////////////////
HINSTANCE	 GetDLLInstance()
{
	AFX_MANAGE_STATE( ::AfxGetStaticModuleState() );
	return ::AfxGetInstanceHandle();
}

//////////////////////////////////////////////////////////////////////////
//options
//////////////////////////////////////////////////////////////////////////
const TString ExportConfig::EXPORT_FMT_BINARY = BTString("Binary");
const TString ExportConfig::EXPORT_FMT_XML = BTString("XML");
const TString ExportConfig::EXPORT_NORMAL_NONE = BTString("None");
const TString ExportConfig::EXPORT_NORMAL_VECTOR = BTString("Normal Vector");
const TString ExportConfig::EXPORT_NORMAL_TANGENTFRAME = BTString("Tangent Space");
const TString ExportConfig::EXPORT_SINGLE_ANIMATION = BTString("Single Animation");
const TString ExportConfig::EXPORT_MULTIPLE_ANIMATION = BTString("Multiple Animations");
const TString ExportConfig::EXPORT_TANGENT_LEFT_HANDED = BTString("Left Handed");
const TString ExportConfig::EXPORT_TANGENT_RIGHT_HANDED = BTString("Right Handed");

ExportConfig::ExportConfig()
{
	//set default value
	mNormalType = EXPORT_NORMAL_TANGENTFRAME;
	mFormat = EXPORT_FMT_BINARY;
	mTSHandedness = EXPORT_TANGENT_RIGHT_HANDED;	//use Blade default.

	mFPS = 25;
	mAngleError = 0.4;
	mPositionError = 0.004;
	mAnimationType = EXPORT_SINGLE_ANIMATION;
	mAnimationName = TString::EMPTY;
	mStartFrame = -1;
	mEndFrame = -1;
	mLoop = true;
	mAnimationListDesc = TString::EMPTY;

	mExportDummyHelper = false;
	mExportLight = false;
	mExportCamera = false;

	mExportSelected = false;
	mExportModel = false;
	mExportAnimation = false;
}

//////////////////////////////////////////////////////////////////////////
const TString BladeMaxExport::getConfigFile()
{
	TString path = ::GetCOREInterface()->GetDir(APP_PLUGCFG_DIR);
	if( mConfig.mExportModel )
		path += BTString("\\") + BTString("BladeMaxModelExport.xml");
	else if( mConfig.mExportAnimation )
		path += BTString("\\") + BTString("BladeMaxAnimationExport.xml");
	else
		path += BTString("\\") + BTString("BladeMaxExport.xml");
	return path;
}

//////////////////////////////////////////////////////////////////////////
void	BladeMaxExport::InitliazeConfig(bool model, bool animation, bool selected, int startFrame, int endFrame, const TCHAR* maxFilePath)
{
	mConfig.mExportModel = model;
	mConfig.mExportAnimation = animation;
	mConfig.mExportSelected = selected;

	{
		DataSourceWriter<ExportConfig> writer( mModelConfig );
		writer.beginVersion( Version(0, 2,'C') );
		//TODO: add XML format for mesh/skeleton/animations
		//TStringParam formatOptions;
		//formatOptions.push_back( ExportConfig::EXPORT_FMT_BINARY );
		//formatOptions.push_back( ExportConfig::EXPORT_FMT_XML );
		//writer << BTString("File Format")	<<	formatOptions	<<	&ExportConfig::mFormat;

		TStringParam normalOptions;
		normalOptions.push_back( ExportConfig::EXPORT_NORMAL_NONE );
		normalOptions.push_back( ExportConfig::EXPORT_NORMAL_VECTOR );
		normalOptions.push_back( ExportConfig::EXPORT_NORMAL_TANGENTFRAME );
		writer << BTString("Normal Option")	<<	normalOptions << &ExportConfig::mNormalType;

		TStringParam tangentOptions;
		tangentOptions.push_back( ExportConfig::EXPORT_TANGENT_LEFT_HANDED );
		tangentOptions.push_back( ExportConfig::EXPORT_TANGENT_RIGHT_HANDED );
		writer << BTString("Tangent Space Handedness") << tangentOptions << &ExportConfig::mTSHandedness;

		//writer << BTString("Mirrored Texture") <<	&ExportConfig::mHasMirror;
		writer << BTString("Export Dummy") <<	&ExportConfig::mExportDummyHelper;
		writer << BTString("Export Cameras") <<	&ExportConfig::mExportCamera;
		writer << BTString("Skeleton Reference") << &ExportConfig::mSkeltonRef;
		writer.endVersion();
	}
	
	{
		DataSourceWriter<ExportConfig> writer( mAnimationConfig );
		writer.beginVersion( Version(0, 2,'C'));
		writer << BTString("Sampling Frame Rate") << &ExportConfig::mFPS;
		writer << BTString("Optimization Angle(Degree) Error") << &ExportConfig::mAngleError;
		writer << BTString("Optimization Position Error") << &ExportConfig::mPositionError;

		TStringParam animtionType;
		animtionType.push_back( ExportConfig::EXPORT_SINGLE_ANIMATION );
		animtionType.push_back( ExportConfig::EXPORT_MULTIPLE_ANIMATION );
		DataSourceWriter<ExportConfig> animTypeWriter = (writer << BTString("Animtaion Definition") << animtionType << DS_GROUP<ExportConfig>(&ExportConfig::mAnimationType) );

		animTypeWriter[ExportConfig::EXPORT_SINGLE_ANIMATION] << BTString("Animation Name") << &ExportConfig::mAnimationName;
		animTypeWriter[ExportConfig::EXPORT_SINGLE_ANIMATION] << BTString("Start Frame") << &ExportConfig::mStartFrame;
		animTypeWriter[ExportConfig::EXPORT_SINGLE_ANIMATION] << BTString("End Frame") << &ExportConfig::mEndFrame;
		animTypeWriter[ExportConfig::EXPORT_SINGLE_ANIMATION] << BTString("Is Looping") << &ExportConfig::mLoop;

		CONFIG_UIHINT customHint = CUIH_CUSTOM;
		customHint.mCustomCall = Delegate(this, &BladeMaxExport::ConfigAnimationList);
		animTypeWriter[ExportConfig::EXPORT_MULTIPLE_ANIMATION] << BTString("Animations List") << customHint << &ExportConfig::mAnimationListDesc;

		writer.endVersion();
	}
	mModelConfig->bindTarget( &mConfig );
	mAnimationConfig->bindTarget( &mConfig );

	//
	IConfigManager::getSingleton().clearAllConfigs();
	if( model )
		IConfigManager::getSingleton().addConfig( mModelConfig );
	if( animation )
		IConfigManager::getSingleton().addConfig( mAnimationConfig );
	HSTREAM stream = IResourceManager::getSingleton().loadStream( this->getConfigFile() );
	if( stream != NULL )
		IConfigManager::getSingleton().loadConfig(stream);

	//limit range
	if( mConfig.mStartFrame < startFrame )
		mConfig.mStartFrame = startFrame;
	if( mConfig.mEndFrame > endFrame || mConfig.mEndFrame < 0)
		mConfig.mEndFrame = endFrame;

	mCollector.mAnimations.clear();

	//try loading animation list
	TString path = maxFilePath, dir, filename;
	TStringHelper::getFilePathInfo(path, dir, filename);
	TString animDefFile = dir + BTString("\\") + BTString("animdef.txt");
	mConfig.mAnimationListDesc = animDefFile;
	stream = IResourceManager::getSingleton().loadStream( mConfig.mAnimationListDesc );
	ConfigFile file;
	if( !file.open( stream ) )
		return;

	const CFGFILE_SETTING* setting = file.getAllConfigs();
	if( setting == NULL || setting->getSectionCount() != 1 || setting->getSection(0).getEntryCount() == 0)
		return;

	const CFGFILE_SECTION& section = setting->getSection(0);
	bool configValid = true;
	for(size_t i = 0; i < section.getEntryCount(); ++i)
	{
		const CFGFILE_ENTRY& entry = section.getEntry(i);
		const TString& name = entry.getName();
		const TString& value = entry.getValue();
		if( name == TString::EMPTY || value == TString::EMPTY )
		{
			configValid = false;
			break;
		}
		TStringTokenizer tokenizer;
		tokenizer.tokenize(value, TEXT(","));
		if( tokenizer.size() != 3 )
		{
			configValid = false;
			break;
		}
		int start = TStringHelper::toInt( tokenizer[0] );
		int end = TStringHelper::toInt( tokenizer[1] );
		bool loop = TStringHelper::toBool(tokenizer[2]);
		if( start >= end || start < startFrame || end > endFrame )
		{
			configValid = false;
			break;
		}

		AnimationDef def;
		def.Name = name;
		def.StartFrame = start;
		def.EndFrame = end;
		def.Loop = loop;
		mCollector.mAnimations.push_back( def );
	}//for

	if( !configValid )
		mCollector.mAnimations.clear();
}

//////////////////////////////////////////////////////////////////////////
bool	BladeMaxExport::CheckAnimationList(int startFrame, int endFrame)
{
	if( !mConfig.mExportAnimation )
		return true;

	if( mConfig.mAnimationType == ExportConfig::EXPORT_SINGLE_ANIMATION )
	{
		if( mConfig.mAnimationName == TString::EMPTY )
		{
			::MessageBox( ::GetActiveWindow(), TEXT("Animation Name must be valid value."), TEXT("Invalid Value"), MB_ICONEXCLAMATION );
			return false;
		}
		else if( mConfig.mStartFrame >= mConfig.mEndFrame || mConfig.mEndFrame > endFrame || mConfig.mStartFrame < startFrame )
		{
			::MessageBox( ::GetActiveWindow(), TEXT("Start Frame & End Frame must be valid value."), TEXT("Invalid Value"), MB_ICONEXCLAMATION );
			return false;
		}

		AnimationDef def;
		def.Name = mConfig.mAnimationName;
		def.StartFrame = mConfig.mStartFrame;
		def.EndFrame = mConfig.mEndFrame;
		def.Loop = true;
		mCollector.mAnimations.push_back( def );
		return true;
	}
	else if( mConfig.mAnimationType == ExportConfig::EXPORT_MULTIPLE_ANIMATION )
	{
		if( mCollector.mAnimations.size() == 0 )
		{
			::MessageBox( ::GetActiveWindow(), TEXT("No Animations in animation list."), TEXT("Invalid Value"), MB_ICONEXCLAMATION );
			return false;
		}
		return true;
	}
	assert(false);
	return false;
}

//////////////////////////////////////////////////////////////////////////
void		BladeMaxExport::ConfigAnimationList(void)
{
	//now put the dialog CWnd into UIPlugin(DLL), we only need local resources
	//AFX_MANAGE_STATE(::AfxGetStaticModuleState());
	AnimDefDialog dlg;
	dlg.setMaxFrame((uint32)mConfig.mEndFrame);
	AnimationDefList animList = mCollector.mAnimations;
	dlg.setAnimationList(&animList);
	//save resource for child windows, located in UIPluing
	//note: ConfigAnimationList is called from UI plugin(Config Dialog), so current 
	//MFC Instance handle & Resource handle is UIPlugin, but
	//we need more flexibility, in case that ConfigAnimationList() is not called from UIPlugin
	HINSTANCE hPrev = AfxGetResourceHandle();
	dlg.SetResourceHandle(mUIInstance);
	int result = -1;
	try
	{
		//the local DLL holds resource for the dialog
		AfxSetResourceHandle(GetDLLInstance());
		result = dlg.DoModal();
	}
	catch(Exception& /*e*/)
	{
		result = IDCANCEL;
	}
	catch(std::exception& /*e*/)
	{
		result = IDCANCEL;
	}
	catch(...)
	{
		result = IDCANCEL;
	}
	//restore
	AfxSetResourceHandle(hPrev);

	if(result != IDOK)
		return;

	mCollector.mAnimations = animList;

	//save animation list
	HSTREAM stream = IResourceManager::getSingleton().openStream( mConfig.mAnimationListDesc, false);
	TextFile file;
	if( !file.open( stream ) )
		return;

	file.setDestEncoding(TE_UTF8, true);
	file.write(BTString("//name=startFrame,endFrame,loop\r\n"));
	
	for(size_t i = 0; i < animList.size(); ++i)
	{
		const AnimationDef& def = animList[i];
		TString line = def.Name
			+ BTString("=") + TStringHelper::fromInt(def.StartFrame)
			+ BTString(",") + TStringHelper::fromInt(def.EndFrame)
			+ BTString(",") + TStringHelper::fromBool(def.Loop)
			+ BTString("\r\n");
		file.write(line);
	}
}