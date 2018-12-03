/********************************************************************
	created:	2013/12/28
	filename: 	ITouchDevice.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_ITouchDevice_h__
#define __Blade_ITouchDevice_h__
#include <interface/public/input/ITouchView.h>
#include <interface/public/IDevice.h>

namespace Blade
{
	class IWindow;

	class ITouchDevice : public ITouchView
	{
	public:
		virtual ~ITouchDevice() {}

		/**
		@describe
		@param
		@return
		*/
		virtual void		attach(IWindow* window) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool		addListener(IListener* listener) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool		removeListener(IListener* listener) = 0;
	};//class ITouchDevice

	extern template class BLADE_BASE_API Factory<ITouchDevice>;

	typedef Handle<ITouchDevice> HTOUCHDEVICE;
	typedef Factory<ITouchDevice> TouchDeviceFactory;

}//namespace Blade

#endif //  __Blade_ITouchDevice_h__
