/********************************************************************
	created:	2014/06/17
	filename: 	ITimeDevice.h
	author:		Crazii
	
	purpose:	moved from utility/Timer.h
*********************************************************************/
#ifndef __Blade_ITimeDevice_h__
#define __Blade_ITimeDevice_h__
#include <BladeBase.h>
#include <ctime>

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable:4996)
#endif

namespace Blade
{

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class BLADE_BASE_API ITimeDevice
	{
	public:
		enum EPool
		{
			TP_DEFAULT,
			TP_TEMPORARY,
		};
	public:
		virtual ~ITimeDevice()	{}

		/** @brief  */
		inline void	reset()
		{
			mElapsedMilliSecondFP64 = 0;
			mElapsedMilliseconds = 0;
			this->resetImpl();
		}

		/** @brief  */
		inline void	update()
		{
			this->updateImpl();
			mElapsedSeconds = scalar(mElapsedMilliseconds)/scalar(1000);
		}

		/** @brief  */
		inline fp64					getMillisecondsHP() const
		{
			return mElapsedMilliSecondFP64;
		}

		/** @brief  */
		inline uint64				getMilliseconds() const
		{
			return mElapsedMilliseconds;
		}

		/** @brief  */
		inline scalar				getSeconds() const
		{
			return mElapsedSeconds;
		}

		/**
		@describe 
		@param 
		@return 
		*/
		static ITimeDevice*	create(EPool pool = TP_DEFAULT);

	protected:

		/** @brief reset high precision time */
		virtual void	resetImpl() = 0;

		/** @brief update high precision time */
		virtual void	updateImpl() = 0;

		fp64			mElapsedMilliSecondFP64;
		uint64			mElapsedMilliseconds;
		scalar			mElapsedSeconds;
	};
	
}//namespace Blade



#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(pop)
#endif

#endif // __Blade_ITimeDevice_h__
