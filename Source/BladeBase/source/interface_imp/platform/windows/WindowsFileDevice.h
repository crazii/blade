/********************************************************************
	created:	2014/06/17
	filename: 	WindowsFileDevice.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_WindowsFileDevice_h__
#define __Blade_WindowsFileDevice_h__
#include <BladePlatform.h>

#if BLADE_IS_WINDOWS_CLASS_SYSTEM
#include "../FileDevice.h"

namespace Blade
{
	class WindowsFileDevice : public FileDevice , public Allocatable
	{
	public:
		WindowsFileDevice();
		~WindowsFileDevice();

		/************************************************************************/
		/* IFileDevice interfaces                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			findFile(TStringParam& result,const TString& pattern, int findFlag = FF_DIR|FF_FILE);

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/

	protected:

	};//class WindowsFileDevice
	
	typedef WindowsFileDevice DefaultFileDevice;
}//namespace Blade

#endif//BLADE_IS_WINDOWS_CLASS_SYSTEM

#endif //__Blade_WindowsFileDevice_h__