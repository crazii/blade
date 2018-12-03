/********************************************************************
	created:	2010/05/07
	filename: 	BladeGeometry.h
	author:		Crazii
	purpose:	
*********************************************************************/

#ifndef __Blade_BladeGeometry_h__
#define __Blade_BladeGeometry_h__
#include <BladePlatform.h>
#include <utility/String.h>
#include <interface/public/ISubsystem.h>
#include <ElementBase.h>
#include <interface/public/geometry/GeomTypes.h>
#include <interface/public/geometry/IGeometry.h>
#include <ExceptionEx.h>

#if !defined(BLADE_STATIC)	//dynamic link lib

#	if BLADE_COMPILER == BLADE_COMPILER_MSVC
#		ifdef BLADE_GEOMETRY_EXPORTS
#			define BLADE_GEOMETRY_API __declspec( dllexport )
#		else
#			define BLADE_GEOMETRY_API __declspec( dllimport )
#		endif
#	elif BLADE_COMPILER_GNU_EXTENSION
#		define BLADE_GEOMETRY_API __attribute__ ((visibility("default")))
#	endif

#else
#	define BLADE_GEOMETRY_API	//static link lib
#endif


namespace Blade
{
	class IScene;
	class ParaStateQueue;
	class GeometryUpdater;

	class GeometryElementBase : public ElementBase
	{
	public:
		GeometryElementBase(const TString& type, EParaPriority pp)
			:ElementBase(GeomConsts::GEOMETRY_SYSTEM_TYPE, type, pp) {}

		/** @brief  */
		virtual void initialize(IScene* scene, ParaStateQueue* queue, GeometryUpdater* updater) = 0;
	};
	typedef Factory<GeometryElementBase> GeometryFactory;
	
}//namespace Blade

#endif //__Blade_BladeGeometry_h__