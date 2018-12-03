/********************************************************************
	created:	2015/08/11
	filename: 	LangEditorPlugin.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <BladeLangEditor.h>
#include "LangEditorPlugin.h"
#include "interface_imp/LangEditor.h"
#include "interface_imp/LangEditorFile.h"
#include <interface/IPlatformManager.h>
#include <interface/IEditorFramework.h>

namespace Blade
{
	static const TString PLUGIN_NAME = BTString("Editor_LanguageEditPlugin");
	BLADE_DEF_PLUGIN(LangEditorPlugin);

	//////////////////////////////////////////////////////////////////////////
	LangEditorPlugin::LangEditorPlugin()
		:PluginBase(PLUGIN_NAME)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	LangEditorPlugin::~LangEditorPlugin()
	{

	}

	/************************************************************************/
	/* IPlugin interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void				LangEditorPlugin::getSupportList(TStringParam& supportList) const
	{
		supportList.push_back( BTString("LangEditor") );
	}

	//////////////////////////////////////////////////////////////////////////
	void				LangEditorPlugin::getDependency(TStringParam& /*dependencyList*/) const
	{
		
	}

	//////////////////////////////////////////////////////////////////////////
	void				LangEditorPlugin::install()
	{
		//register file class type
		NameRegisterFactory(LangEditorFile, IEditorFile, LangEditorFile::LANG_EDITOR_FILE);
	}

	//////////////////////////////////////////////////////////////////////////
	void				LangEditorPlugin::uninstall()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void				LangEditorPlugin::initialize()
	{
		IEditorFramework::getSingleton().addEditor( LangEditor::getSingletonPtr() );
	}

	//////////////////////////////////////////////////////////////////////////
	void				LangEditorPlugin::shutdown()
	{
		IEditorFramework::getSingleton().removeEditor( LangEditor::getSingletonPtr() );
	}
	
}//namespace Blade
