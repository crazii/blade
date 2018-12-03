/********************************************************************
	created:	2015/08/18
	filename: 	BPKEditorController.h
	author:		Crazii
	
	purpose:	Controller of a MVC
*********************************************************************/
#ifndef __Blade_BPKEditorController_h__
#define __Blade_BPKEditorController_h__
#include <utility/StringHelper.h>
#include <utility/Variant.h>
#include <utility/BladeContainer.h>
#include <BladeBase_blang.h>
#include <ConfigTypes.h>
#include <interface/ILangTable.h>
#include <interface/IEditorFramework.h>
#include <interface/public/IEditorFile.h>
#include <interface/public/ui/uiwindows/ITableWindow.h>
#include <interface/public/ui/IIconManager.h>
#include <interface/public/ui/IMenuManager.h>
#include <interface/public/ui/IUIToolBox.h>
#include "BPKExplorer.h"
#include "BPKEditorFile.h"
#include "BPKEditorCommand.h"

namespace Blade
{
	class BPKEditorController : public IBPKEditorController, public ITableWindow::IController, public Singleton<BPKEditorController> 
	{
	public:
		using Singleton<BPKEditorController>::getSingleton;
		using Singleton<BPKEditorController>::getSingletonPtr;
	public:
		BPKEditorController()
		{
			mFolderIcon = IIconManager::getSingleton().loadSingleIconImage(BTString("folder.png") );
			mEmptyFolderIcon = IIconManager::getSingleton().loadSingleIconImage(BTString("folder_empty.png") );
			mDefaultIcon = IIconManager::getSingleton().loadSingleIconImage(BTString("file.png") );

			mColumnDesc.resize(3);
			//mColumnDesc[0].bind( BLADE_NEW ConfigAtom(BXLang("Name"), TString::EMPTY, CAF_READWRITE) );	//disable rename
			mColumnDesc[0].bind( BLADE_NEW ConfigAtom(BXLang(BLANG_NAME), TString::EMPTY, CAF_READ) );
			mColumnDesc[1].bind( BLADE_NEW ConfigAtom(BXLang("Size"), TString::EMPTY, CAF_READ) );
			mColumnDesc[2].bind( BLADE_NEW ConfigAtom(BXLang("Date Modified"), TString::EMPTY, CAF_READ) );
			//TODO: add file description - this need Win32 SHELL API. but our BPK Explorer should be platform independent.
			//HOWTO
		}
		virtual ~BPKEditorController()	{}

		/************************************************************************/
		/* IBPKEditorController interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual void				update(IEditorFile* /*file*/)	{}
		/** @brief setup window related params after file is loaded */
		virtual void				setupWindow(IUIWindow* window)
		{
			//TODO: add menu & toolbars
			//call this before table window initialize
			IMenu* menu = BPKExplorer::getSingleton().getContextMenu();
			IMenu* menuBack = menu->getSubMenu(0);
			IMenu* menuFwd = menu->getSubMenu(1);
			IMenu* menuUp = menu->getSubMenu(2);
			//add toolbar
			IUIToolBox* toolbox = window->createToolBox(BTString("BPKNavigation"), IS_16);
			toolbox->addButton(menuBack);
			toolbox->addButton(menuFwd);
			toolbox->addButton(menuUp);
			toolbox->addTool(BPKExplorer::getSingleton().getNaviInputConfig(), 500);

			//TODO:
			//call this before table window initialize
			IEditorFile* file = IEditorFramework::getFileFromWindow(window);
			BPKEditorFile* f = static_cast<BPKEditorFile*>(file);

			ITableWindow* win = static_cast<ITableWindow*>(window);
			win->initialize(&mColumnDesc[0], mColumnDesc.size(), 
				ITableWindow::TES_FULL_ROW_SELECTION, f, this);

			this->refresh(f, win);
		}
		/** @brief  */
		virtual void				onActivate(IEditorFile* file, bool active)
		{
			//set focus on table editor window
			if( active )
			{
				ITableWindow* window = static_cast<ITableWindow*>(file->getEditorWindow());
				window->activate();
			}
		}
				/** @brief  */
		virtual bool	refresh(BPKEditorFile* file, ITableWindow* window)
		{
			window->clearRows();
			const BPKFile& bpk = file->getPackage();
			const TString& curPath = file->getCurrentPath();

			BPKEntryList entries;
			bpk.findEntries(curPath + BTString("/*"),  entries);
			std::sort(entries.begin(), entries.end(), BPKSorter(bpk));

			window->beginAddRow( entries.size() );
			for(size_t i = 0; i < entries.size(); ++i)
			{
				BPK_ENTRY* entry = entries[i];
				IconIndex icon;

				TString entryPath, name, ext;
				entryPath = bpk.getEntryName(entry);
				TStringHelper::getFileInfo(entryPath, name, ext);
				if( !entry->isDirectory() )
					icon = IIconManager::getSingleton().loadIconForFileType(ext);
				else
				{
					if(entry->mDir.mSize > 0)
						icon = mFolderIcon;
					else
						icon = mEmptyFolderIcon;
				}
				if( icon == INVALID_ICON_INDEX  )
					 icon = mDefaultIcon;

				Time tm;
				entry->mLastTime.toTime(tm);
				TString sizeString = entry->isDirectory() ? TStringHelper::fromInt64(entry->mDir.mSize) : TString(TStringHelper::fromInt64((entry->mFile.mSize+1023)/1024) + BTString(" KB"));
				TString time = TStringHelper::fromTime(tm);
				//name	size	time
				Variant rowItems[] = {entryPath, sizeString, time};

				window->appendRow(rowItems, mColumnDesc.size(), icon);
			}
			window->endAddRow();
			window->navigateTo(ITableWindow::TN_NONE, false, true);

			window->getToolBox(BTString("BPKNavigation"))->updateTools();	//update navigation input bar content
			return true;
		}

