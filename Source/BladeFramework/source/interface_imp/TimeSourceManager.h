/********************************************************************
	created:	2010/09/10
	filename: 	TimeSourceManager.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_TimeSourceManager_h__
#define __Blade_TimeSourceManager_h__
#include <interface/ITimeService.h>
#include <Singleton.h>

namespace Blade
{
	class TimeSourceManager : public ITimeService, public Singleton<TimeSourceManager>
	{
	public:
		using Singleton<TimeSourceManager>::getSingleton;
		using Singleton<TimeSourceManager>::getSingletonPtr;
	public:
		TimeSourceManager();
		~TimeSourceManager();

		/************************************************************************/
		/* ITimerService interface                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		virtual const TimeSource&		getTimeSource() const;

		/*
		@describe
		@param
		@return
		*/
		virtual void					resetTimeSource();

		/*
		@describe
		@param
		@return
		*/
		virtual void					setLoopTimeLimit(scalar fLoopTime = 0.016667f);

		/************************************************************************/
		/* custom method internal                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		TimeSource&						getTimeSource();

	protected:
		TimeSource			mTimeSource;
	};//class TimeSourceManager
	
}//namespace Blade


#endif //__Blade_TimeSourceManager_h__