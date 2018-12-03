/********************************************************************
	created:	2017/2/27
	filename: 	ModelEditorElement.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_ModelEditorElement_h__
#define __Blade_ModelEditorElement_h__
#include <interface/EditorElement.h>

namespace Blade
{

	class ModelEditorElement : public EditorElement
	{
	public:
		static const TString MODELEDITOR_ELEMENT;
	public:
		ModelEditorElement();
		~ModelEditorElement();

		/************************************************************************/
		/* ISerializable interface                                                                     */
		/************************************************************************/
		/** @brief called after loading data in main synchronous state */
		virtual void	postProcess(const ProgressNotifier& notifier);
	};
	
}//namespace Blade


#endif//__Blade_ModelEditorElement_h__