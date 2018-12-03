/********************************************************************
	created:	2014/08/02
	filename: 	Time.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_Time_h__
#define __Blade_Time_h__
#include <Lock.h>
#include <ctime>

namespace Blade
{
#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#	pragma warning(push)
#	pragma warning(disable:4996)
#endif

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class BLADE_BASE_API Time : public std::tm
	{
	public:
		///init to current time
		inline Time()
		{
			this->reset();
		}

		explicit inline Time(std::time_t t)
		{
			mTimeInSec = t;
			ScopedLock _lock(msLock);
			static_cast<std::tm&>(*this) = *std::gmtime(&t);
		}

		inline ~Time()	{}

		/** @brief reset to current time */
		inline void	reset()
		{
			mTimeInSec = 0;
			ScopedLock _lock(msLock);
			std::time(&mTimeInSec);	//thread safe
			//note: gmtime is not guaranteed to be thread safe
			//ref: http://pubs.opengroup.org/onlinepubs/009695399/functions/xsh_chap02_09.html
			static_cast<std::tm&>(*this) = *std::gmtime(&mTimeInSec);
		}
		time_t mTimeInSec;
	protected:
		static StaticLock msLock;
	};

	class BLADE_BASE_API LocalTime : public std::tm
	{
	public:
		///init to current time
		inline LocalTime()
		{
			this->reset();
		}

		explicit inline LocalTime(std::time_t t)
		{
			ScopedLock _lock(msLock);
			mTimeInSec = t;
			static_cast<std::tm&>(*this) = *std::gmtime(&t);
		}

		inline ~LocalTime()	{}

		/** @brief reset to current time */
		inline void	reset()
		{
			ScopedLock _lock(msLock);
			std::time(&mTimeInSec);
			//note: localtime is not guaranteed to be thread safe
			static_cast<std::tm&>(*this) = *std::localtime(&mTimeInSec);
		}
		time_t mTimeInSec;
	protected:
		static StaticLock msLock;
	};


	/************************************************************************/
	/* low precision timer. for high-precision timing, use ITimeDevice                                                                     */
	/************************************************************************/
	class BLADE_BASE_API Timer
	{
	public:
		inline Timer()
		{
			this->reset();
		}

		/** @brief  */
		inline void	reset()
		{
			mSeconds = 0;
			mMilliseconds = 0;
			mStartTime = std::clock();
		}

		/** @brief  */
		inline void	update()
		{
			mSeconds = scalar(std::clock()-mStartTime)/(scalar)CLOCKS_PER_SEC;
			mMilliseconds = (clock_t)(mSeconds*1000);
		}

		/** @brief  */
		inline unsigned long		getMilliseconds() const
		{
			return (unsigned long)mMilliseconds;
		}

		/** @brief  */
		inline scalar				getSeconds() const
		{
			return mSeconds;
		}

	protected:
		scalar			mSeconds;
		std::clock_t	mStartTime;
		std::clock_t	mMilliseconds;
	};


	/************************************************************************/
	/* timestamp for serialization (independent & consistent member sizes)                                                                    */
	/************************************************************************/
	typedef struct SFileTimeStamp
	{
		uint16	year;	///year
		uint8	day;	///day of month 1-31
		uint8	month;	///month 0-11
		uint8	hour;	///0-23
		uint8	minute;	///0-59
		uint8	second;	///0-59
		uint8	unused;

		inline SFileTimeStamp()
		{

		}

		inline SFileTimeStamp(const Time& t)
		{
			year = (uint16)t.tm_year;
			day = (uint8)t.tm_mday;
			month = (uint8)t.tm_mon;
			hour = (uint8)t.tm_hour;
			minute = (uint8)t.tm_min;
			second = (uint8)t.tm_sec;
		}

		/** @brief  */
		inline void toTime(Time& t)
		{
			t.reset();
			t.tm_year = year;
			t.tm_mday = day;
			t.tm_mon = month;
			t.tm_hour = hour;
			t.tm_min = minute;
			t.tm_sec = second;
		}

		/** @brief  */
		inline bool operator==(const SFileTimeStamp& rhs) const
		{
			return (this == &rhs) || (year == rhs.year && day == rhs.day && month == rhs.month && hour == rhs.hour && minute == rhs.minute && second == rhs.second);
		}

		/** @brief  */
		inline bool operator!=(const SFileTimeStamp& rhs) const {return !(*this == rhs); }

		/** @brief  */
		inline bool operator<(const SFileTimeStamp& rhs) const
		{
			return year < rhs.year || day < rhs.day || month < rhs.month || hour < rhs.hour || minute < rhs.minute || second == rhs.second;
		}
	}FILE_TIME;

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#	pragma warning(pop)
#endif
}//namespace Blade


#endif // __Blade_Time_h__