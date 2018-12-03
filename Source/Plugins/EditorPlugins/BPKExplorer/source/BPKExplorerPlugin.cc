/********************************************************************
	created:	2015/08/17
	filename: 	BPKExplorerPlugin.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <BladeBPKExplorer.h>
#include <interface/IPlatformManager.h>
#include <interface/IEditorFramework.h>

#include "BPKExplorerPlugin.h"
#include "interface_imp/BPKExplorer.h"
#include "interface_imp/BPKEditorFile.h"
#include "interface_imp/BPKEditorController.h"

namespace Blade
{
	static const TString PLUGIN_NAME = BTString("Editor_BPKExplorerPlugin");
	BLADE_DEF_PLUGIN(BPKExplorerPlugin);

	//////////////////////////////////////////////////////////////////////////
	BPKExplorerPlugin::BPKExplorerPlugin()
		:PluginBase(PLUGIN_NAME)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	BPKExplorerPlugin::~BPKExplorerPlugin()
	{

	}

	/************************************************************************/
	/* IPlugin interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void				BPKExplorerPlugin::getSupportList(TStringParam& supportList) const
	{
		supportList.push_back( BTString("BPKExplorer") );
	}

	//////////////////////////////////////////////////////////////////////////
	void				BPKExplorerPlugin::getDependency(TStringParam& /*dependencyList*/) const
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void				BPKExplorerPlugin::install()
	{
		//register file class type
		NameRegisterFactory(BPKEditorFile, IEditorFile, BPKEditorFile::BPK_FILE);
		RegisterSingleton(BPKEditorController, IBPKEditorController);
	}

	//////////////////////////////////////////////////////////////////////////
	void				BPKExplorerPlugin::uninstall()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void				BPKExplorerPlugin::initialize()
	{
		IEditorFramework::getSingleton().addEditor( BPKExplorer::getSingletonPtr() );
	}

	//////////////////////////////////////////////////////////////////////////
	void				BPKExplorerPlugin::shutdown()
	{
		IEditorFramework::getSingleton().removeEditor( BPKExplorer::getSingletonPtr() );
	}

}