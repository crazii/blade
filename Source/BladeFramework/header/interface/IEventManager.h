/********************************************************************
	created:	2010/04/09
	filename: 	IEventManager.h
	author:		Crazii
	
	purpose:
	12/10/2015:	name changed from IEventManager to IEventManager
*********************************************************************/
#ifndef __Blade_IEventManager_h__
#define __Blade_IEventManager_h__
#include <BladeFramework.h>
#include <interface/InterfaceSingleton.h>
#include <utility/Delegate.h>
#include <Event.h>

namespace Blade
{
	class BLADE_FRAMEWORK_API EventDelegate : public Delegate
	{
	public:
		/** @brief use priority to solve order conflict. higher priority get called first */
		enum EPriority
		{
			P_HIGH,
			P_ABOVENORMAL,
			P_NORMAL,
			P_BELOWNORMAL,
			P_LOW,

			P_COUNT,
			P_START = P_HIGH,
			P_DEFAULT = P_NORMAL,
		};
	public:
		/** @brief  */
		template<typename T>
		EventDelegate(T* ptr, void (T::*mfn)(const Event&), EPriority priority = P_DEFAULT)
			:Delegate(ptr, mfn)
			,mPriority(priority)
		{
		}

		/** @brief  */
		EventDelegate(pfnEventHandler handler, EPriority priority = P_DEFAULT)
			:Delegate(handler)
			,mPriority(priority)
		{

		}

		EPriority getPriority() const	{return mPriority;}
	protected:
		EPriority mPriority;
	};

	class IEventManager : public InterfaceSingleton<IEventManager>
	{
	public:
		virtual ~IEventManager()		{}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool			registerEvent(const TString& state) = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual bool			unRegisterEvent(const TString& state) = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual void			dispatchEvent(const Event& state) const = 0;

		/**
		@describe
		@note handlers for the same state are called in order of priority,
		for handlers of the same priority, orders are undefined.
		@param
		@return
		*/
		virtual void			addEventHandler(const TString& state, const EventDelegate& stateDelegate) = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual void			removeEventHandler(const TString& state, const EventDelegate& stateDelegate) = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual void			removeEventHandlers(const TString& state, const void* pTargetObject) = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual void			removeEventHandlers(const void* pTargetObject) = 0;

	};//class IEventManager

	extern template class BLADE_FRAMEWORK_API Factory<IEventManager>;
	
}//namespace Blade


#endif //__Blade_IEventManager_h__