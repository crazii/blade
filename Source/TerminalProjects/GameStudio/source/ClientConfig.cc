/********************************************************************
	created:	2011/08/29
	filename: 	ClientConfig.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <ClientConfig.h>
#include <Application.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	const TString&	ClientConfig::getAppName()
	{
		if( mMode == NULL || mMode->getValue() == TString::EMPTY )
			return mAppName;
		else
			return this->getMode();
	}

	//////////////////////////////////////////////////////////////////////////
	void			ClientConfig::setupAppCmdArgument()
	{
		AppConfig::setupAppCmdArgument();

		TStringConcat lists(TString::EMPTY);

		for(size_t i = 0; i < AppFactory::getSingleton().getNumRegisteredClasses(); i++)
		{
			lists += AppFactory::getSingleton().getRegisteredClass(i);

			if( i != AppFactory::getSingleton().getNumRegisteredClasses() - 1)
				lists += BTString(",");
		}

		//TString modes = lists;

		//add custom parameters
		mMode = &ICmdParser::getSingleton().addOption( 
			BTString("@mode"),
			BTString("set application mode, the available modes are:") + lists + BTString("."),
			TEXT('m'),
			CAT_STRING,
			TString::EMPTY
			);
	}


	//////////////////////////////////////////////////////////////////////////
	const TString&	ClientConfig::getMode() const
	{
		if( mMode != NULL )
			return (const TString&)(mMode->getValue());
		else
			return TString::EMPTY;
	}

	

}//namespace Blade
