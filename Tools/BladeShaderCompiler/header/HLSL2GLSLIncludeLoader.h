/********************************************************************
	created:	2015/01/01
	filename: 	HLSL2GLSLIncludeLoader.h
	author:		Crazii
	
	purpose:	moved form ShaderCompilerApplicatoin.cc
*********************************************************************/
#ifndef __Blade_HLSL2GLSLIncludeLoader_h__
#define __Blade_HLSL2GLSLIncludeLoader_h__
#include <utility/String.h>
#include <utility/StringHelper.h>
#include <utility/StringList.h>
#include <utility/IOBuffer.h>
#include <ToolLog.h>
#include <interface/public/file/IFileDevice.h>

namespace Blade
{

	class HLSL2GLSLIncludeLoader
	{
	public:
		/* @brief  */
		inline void addPath(const TString& path)
		{
			mPathList.push_back(path);
		}

		/* @brief  */
		inline void clearPaths()
		{
			mPathList.clear();
		}


		/* @brief  */
		bool	open(const char* fileName, std::string& output)
		{
			TString FileName = StringConverter::StringToTString(fileName, Char_Traits<char>::length(fileName) );

			HFILEDEVICE hFolder;
			hFolder.bind( BLADE_FACTORY_CREATE(IFileDevice, IFileDevice::DEFAULT_FILE_TYPE) );

			index_t index = INVALID_INDEX;
			HSTREAM hStream;
			for(int i = (int)mPathList.size()-1; i >= 0; --i)
			{
				if( hFolder->load( mPathList[(index_t)i] ) )
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
				BLADE_LOGU(ToolLog::Error, BTString("Cannot open include file: '") + FileName + BTString("': No such file or directory") );
				return false;
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

			IOBuffer buffer;
			buffer.loadFromStream(hStream);
			output.resize( buffer.getSize() );
			::memcpy(&output[0], buffer.getData(), buffer.getSize() );
			return true;
		}
	protected:
		TStringParam	mPathList;
	};
	
}//namespace Blade


#endif // __Blade_HLSL2GLSLIncludeLoader_h__