		/************************************************************************/
		/* ITableWindow::IController interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual bool onKeyDown(ITableWindow* window, EKeyCode kc, uint32 modifierMask)
		{
			BPKEditorFile* file = this->getCurrentFile();

			if( kc == KC_ENTER )
			{
				ITableWindow::CELLPOS cellPos;
				if( window->getSelectedCells(&cellPos) >= 1)
					this->enterEntry(file, window, cellPos.row);
				return true;
			}
			else if( (kc == KC_BACKSPACE && modifierMask == 0) || (kc == KC_LEFT && modifierMask == KM_ALT) )
			{
				BPKNaviBackCmd::getSingleton().executeImpl(file);
				return true;
			}
			else if( kc == KC_RIGHT && modifierMask == KM_ALT )
			{
				BPKNaviForwardCmd::getSingleton().executeImpl(file);
				return true;
			}
			else if( kc == KC_UP && modifierMask == KM_ALT )
			{
				BPKNaviUpperFolderCmd::getSingleton().executeImpl(file);
				return true;
			}
			else if( kc == KC_DELETE )
			{
				//delete file
				return true;
			}
			return false;
		}

		/** @brief  */
		virtual bool onMouseClick(ITableWindow* window, MSBUTTON button, const POINT2I& pos)
		{
			if( button != MSB_RIGHT )
				return false;

			//bool deselect = (window->getStyle()&ITableWindow::TES_MULTI_SELECTION) == 0;

			ITableWindow::CELLPOS cellPos;
			if( window->hitCell(pos, cellPos) )
			{
				window->editCurrentCell(false, true);
				window->navigateTo(cellPos.row, cellPos.col, false, true);
			}

			//show popup menu
			IMenuManager::getSingleton().popupMenu( BPKExplorer::getSingleton().getContextMenu(), pos.x, pos.y, window);
			return true;
		}

		/** @brief  */
		virtual bool onMouseDBClick(ITableWindow* window, MSBUTTON button, const POINT2I& pos)
		{
			if( button != MSB_LEFT )
				return false;

			ITableWindow::CELLPOS cellPos;
			if( window->hitCell(pos, cellPos) )
				this->enterEntry(this->getCurrentFile(), window, cellPos.row);
			return true;
		}

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		BPKEditorFile*	getCurrentFile() const
		{
			IEditorFile* f = IEditorFramework::getSingleton().getCurrentFile();
			if( f == NULL || f->getType() != BPKEditorFile::BPK_FILE )
			{
				assert(false);
				return NULL;
			}
			return static_cast<BPKEditorFile*>(f);
		}

		/** @brief  */
		void	enterEntry(BPKEditorFile* file, ITableWindow* window, index_t row)
		{
			const BPKFile& bpk = file->getPackage();

			index_t col = window->findColumn(BTString("Name"));

			const TString& name = window->getCell(row, col);
			TString fullpath = file->getCurrentPath() + BTString("/") + name;
			BPK_ENTRY* entry = bpk.findEntry(fullpath);
			if( entry == NULL )
				assert(false);
			if( !entry->isDirectory() )
				return;
			file->setCurrentPath(fullpath);
			this->refresh(file, window);
		}

		/** @brief  */
		class BPKSorter : public NonAssignable
		{
			const BPKFile& mPackage;
		public:
			BPKSorter(const BPKFile& bpk) :mPackage(bpk)	{}

			bool operator()(const void* left,const void* right) const
			{
				const BPK_ENTRY* lhs = (const BPK_ENTRY*)left;
				const BPK_ENTRY* rhs = (const BPK_ENTRY*)right;
				return (lhs->isDirectory() && !rhs->isDirectory()) || 
					(((lhs->isDirectory() || !rhs->isDirectory()) 
					&& FnTCharNCLess::compare(mPackage.getEntryName(lhs), mPackage.getEntryName(rhs))));
			}
		};

	protected:
		typedef TStringMap<IconIndex>	ExtIconMap;	//extension to icon map
		typedef Vector<HCONFIG>			ColumnDesc;

		IconIndex	mFolderIcon;
		IconIndex	mEmptyFolderIcon;
		IconIndex	mDefaultIcon;
		ExtIconMap	mIconMap;
		ColumnDesc	mColumnDesc;
	};//class BPKEditorController

	
}//namespace Blade


#endif // __Blade_BPKEditorController_h__
