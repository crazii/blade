/********************************************************************
	created:	2010/04/10
	filename: 	IResourceManager.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_IResourceManager_h__
#define __Blade_IResourceManager_h__
#include <BladeFramework.h>
#include <interface/InterfaceSingleton.h>
#include <interface/public/file/IFileDevice.h>
#include <interface/public/IResource.h>

namespace Blade
{
	class ITask;

	class IResourceManager : public InterfaceSingleton<IResourceManager>
	{
	public:
		virtual ~IResourceManager()		{}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool		initialize(const TString& processPath, const TString& mediaPath, const TString& workingPath) = 0;

		/**
		@describe read config file on specific file stream
		@note the stream should be opened in binary mode
		@param 
		@return 
		*/
		virtual void		readConfig(const HSTREAM& configStream) = 0;

		/**
		@describe indicates that whether to throw an exception if resource/scheme are not found. \n
		true: an exception will be throwed if resource/scheme not found
		false: none exception throwed,the loading result returned by function value
		default : true
		@param
		@return the old value of this setting
		*/
		virtual bool		enableException(bool bEnable) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual HFILEDEVICE	loadFolder(const TString& pathname,const TString& type = TString::EMPTY) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool		registerFileExtension(const TString& resType,const TString& extension) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool		registerDeviceExtension(const TString& fileDeviceType, const TString& extension) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool		addSearchPath(const TString& resType, const TString& path, bool recursive = false) = 0;

		/**
		@describe loads resources in another background thread
		@note this method can be called in any thread for different resources
		@param 
		@return 
		*/
		virtual bool		loadResource(const TString& resType,const TString& path,
			IResource::IListener* listener = NULL,const TString& serialType = TString::EMPTY, const ParamList* params = NULL) = 0;

		/**
		@describe loads resources in another background thread
		@note this method can be called in any thread for different resources
		@param 
		@return 
		@remark resource type implicit included in the path, as file extension, the extension should be registered
		*/
		virtual bool		loadResource(const TString& path,
			IResource::IListener* listener = NULL, const ParamList* params = NULL) = 0;

		/**
		@describe this method force the resource manager immediately load resources in current thread, and return the loaded resource.
		@note this function call be called on any thread, but some resources need to be post processed in main sync state, which may cause an exception.
		@param [in] resType indicate the resource class type, for factory creation
		@param [in] path the resource path
		@param [in] serialType the resource serializer type,default uses the same as resType
		@return
		*/
		virtual HRESOURCE	loadResourceSync(const TString& resType,const TString& path,const TString& serialType,
			const ParamList* params = NULL) = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual HRESOURCE	loadResourceSync(const TString& path,
			const TString& serialType = TString::EMPTY,const ParamList* params = NULL) = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual HRESOURCE	loadResourceSync(const HSTREAM& stream,
			const TString& resType = TString::EMPTY,const TString& serialType = TString::EMPTY, const ParamList* params = NULL) = 0;

		/**
		@describe load READ-ONLY stream without directly using archive or scheme
		@note this is performed on current thread (synchronous mode)
		@param
		@return
		*/
		virtual HSTREAM		loadStream(const TString& path,IStream::EAccessMode mode = IStream::AM_READ) = 0;

		/**
		@describe open/create a writable stream
		@param [in] createFolder create none exist folder contained in path
		@return 
		*/
		virtual HSTREAM		openStream(const TString& path, bool createFolder, IStream::EAccessMode mode = IStream::AM_OVERWRITE) = 0;

		/**
		@describe
		@note this method can be called in MAIN_SYNC state only
		the reloading operation performed immediately
		@param 
		@return 
		*/
		virtual bool		reloadResourceSync(HRESOURCE& hRes,const TString& path, const ParamList* params = NULL) = 0;

		/**
		@describe load resource re-using existing object/data,commonly to avoid data deallocation/reallocation,\n
		and to change existing resource no matter where it is used (for resource reload & auto-refresh).
		@note this method can be called in any thread for different resources
		the reloading operation in queued in background loading task
		@param
		@return
		*/
		virtual bool		reloadResource(HRESOURCE& hRes,const TString& path,
			IResource::IListener* listener = NULL, const ParamList* params = NULL) = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual const HRESOURCE&	createResource(const TString& resType,const TString& path, ParamList& params) = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual const HRESOURCE&	createResource(const TString& path, ParamList& params) = 0;

		/**
		@describe 
		@param [in] createFolder: create sub folder if specific path of hRes not exist
		@return 
		*/
		virtual bool		saveResourceSync(const HRESOURCE& hRes, const HSTREAM& stream, const TString& serialType = TString::EMPTY) = 0;

		inline bool		saveResourceSync(const HRESOURCE& hRes, const TString& path, bool createFolder, const TString& serialType = TString::EMPTY)
		{
			if (hRes == NULL)
				return false;

			HSTREAM stream = this->openStream(path, createFolder, IStream::AM_OVERWRITE );
			if( stream == NULL )
				return false;
			return this->saveResourceSync(hRes, stream, serialType);
		}

		inline bool		saveResourceSync(const HRESOURCE& hRes, bool createFolder, const TString& serialType = TString::EMPTY)
		{
			return this->saveResourceSync(hRes, hRes->getSource(), createFolder, serialType);
		}

		inline bool		saveResourceSync(const TString& resourcePath, bool createFolder, const TString& resType = TString::EMPTY, const TString& serialType = TString::EMPTY)
		{
			HRESOURCE hRes = this->getResource(resourcePath,resType);
			if( hRes == NULL )
				return false;
			return this->saveResourceSync(hRes,createFolder,serialType);
		}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual HRESOURCE	getResource(const TString& path,const TString& type = TString::EMPTY) const= 0;

		/**
		@describe this method is thread safe and it can be called in any thread
		@param
		@return
		*/
		virtual bool		unloadResource(HRESOURCE& hResource) = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual void		removeAllResources() = 0;

		/**
		@describe 
		@param [in] createFolder create sub directory tree if not exist
		@return 
		*/
		virtual bool		registerScheme(const TString& name,const TString& path, bool createFolder) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool		unRegisterScheme(const TString& name) = 0;

		/**
		@describe convert the scheme-path to real path,do nothing if it is already a real path.
		@param 
		@return 
		*/
		virtual TString		schemePathToRealPath(const TString& schemePath) = 0;

		/**
		@describe update the resource information, if there are pending resources to be loaded, \n
		the return value is not NULL
		@param 
		@return 
		*/
		virtual ITask*		update() = 0;

		/**
		@describe get whether the resource loading thread is running
		@param 
		@return 
		*/
		virtual bool		isLoadingResources() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const TString&		getSchemePath(const TString& schemeName) const = 0;

	protected:
		/** @brief  */
		inline static void setResourceSource(IResource* resource, const TString& source)
		{
			resource->setSource(source);
		}

		/** @brief  */
		inline static void	setResourceFlag(IResource* resource, uint16 flag)
		{
			resource->setFlag(flag);
		}

		/** @brief  */
		inline static uint16 getResourceFlag(IResource* resource)
		{
			return resource->getFlag();
		}
	};//class IResourceManager

	extern template class BLADE_FRAMEWORK_API Factory<IResourceManager>;
	
}//namespace Blade


#endif //__Blade_IResourceManager_h__