/********************************************************************
	created:	2015/03/24
	filename: 	App.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_App_h__
#define __Blade_App_h__
#include <interface/public/logic/ILogicWorld.h>

namespace Blade
{
	class Event;

	/** @brief render device ready handler for android, to pre-compile shaders */
	extern void onRenderDeviceReady(const Event&);

	/** @brief  */
	extern void  initGameFSM();

	/** @brief default world/stage info, shared by app */
	struct APP_DEF_WORLD_INFO
	{
		WORLD_INFO	winfo;
		ILogicWorld* world;

		APP_DEF_WORLD_INFO() :winfo((IStage*)NULL), world(NULL) {}
	};
	extern APP_DEF_WORLD_INFO DEF_WORLD_INFO;
	
}//namespace Blade


#endif // __Blade_App_h__