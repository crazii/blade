/********************************************************************
	created:	2011/08/29
	filename: 	IAppConfig.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_IAppConfig_h__
#define __Blade_IAppConfig_h__
#include <BladeApplication.h>
#include <utility/Version.h>

namespace Blade
{

	class BLADE_APPLICATION_API IAppConfig
	{
	public:
		virtual ~IAppConfig() {}


		/*
		@describe 
		@param 
		@return 
		*/
		virtual const TString&	getAppName() = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const TString&	getDescription() = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual Version			getVersion() = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			setupAppCmdArgument() = 0;

	};
	

}//namespace Blade



#endif // __Blade_IAppConfig_h__