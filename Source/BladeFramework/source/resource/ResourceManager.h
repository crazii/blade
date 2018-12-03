/********************************************************************
	created:	2010/04/10
	filename: 	ResourceManager.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_ResourceManager_h__
#define __Blade_ResourceManager_h__
#include <interface/IResourceManager.h>
#include <interface/public/file/IFileDevice.h>
#include <utility/BladeContainer.h>
#include "ResourceScheme.h"
#include "LoadingTask.h"


namespace Blade
{
	enum EResourceLoadMethod
	{
		RLM_ASYN	= 0x01,	//asynchronous load
		RLM_SYNC	= 0x02,	//synchronous load/reload
		RLM_RELOAD	= 0x10,	//reload
	};

	enum EResourceFlag
	{
		RF_UNDEFINED= 0x00000000,
		RF_READY	= 0x00000001,
		RF_FAILED   = 0x00000002,
		RF_SYNC		= 0x00000004,
		RF_RELOADING= 0x00000008,
		RF_RELEASING= 0x00000010,
	};

	class ConfigFile;

	///group resource by type
	///theoretically resources with same path contain different types
	class ResourceGroup : public TStringMap<HRESOURCE>
	{
	public:
		Lock&	getLock() const	{return mLock;}
	protected:
		mutable Lock	mLock;
	};
	class ResourceTypeGroup : public TStringMap<ResourceGroup>
	{
	public:
		Lock&	getLock() const	{return mLock;}
	protected:
		mutable Lock	mLock;
	};

	typedef std::pair<ResourceTypeGroup::iterator, ResourceGroup::iterator> ResourceInfoPair;
	typedef TempConQueue< ResourceInfoPair >	ReleasingResource;

	class SyncResourceSet
	{
	public:
		/** @brief  */
		inline bool addResource(HRESOURCE& res)
		{
			ResourceGroup& group = mSet[res->getType()];
			std::pair<ResourceGroup::iterator, bool> ret = group.insert(std::make_pair(res->getSource(), res));
			if (!ret.second)
				res = ret.first->second;
			return ret.second;
		}
		/** @brief  */
		inline bool removeResource(HRESOURCE& resource)
		{
			ResourceTypeGroup::iterator iter = mSet.find(resource->getType());
			if( iter == mSet.end() )
				return false;

			ResourceGroup& group = iter->second;
			ResourceGroup::iterator i = group.find(resource->getSource());
			if( i == group.end() )
				return false;

			resource.clear();
			if( i->second.refcount() == 1 )
			{
				group.erase(i);
				if(group.empty())
					mSet.erase(iter);
			}
			return true;
		}
		/** @brief  */
		inline const HRESOURCE&	findResource(const TString& path, const TString type) const
		{
			ResourceTypeGroup::const_iterator iter = mSet.find(type);
			if( iter == mSet.end() )
				return HRESOURCE::EMPTY;

			const ResourceGroup& group = iter->second;
			ResourceGroup::const_iterator i = group.find(path);
			return i != group.end() ? i->second : HRESOURCE::EMPTY;
		}
		/** @brief  */
		inline bool	empty() const
		{
			return mSet.empty();
		}
		/** @brief  */
		inline const HRESOURCE& front()
		{
			if( !mSet.empty() )
			{
				ResourceGroup& group = mSet.begin()->second;
				if(!group.empty())
					return group.begin()->second;
			}
			return HRESOURCE::EMPTY;
		}
		
		/** @brief  */
		inline void	pop()
		{
			if( !mSet.empty() )
			{
				ResourceGroup& group = mSet.begin()->second;
				if(!group.empty())
					group.erase( group.begin() );
				if( group.empty() )
					mSet.erase( mSet.begin() );
			}
		}

		/** @brief  */
		inline Lock& getLock() const
		{
			return mSet.getLock();
		}

	protected:
		ResourceTypeGroup		mSet;
	};

	class ResourceManager : public IResourceManager, public Singleton<ResourceManager>
	{
	public:
		using Singleton<ResourceManager>::getSingleton;
		using Singleton<ResourceManager>::getSingletonPtr;
	public:
		ResourceManager();
		~ResourceManager();

		/************************************************************************/
		/* IResourceManager exposed                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool		initialize(const TString& processPath, const TString& mediaPath, const TString& workingPath);

		/*
		@describe read config file on specific file stream
		@param 
		@return 
		*/
		virtual void		readConfig(const HSTREAM& configStream);

		/*
		@describe indicates that whether to throw an exception if resource/scheme are not found. \n
		default : true
		@param
		@return the old value of this setting
		*/
		virtual bool		enableException(bool bEnable);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual HFILEDEVICE	loadFolder(const TString& pathname, const TString& type = TString::EMPTY)
		{
			return this->loadFolderImpl(pathname, type, false);
		}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool		registerFileExtension(const TString& resType,const TString& extension);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool		registerDeviceExtension(const TString& fileDeviceType, const TString& extension);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool		addSearchPath(const TString& resType, const TString& path, bool recursive = false);

		/**
		@describe loads resources in another background thread
		@note this method can be called in any thread for different resources
		@param 
		@return 
		*/
		virtual bool		loadResource(const TString& resType,const TString& path,
			IResource::IListener* listener = NULL,const TString& serialType = TString::EMPTY, const ParamList* params = NULL);


		/**
		@describe loads resources in another background thread
		@note this method can be called in any thread for different resources
		@param 
		@return 
		@remark resource type implicit included in the path, as file extension, the extension should be registered
		*/
		virtual bool		loadResource(const TString& path,
			IResource::IListener* listener = NULL, const ParamList* params = NULL);

		/**
		@describe this method force the resource manager immediately load resources in current thread, and return the loaded resource.
		@note this function call be called on any thread, but some resources need to be post processed in main sync state, which may cause an exception.
		@param [in] resType indicate the resource class type, for factory creation
		@param [in] path the resource path
		@param [in] serialType the resource serializer type,default uses the same as resType
		@return
		*/
		virtual HRESOURCE	loadResourceSync(const TString& resType,const TString& path,const TString& serialType,
			const ParamList* params = NULL);

		/*
		@describe
		@param
		@return
		*/
		virtual HRESOURCE	loadResourceSync(const TString& path,
			const TString& serialType = TString::EMPTY, const ParamList* params = NULL);

		/*
		@describe
		@param
		@return
		*/
		virtual HRESOURCE	loadResourceSync(const HSTREAM& stream,
			const TString& resType = TString::EMPTY,const TString& serialType = TString::EMPTY, const ParamList* params = NULL);

		/*
		@describe load stream without directly using archive or scheme
		@note this is performed on current task (synchronous mode)
		@param
		@return
		*/
		virtual HSTREAM		loadStream(const TString& path,IStream::EAccessMode mode = IStream::AM_READ);

		/*
		@describe open/create a writable stream
		@param [in] createFolder create none exist folder contained in path
		@return 
		*/
		virtual HSTREAM		openStream(const TString& path, bool createFolder, IStream::EAccessMode mode = IStream::AM_READWRITE);

		/**
		@describe
		@note this method can be called in any thread for different resources
		the reloading operation performed immediately
		@param 
		@return 
		*/
		virtual bool		reloadResourceSync(HRESOURCE& hRes,const TString& path, const ParamList* params = NULL);

		/**
		@describe load resource re-using existing object/data,commonly to avoid data deallocation/reallocation,\n
		and to change existing resource no matter where it is used (for resource reload & auto-refresh).
		@note this method can be called in any thread for different resources
		the reloading operation in queued in background loading task
		@param
		@return
		*/
		virtual bool		reloadResource(HRESOURCE& hRes,const TString& path,
			IResource::IListener* listener = NULL, const ParamList* params = NULL);

		/*
		@describe
		@param
		@return
		*/
		virtual const HRESOURCE&	createResource(const TString& resType,const TString& path, ParamList& params);

		/*
		@describe
		@param
		@return
		*/
		virtual const HRESOURCE&	createResource(const TString& path, ParamList& params);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool				saveResourceSync(const HRESOURCE& hRes, const HSTREAM& stream, const TString& serialType = TString::EMPTY);

		/*
		@deprecated
		@describe 
		@param 
		@return 
		*/
		virtual HRESOURCE	getResource(const TString& path, const TString& type = TString::EMPTY) const
		{
			LOCATOR locator;
			return this->getResource(path, locator, type);
		}

		/*
		@describe
		@param
		@return
		*/
		virtual bool				unloadResource(HRESOURCE& hResource);

		/*
		@describe
		@param
		@return
		*/
		virtual void				removeAllResources();

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool				registerScheme(const TString& name,const TString& path, bool createFolder);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool				unRegisterScheme(const TString& name);

		/*
		@describe convert the scheme-path to real path,do nothing if it is already a real path.
		@param 
		@return 
		*/
		virtual TString				schemePathToRealPath(const TString& schemePath);

		/*
		@describe update the resource information, if there're pending resources to be loaded, \n
		the return value is not NULL
		@param 
		@return 
		*/
		virtual ITask*				update();

		/*
		@describe get whether the resource loading task is running
		@param 
		@return 
		*/
		virtual bool				isLoadingResources() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const TString&		getSchemePath(const TString& schemeName) const;

		/************************************************************************/
		/* custom method                                                                     */
		/************************************************************************/
		/**
		@describe 
		@param
		@return
		*/
		virtual HFILEDEVICE			loadFolderImpl(const TString& pathname, const TString& type = TString::EMPTY, bool skipScheme = false);

		/*
		@describe
		@param
		@return
		*/
		bool						getArchiveType(const TString& path, TString& type, TString* name = NULL, TString* subPath = NULL) const;

		/*
		@describe
		@param
		@return
		*/
		bool						loadResourceImpl(const HRESOURCE& hResource, ISerializer* loader,
			const LOCATOR& locator, IResource::IListener* listener,
			const ParamList* params = NULL, int loadmethod = RLM_ASYN);

		/*
		@describe
		@param
		@return
		*/
		HRESOURCE					loadResourceStreamSyncImpl(const HSTREAM& stream, const TString& path,
			const TString& resType, const TString& serialType, const ParamList& params);

		/*
		@describe 
		@param 
		@return 
		@note this function is internal used,so make it inline
		*/
		inline ResourceScheme*		getScheme(const TString& name) const;

		/*
		@describe 
		@param 
		@return 
		*/
		inline void					loadConfig(ConfigFile& config);

		/*
		@describe
		@param
		@return
		*/
		void						removeResource(const TString& name);

		/*
		@describe 
		@param 
		@return 
		*/
		HSTREAM						loadOrCreateStreamSync(const TString& path, bool createFolder, IStream::EAccessMode mode);

		/*
		@describe 
		@param 
		@return 
		*/
		ResourceScheme*				getSchemeByPath(const TString& path,TString& schemeSubPath) const;

		/*
		@describe 
		@param 
		@return 
		*/
		void						getFullResourcePath(const TString& path, TString& outFullPath, const TString& resType = TString::EMPTY) const;

		/*
		@describe 
		@param 
		@return 
		*/
		void						getResourceLocator(const TString& path, LOCATOR& outLocator, const TString& resType = TString::EMPTY) const;

		/*
		@describe 
		@param 
		@return 
		*/
		bool						getResourceTypeByPath(const TString& path,TString& outResourceType) const;

		/**
		@describe 
		@param
		@return
		*/
		HRESOURCE					getResource(const LOCATOR& locator, const TString& resType) const;

		/**
		@describe 
		@param
		@return
		*/
		HRESOURCE					getResource(const TString& path, LOCATOR& outLocator, const TString& type = TString::EMPTY) const;

	protected:

		typedef		TStringMap<ResourceScheme*>		SchemeMap;
		typedef		TStringMap<TString>				ExtensionMap;
		typedef		TStringMap<bool>				SearchPathList;
		typedef		TStringMap<SearchPathList>		TypePathMap;

		ExtensionMap		mExtensionTypeMap;
		ExtensionMap		mArchiveExtensionMap;
		TypePathMap			mTypePathMap;
		SchemeMap			mSchemeList;
		TString				mProcessFolder;			//dir of running process
		ResourceTypeGroup	mLoadedResources;		//read by any thread, write in main sync mode: doesn't need lock
		ListenerTypeGroup	mListeners;				//read & write by any thread: need lock

		WorkQueue			mProcessQueue;
		WorkQueue			mLoadingQueue;
		WorkQueue			mReadyQueue;
		ReleasingResource	mReleasingQueue;
		SyncResourceSet		mSyncResource;	//resource loaded in sync(immediate) mode

		LoadingTask			mBackgroundTask;
		StaticHandle<ITimeDevice> mTimer;
		mutable bool		mResourceException;
	private:
		ResourceManager&	operator=(const ResourceManager&);
	};//class ResourceManager
	
}//namespace Blade


#endif //__Blade_ResourceManager_h__