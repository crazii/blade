/********************************************************************
	created:	2011/08/14
	filename: 	D3DIncludeLoaderEx.h
	author:		Crazii
	purpose:	the only difference between this loader and the original is :
					this one support loading from an archive, such as a zip-file.
*********************************************************************/
#ifndef __Blade_D3DIncludeLoaderEx_h__
#define __Blade_D3DIncludeLoaderEx_h__
#include <utility/StringHelper.h>
#include <interface/IResourceManager.h>
#include <interface/ILog.h>
#include <graphics/windows/D3DIncludeLoader.h>
#include <graphics/windows/D3D9/ID3D9Helper.h>

namespace Blade
{

	class D3DIncludeLoaderEx : public D3DIncludeLoader
	{
	public:

		STDMETHOD(Open)(D3D_INCLUDE_TYPE /*IncludeType*/, LPCSTR pFileName, LPCVOID /*pParentData*/, LPCVOID *ppData, UINT *pByteLen) throw()
		{
			TString FileName = StringConverter::StringToTString( pFileName, Char_Traits<char>::length(pFileName) );
			HFILEDEVICE hFolder;
			HSTREAM hStream;

			index_t index = INVALID_INDEX;
			for(int i = (int)mPathList.size()-1; i >= 0; --i)
			{
				hFolder = IResourceManager::getSingleton().loadFolder( mPathList[(index_t)i] );
				if( hFolder != NULL && hFolder->isLoaded() )
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
			*pByteLen = (UINT)buffer->getCapacity();
			*ppData = buffer->getData();

			mHeaderBuffer[buffer->getData()] = buffer;
			return S_OK;
		}

	};//class D3DIncludeLoaderEx
	

}//namespace Blade



#endif // __Blade_D3DIncludeLoaderEx_h__