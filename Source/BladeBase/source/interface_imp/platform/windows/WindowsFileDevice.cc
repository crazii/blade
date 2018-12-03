/********************************************************************
	created:	2014/06/17
	filename: 	WindowsFileDevice.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#if BLADE_IS_WINDOWS_CLASS_SYSTEM
#include "WindowsFileDevice.h"
#include <sys/stat.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	WindowsFileDevice::WindowsFileDevice()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	WindowsFileDevice::~WindowsFileDevice()
	{

	}

	/************************************************************************/
	/* IFileDevice interfaces                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void WindowsFileDevice::findFile(TStringParam& result,const TString& pattern, int findFlag/* = FF_DIR|FF_FILE*/)
	{
		if( (findFlag&(FF_DIR|FF_FILE)) == 0 )
			return;

		TString directory;
		TString	subPattern;
		TStringHelper::getFilePathInfo(pattern, directory, subPattern);
		//list all files/sub-folders in target folder
		TString base_dir = mPathName + BTString("/") + directory + BTString("/*");

#define IS_RELATIVE_DIR(dir) ( (dir)[0] == TEXT('.') && (dir)[1] == TEXT('.') && (dir)[2] == TEXT('\0') || (dir)[0] == TEXT('.') && (dir)[1] == TEXT('\0') )

		intptr_t lHandle, res;
		_tfinddata_t findData;

		lHandle = (intptr_t)::_tfindfirst(base_dir.c_str(), &findData);
		res = 0;
		while (lHandle != -1 && res != -1)
		{
			if( !IS_RELATIVE_DIR(findData.name) )
			{
				bool dirtype = (findData.attrib & _A_SUBDIR) != 0;
				TString fileName(findData.name, Char_Traits<tchar>::length(findData.name), TString::CONST_HINT);
				//note: fileName is const buffer but not persistant and we need to duplicate it by adding EMPTY
				TStringConcat fullPath = directory == TString::EMPTY ? TString::EMPTY + fileName : (directory + BTString("/") + fileName);

				if ( (!dirtype && (findFlag&FF_FILE) || dirtype && (findFlag&FF_DIR)) && TStringHelper::wildcardMatch(subPattern, fileName)  )
					result.push_back(fullPath);

				if( dirtype && (findFlag&FF_RECURSIVE) )
					this->findFile(result, fullPath + BTString("/") + subPattern, findFlag);
			}
			res = ::_tfindnext( lHandle, &findData );
		}

		if(lHandle != -1)
			::_findclose(lHandle);
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	
}//namespace Blade

#endif//BLADE_IS_WINDOWS_CLASS_SYSTEM