/********************************************************************
	created:	2015/03/24
	filename: 	AppEventHandle.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <interface/IPlatformManager.h>
#include <interface/IFramework.h>
#include <interface/IResourceManager.h>
#include <interface/public/graphics/IGraphicsService.h>
#include <App.h>
#include <ClientConfig.h>
#include <utility/BladeContainer.h>
#include <utility/StringHelper.h>
#include <utility/StringHelperEx.h>

#if BLADE_PLATFORM == BLADE_PLATFORM_ANDROID
#include <sys/stat.h>
#include <sys/types.h>
#endif

namespace Blade
{

	//////////////////////////////////////////////////////////////////////////
	void onRenderDeviceReady(const Event&)
	{
#if BLADE_PLATFORM == BLADE_PLATFORM_ANDROID
		//pre-compile all shaders
		static const TString LOCAL_SHADER_PATH = BTString("cwd:/shaders");

		const TString path = BTString("shader:");
		HFILEDEVICE hFolder = IResourceManager::getSingleton().loadFolder( path );
		assert( hFolder != NULL );
		if( hFolder == NULL )
			return;
		TStringParam fileList;
		hFolder->findFile(fileList, BTString("*.blshader"), IFileDevice::FF_FILE);


		//first check all shaders existence & timestamp
		bool needCompile = false;
		{
			TStringParam localFileList;
			HFILEDEVICE hLocalFolder = IResourceManager::getSingleton().loadFolder( LOCAL_SHADER_PATH );

			if( hLocalFolder == NULL )
				needCompile = true;
			else
				hLocalFolder->findFile(localFileList, BTString("*.blshader"), IFileDevice::FF_FILE);

			if( !needCompile && localFileList.size() < fileList.size() )
				needCompile = true;

			if( !needCompile )
			{
				typedef TempSet<TString> FileSet;
				FileSet files(localFileList.getBuffer(), localFileList.getBuffer()+localFileList.size());

				for(size_t i = 0; i < fileList.size(); ++i)
				{
					if( files.find( fileList[i] ) == files.end() )
					{
						needCompile = true;
						break;
					}
				}
			}
		}

		if( !needCompile )
		{
			TempBuffer buffer;
			const TString& obb = ClientConfig::getSingleton().getDataPath();
			const char* obbPath = StringConverter::TStringToUTF8String(buffer, obb);
			struct ::stat obbStat;
			int ret = ::stat(obbPath, &obbStat);
			assert( ret == 0 );
			if( ret != 0 )
				return;

			//for(size_t i = 0; i < fileList.size(); ++i)
			//just check on file time will do
			size_t i = 0;
			{
				TString fullpath = IPlatformManager::getSingleton().getCurrentWorkingPath() + BTString("/shaders/") + fileList[i];
				const char* filePath = StringConverter::TStringToUTF8String(buffer, fullpath);
				struct ::stat fileStat;
				if( ::stat(filePath, &fileStat) != 0 || std::difftime(fileStat.st_mtime, obbStat.st_mtime) < 0 )
				{
					needCompile = true;
					//break;
				}
			}
		}

		//TODO: compare timestamp for each file (BPK now don't record original timestamp, it is written while packaging)

		if( needCompile )
		{
			TString dir, name;
			for(size_t i = 0; i < fileList.size(); ++i)
			{
				const TString& file = fileList.at(i);
				HSTREAM stream = hFolder->openFile( file );
				HRESOURCE hRes = IResourceManager::getSingleton().loadResourceSync(stream, BTString("Shader"), BTString("HybridShader"));
				assert( hRes != NULL );
				if( hRes != NULL )
				{
					const TString& path = hRes->getSource();
					TStringHelper::getFilePathInfo(path, dir, name);
					bool ret = IResourceManager::getSingleton().saveResourceSync(hRes, LOCAL_SHADER_PATH + BTString("/") + name, true, BTString("BinaryShader") );
					assert( ret );
					ret = IResourceManager::getSingleton().unloadResource(hRes);
					assert(ret);
					BLADE_UNREFERENCED(ret);
				}
			}
		}

		//change scheme of "shader:"
		IResourceManager::getSingleton().unRegisterScheme(BTString("shader"));
		bool ret = IResourceManager::getSingleton().registerScheme(BTString("shader"), LOCAL_SHADER_PATH, false);
		assert(ret);
		BLADE_UNREFERENCED(ret);
#endif
	}
	
}//namespace Blade
