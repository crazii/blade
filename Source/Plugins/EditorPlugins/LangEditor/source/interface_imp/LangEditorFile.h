/********************************************************************
	created:	2015/08/11
	filename: 	LangEditorFile.h
	author:		Crazii
	
	purpose:	Lang Editor file definition, this is actually a MODEL of MVC pattern, but Blade is free to name it.
				MODEL		- LangEditorFile
				VIEW		- ITableWindow
				CONTROLLER	- LangEditorController
*********************************************************************/
#ifndef __Blade_LangEditorFile_h__
#define __Blade_LangEditorFile_h__
#include <interface/ILangTable.h>
#include <utility/BladeContainer.h>
#include <interface/public/ui/IMenu.h>
#include <interface/public/EditorFileBase.h>
#include <interface/public/ui/uiwindows/ITableWindow.h>

namespace Blade
{
	class LangEditorFile : public EditorFileBase, public ITableWindow::IListener, public Allocatable
	{
	public:
		typedef Vector<HCONFIG>	LangColmnDesc;

		static const TString LANG_EDITOR_FILE;
	public:
		LangEditorFile();
		virtual ~LangEditorFile();

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

		/** @brief declare base function to make compiler happy */
		inline virtual void			initialize(const SEditorFileInfo* info, FileInstanceID fileID, IUIWindow* win)
		{
			return EditorFileBase::initialize(info, fileID, win);
		}


		/************************************************************************/
		/* ITableWindow::IListener interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual bool onItemChange(index_t /*row*/, index_t /*col*/, const Variant& /*val*/)
		{
			//we don't record the change here. instead, we get all of the table data from table editor window on saving
			if( this->isOpened() )
				this->setModified();
			return true;
		}

		/************************************************************************/
		/* custom method                                                                     */
		/************************************************************************/
		/** @brief init data after open/new */
		bool		initialize();
		/** @brief  */
		bool		importText(const TString& path, bool eraseOriginal = false, bool stripOriginal = true);
		/** @brief  */
		bool		addLanguage(const TString& lang, const TString& copyFromLang);
		/** @brief  */
		size_t		getLanguages(TStringParam& langs);
		/** @brief  */
		bool		setLanguageVisibility(const TString& lang,  bool visible);
		/** @brief  */
		const TString& getStringToFind() const	{return mStringToFind;}
		/** @brief  */
		void		setStringToFind(const TString& str2fnd) {if( mStringToFind != str2fnd ) mStringToFind = str2fnd;}
		/** @brief  */
		inline const LangColmnDesc& getColumnDesc() const	{return mColumnDesc;}
		/** @brief  */
		const ILangTable* getLangTable() const				{return mTable;}

	protected:


		TString			mFile;
		ILangTable*		mTable;
		LangColmnDesc	mColumnDesc;
		TString			mStringToFind;
	};
	
}//namespace Blade


#endif // __Blade_LangEditorFile_h__