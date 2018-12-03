/********************************************************************
	created:	2018/11/18
	filename: 	FrameEvents.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <FrameEvents.h>
#include <interface/IEventManager.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	const TString	FrameBeginEvent::NAME = BTString("Render::BeginFrame");
	const TString	FrameEndEvent::NAME = BTString("Render::EndFrame");

	namespace Impl
	{
		//////////////////////////////////////////////////////////////////////////
		static inline IEventManager& getEventManager()
		{
			static IEventManager& manager = IEventManager::getSingleton();
			return manager;
		}

	}//namespace Impl

	//////////////////////////////////////////////////////////////////////////
	FrameFlag::FrameFlag()
	{
		mFlag[0] = mFlag[1] = mFlag[2] = mFlag[3] = 0;
		Impl::getEventManager().addEventHandler(FrameBeginEvent::NAME, EventDelegate(this, &FrameFlag::onFrameBegin));
	}

	//////////////////////////////////////////////////////////////////////////
	FrameFlag::FrameFlag(bool true_false)
	{
		mFlag[0] = mFlag[1] = mFlag[2] = mFlag[3] = true_false ? 1u : 0;
		Impl::getEventManager().addEventHandler(FrameBeginEvent::NAME, EventDelegate(this, &FrameFlag::onFrameBegin));
	}

	//////////////////////////////////////////////////////////////////////////
	FrameFlag::~FrameFlag()
	{
		Impl::getEventManager().removeEventHandlers(FrameBeginEvent::NAME, this);
	}

	//////////////////////////////////////////////////////////////////////////
	void			FrameFlag::onFrameBegin(const Event& data)
	{
		BLADE_UNREFERENCED(data);
		mFlag[0] = mFlag[1] = mFlag[2] = mFlag[3] = 0;
	}
	
}//namespace Blade