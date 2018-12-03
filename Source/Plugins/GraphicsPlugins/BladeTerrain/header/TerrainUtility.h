/********************************************************************
	created:	2012/12/04
	filename: 	TerrainUtility.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_TerrainUtility_h__
#define __Blade_TerrainUtility_h__
#include <utility/StringHelper.h>

namespace Blade
{
	namespace TerrainUtility
	{
		//////////////////////////////////////////////////////////////////////////
		inline TString		getNormalTextureName(const TString& diffuseTexName)
		{
			TString dir,file,base,ext;
			TStringHelper::getFilePathInfo(diffuseTexName, dir, file, base, ext);
			assert(base.size() > 0 && base[base.size()-1] == TEXT('D') );
			base[base.size()-1] = TEXT('N');
			return dir + BTString("/normal/") + base;
		}

		//////////////////////////////////////////////////////////////////////////
		inline bool			isNormalTexture(const TString& file)
		{
			TString base,ext;
			TStringHelper::getFileInfo(file,base,ext);
			return TStringHelper::isEndWith(base, BTString("N"), true);
		}

		//////////////////////////////////////////////////////////////////////////
		inline bool			isSpecularTexture(const TString& file)
		{
			TString base,ext;
			TStringHelper::getFileInfo(file,base,ext);
			return TStringHelper::isEndWith(base, BTString("S"), true);
		}
	}
	
}//namespace Blade

#endif//__Blade_TerrainUtility_h__