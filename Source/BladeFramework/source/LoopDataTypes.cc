/********************************************************************
	created:	2011/10/23
	filename: 	LoopDataTypes.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <LoopDataTypes.h>
#include <interface/IEventManager.h>
#include <ConstDef.h>
#include <interface/public/graphics/GraphicsSynchronizedEvent.h>


namespace Blade
{
	namespace Impl
	{
		//////////////////////////////////////////////////////////////////////////
		static inline IEventManager& getEventManager()
		{
			static IEventManager& manager = IEventManager::getSingleton();
			return manager;
		}

	}//namespace Impl


	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	LoopFlag::LoopFlag()
	{
		mFlag[0] = mFlag[1] = mFlag[2] = mFlag[3] = 0;
		//this is aligned so that this&0x1 == 0, for Delegate implementation
		Impl::getEventManager().addEventHandler(ConstDef::EVENT_LOOP_DATA_CLEAR,EventDelegate(this,&LoopFlag::onLoopBegin) );
	}

	//////////////////////////////////////////////////////////////////////////
	LoopFlag::LoopFlag(bool true_false)
	{
		mFlag[0] = mFlag[1] = mFlag[2] = mFlag[3] = true_false ? 1u : 0;
		Impl::getEventManager().addEventHandler(ConstDef::EVENT_LOOP_DATA_CLEAR,EventDelegate(this,&LoopFlag::onLoopBegin) );
	}

	//////////////////////////////////////////////////////////////////////////
	LoopFlag::~LoopFlag()
	{
		Impl::getEventManager().removeEventHandlers(ConstDef::EVENT_LOOP_DATA_CLEAR,this );
	}

	//////////////////////////////////////////////////////////////////////////
	void			LoopFlag::onLoopBegin(const Event& data)
	{
		BLADE_UNREFERENCED(data);
		mFlag[0] = mFlag[1] = mFlag[2] = mFlag[3] = 0;
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	LoopRefCount::LoopRefCount()
	{
		Impl::getEventManager().addEventHandler(ConstDef::EVENT_LOOP_DATA_CLEAR,EventDelegate(this,&LoopRefCount::onLoopBegin) );
	}

	//////////////////////////////////////////////////////////////////////////
	LoopRefCount::LoopRefCount(size_t count)
		:mRefCount(count)
	{
		Impl::getEventManager().addEventHandler(ConstDef::EVENT_LOOP_DATA_CLEAR,EventDelegate(this,&LoopRefCount::onLoopBegin) );
	}

	//////////////////////////////////////////////////////////////////////////
	LoopRefCount::~LoopRefCount()
	{
		Impl::getEventManager().removeEventHandlers(ConstDef::EVENT_LOOP_DATA_CLEAR,this );
	}

	//////////////////////////////////////////////////////////////////////////
	void			LoopRefCount::onLoopBegin(const Event& data)
	{
		BLADE_UNREFERENCED(data);
		mRefCount.safeSet(0);
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	LoopPointer::LoopPointer()
		:mPtr(NULL)
	{
		Impl::getEventManager().addEventHandler(ConstDef::EVENT_LOOP_DATA_CLEAR,EventDelegate(this,&LoopPointer::onLoopBegin) );
	}

	//////////////////////////////////////////////////////////////////////////
	LoopPointer::LoopPointer(const void* ptr)
		:mPtr(ptr)
	{
		Impl::getEventManager().removeEventHandlers(ConstDef::EVENT_LOOP_DATA_CLEAR,this );
	}

	//////////////////////////////////////////////////////////////////////////
	LoopPointer::~LoopPointer()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void	LoopPointer::onLoopBegin(const Event& data)
	{
		BLADE_UNREFERENCED(data);
		mPtr = NULL;
	}

}//namespace Blade
