/********************************************************************
	created:	2011/08/27
	filename: 	D3D9Helper.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_D3D9Helper_h__
#define __Blade_D3D9Helper_h__
#include <BladePlatform.h>
#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
#include <graphics/windows/D3D9/ID3D9Helper.h>

namespace Blade
{
	class D3D9Helper : public ID3D9Helper , public Singleton<D3D9Helper>
	{
	public:
		D3D9Helper();
		virtual ~D3D9Helper();

		/************************************************************************/
		/*                                                                      */
		/************************************************************************/
		/** @brief  */
		virtual void outputString(const TString& str, ILog::ELogLevel level = ILog::LL_CRITICAL);
	};	

}//namespace Blade

#endif//BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS

#endif // __Blade_D3D9Helper_h__