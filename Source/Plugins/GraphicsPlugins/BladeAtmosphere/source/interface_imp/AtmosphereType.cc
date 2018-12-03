/********************************************************************
	created:	2011/09/03
	filename: 	AtmosphereType.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "AtmosphereType.h"
#include <interface/IMaterialManager.h>
#include <BladeAtmosphere_blang.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	AtmosphereType::AtmosphereType()
		:RenderType(BTString(BLANG_ATMOSPHERE))
	{
	}

	//////////////////////////////////////////////////////////////////////////
	Material*		AtmosphereType::getMaterial() const
	{
		return IMaterialManager::getSingleton().getMaterial( BTString("sky") );
	}
}//namespace Blade
