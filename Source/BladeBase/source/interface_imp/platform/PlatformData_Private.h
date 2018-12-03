/********************************************************************
	created:	2015/08/05
	filename: 	PlatformData_Private.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_PlatformData_Private_h__
#define __Blade_PlatformData_Private_h__
#include <utility/String.h>
#include <Singleton.h>
#include <utility/BladeContainer.h>

namespace Blade
{
	class PlatformData_Private;
	extern template class BLADE_BASE_API Singleton<PlatformData_Private>;

	class PlatformData_Private : public Singleton<PlatformData_Private>
	{
	public:
		String	mCodePage;
		TString	mLanguage;
		Vector<TString>	mSupportedLanguages;
	};

}//namespace Blade


#endif // __Blade_PlatformData_Private_h__