/********************************************************************
	created:	2015/09/24
	filename: 	ImporterManager.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_ImporterManager_h__
#define __Blade_ImporterManager_h__
#include <interface/IImporterManager.h>
#include <RefCount.h>
#include <utility/BladeContainer.h>
#include <interface/IEditorFramework.h>

namespace Blade
{
	//import helper, it will auto do clean up work on ~ImportHelper()
	//so you MUST keep this object alive until file is opened
	class ImportHelper : public TempAllocatable
	{
	public:
		~ImportHelper();

		const SEditorImporterInfo*	mInfo;
		SEditorFileInfoEx*			mTargetInfo;

	private:
		TString mPath;
		TString	mFile;
		TStringList mExtras;

		ImportHelper() { mInfo = NULL; mTargetInfo = NULL; }
		friend class ImporterManager;
	};
	typedef Handle<ImportHelper> HIMPORT;

	class ImporterManager : public IImporterManager, public Singleton<ImporterManager>
	{
	public:
		using Singleton<ImporterManager>::getSingleton;
		using Singleton<ImporterManager>::getSingletonPtr;
	public:
		ImporterManager();
		~ImporterManager();

		/************************************************************************/
		/* IImporterManager interface                                                                      */
		/************************************************************************/
		/*
		@describe 
		@param
		@return
		*/
		virtual bool	registerImporter(const SEditorImporterInfo& impInfo);

		/*
		@describe 
		@param
		@return
		*/
		virtual size_t	getImporterCount() const;

		/*
		@describe 
		@param
		@return
		*/
		virtual const SEditorImporterInfo* getImporterInfo(index_t index) const;

		/*
		@describe get impoerter info by importer type index: SEditorImporterInfo::mTypeIndex
		@param
		@return
		*/
		virtual const SEditorImporterInfo* getImporterInfoByType(FileTypeID typeID) const;

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		HIMPORT	doImport(TString& dir, const TString& file, FileTypeID typeID) const;

		/** @brief  */
		HIMPORT	doImport(TString& dir, const TString& file, const TString& extension) const;

	protected:
		typedef StaticSet<SEditorImporterInfo> ImporterInfoSet;

		ImporterInfoSet mImporters;
		RefCount		mID;
	};

	
}//namespace Blade


#endif // __Blade_ImporterManager_h__