/********************************************************************
	created:	2015/09/24
	filename: 	IImporter.h
	author:		Crazii
	
	purpose:	importer interface for editor framework,
				an importer can open a stream and convert to a target format of IEditorFile object
				and then the IEditorFile object can be opened in EditorFramework.
*********************************************************************/
#ifndef __Blade_IImporter_h__
#define __Blade_IImporter_h__
#include <BladeEditor.h>
#include <utility/StringList.h>
#include <interface/public/file/IStream.h>
#include <interface/public/ISerializable.h>
#include <interface/EditorFileInfo.h>

namespace Blade
{
	struct SEditorImporterInfo
	{
		enum
		{
			IMPORTER_ID_TAG = 0x80000000,
		};
		TString			mName;			///factory class
		TString			mTarget;		///target IEditorFile type
		TString			mTargetExt;		///target file extension
		TString			mDescription;	///description
		TStringList		mExtensions;	///supported extensions
		//importer type: assigned by framework
		FileTypeID		mTypeID;		///importer type IDs are in the same space of editor file type ids
										///FileTypeID with IMPORTER_ID_TAG represent a importer.
		IconIndex		mIconID;

		inline bool operator<(const SEditorImporterInfo& rhs) const {return mName < rhs.mName;}
		static inline bool comparePtr(const SEditorImporterInfo* lhs, const SEditorImporterInfo* rhs) {return *lhs < *rhs;}
	};

	class BLADE_EDITOR_API IImporter : public TempAllocatable
	{
	public:
		virtual ~IImporter() {}

		/**
		@describe get the factory class name of the importer,
		corresponding to SEditorImporterInfo::mName
		@param
		@return
		*/
		virtual const TString&	getName() const = 0;

		/**
		@describe 
		@param source: input source file stream that importer can support
		@param dest: output converted/imported format recognized by framework & plugins
		@param params: extra parameters used for importing
		@param extraFiles: extra file created by importer. extra files can only contains file names, files should be created at the same path/folder of input source file.
		@param callback: callback for importing progress
		@return
		*/
		virtual bool	import(const HSTREAM& source, const HSTREAM& dest, const ParamList& params, TStringParam& extraFiles, ProgressNotifier& callback) = 0;
	};

	extern template class BLADE_EDITOR_API Factory<IImporter>;
	typedef Factory<IImporter> ImporterFactory;
	
}//namespace Blade


#endif // __Blade_IImporter_h__