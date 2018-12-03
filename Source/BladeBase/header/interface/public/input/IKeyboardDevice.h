/********************************************************************
	created:	2010/04/29
	filename: 	IKeyboardDevice.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_IKeyboardDevice_h__
#define __Blade_IKeyboardDevice_h__
#include <interface/public/IDevice.h>
#include <interface/public/input/IKeyboard.h>

namespace Blade
{
	class IWindow;

	class IKeyboardDevice : public IDevice,public IKeyboard
	{
	public:
		virtual ~IKeyboardDevice()	{}

		/**
		@describe
		@param
		@return
		*/
		virtual void				attach(IWindow* window) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool				addListener(IListener* listener) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool				removeListener(IListener* listener) = 0;

	};//class IKeyboardDevice

	extern template class BLADE_BASE_API Factory<IKeyboardDevice>;

	typedef Handle<IKeyboardDevice> HKBDDEVICE;
	typedef Factory<IKeyboardDevice> KeyboardDeviceFactory;
	
}//namespace Blade


#endif //__Blade_IKeyboardDevice_h__