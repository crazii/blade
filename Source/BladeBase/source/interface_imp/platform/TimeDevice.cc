/********************************************************************
	created:	2014/06/18
	filename: 	TimeDevice.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "unix/UnixTimeDevice.h"
#include "windows/WindowsTimeDevice.h"

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	class TempTimeDevice : public TimeDevice, public TempAllocatable
	{
	public:
		using TempAllocatable::operator new;
		using TempAllocatable::operator delete;
	};

	//////////////////////////////////////////////////////////////////////////
	//static
	ITimeDevice*	ITimeDevice::create(EPool pool/* = TP_DEFAULT*/)
	{
		if( pool == TP_DEFAULT )
			return BLADE_NEW TimeDevice();
		else
			return BLADE_NEW TempTimeDevice();
	}
	
}//namespace Blade
