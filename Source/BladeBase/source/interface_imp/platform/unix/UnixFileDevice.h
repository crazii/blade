/********************************************************************
	created:	2014/06/17
	filename: 	UnixFileDevice.h
	author:		Crazii
	
	purpose:	*nix FileDevice implementation, including Linux, IOS, Android, etc.
*********************************************************************/
#ifndef __Blade_UnixFileDevice_h__
#define __Blade_UnixFileDevice_h__
#include <BladePlatform.h>

#if BLADE_IS_UNIX_CLASS_SYSTEM
#include "../FileDevice.h"

namespace Blade
{
	class UnixFileDevice : public FileDevice , public Allocatable
	{
	public:
		UnixFileDevice();
		~UnixFileDevice();

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


	};//class UnixFileDevice

	typedef UnixFileDevice DefaultFileDevice;
	
}//namespace Blade

#endif//BLADE_IS_UNIX_CLASS_SYSTEM

#endif //__Blade_UnixFileDevice_h__