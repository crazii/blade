/********************************************************************
	created:	2010/09/06
	filename: 	GraphicsSynchronizedEvent.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_GraphicsSynchronizedEvent_h__
#define __Blade_GraphicsSynchronizedEvent_h__
#include <BladeFramework.h>
#include <Event.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	//dispatched in main sync state
	struct RenderDeviceReadyEvent : public Event
	{
		static BLADE_FRAMEWORK_API const TString NAME;

		RenderDeviceReadyEvent(const TString& deviceProfile, const TString& selectedProfile)
			:Event(NAME)
			,mDeviceProfie(deviceProfile)
			,mCurrentProfile(selectedProfile)
		{

		}
		TString	mDeviceProfie;
		TString	mCurrentProfile;
	};
	struct RenderDeviceCloseEvent : public Event
	{
		static BLADE_FRAMEWORK_API const TString NAME;
		RenderDeviceCloseEvent() :Event(NAME) {}
	};


}//namespace Blade


#endif //__Blade_GraphicsSynchronizedEvent_h__