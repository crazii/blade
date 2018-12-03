/********************************************************************
	created:	2015/09/24
	filename: 	ImporterManager.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "ImporterManager.h"
#include "EditorFramework.h"
#include <BladeBase_blang.h>
#include <interface/IPlatformManager.h>
#include <interface/IResourceManager.h>


namespace Blade
{
	template class Factory<IImporter>;
	template class Factory<IImporterManager>;

	ImportHelper::~ImportHelper()
	{
		//delete cache files in memory file device
		//TODO: some files takes too much time. user manually delete cache to avoid imported again?
		if( mPath != TString::EMPTY && mFile != TString::EMPTY )
		{
			HFILEDEVICE memDevice = IResourceManager::getSingleton().loadFolder(mPath);
			memDevice->deleteFile(mFile);

			for(size_t i = 0; i < mExtras.size(); ++i)
				memDevice->deleteFile( mExtras[i] );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	ImporterManager::ImporterManager()
	{
		mID = 0;
	}

	//////////////////////////////////////////////////////////////////////////
	ImporterManager::~ImporterManager()
	{

	}

	/************************************************************************/
	/* IImporterManager interface                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool	ImporterManager::registerImporter(const SEditorImporterInfo& impInfo)
	{
		BLADE_TS_VERIFY(TS_MAIN_SYNC);

		SEditorImporterInfo info = impInfo;
		info.mTypeID = (FileTypeID)( mID.increment() | SEditorImporterInfo::IMPORTER_ID_TAG );
		std::pair<ImporterInfoSet::iterator,bool> ret = mImporters.insert(info);

		if(!ret.second)
			mID.decrement();
		else
		{
			const TStringList& extensions = impInfo.mExtensions;
			//check extensions
			//note we cannot check for IEditorFile extension, because by this time of registering, not all file types are registered yet.
			for(ImporterInfoSet::iterator i = mImporters.begin(); i != mImporters.end(); ++i)
			{
				if( i == ret.first )
					continue;

				const TStringList& existingExts = i->mExtensions;
				for(size_t j = 0; j < extensions.size(); ++j)
				{
					bool conflict = existingExts.find(extensions[j]) != INVALID_INDEX || EditorFramework::getSingleton().getFileTypeByExtension(extensions[j]) != NULL;
					if(conflict)
					{
						assert(false && "extension already exist for another importer or editor file format.");
						mImporters.erase(ret.first);
						ret.second = false;
						break;
					}
				}
			}
		}
		assert(ret.second);
		return ret.second;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t	ImporterManager::getImporterCount() const
	{
		return mImporters.size();
	}

	//////////////////////////////////////////////////////////////////////////
	const SEditorImporterInfo* ImporterManager::getImporterInfo(index_t index) const
	{
		if( index < mImporters.size() )
		{
			ImporterInfoSet::iterator i = mImporters.begin();
			std::advance(i, index);
			return &(*i);
		}
		assert(false);
		return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	const SEditorImporterInfo* ImporterManager::getImporterInfoByType(FileTypeID typeID) const
	{
		assert( (typeID&SEditorImporterInfo::IMPORTER_ID_TAG) != 0 );
		for(ImporterInfoSet::iterator i = mImporters.begin(); i != mImporters.end(); ++i)
		{
			if( i->mTypeID == typeID )
				return &(*i);
		}
		assert(false);
		return NULL;
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	HIMPORT	ImporterManager::doImport(TString& dir, const TString& file, FileTypeID typeID) const
	{
		bool isImporter = (typeID&SEditorImporterInfo::IMPORTER_ID_TAG) != 0;
		if( !isImporter )
			return HIMPORT::EMPTY;

		const SEditorImporterInfo* impInfo = this->getImporterInfoByType(typeID);
		if( impInfo == NULL )
		{
			assert(false);
			return HIMPORT::EMPTY;
		}
		//target may not registered
		SEditorFileInfoEx* pInfo = EditorFramework::getSingleton().getFileType(impInfo->mTarget);
		if( pInfo == NULL )
		{
			//assert(false);
			return HIMPORT::EMPTY;
		}

		Handle<IImporter> importer( BLADE_FACTORY_CREATE(IImporter, impInfo->mName) );
		HSTREAM source = IResourceManager::getSingleton().loadStream(dir + BTString("/") + file);
		HSTREAM cache = IResourceManager::getSingleton().openStream(BTString("memory:/eidtor/importer/") + file, true);

		IProgressCallback* callback = EditorFramework::getSingleton().getEditorUI()->showProgress(BXLang("Importing..."));
		ProgressNotifier callbackRef;
		callbackRef = callback;

		ParamList params;
		TStringParam extras;
		if( source == NULL || cache == NULL || !importer->import(source, cache, params, extras, callbackRef) )
		{
			//force hide just in case
			callback->onNotify(1.0);

			IPlatformManager::getSingleton().showMessageBox(BTLang("cannnot open file - opening file failed.").c_str(),
				BTLang(BLANG_ERROR).c_str(), IPlatformManager::MBT_OK);
			return HIMPORT::EMPTY;
		}
		callback->onNotify(1.0);
		cache->rewind();

		HIMPORT helper( BLADE_NEW ImportHelper() );
		helper->mInfo = impInfo;
		helper->mTargetInfo = pInfo;
		helper->mPath = BTString("memory:/eidtor/importer");
		helper->mFile = file;
		helper->mExtras = extras;
		dir = helper->mPath;
		return helper;
	}

	//////////////////////////////////////////////////////////////////////////
	HIMPORT	ImporterManager::doImport(TString& dir, const TString& file, const TString& extension) const
	{
		FileTypeID typeID = INVALID_FILETYPE;
		for (ImporterInfoSet::iterator i = mImporters.begin(); typeID == INVALID_FILETYPE && i != mImporters.end(); ++i)
		{
			for (size_t j = 0; typeID == INVALID_FILETYPE && j < i->mExtensions.size(); ++j)
				if (i->mExtensions[j] == extension)
					typeID = i->mTypeID;
		}

		if (typeID == INVALID_FILETYPE)
		{
			assert(false);
			return HIMPORT::EMPTY;
		}
		return this->doImport(dir, file, typeID);
	}
	
}//namespace Blade