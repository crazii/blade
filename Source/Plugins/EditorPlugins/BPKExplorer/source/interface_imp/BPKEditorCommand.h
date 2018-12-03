/********************************************************************
	created:	2015/08/18
	filename: 	BPKEditorCommand.h
	author:		Crazii
	
	purpose:	
				The BPK Explorer's purpose is to provide a tool to browse the package when problem occurs,
				then users can check & compare the files. So for now, the DELETE/COPY/CUT/RENAME feature which involves
				editing the package, is NOT SUPPORTED yet.
				the exporting function is enough to export files to local system, and then they can be compared
				with files of other builds.
*********************************************************************/
#ifndef __Blade_BPKEditorCommand_h__
#define __Blade_BPKEditorCommand_h__
#include <interface/IPlatformManager.h>
#include <interface/IResourceManager.h>
#include <interface/public/ui/IUICommand.h>
#include <interface/IEditorFramework.h>
#include "BPKEditorFile.h"

namespace Blade
{

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class BPKEditorCommand : public IUICommand
	{
	public:
		/** @brief  */
		BPKEditorFile* getCurrentFile() const
		{
			IEditorFile* f = IEditorFramework::getSingleton().getCurrentFile();
			if( f == NULL || f->getType() != BPKEditorFile::BPK_FILE)
			{
				//assert(false);
				return NULL;
			}

			return static_cast<BPKEditorFile*>(f);
		}

		/** @brief  */
		virtual void executeImpl(BPKEditorFile* file) = 0;

		/** @brief  */
		virtual bool updateImpl(ICommandUI* cui, BPKEditorFile* file) = 0;

		/** @brief  */
		virtual void execute(ICommandUI* /*cui*/)
		{
			BPKEditorFile* file = this->getCurrentFile();
			assert(file != NULL );	//button/menu only available on current type of file
			if( file != NULL )
				this->executeImpl(file);
		}

		/** @brief  */
		virtual bool update(ICommandUI* cui, index_t)
		{
			BPKEditorFile* file = this->getCurrentFile();
			//update may happen after window deactivate, when current file is NOT BPKEditorFile
			if( file != NULL )
				return this->updateImpl(cui, file);
			return false;
		}
	};


	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class BPKNaviBackCmd : public BPKEditorCommand, public Singleton<BPKNaviBackCmd>
	{
	public:
		/** @brief  */
		virtual void executeImpl(BPKEditorFile* file)
		{
			index_t index = file->getHistoryCursor();
			//const BPKEditorFile::HistoryList& history = file->getHistory();

			if( index == 0 )	//it's not gonna happen except manually called
				return;

			file->setHistoryCursor(index-1);

			IBPKEditorController::getSingleton().refresh(file, static_cast<ITableWindow*>(file->getEditorWindow()));
		}

		/** @brief  */
		virtual bool updateImpl(ICommandUI* cui, BPKEditorFile* file)
		{
			index_t index = file->getHistoryCursor();
			cui->setEnabled(index != 0);
			return true;
		}
	};

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class  BPKNaviForwardCmd : public BPKEditorCommand, public Singleton<BPKNaviForwardCmd>
	{
	public:
		/** @brief  */
		virtual void executeImpl(BPKEditorFile* file)
		{
			index_t index = file->getHistoryCursor();
			const BPKEditorFile::HistoryList& history = file->getHistory();

			if( index >= history.size()-1 )	//it's not gonna happen except manually called
				return;

			file->setHistoryCursor(index+1);
			IBPKEditorController::getSingleton().refresh(file, static_cast<ITableWindow*>(file->getEditorWindow()));
		}

		/** @brief  */
		virtual bool updateImpl(ICommandUI* cui, BPKEditorFile* file)
		{
			index_t index = file->getHistoryCursor();
			const BPKEditorFile::HistoryList& history = file->getHistory();
			cui->setEnabled(index < history.size()-1);
			return true;
		}
	};

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class BPKNaviUpperFolderCmd : public BPKEditorCommand, public Singleton<BPKNaviUpperFolderCmd>
	{
	public:
		/** @brief  */
		virtual void executeImpl(BPKEditorFile* file)
		{
			TString path = file->getCurrentPath();
			index_t pos = path.find_last_not_of(TEXT('/'));
			pos = path.find_last_of(TEXT('/'), pos);
			file->setCurrentPath( path.substr(0, pos+1) );
			IBPKEditorController::getSingleton().refresh(file, static_cast<ITableWindow*>(file->getEditorWindow()));
		}

		/** @brief  */
		virtual bool updateImpl(ICommandUI* cui, BPKEditorFile* file)
		{
			TString path = file->getCurrentPath();
			cui->setEnabled( path != BTString("/") );
			return true;
		}
	};

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class BPKNaviInputCmd : public Bindable, public Singleton<BPKNaviInputCmd>
	{
	public:
		/** @brief  */
		BPKEditorFile* getCurrentFile() const
		{
			IEditorFile* f = IEditorFramework::getSingleton().getCurrentFile();
			if( f == NULL || f->getType() != BPKEditorFile::BPK_FILE)
				return NULL;
			return static_cast<BPKEditorFile*>(f);
		}

		/** @brief  */
		const TString&	getCurrentPath(index_t) const
		{
			BPKEditorFile* file = this->getCurrentFile();
			if( file != NULL )
				return file->getCurrentPath();
			else
				return TString::EMPTY;
		}

		/** @brief  */
		bool setCurrentPath(index_t, const TString& gotoPath)
		{
			BPKEditorFile* file = this->getCurrentFile();
			if( file == NULL )
				return false;

			const BPKFile& bpk = file->getPackage();
			BPK_ENTRY* entry = bpk.findEntry(gotoPath);
			if( entry != NULL )
			{
				file->setCurrentPath(gotoPath);
				IBPKEditorController::getSingleton().refresh(file, static_cast<ITableWindow*>(file->getEditorWindow()));
			}
			else
				IPlatformManager::getSingleton().showMessageBox(IPlatformManager::MBT_OK, TEXT("Invald Path"), TEXT("The path not found in the package."));
			return true;
		}
	};


	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class BPKExportCmd : public BPKEditorCommand, public Singleton<BPKExportCmd>
	{
	public:
		/** @brief  */
		virtual void executeImpl(BPKEditorFile* file)
		{
			ITableWindow* window = static_cast<ITableWindow*>(file->getEditorWindow());
			ITableWindow::CELLPOS pos;
			if( window->getSelectedCells(&pos) > 0 )
			{
				//popup dialog to save file
				//we need a new UI from EditorUI to perform save as
				IUIMainWindow* ui = IEditorFramework::getSingleton().getEditorUI();
				const TString& filename = window->getCell(pos.row, pos.col);
				TString base, ext;
				TStringHelper::getFileInfo(filename, base, ext);

				TString saveDir, saveFilename;
				if( ui->showSaveFileUI(saveDir, saveFilename, ext) )
				{
					//TODO: save file
					HSTREAM stream = IResourceManager::getSingleton().openStream(saveDir + BTString("/") + saveFilename, false);
					const BPKFile& bpk = file->getPackage();
					BPKFile::FILE* fp = bpk.fopen(filename);
					assert( fp != NULL );

					if( stream == NULL || fp == NULL )
					{
						IPlatformManager::getSingleton().showMessageBox(IPlatformManager::MBT_OK, TEXT("Export Error"), TEXT("Failed to open exported file."));
						return;
					}

					char buffer[4096];
					IStream::Size size;
					while( (size=bpk.fread(buffer, sizeof(buffer), fp)) > 0 )
						stream->writeData(buffer, size);
					stream->close();
					bpk.fclose(fp);
				}
			}
		}

		/** @brief  */
		virtual bool updateImpl(ICommandUI* cui, BPKEditorFile* file)
		{
			ITableWindow* window = static_cast<ITableWindow*>(file->getEditorWindow());
			ITableWindow::CELLPOS pos;
			cui->setEnabled( window->getSelectedCells(&pos) > 0 );
			return true;
		}
	};

}//namespace Blade


#endif // __Blade_BPKEditorCommand_h__