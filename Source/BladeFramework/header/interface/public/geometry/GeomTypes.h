/********************************************************************
	created:	2010/05/07
	filename: 	GeomTypes.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_GeomTypes_h__
#define __Blade_GeomTypes_h__
#include "GeometryInterface_blang.h"

namespace Blade
{
	///update timing for geometry system
	typedef enum EGeometryUpdateMethod
	{
		GUM_ON_TASK_RUN,
		GUM_ON_TASK_UPDATE,
		GUM_MANUAL,
	}GEOM_UPDATE_METHOD;

	///geometry transform space
	typedef enum EGeometrySpace
	{
		GS_OBJECT = 0,	//relative to self
		GS_LOCAL,		//relative to parent
		GS_WORLD,		//relative to world
	}GEOM_SPACE;

	namespace GeomConsts
	{
		static const TString GEOMETRY_SYSTEM_TYPE = BTString(BLANG_GEOMETRY_SYSTEM_TYPE);

		static const TString GEOMETRY_ELEMENT = BTString(BLANG_GEOMETRY_ELEMENT);

		static const TString GEOMETRY_ELEMENT_READONLY = BTString(BLANG_GEOMETRY_ELEMENT_READONLY);

	}//namespace GeomConsts
	
}//namespace Blade


#endif //__Blade_GeomTypes_h__