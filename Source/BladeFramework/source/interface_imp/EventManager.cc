/********************************************************************
	created:	2010/08/12
	filename: 	EventManager.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "EventManager.h"
#include <ExceptionEx.h>

namespace Blade
{

	//////////////////////////////////////////////////////////////////////////
	EventManager::EventManager()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	EventManager::~EventManager()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	bool			EventManager::registerEvent(const TString& state)
	{
		BLADE_TS_VERIFY(TS_MAIN_SYNC);
		std::pair<StateHandlerMap::iterator, bool> ret = mHandlers.insert( std::make_pair(state, (HandlerInfo*)NULL) );
		if( ret.second )
		{
			HandlerInfo*& infos = ret.first->second;
			infos = BLADE_NEW HandlerInfo[EventDelegate::P_COUNT];
		}
		return ret.second;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	EventManager::unRegisterEvent(const TString& state)
	{
		BLADE_TS_VERIFY(TS_MAIN_SYNC);
		StateHandlerMap::iterator i = mHandlers.find(state);
		if( i != mHandlers.end() )
		{
			HandlerInfo* infos = i->second;
			BLADE_DELETE[] infos;
			mHandlers.erase(i);
			return true;
		}
		else
			return false;
	}

	//////////////////////////////////////////////////////////////////////////
	void			EventManager::dispatchEvent(const Event& state) const
	{
		StateHandlerMap::const_iterator iter = mHandlers.find( state.mName );
		if(iter == mHandlers.end() )
			BLADE_EXCEPT(EXC_NEXIST,BTString("state with the name \"")+state.mName+BTString(" not registered.") );

		const HandlerInfo* infos = iter->second;

		for(int i = EventDelegate::P_START; i < EventDelegate::P_COUNT; ++i)
		{
			const HandlerInfo& info = infos[i];

			const StateHandlerSet& list = info.mlist;
			for( StateHandlerSet::const_iterator it = list.begin(); it != list.end(); ++it )
				it->call(state);

			const GroupHandlerList& group = info.mGroupedList;
			for( GroupHandlerList::const_iterator git = group.begin(); git != group.end(); ++git )
			{
				const StateHandlerSet& mfList = git->second;
				for( StateHandlerSet::const_iterator it = mfList.begin(); it != mfList.end(); ++it )
				{
					it->call(state);
				}
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void			EventManager::addEventHandler(const TString& state,const EventDelegate& stateDelegate)
	{

		StateHandlerMap::iterator i = mHandlers.find( state );
		if( i == mHandlers.end() )
			BLADE_EXCEPT(EXC_NEXIST,BTString("state with the name \"")+state+BTString(" not registered.") );
		if( stateDelegate.getPriority() < EventDelegate::P_START || stateDelegate.getPriority() >= EventDelegate::P_COUNT )
			BLADE_EXCEPT(EXC_INVALID_PARAM, BTString("invalid enum.") );

		HandlerInfo* infos = i->second;
		HandlerInfo& info = infos[ stateDelegate.getPriority() ];

		if( !stateDelegate.isBound() )
		{
			ScopedLock lock(info.mListLock);
			if( info.mlist.insert( stateDelegate ).second == false )
				BLADE_EXCEPT(EXC_REDUPLICATE,BTString("state handler entry has already added.") );
		}
		else
		{
			ScopedLock lock(info.mGroupedLock);
			StateHandlerSet& mfList = info.mGroupedList[stateDelegate.getBoundTarget()];
			if( mfList.insert( stateDelegate ).second == false )
				BLADE_EXCEPT(EXC_REDUPLICATE,BTString("state handler entry has already added.") );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void			EventManager::removeEventHandler(const TString& state, const EventDelegate& stateDelegate)
	{
		StateHandlerMap::iterator i = mHandlers.find( state );
		if( i == mHandlers.end() )
			BLADE_EXCEPT(EXC_NEXIST,BTString("cannot remove handler : state with the name \"")+state+BTString(" not registered.") );
		if( stateDelegate.getPriority() < EventDelegate::P_START || stateDelegate.getPriority() >= EventDelegate::P_COUNT )
			BLADE_EXCEPT(EXC_INVALID_PARAM, BTString("invalid enum.") );

		HandlerInfo* infos = i->second;
		HandlerInfo& info = infos[stateDelegate.getPriority()];

		if( !stateDelegate.isBound() )
		{
			ScopedLock lock(info.mListLock);
			if( info.mlist.erase( stateDelegate ) != 1 )
				BLADE_EXCEPT(EXC_REDUPLICATE,BTString("cannot remove handler : state handler entry not added.") );
		}
		else
		{
			ScopedLock lock(info.mGroupedLock);
			GroupHandlerList::iterator n = info.mGroupedList.find( stateDelegate.getBoundTarget() );
			if( n == info.mGroupedList.end() )
				BLADE_EXCEPT(EXC_REDUPLICATE,BTString("cannot remove handler : state handler entry not added.") );
			StateHandlerSet& mfList = n->second;
			if( mfList.erase( stateDelegate ) != 1 )
				BLADE_EXCEPT(EXC_REDUPLICATE,BTString("cannot remove handler : state handler entry not added.") );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void			EventManager::removeEventHandlers(const TString& state, const void* pTargetObject)
	{
		StateHandlerMap::iterator iter = mHandlers.find( state );
		if( !this->removeEventHandlersImpl(iter, pTargetObject) )
			BLADE_LOG(Information, BTString("cannot remove handler : state with the name \"") + state + BTString("\" not registered."));
	}

	//////////////////////////////////////////////////////////////////////////
	void			EventManager::removeEventHandlers(const void* pTargetObject)
	{
		for (StateHandlerMap::iterator iter = mHandlers.begin(); iter != mHandlers.end(); ++iter)
			this->removeEventHandlersImpl(iter, pTargetObject, false);
	}

	//////////////////////////////////////////////////////////////////////////
	bool			EventManager::removeEventHandlersImpl(StateHandlerMap::iterator iter, const void* pTargetObject, bool except/* = true*/)
	{
		if (iter == mHandlers.end())
			return false;

		bool found = false;
		HandlerInfo* infos = iter->second;
		for (int i = EventDelegate::P_START; i < EventDelegate::P_COUNT; ++i)
		{
			HandlerInfo& info = infos[i];
			ScopedLock lock(info.mGroupedLock);
			if (info.mGroupedList.erase(pTargetObject) == 1)
				found = true;	//Don' break
		}

		if (!found && except)
			BLADE_EXCEPT(EXC_NEXIST, BTString("cannot remove handler : state handler entry not added."));
		return found;
	}

}//namespace Blade