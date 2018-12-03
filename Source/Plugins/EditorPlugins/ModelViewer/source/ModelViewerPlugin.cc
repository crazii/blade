/********************************************************************
	created:	2013/04/09
	filename: 	ModelViewerPlugin.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "BladeModelViewer.h"
#include "ModelViewerPlugin.h"

#include "interface_imp/ModelViewerFile.h"
#include "interface_imp/ModelViewer.h"
#include "interface_imp/ModelViewElement.h"
#include "interface_imp/ModelViewTool.h"

#include <interface/IPlatformManager.h>
#include <interface/IEditorFramework.h>
#include <BladeModelViewer_blang.h>

#include <interface/IModelConfigManager.h>
#include <databinding/DataSourceWriter.h>

namespace Blade
{
	static const TString PLUGIN_NAME = BTString("Editor_ModelViewerPlugin");
	BLADE_DEF_PLUGIN(ModelViewerPlugin);

	//////////////////////////////////////////////////////////////////////////
	ModelViewerPlugin::ModelViewerPlugin()
		:PluginBase(PLUGIN_NAME)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	ModelViewerPlugin::~ModelViewerPlugin()
	{

	}

	/************************************************************************/
	/* IPlugin interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void				ModelViewerPlugin::getSupportList(TStringParam& supportList) const
	{
		supportList.push_back( BTString("ModelViewer") );
	}

	//////////////////////////////////////////////////////////////////////////
	void				ModelViewerPlugin::getDependency(TStringParam& dependencyList) const
	{
		dependencyList.push_back( BTString("GraphicsModel") );
	}

	//////////////////////////////////////////////////////////////////////////
	void				ModelViewerPlugin::install()
	{
		//register file class type
		NameRegisterFactory(ModelViewerFile,IEditorFile, MODEL_FILE_TYPE);
		NameRegisterFactory(ModelViewElement, EditorElement, ModelViewElement::MODELVIEW_ELEMENT_TYPE);
		NameRegisterFactory(IKTestHelperElement, EditorElement, IKTestHelperElement::IKHELPER_TYPE);

		{
			DataSourceWriter<IKTestHelperElement> writer(IKTestHelperElement::IKHELPER_TYPE);
			writer.beginVersion(Version(0, 1, 'a'));
			Variant effectors[] = { ModelConsts::EFFECTOR_HAND, ModelConsts::EFFECTOR_FOOT };
			writer << BXLang("Effectors") << effectors << &IKTestHelperElement::mEffector;
			writer << BXLang("Effector Sub Index") << &IKTestHelperElement::mEffectorIndex;
			writer.endVersion();
		}

		{
			DataSourceWriter<ModelViewElement> writer(ModelViewElement::MODELVIEW_ELEMENT_TYPE, DS_SIGNATURE(EditorElement));
			writer.beginVersion(Version(0, 1, 'a'));
			writer << BTString("Animations") << CONFIG_UIHINT(CUIH_NONE, ModelConsts::MODEL_ANIMATION_ITEM_HINT) << &ModelViewElement::mAnimationList;
			writer << BTString("Bones") << CONFIG_UIHINT(CUIH_NONE, ModelConsts::MODEL_BONE_ITEM_HINT) << &ModelViewElement::mBoneList;
			writer.endVersion();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void				ModelViewerPlugin::uninstall()
	{
		
	}

	//////////////////////////////////////////////////////////////////////////
	void				ModelViewerPlugin::initialize()
	{
		IModelConfigManager::getInterface().setEditingMode(true);
		IEditorFramework::getSingleton().addEditor( ModelViewer::getSingletonPtr() );
	}

	//////////////////////////////////////////////////////////////////////////
	void				ModelViewerPlugin::shutdown()
	{
		IEditorFramework::getSingleton().removeEditor( ModelViewer::getSingletonPtr() );
	}
	
}//namespace Blade