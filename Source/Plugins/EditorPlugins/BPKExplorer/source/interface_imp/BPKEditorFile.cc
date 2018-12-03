/********************************************************************
	created:	2015/08/17
	filename: 	BPKEditorFile.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "BPKEditorFile.h"
#include <interface/IResourceManager.h>
#include "BPKEditorController.h"

namespace Blade
{

	const TString BPKEditorFile::BPK_FILE = BTString("Blade Package File");

	//////////////////////////////////////////////////////////////////////////
	BPKEditorFile::BPKEditorFile()
		:EditorFileBase(BPK_FILE)
	{
		this->setUpdater(BPKEditorController::getSingletonPtr());
	}

	//////////////////////////////////////////////////////////////////////////
	BPKEditorFile::~BPKEditorFile()
	{

	}

	/************************************************************************/
	/* IEditorFile interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void				BPKEditorFile::openFile(const TString& dir,const TString& name)
	{
		if( this->isFileOpened() || this->getEditorWindow() == NULL )
		{
			assert(false);
			return;
		}

		TString file = dir + BTString("/") + name;
		HSTREAM stream = IResourceManager::getSingleton().loadStream(file, IStream::AM_READWRITE);

		if( mFile.openFile(stream) )
		{
			this->setOpened();
			mHistoryCusor = 0;
			mHistory.reserve(64);
			mHistory.push_back(BTString("/"));
		}
		else
			assert(false);
	}

	//////////////////////////////////////////////////////////////////////////
	void				BPKEditorFile::saveFile()
	{
		if( this->isOpened() && this->isModified() && mFile.isOpen() )
			mFile.writeFile();

		this->setUnModified();
	}

	//////////////////////////////////////////////////////////////////////////
	void				BPKEditorFile::newFile(const TString& dir, const TString& name)
	{
		if( this->isFileOpened() || this->getEditorWindow() == NULL )
		{
			assert(false);
			return;
		}

		TString file = dir + BTString("/") + name;
		HSTREAM stream = IResourceManager::getSingleton().openStream(file, false, IStream::AM_OVERWRITE);

		if( mFile.openFile(stream) )
		{
			this->setOpened();
			mHistoryCusor = 0;
			mHistory.reserve(64);
			mHistory.push_back(BTString("/"));
		}
		else
			assert(false);
	}

	//////////////////////////////////////////////////////////////////////////
	void				BPKEditorFile::closeFile()
	{
		if( this->isFileOpened() )
		{
			mFile.closeFile();
		}
	}
	
}//namespace Blade