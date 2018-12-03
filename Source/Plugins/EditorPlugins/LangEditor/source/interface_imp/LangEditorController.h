/********************************************************************
	created:	2015/08/17
	filename: 	LangEditorController.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_LangEditorController_h__
#define __Blade_LangEditorController_h__
#include <interface/public/IEditorFile.h>
#include <interface/public/ui/uiwindows/ITableWindow.h>
#include <interface/public/ui/IUIToolBox.h>
#include "LangEditor.h"
#include "LangEditorCommand.h"
#include "LangEditorFile.h"

namespace Blade
{
	class LangEditorController : public ITableWindow::IController, public IEditorFileUpdater, public Singleton<LangEditorController>
	{
	public:
		virtual ~LangEditorController()	{}

		/************************************************************************/
		/* ITableWindow::IController interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual bool onKeyDown(ITableWindow* /*window*/, EKeyCode kc, uint32 modifierMask)
		{
			if( kc == KC_F3 && modifierMask == 0)
			{
				const TString& lastFindString = FindStringCommand::getSingleton().getString(0);
				if( !lastFindString.empty() )
					FindStringCommand::getSingleton().execute(0, lastFindString);
				return true;
			}
			else
				//don't process: use default
				return false;
		}

		/************************************************************************/
		/* IEditorFileUpdater interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual void				update(IEditorFile* /*file*/)
		{

		}

		/** @brief setup window related params after file is loaded */
		virtual void				setupWindow(IUIWindow* window)
		{
			//call this before table window initialize
			IMenu* menu = LangEditor::getSingleton().getLangEditorMenu();
			IMenu* menuImportLang = menu->getSubMenu(0);
			IMenu* menuAddLang = menu->getSubMenu(1);
			IMenu* menuVisibility = menu->getSubMenu(2);
			IMenu* gotoUntranslated = menu->getSubMenu(3);
			IMenu* gotoEmpty = menu->getSubMenu(4);
			//add toolbar
			IUIToolBox* toolbox = window->createToolBox(BTString("LangEditTools"), IS_16);
			toolbox->addButton(menuImportLang);
			toolbox->addButton(menuAddLang);
			toolbox->addButton(menuVisibility);
			toolbox->addButton(gotoUntranslated);
			toolbox->addButton(gotoEmpty);
			toolbox->addTool(LangEditor::getSingleton().getFindConfig());
			//call this before table window initialize
			IEditorFile* file = IEditorFramework::getFileFromWindow(window);
			this->resetView(file, static_cast<ITableWindow*>(window));
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

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/

		/** @brief  */
		void		resetView(IEditorFile* editorFile, ITableWindow* window)
		{
			LangEditorFile* file = static_cast<LangEditorFile*>(editorFile);

			const LangEditorFile::LangColmnDesc& desc = file->getColumnDesc();
			if( desc.size() == 0 )
				return;

			const ILangTable* table = file->getLangTable();
			assert(table != NULL);

			//reset columns
			window->clearRows();
			window->clearColumns();
			window->initialize(&desc[0], desc.size(), 
				ITableWindow::TES_GRIDVIEW | ITableWindow::TES_SHOW_ROWNO,
				file, this);

			//reset rows
			size_t rowCount = table->getSize();
			window->beginAddRow(rowCount);

			size_t colCount = desc.size();
			TempVector<Variant> row(colCount, TString::EMPTY);
			//get column index
			TempVector<index_t> indices(colCount, INVALID_INDEX);
			ParamList translations;
			for(size_t i = 0; i < rowCount; ++i)
			{
				bool ret = table->getTranslation(i, translations);
				assert(ret);
				if( !ret )
					continue;

				row[0] = translations[ILangTableManager::DEFAULT_LANG];


				for(size_t j = 0; j < translations.size(); ++j)
				{
					const NamedVariant& v = translations.at(j);
					if( v.getName() == ILangTableManager::DEFAULT_LANG)
						continue;

					//note: the language order should be the same for all translations, 
					//so the index should be fixed, we need only calculate once.
					if( indices[j] == INVALID_INDEX )
					{
						const LangEditorFile::LangColmnDesc::const_iterator iter = std::find_if(desc.begin(), desc.end(), FnHConfigFindByName(v.getName()));
						if( iter != desc.end() )
							indices[j] = (index_t)(iter - desc.begin());
						else
							assert(false);
					}
				
					index_t index = indices[j];
					if( index != INVALID_INDEX )
						row[index] = v.getValue();
				}
				window->appendRow(&row[0], colCount);
			}

			window->endAddRow();
		}
	};
	
}//namespace Blade


#endif // __Blade_LangEditorController_h__
