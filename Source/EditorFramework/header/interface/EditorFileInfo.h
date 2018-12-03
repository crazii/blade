/********************************************************************
	created:	2010/06/13
	filename: 	EditorFileInfo.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_EditorFileInfo_h__
#define __Blade_EditorFileInfo_h__
#include <utility/StringList.h>
#include <interface/IConfig.h>
#include <BladeEditor.h>

namespace Blade
{
	typedef uint FileInstanceID;
	static const FileInstanceID	INVALID_FILE_INSTANCE = 0xFFFFFFFF;

	typedef uint FileTypeID;
	static const FileTypeID INVALID_FILETYPE = 0xFFFFFFFF;

	enum EEditorFileOperation
	{
		EFO_OPEN	= 0x0001,
		EFO_NEW		= 0x0002,	///create a file
		EFO_SAVE	= 0x0004,	///save a file
	};

	struct SEditorFileInfo
	{
		TStringList		mSupportedScenes;

		TString			mFileExtension;	///file type extension without '.', NOT case sensitive
		TString			mFileDescription;
		TString			mDefaultPath;
		IConfig*		mNewFileConfig;
		uint16			mMaxOpenLimit;
		IconIndex		mIconID;
		uint16			mOperations;	///supported operations : @see EEditorFileOperation, for read-only files, specify it to 0
		bool			mNewFolder;		///create new folder (named as file name) for new file

		//not used for registration, used at runtime by editor framework/editorUI
		FileTypeID		mTypeID;		///type ID assigned by framework
		uint			mOpenedCount;	///opened file instance,

		//sorting helper
		inline bool operator<(const SEditorFileInfo& rhs) const	{return mFileDescription < rhs.mFileDescription;}
		static inline bool comparePtr(const SEditorFileInfo* lhs, const SEditorFileInfo* rhs) {return *lhs < *rhs;}
	};
	

	
}//namespace Blade


#endif //__Blade_EditorFileInfo_h__