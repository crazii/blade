/********************************************************************
	created:	2011/08/29
	filename: 	ClientConfig.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_ClientConfig_h__
#define __Blade_ClientConfig_h__

#include <AppConfig.h>
#include <ICmdParser.h>

namespace Blade
{

	class ClientConfig : public AppConfig, public Singleton<ClientConfig>
	{
	public:
		ClientConfig():AppConfig(TString::EMPTY, TString::EMPTY, Version() ),mMode(NULL)
		{
		}
		ClientConfig(const TString& name, const TString &desc, Version ver)
			:AppConfig(name, desc, ver), mMode(NULL)
		{
		}

		/** @brief  */
		virtual const TString&	getAppName();

		/** @brief  */
		virtual void		setupAppCmdArgument();

		/** @brief  */
		const TString&		getMode() const;

	protected:
		const CmdOption*	mMode;
	};//ClientConfig
	

}//namespace Blade




#endif // __Blade_ClientConfig_h__