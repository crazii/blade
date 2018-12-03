/********************************************************************
	created:	2018/05/27
	filename: 	GrassEditable.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include "GrassEditable.h"
#include <BladeGrass_blang.h>

namespace Blade
{
	const TString GrassEditable::GRASS_EDITABLE_TYPE = BTString("Grass_Editable");

	//////////////////////////////////////////////////////////////////////////
	GrassEditable::GrassEditable()
		:EditableBase(GRASS_EDITABLE_TYPE, GRASS_EDITABLE_TYPE)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	GrassEditable::~GrassEditable()
	{

	}

	/************************************************************************/
	/* IEditable interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool			GrassEditable::initialize(EditorElement* element)
	{
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			GrassEditable::save()
	{
		return true;
	}
	
}//namespace Blade