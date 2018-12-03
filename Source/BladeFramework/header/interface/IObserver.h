/********************************************************************
	created:	2010/04/09
	filename: 	IObserver.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_IObserver_h__
#define __Blade_IObserver_h__
#include <BladeFramework.h>

namespace Blade
{
	class ISubject;

	class BLADE_FRAMEWORK_API IObserver
	{
	public:
		virtual ~IObserver()	{}


		/**
		@describe 
		@param 
		@return 
		*/
		virtual void onNotify(ISubject* changedSubject) = 0;


		/**
		@describe 
		@param 
		@return 
		*/
		virtual void onAttached(ISubject* subject) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void onDetached(ISubject* subject) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void update() = 0;

	};//class IObserver

	
}//namespace Blade


#endif //__Blade_IObserver_h__