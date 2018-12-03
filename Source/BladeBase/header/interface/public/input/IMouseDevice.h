/********************************************************************
	created:	2010/04/29
	filename: 	IMouseDevice.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_IMouseDevice_h__
#define __Blade_IMouseDevice_h__
#include <interface/public/IDevice.h>
#include <interface/public/input/IMouse.h>

namespace Blade
{
	class IWindow;

	class IMouseDevice : public IDevice, public IMouse
	{
	public:
		virtual ~IMouseDevice()		{}

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
		virtual void				notifyWindowSize(size_t width,size_t height) = 0;

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

	};//class IMouseDevice

	extern template class BLADE_BASE_API Factory<IMouseDevice>;

	typedef Handle<IMouseDevice> HMOUSEDEVICE;
	typedef Factory<IMouseDevice> MouseDeviceFactory;
	
}//namespace Blade


#endif //__Blade_IMouseDevice_h__