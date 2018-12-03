/********************************************************************
	created:	2011/11/02
	filename: 	Exports.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <interface/public/logic/ILogic.h>
#include <interface/public/logic/ILogicService.h>
#include <interface/public/logic/ILogicWorld.h>
#include <interface/public/logic/ILogicScene.h>
#include <interface/public/logic/ICameraController.h>
#include <interface/public/logic/ILogicWorldManager.h>

namespace Blade
{

	template class Factory<ILogic>;
	template class Factory<ILogicWorld>;
	template class Factory<ILogicWorldManager>;
	
}//namespace Blade