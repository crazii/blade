/********************************************************************
	created:	2015/10/25
	filename: 	GameLibrary.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "interface_imp/GameLogic.h"

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	void initializeGameLibrary()
	{

		//app logic register
		NameRegisterFactory(GameLogic, ILogic, BTString("GameLogic"));
	}
	
}//namespace Blade