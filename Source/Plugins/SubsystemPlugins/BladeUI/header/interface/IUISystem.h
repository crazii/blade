/********************************************************************
	created:	2010/04/29
	filename: 	IUISystem.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_IUISystem_h__
#define __Blade_IUISystem_h__
#include <BladeUI.h>
#include <interface/public/ui/IUIService.h>
#include <interface/public/ISubsystem.h>

namespace Blade
{
	class IUISystem : public ISubsystem, public IUIService
	{
	public:
		virtual ~IUISystem()	{}

	};//class IUISystem
	
}//namespace Blade


#endif //__Blade_IUISystem_h__