/********************************************************************
	created:	2009/02/14
	created:	14:2:2009   8:42
	filename: 	IDevice.h
	author:		Crazii
	
	purpose:	device abstraction
*********************************************************************/
#ifndef __Blade_IDevice_h__
#define __Blade_IDevice_h__
#include <Handle.h>
#include <Factory.h>
#include <utility/Variant.h>
#include <utility/StringList.h>

namespace Blade
{
	
	class BLADE_BASE_API IDevice
	{
	public:
		typedef TNamedList<TStringParam, TempAllocator< NamedType<TStringParam> >, TempAllocatable> Options;

		///pure virtual dector
		virtual ~IDevice()		{}

		/** @brief open a device */
		virtual bool	open() = 0;

		/** @brief check if it is open */
		virtual bool	isOpen() const = 0;

		/** @brief close the device */
		virtual void	close() = 0;

		/** @brief reset the device */
		virtual bool	reset() = 0;

		/** @brief update data(I/O) */
		virtual bool	update() = 0;

		/** @brief value with possible options. outOption is optional. */
		virtual void	getDeviceOption(ParamList& defVal, Options& outOption) { BLADE_UNREFERENCED(defVal); BLADE_UNREFERENCED(outOption); }

		/** @brief  */
		virtual bool	applyDeviceOption(const ParamList& /*result*/) {return true;}

	};//class IDevice
	typedef Handle<IDevice> HDEVICE;

	extern template class BLADE_BASE_API Factory<IDevice>;
	typedef Factory<IDevice> DeviceFactory;

}//namespace Blade

#endif // __Blade_IDevice_h__