/********************************************************************
	created:	2011/08/13
	filename: 	D3DIncludeLoader.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
#pragma warning(disable: 4917) //a GUID can only be associated with a class, interface or namespace
#include <BladeWin32API.h>
#include <graphics/windows/D3DIncludeLoader.h>
#include <graphics/windows/D3D9/ID3D9Helper.h>

namespace Blade
{

	TString	D3DIncludeLoader::msLoaderType = BTString("Default");

	HRESULT D3DIncludeLoader::Open(D3D_INCLUDE_TYPE /*IncludeType*/, LPCSTR pFileName,LPCVOID /*pParentData*/,LPCVOID *ppData,UINT *pByteLen)
	{
		TString FileName = StringConverter::StringToTString( pFileName,Char_Traits<char>::length(pFileName) );

		HFILEDEVICE hFolder(BLADE_FACTORY_CREATE(IFileDevice, IFileDevice::DEFAULT_FILE_TYPE));

		index_t index = INVALID_INDEX;
		HSTREAM hStream;
		for(int i = (int)mPathList.size()-1; i >= 0; --i)
		{
			if( hFolder->load( mPathList[(size_t)i] ) )
			{
				hStream = hFolder->openFile(FileName);
				if( hStream != NULL )
				{
					index = (index_t)i;
					break;
				}
			}
		}

		if( hStream == NULL )
		{
			ID3D9Helper::getSingleton().outputString(BTString("error : Cannot open include file: '") + FileName + BTString("': No such file or directory"));
			*pByteLen = 0;
			*ppData  = NULL;
			return S_FALSE;
		}

		while( mPathList.size() > index+1 )
			mPathList.pop_back();

		TString file,dir;
		TStringHelper::getFilePathInfo(FileName, dir, file);
		if( dir != TString::EMPTY )
		{
			TString searchPath = mPathList[index] + BTString("/") + dir;
			searchPath = TStringHelper::standardizePath(searchPath);
			bool exist = false;
			for(size_t i = 0; i < mPathList.size(); ++i)
			{
				if( mPathList[i] == searchPath )
				{
					exist = true;
					break;
				}
			}
			if( !exist )
				mPathList.push_back(searchPath);
		}

		IncludeBuffer* buffer = BLADE_NEW IncludeBuffer();
		buffer->loadFromStream(hStream);
		*pByteLen = (UINT)buffer->getSize();
		*ppData = buffer->getData();

		mHeaderBuffer[buffer->getData()] = buffer;
		return S_OK;
	}

}//namespace Blade



#endif//BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS