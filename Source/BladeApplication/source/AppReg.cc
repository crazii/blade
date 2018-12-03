/********************************************************************
	created:	2011/08/29
	filename: 	AppReg.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <GameApplication.h>
#include <EditorApplication.h>
#include "CmdParser.h"

namespace Blade
{
	template class Factory<Application>;
	template class Factory<ICmdParser>;

	//////////////////////////////////////////////////////////////////////////
	bool	initializeAppFramework()
	{
		//register the app classes in the following order:
		//the default( first one ) is game application
		NameRegisterSingleton(GameApplication, Application, GameApplication::TYPE);
#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
		NameRegisterSingleton(EditorApplication, Application, EditorApplication::TYPE);
#endif
		RegisterSingleton(CmdParser,ICmdParser);
		return true;
	}

}//namespace Blade
