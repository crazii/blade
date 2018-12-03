/********************************************************************
	created:	2010/08/12
	filename: 	EventManager.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_EventManager_h__
#define __Blade_EventManager_h__
#include <interface/IEventManager.h>
#include <utility/BladeContainer.h>

namespace Blade
{

	class EventManager : public IEventManager ,public Singleton<EventManager>
	{
	public:
		EventManager();
		~EventManager();

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			registerEvent(const TString& state);

		/*
		@describe
		@param
		@return
		*/
		virtual bool			unRegisterEvent(const TString& state);

		/*
		@describe
		@param
		@return
		*/
		virtual void			dispatchEvent(const Event& state) const;

		/*
		@describe
		@param
		@return
		*/
		virtual void			addEventHandler(const TString& state, const EventDelegate& stateDelegate);

		/*
		@describe
		@param
		@return
		*/
		virtual void			removeEventHandler(const TString& state, const EventDelegate& stateDelegate);

		/*
		@describe
		@note: if you register a object of base class, and the same object with derived (different type of) class,
		you should call removeEventHandlers with the same pointer for both types, this is by design.
		@param
		@return
		*/
		virtual void			removeEventHandlers(const TString& state, const void* pTarget);

		/**
		@describe 
		@param
		@return
		*/
		virtual void			removeEventHandlers(const void* pTargetObject);

	protected:
		typedef	Set<Delegate>						StateHandlerSet;
		typedef Map<const void*, StateHandlerSet>	GroupHandlerList;

		struct HandlerInfo : public Allocatable
		{
			StateHandlerSet		mlist;
			GroupHandlerList	mGroupedList;
			Lock				mListLock;
			Lock				mGroupedLock;
		};
		typedef TStringMap<HandlerInfo*>			StateHandlerMap;

		/** @brief  */
		bool			removeEventHandlersImpl(StateHandlerMap::iterator iter, const void* pTargetObject, bool except = true);

		StateHandlerMap			mHandlers;
	};//class EventManager 

	
}//namespace Blade


#endif //__Blade_EventManager_h__