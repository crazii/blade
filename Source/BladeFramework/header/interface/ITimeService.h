/********************************************************************
	created:	2010/09/10
	filename: 	ITimeService.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_ITimeService_h__
#define __Blade_ITimeService_h__
#include <interface/InterfaceSingleton.h>
#include <TimeSource.h>

namespace Blade
{
	class ITimeService : public InterfaceSingleton<ITimeService>
	{
	public:
		virtual ~ITimeService()		{}

		/**
		@describe
		@param
		@return
		*/
		virtual const TimeSource&		getTimeSource() const = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual void					resetTimeSource() = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual void					setLoopTimeLimit(scalar fLoopTime = 0.016667f) = 0;

	};//class ITimeService

	extern template class BLADE_FRAMEWORK_API Factory<ITimeService>;
	
}//namespace Blade


#endif //__Blade_ITimeService_h__