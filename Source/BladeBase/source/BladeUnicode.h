/********************************************************************
	created:	2009/04/09
	filename: 	BladeUnicode
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __blade_bldunicode_h__
#define __blade_bldunicode_h__

//here I used external file, which comes from the UNICODE official website
#include "3rdparty/ConvertUTF.h"
#include <BladeBase.h>


namespace Blade
{
	//to decrease complexity  I just use one format.:D
	//Byte Order Masks for encoding format:
	BLADE_BASE_API extern const char UTF8_BOM[3];

	extern BLADE_BASE_API const char UTF16LE_BOM[2];
	extern BLADE_BASE_API const char UTF16BE_BOM[2];
	extern BLADE_BASE_API const char UTF32LE_BOM[4];
	extern BLADE_BASE_API const char UTF32BE_BOM[4];

}//namespace Blade


#endif // __blade_bldunicode_h__
