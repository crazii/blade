/********************************************************************
	created:	2015/08/17
	filename: 	BPKEditorFile.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_BPKEditorFile_h__
#define __Blade_BPKEditorFile_h__
#include <utility/StringHelper.h>
#include <utility/BladeContainer.h>
#include <utility/BPKFile.h>
#include <interface/public/EditorFileBase.h>
#include <interface/public/ui/uiwindows/ITableWindow.h>

namespace Blade
{

	class BPKEditorFile : public EditorFileBase, public ITableWindow::IListener, public Allocatable
	{
	public:
		static const TString BPK_FILE;

		typedef Vector<TString>	HistoryList;
	public:
		BPKEditorFile();
		~BPKEditorFile();

		/************************************************************************/
		/* IEditorFile interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual void				openFile(const TString& dir,const TString& name);

		/** @brief  */
		virtual void				saveFile();

		/** @brief  */
		virtual void				newFile(const TString& dir,const TString& name);

		/** @brief  */
		virtual void				closeFile();

		/************************************************************************/
		/* ITableWindow::IListener interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual bool onItemChange(index_t /*row*/, index_t /*col*/, const Variant& /*val*/)
		{
			//we don't record the change here. instead, we get all of the table data from table editor window
			if( this->isOpened() )
			{
				//TODO: change entry name within BPK package?
				this->setModified();
			}
			return true;
		}

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		const BPKFile&	getPackage() const			{return mFile;}
		/** @brief current path within package */
		const TString&	getCurrentPath()const
		{
			assert( mHistoryCusor < mHistory.size() );
			return mHistory[mHistoryCusor];
		}
		/** @brief  */
		void setCurrentPath(const TString& path)
		{
			if( path.empty() )
				return;
			TString stdPath = TStringHelper::standardizePath(path);
			if( this->getCurrentPath() != stdPath)
			{
				assert( mHistoryCusor < mHistory.size() );
				if(mHistoryCusor < mHistory.size()-1 )
				{
					mHistory.resize(mHistoryCusor+2);
					mHistory[++mHistoryCusor] = stdPath;
				}
				else
				{
					mHistory.push_back(stdPath);
					++mHistoryCusor;
				}
			}
		}
		/** @brief  */
		index_t		getHistoryCursor() const	{return mHistoryCusor;}
		/** @brief  */
		const HistoryList&	getHistory() const	{return mHistory;}
		/** @brief  */
		bool		setHistoryCursor(index_t index)
		{
			if( index >= mHistory.size() )
			{
				assert(false);
				return false;
			}
			mHistoryCusor = index;
			return true;
		}

	protected:
		

		BPKFile		mFile;
		HistoryList	mHistory;
		index_t		mHistoryCusor;
	};//class BPKEditorFile


	//////////////////////////////////////////////////////////////////////////
	//note: the controller is not bound to any file/window, it is a singleton
	class IBPKEditorController : public IEditorFileUpdater, public InterfaceSingleton<IBPKEditorController>
	{
	public:
		virtual bool	refresh(BPKEditorFile* file, ITableWindow* window) = 0;
	};//class IBPKEditorController
	//note: don't need factory export because the singleton is used internally.
	
}//namespace Blade


#endif // __Blade_BPKEditorFile_h__
