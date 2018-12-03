/********************************************************************
	created:	2010/09/08
	filename: 	TimeSource.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_TimeSource_h__
#define __Blade_TimeSource_h__
#include <interface/public/time/ITimeDevice.h>
#include <BladeFramework.h>

namespace Blade
{
	typedef uint64 LoopID;

	class BLADE_FRAMEWORK_API TimeSource
	{
	public:
		TimeSource();
		~TimeSource();

		/**
		@describe
		@param
		@return
		*/
		void					reset();

		/**
		@describe
		@param
		@return
		*/
		void					update();

		/**
		@describe
		@param
		@return
		*/
		inline const ITimeDevice&	getTimer() const	{return *mTimer;}

		/**
		@describe 
		@param
		@return
		*/
		inline scalar			getTime() const					{return this->getTimer().getSeconds();}

		/**
		@describe
		@param
		@return
		*/
		inline	LoopID			getLoopID() const				{return mLoopCount;}

		/**
		@describe
		@param
		@return
		*/
		inline scalar			getTimeThisLoop() const			{return mLoopTime;}

		/**
		@describe
		@param
		@return
		*/
		inline scalar			getTimePerLoop() const			{return mAverageLoopTime;}

		/**
		@describe when loop time is smaller than the loop limit ,then the loop time will be made up to it.
		@param
		@return
		*/
		inline void				setLoopTimeLimit(scalar limit)	{mLoopTimeLimit = limit;}


	protected:
		ITimeDevice*	mTimer;
		LoopID	mLoopCount;
		scalar	mLoopTime;
		scalar	mAverageLoopTime;
		scalar	mLastTime;
		scalar	mLoopTimeLimit;
	};//class TimeSource
	
}//namespace Blade


#endif //__Blade_TimeSource_h__