/********************************************************************
	created:	2016/07/31
	filename: 	BladeQtDevice.h
	author:		Crazii
	
	purpose:	[private] project shared header for export DLL interfaces to Blade framework
*********************************************************************/
#ifndef __Blade_BladeQtDevice_h__
#define __Blade_BladeQtDevice_h__
#include <BladePlatform.h>
#include <BladeTypes.h>
#include <BladeException.h>
#include <QtCore/QString>
#include <QtGui/qevent.h>

#if !defined(BLADE_STATIC)	//dynamic link lib

#	if BLADE_COMPILER == BLADE_COMPILER_MSVC
#		ifdef BLADE_QTDEVICEPLUGIN_EXPORTS
#			define BLADE_QTDEVICE_PLUGIN_API __declspec( dllexport )
#		else
#			define BLADE_QTDEVICE_PLUGIN_API __declspec( dllimport )
#		endif
#	elif BLADE_COMPILER_GNU_EXTENSION
#		define BLADE_QTDEVICE_PLUGIN_API __attribute__ ((visibility("default")))
#	endif

#else

#	define BLADE_QTDEVICE_PLUGIN_API	//static link lib

#endif


namespace Blade
{
	//need unicode build to bridge with QChar
	static_assert(sizeof(QChar) == sizeof(tchar), "need unicode build, define UNICODE or _UNICODE.");

	//////////////////////////////////////////////////////////////////////////
	static inline QString TString2QString(const TString& btstr)
	{
		return QString( (QChar*)btstr.c_str(), (int)btstr.size() );
	}

	//////////////////////////////////////////////////////////////////////////
	static inline TString QString2TString(const QString& qstr)
	{
		return TString( (tchar*)qstr.constData() );
	}
	
}//namespace Blade

#endif // __Blade_BladeQtDevice_h__