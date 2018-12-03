/********************************************************************
	created:	2014/06/17
	filename: 	UnixTimeDevice.h
	author:		Crazii
	
	purpose:	*nix TimeDevice for linux, android, ios
*********************************************************************/
#ifndef __Blade_UnixTimeDevice_h__
#define __Blade_UnixTimeDevice_h__
#include <BladePlatform.h>
#if BLADE_IS_UNIX_CLASS_SYSTEM

#include <interface/public/time/ITimeDevice.h>
#include <sys/time.h>

namespace Blade
{
	class TimeDevice : public ITimeDevice, public Allocatable
	{
	public:
		TimeDevice();
		~TimeDevice();

		/** @brief  */
		virtual void				resetImpl();

		/** @brief  */
		virtual void				updateImpl();

	protected:
		::timespec	mStart;
	};
	
}//namespace Blade

#endif//BLADE_IS_UNIX_CLASS_SYSTEM


#endif // __Blade_UnixTimeDevice_h__
