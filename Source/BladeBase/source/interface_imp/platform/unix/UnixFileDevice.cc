/********************************************************************
	created:	2014/06/17
	filename: 	UnixFileDevice.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "UnixFileDevice.h"

#if BLADE_IS_UNIX_CLASS_SYSTEM
#include <sys/stat.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	UnixFileDevice::UnixFileDevice()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	UnixFileDevice::~UnixFileDevice()
	{

	}

	/************************************************************************/
	/* IFileDevice interfaces                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void UnixFileDevice::findFile(TStringParam& result,const TString& pattern, int findFlag/* = FF_DIR|FF_FILE*/)
	{
		if( (findFlag&(FF_DIR|FF_FILE)) == 0 )
			return;

		TString directory;
		TString	subPattern;
		TStringHelper::getFilePathInfo(pattern, directory, subPattern);
		//list all files/sub-folders in target folder
		
		TString base_dir = mPathName + BTString("/") + directory;
		base_dir = TStringHelper::standardizePath(base_dir);

#define IS_RELATIVE_DIR(dir) ( ((dir)[0] == '.' && (dir)[1] == '.' && (dir)[2] == '\0') || ((dir)[0] == '.' && (dir)[1] == '\0') )

		TempBuffer utf8Buffer;

		struct dirent **namelist = NULL;
		int n = ::scandir( StringConverter::TStringToUTF8String(utf8Buffer, base_dir), &namelist, NULL, ::alphasort );
		if( n == -1 )
			return;

		for(int i = 0; i < n; ++i)
		{
			if( !IS_RELATIVE_DIR(namelist[i]->d_name) )
			{
				bool dirtype = (namelist[i]->d_type & DT_DIR) != 0;
				size_t count = 0;
				const tchar* name = StringConverter::UTF8StringToTString(utf8Buffer, namelist[i]->d_name, 0, &count);
				TString fileName(name, count);
				TString fullPath = directory + BTString("/") + fileName;

				if ( ((!dirtype && (findFlag&FF_FILE)) || (dirtype && (findFlag&FF_DIR))) && TStringHelper::wildcardMatch(subPattern, fileName) )
					result.push_back(fullPath);

				if ( dirtype && (findFlag&FF_RECURSIVE) )
					this->findFile(result, fullPath + BTString("/") + subPattern, findFlag);
			}
			std::free(namelist[i]);
		}
		std::free(namelist);
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	
}//namespace Blade

#endif//BLADE_IS_UNIX_CLASS_SYSTEM