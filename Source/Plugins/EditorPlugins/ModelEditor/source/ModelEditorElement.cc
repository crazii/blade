/********************************************************************
	created:	2017/2/27
	filename: 	ModelEditorElement.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include "ModelEditorElement.h"

namespace Blade
{
	const TString ModelEditorElement::MODELEDITOR_ELEMENT = BTString("ModelEditor");

	//////////////////////////////////////////////////////////////////////////
	ModelEditorElement::ModelEditorElement()
		:EditorElement(ModelEditorElement::MODELEDITOR_ELEMENT)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	ModelEditorElement::~ModelEditorElement()
	{

	}

	/************************************************************************/
	/* ISerializable interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void				ModelEditorElement::postProcess(const ProgressNotifier& notifier)
	{
		EditorElement::postProcess(notifier);
	}

	
}//namespace Blade