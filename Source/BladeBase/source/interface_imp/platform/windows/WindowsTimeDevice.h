/********************************************************************
	created:	2014/06/17
	filename: 	WindowsTimeDevice.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_WindowsTimeDevice_h__
#define __Blade_WindowsTimeDevice_h__
#include <BladePlatform.h>
#if BLADE_IS_WINDOWS_CLASS_SYSTEM
#include "../../../BladeWin32API.h"
#include <interface/public/time/ITimeDevice.h>

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

		static LARGE_INTEGER	msFrequency;

		/** @brief  */
		static void	staticInit()
		{
			static bool bInited = false;
			if( !bInited )
			{
				::QueryPerformanceFrequency( &msFrequency );
				bInited = true;
			}
		}

		DWORD			mLastTick;
		LARGE_INTEGER	mStartTime;
		LONGLONG		mLastTime;
	};
	
}//namespace Blade
#endif//BLADE_IS_WINDOWS_CLASS_SYSTEM

#endif // __Blade_WindowsTimeDevice_h__
