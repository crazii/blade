#pragma once

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
// DESCRIPTION: Includes for Plugins
// AUTHOR: 
//***************************************************************************/
#include <BladePlatform.h>
#define NOMINMAX
#include "3dsmaxsdk_preinclude.h"
#include "Max.h"
#include "resource.h"
#include "istdplug.h"
#include "iparamb2.h"
#include "iparamm2.h"
#include "modstack.h"
#include "iskin.h"
#include "decomp.h" 
#include "CS/Phyexp.h"
#include "CS/BIPEXP.H"
//SIMPLE TYPE

#include "IGame/IGame.h"
#include "IGame/IConversionManager.h"
#include "IGame/IGameObject.h"
#include "IGame/IGameModifier.h"

//max sdk macro conflict with Blade::PI
#undef PI
#undef INFINITE	//winbase.h
#undef base_type

#include <utility/BladeContainer.h>
#include <interface/IConfig.h>
#include <interface/IModelResource.h>
#include <interface/ISkeletonResource.h>
#include <interface/IResourceManager.h>
#include <interface/public/ISerializable.h>
#include <interface/IModelBuilder.h>

#include "ExportData.h"

#if BLADE_DOUBLE_PRESITION
#error float/double mismatch.
#endif

/************************************************************************/
/*                                                                      */
/************************************************************************/
extern "C"
{
	__declspec(dllexport) const TCHAR* __cdecl BladeMaxExportGetFileExtension(void);
	__declspec(dllexport) const TCHAR* __cdecl BladeMaxAnimationExportGetFileExtension(void);
	__declspec(dllexport) int __cdecl BladeMaxExportEntry(const TCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts, DWORD options);
};

/* @brief get instance of current DLL (BladeMaxExporter) */
HINSTANCE		GetDLLInstance();


/************************************************************************/
/*                                                                      */
/************************************************************************/
class ExportConfig : public Blade::Bindable
{
public:
	static const Blade::TString EXPORT_FMT_BINARY;
	static const Blade::TString EXPORT_FMT_XML;
	static const Blade::TString EXPORT_NORMAL_NONE;
	static const Blade::TString EXPORT_NORMAL_VECTOR;
	static const Blade::TString EXPORT_NORMAL_TANGENTFRAME;
	static const Blade::TString EXPORT_SINGLE_ANIMATION;
	static const Blade::TString EXPORT_MULTIPLE_ANIMATION;
	static const Blade::TString EXPORT_TANGENT_LEFT_HANDED;
	static const Blade::TString EXPORT_TANGENT_RIGHT_HANDED;

	/* @brief  */
	inline Blade::IModelResource::ENormalType getNormalType() const
	{
		if( mNormalType == EXPORT_NORMAL_TANGENTFRAME )
			return Blade::IModelResource::NT_TANGENT_FRAME;
		else if( mNormalType == EXPORT_NORMAL_VECTOR )
			return Blade::IModelResource::NT_NORMAL_VECTOR;
		else
			return Blade::IModelResource::NT_NONE;
	}

	/* @brief  */
	inline bool	isTangentSpaceLeftHanded() const
	{
		return mTSHandedness == EXPORT_TANGENT_LEFT_HANDED;
	}

	/* @brief  */
	inline bool	isXMLFormat() const
	{
		return mFormat == EXPORT_FMT_XML;
	}

	ExportConfig();
	Blade::TString	mNormalType;
	Blade::TString	mFormat;			//XML export or binary
	Blade::TString	mTSHandedness;		//handedness for TBN
	Blade::TString	mSkeltonRef;		//skeleton for exporting mesh

	Blade::scalar	mFPS;
	Blade::scalar	mAngleError;		//accumulated angle error in degree used for key frame optimization
	Blade::scalar	mPositionError;		//accumulated position error for key frame optimization

	Blade::TString	mAnimationType;		//single or multiple
	Blade::TString	mAnimationName;		//exporting animation name
	int				mStartFrame;
	int				mEndFrame;
	bool			mLoop;
	Blade::TString	mAnimationListDesc;	//animation definition file when exporting multiple animations

	//bool	mHasMirror;
	bool	mExportDummyHelper;
	bool	mExportLight;
	bool	mExportCamera;
	//runtime config
	bool	mExportAnimation;
	bool	mExportModel;
	bool	mExportSelected;
};

/************************************************************************/
/*                                                                      */
/************************************************************************/
class BladeMaxExport : public ITreeEnumProc
{
public:	

	BladeMaxExport(HINSTANCE hUIPlugin);
	~BladeMaxExport();

	/* @brief  */
	int		doExport(const TCHAR *name, ExpInterface *ei, Interface *i, BOOL suppressPrompts = FALSE, DWORD options = 0);

	/* @brief ITreeEnumProc interface */
	virtual int callback(INode *node);
	void	collectBones(ExpInterface *ei);

protected:
	void	CollectCamera(IGameCamera* camera, Blade::IModelResource* target);
	void	CollectMesh(const TCHAR* name, IGameMesh* mesh, IGameMaterial* material);
	void	ExportModel(Blade::IModelResource* target, Blade::ProgressNotifier& callback);
	void	ExportSkeleton(Blade::ISkeletonResource* target, Blade::ProgressNotifier& callback);

	/* @brief  */
	const Blade::TString getConfigFile();
	void	InitliazeConfig(bool model, bool animation, bool selected, int startFrame, int endFrame, const TCHAR* maxFilePath);
	//export skeletons & animation using mBoneList 
	bool	CheckAnimationList(int startFrame, int endFrame);
	void	ConfigAnimationList();

	Blade::HCONFIG	mModelConfig;
	Blade::HCONFIG	mAnimationConfig;
	ExportConfig	mConfig;

	Tab<INode*>		mNodeList;

	Blade::MaxModelCollector	mCollector;

	HINSTANCE		mUIInstance;	//Instance of UIPluign
};