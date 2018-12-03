/********************************************************************
	created:	2011/05/14
	filename: 	DInput8Base.h
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePlatform.h>

#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
#ifndef __Blade_DInput8Base_h__
#define __Blade_DInput8Base_h__
#include <BladeWin32API.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#include <RefCount.h>
#include <utility/BladeContainer.h>

namespace Blade
{

	class DInput8Base
	{
	public:
		DInput8Base();
		~DInput8Base();

	protected:
		/************************************************************************/
		/*                                                                      */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		bool				createDI8Keyboard();

		/*
		@describe 
		@param 
		@return 
		*/
		bool				createDI8Mouse();

		/*
		@describe 
		@param 
		@return 
		*/
		bool				createDI8JoyStick();

		/*
		@describe 
		@param 
		@return 
		*/
		void				releaseDI8Device();

		/************************************************************************/
		/*                                                                      */
		/************************************************************************/
		/** @brief  */
		inline static bool	isDInput8Inited()	{return msDInput8 != NULL;}

		/** @brief  */
		static bool			initDInput8();

		/** @brief  */
		static void			closeDInput8();


		LPDIRECTINPUTDEVICE8	mDI8Device;

		static LPDIRECTINPUT8	msDInput8;
		static RefCount			msInstanceCount;

		typedef struct SShareDevice
		{
			LPDIRECTINPUTDEVICE8	mDevice;
			RefCount				mSharedCount;
		}SHDEV;


		//for shared device with the same window handle
		class SharedDeviceMap : public Map<HWND,SHDEV>
		{
		public:
			typedef Map<HWND,SHDEV> BaseType;
		public:
			/** @brief  */
			bool	add(HWND hwnd,LPDIRECTINPUTDEVICE8  pDev)
			{
				if(hwnd == NULL || pDev == NULL )
					return false;

				SHDEV shdev;
				shdev.mDevice = pDev;
				shdev.mSharedCount.increment();

				std::pair<BaseType::iterator,bool> result = this->insert( BaseType::value_type(hwnd,shdev) );
				return result.second;
			}

			/** @brief  */
			LPDIRECTINPUTDEVICE8	addRef(HWND hwnd)
			{
				BaseType::iterator i = this->BaseType::find(hwnd);
				if( i == this->BaseType::end() )
					return NULL;

				SHDEV& shdev = i->second;
				shdev.mSharedCount.increment();
				return shdev.mDevice;
			}

			/** @brief  */
			bool	removeRef(HWND hwnd, LPDIRECTINPUTDEVICE8 pVerifyingDev)
			{
				BaseType::iterator i = this->BaseType::find(hwnd);
				if( i == this->BaseType::end() )
					return false;

				if( i->second.mDevice != pVerifyingDev )
				{
					assert(false);
					return false;
				}

				if( i->second.mSharedCount.decrement() == 0 )
					this->BaseType::erase(i);

				return true;
			}
		};
	};
	

}//namespace Blade





#endif // __Blade_DInput8Base_h__


#endif//BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS