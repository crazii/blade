/********************************************************************
	created:	2015/08/11
	filename: 	LangEditor.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_LangEditor_h__
#define __Blade_LangEditor_h__
#include <interface/public/IEditor.h>

namespace Blade
{
	class IMenu;

	class LangEditor : public IEditor, public Singleton<LangEditor>
	{
	public:
		LangEditor();
		~LangEditor();

		/************************************************************************/
		/* IEditor interface                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		virtual void			initEditor();

		/*
		@describe
		@param
		@return
		*/
		virtual void			shutdownEditor();

		/************************************************************************/
		/* custom method                                                                     */
		/************************************************************************/
		/** @brief  */
		IMenu*					getLangEditorMenu() const	{return mMenu;}
		/** @brief  */
		HCONFIG					getFindConfig() const		{return mFindConfig;}

	protected:
		IMenu*	mMenu;
		HCONFIG	mFindConfig;
	};//class LangEditor
	
}//namespace Blade


#endif // __Blade_LangEditor_h__