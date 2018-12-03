/********************************************************************
	created:	2011/09/03
	filename: 	AtmosphereElement.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <BladeAtmosphere.h>
#include <interface/ISpace.h>
#include "interface_imp/Atmosphere.h"
#include "AtmosphereElement.h"


namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	AtmosphereElement::AtmosphereElement()
		:GraphicsElement( AtmosphereConsts::ATMOSPHERE_ELEMENT_TYPE )
		,mSkySphere(NULL)
		,mSkyBox(NULL)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	AtmosphereElement::~AtmosphereElement()
	{
		Atmosphere::getSingleton().shutdown();
		BLADE_DELETE mSkyBox;
		BLADE_DELETE mSkySphere;
		mContent = NULL;
	}

	/************************************************************************/
	/* IConfigurabe interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void		AtmosphereElement::postProcess(const ProgressNotifier& notifier)
	{
		Atmosphere::getSingleton().initialize(this);
		notifier.onNotify( 1.0f );
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void		AtmosphereElement::onInitialize()
	{

	}

}//namespace Blade
