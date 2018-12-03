/********************************************************************
	created:	2013/11/17
	filename: 	IEditorLogic.h
	author:		Crazii
	purpose:	editor logic. one instance for each opened file/window
*********************************************************************/
#ifndef __Blade_IEditorLogic_h__
#define __Blade_IEditorLogic_h__
#include <BladeEditor.h>
#include <interface/public/logic/ILogic.h>

namespace Blade
{
	class IEditorFile;
	class IUIWindow;

	class IEditorLogic : public ILogic
	{
	public:
		virtual ~IEditorLogic()		{}
		/** @brief set up window for a new file. i.e. add tool bars */
		virtual bool			initalize(IEditorFile* file, IUIWindow* window) = 0;

		/** @brief  */
		virtual IEditorFile*	getFile() const = 0;
	};

	///note: editor logic factory uses the UI window's type name as logic type name
	///@see IUIWindow::getWindowType
	extern template class BLADE_EDITOR_API Factory<IEditorLogic>;
	typedef Factory<IEditorLogic> EditorLogicFactory;
	
}//namespace Blade

#endif //  __Blade_IEditorLogic_h__