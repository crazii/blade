/********************************************************************
	created:	2009/02/27
	filename: 	IWindowSystem.h
	author:		Crazii
	
	purpose:	abstract interface of WindowSystem
*********************************************************************/
#ifndef __Blade_IWindowSystem_h__
#define __Blade_IWindowSystem_h__
#include <BladeWindow.h>


namespace Blade
{
	class IWindowSystem : public ISubsystem, public IWindowService
	{
	public:
		virtual ~IWindowSystem()	{}

	};//class IWindowSystem
	
}//namespace Blade


#endif // __Blade_IWindowSystem_h__