/********************************************************************
	created:	2009/04/09
	filename: 	bldunicode.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "BladeUnicode.h"

namespace Blade
{
	const char UTF8_BOM[3]		= {'\xEF'	,'\xBB'	,'\xBF'};
	const char UTF16LE_BOM[2]	= {'\xFF'	,'\xFE'};
	const char UTF16BE_BOM[2]	= {'\xFE'	,'\xFF'};
	const char UTF32LE_BOM[4]	= {'\xFF'	,'\xFE'	,'\0'	,'\0'};
	const char UTF32BE_BOM[4]	= {'\0'		,'\0'	,'\xFF'	,'\xFE'};
	
}//namespace Blade

