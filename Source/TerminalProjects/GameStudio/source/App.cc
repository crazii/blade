/********************************************************************
	created:	2018/05/12
	filename: 	App.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <App.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	//shared global
	APP_DEF_WORLD_INFO DEF_WORLD_INFO;

	//temp code for demo
	extern void initDemoStates();

	//////////////////////////////////////////////////////////////////////////
	void  initGameFSM()
	{
		initDemoStates();
	}

}//namespace Blade