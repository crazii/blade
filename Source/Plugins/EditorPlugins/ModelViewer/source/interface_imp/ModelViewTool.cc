/********************************************************************
	created:	2015/10/25
	filename: 	ModelViewTool.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "ModelViewTool.h"
#include <EditorHelper.h>
#include <interface/IEditorFramework.h>
#include "ModelViewerFile.h"

namespace Blade
{
	const TString MODEL_TOOL_CATEGORY = BTString(BLANG_MODEL);
	const TString IKTestHelperElement::IKHELPER_TYPE = BTString("IKTest Helper Element");

	//////////////////////////////////////////////////////////////////////////
	IKTestHelperElement::IKTestHelperElement()
		:EditorElement(IKHELPER_TYPE)
		,mEffector(ModelConsts::EFFECTOR_HAND)
		,mEffectorIndex(0)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	ModelIKTestTool::ModelIKTestTool(IconIndex icon)
		:TranslateTool(icon)
	{
		//change name of original translate tool
		mName = BXLang("IK Testing tool");
	}

	//////////////////////////////////////////////////////////////////////////
	ModelIKTestTool::~ModelIKTestTool()
	{
		
	}

	//////////////////////////////////////////////////////////////////////////
	void				ModelIKTestTool::onEnable(bool bEnabled)
	{
		TranslateTool::onEnable(bEnabled);

		if( bEnabled && mHelper == NULL )
		{
			IEditorFile* file = IEditorFramework::getSingleton().getCurrentFile();
			assert(file != NULL && file->getType() == MODEL_FILE_TYPE );
			if( file != NULL && file->getType() == MODEL_FILE_TYPE )
			{
				mHelper.bind( EditorHelper::createDummyEditable(file, BTString("IK Test helper"), IKTestHelperElement::IKHELPER_TYPE) );
			}
		}
		else if( !bEnabled && mHelper != NULL )
		{
			BLADE_TS_VERIFY(TS_MAIN_SYNC);
			mHelper.clear();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void				ModelIKTestTool::onSelection()
	{
		IEditorFile* file = IEditorFramework::getSingleton().getCurrentFile();
		assert(file != NULL && file->getType() == MODEL_FILE_TYPE );

		if( this->isSelected() )
		{
			mHelper->setSelected(true);
			file->setSelectedEditable(mHelper);
			this->addEditableGizmo(mHelper);
		}
		else
		{
			mHelper->setSelected(false);
			file->setSelectedEditable(NULL);
			this->removeEditableGizmo(mHelper);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool				ModelIKTestTool::mainUpdate(scalar x,scalar y,uint32 keyModifier,scalar deltaTime,IEditorFile* file)
	{
		assert(file != NULL && file->getType() == MODEL_FILE_TYPE );

		if( !this->isEnable() || !this->isSelected() )
			return false;

		bool ret = TranslateTool::mainUpdate(x, y, keyModifier, deltaTime, file);
		IElement* elem = mHelper->getTargetEntity()->getElementByType(GeomConsts::GEOMETRY_ELEMENT);
		IKTestHelperElement* helper = static_cast<IKTestHelperElement*>(mHelper->getTargetEntity()->getElementByType(IKTestHelperElement::IKHELPER_TYPE));

		if(elem != NULL && helper != NULL)
		{
			IGeometry* geom = elem->getInterface(IID_GEOMETRY());
			ModelViewerFile* f = static_cast<ModelViewerFile*>(file);
			if( f->getEditable() != NULL )
			{
				ISyncAnimation* syncAnim = f->getEditable()->getTargetEntity()->getInterface(IID_SYNCANIM());
				if(syncAnim != NULL)
					syncAnim->setIKChainPosition(helper->mEffector, (index_t)helper->mEffectorIndex, geom->getGlobalPosition());
			}
		}
		return ret;
	}
	
}//namespace Blade