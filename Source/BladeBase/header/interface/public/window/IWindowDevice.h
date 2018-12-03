/********************************************************************
	created:	2009/02/15
	filename: 	IWindowDevice.h
	author:		Crazii
	
	purpose:	platform independent IWindow IDevice abstraction
*********************************************************************/
#ifndef __Blade_IWindowdevice_h__
#define __Blade_IWindowdevice_h__
#include <interface/public/IDevice.h>
#include <interface/public/window/IWindow.h>

namespace Blade
{

	class IWindowDevice : public IDevice, public IWindow
	{
	public:
		virtual ~IWindowDevice()	{}

		/** @brief specify a external window to the device */
		/* @remark in this situation,when calling destroyWindow, it doesn't destroy the internal window and just detach it */
		virtual void attachWindow(uintptr_t internalImpl) = 0;

		/** @brief set the window's specific color depth */
		virtual void setColorDepth(int depth) = 0;

		/** @brief directly destroy window, without "close" notification */
		virtual void destroyWindow() = 0;

	};//class IWindowDevice

	typedef Handle<IWindowDevice> HWINDEVICE;

	extern template class BLADE_BASE_API Factory<IWindowDevice>;
	typedef Factory<IWindowDevice> WindowDeviceFactory;


}//namespace Blade


#endif // __Blade_IWindowdevice_h__