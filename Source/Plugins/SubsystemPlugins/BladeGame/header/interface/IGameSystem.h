/********************************************************************
	created:	2010/04/28
	filename: 	IGameSystem.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_IGameSystem_h__
#define __Blade_IGameSystem_h__
#include <interface/public/ISubsystem.h>
#include <interface/IGameScene.h>
#include <interface/InterfaceSingleton.h>
#include <interface/public/logic/ILogicService.h>

namespace Blade
{
	class IGameSystem : public ISubsystem, public ILogicService, public InterfaceSingleton<IGameSystem>
	{
	public:
		using InterfaceSingleton<IGameSystem>::getSingleton;
		using InterfaceSingleton<IGameSystem>::getSingletonPtr;
	public:
		virtual ~IGameSystem()		{}

		/*
		@describe
		@param
		@return
		*/
		virtual IGameScene* getGameScene(const TString& name) const = 0;

	};

	extern template class BLADE_GAME_API Factory<IGameSystem>;

	
}//namespace Blade


#endif //__Blade_IGameSystem_h__