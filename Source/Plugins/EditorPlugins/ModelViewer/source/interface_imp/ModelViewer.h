/********************************************************************
	created:	2013/04/09
	filename: 	ModelViewer.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_ModelViewer_h__
#define __Blade_ModelViewer_h__
#include <interface/public/IEditor.h>

namespace Blade
{
	class IEditorTool;

	//"editor" helper class
	class ModelViewer : public IEditor, public Singleton<ModelViewer>
	{
	public:
		ModelViewer();
		~ModelViewer();

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

	protected:
		typedef Vector<IEditorTool*>	ToolList;
		ToolList	mTools;
	};
	
}//namespace Blade

#endif //  __Blade_ModelViewer_h__