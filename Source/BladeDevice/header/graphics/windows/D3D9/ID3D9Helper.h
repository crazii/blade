/********************************************************************
	created:	2011/08/27
	filename: 	ID3D9Helper.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_ID3D9Helper_h__
#define __Blade_ID3D9Helper_h__
#include <BladeDevice.h>
#include <interface/InterfaceSingleton.h>
#include <interface/ILog.h>

namespace Blade
{

	class ID3D9Helper : public InterfaceSingleton<ID3D9Helper>
	{
	public:
		/** @brief  */
		virtual void outputString(const TString& str, ILog::ELogLevel level = ILog::LL_CRITICAL) = 0;
	};
	
	//extern template class BLADE_DEVICE_API Factory<ID3D9Helper>;

}//namespace Blade



#endif // __Blade_ID3D9Helper_h__