/********************************************************************
	created:	2010/04/10
	filename: 	ResourceManager.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "ResourceManager.h"
#include "SubFileDevice.h"
#include <ExceptionEx.h>

namespace Blade
{
	static const TString SCHEME_OF_CURRENT_PATH = BTString("cwd");
	static const TString SCHEME_OF_APP_PATH = BTString("app");
	static const TString SCHEME_OF_MEDIA_PATH = BTString("media");
	static const TString SCHEME_OF_CACHE_PATH = BTString("memory");	//memory stream
	static const TString CACHE_ARCHIVE_NAME = BTString("cache.mem");
	static const TString ROOT_SCHEME = BTString("ROOT");

	ResourceManager::ResourceManager()
		:mLoadingQueue(BLADE_TEMPCONT_INIT)
		,mReadyQueue(BLADE_TEMPCONT_INIT)
		,mBackgroundTask(mLoadingQueue,mReadyQueue)
		,mTimer(ITimeDevice::create(ITimeDevice::TP_DEFAULT))
	{
	}

	ResourceManager::~ResourceManager()
	{
		for( SchemeMap::iterator i = mSchemeList.begin(); i != mSchemeList.end(); ++i )
			BLADE_DELETE i->second;

		mSchemeList.clear();
	}

	/************************************************************************/
	/* IResourceManager exposed                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool		ResourceManager::initialize(const TString& processPath, const TString& mediaPath, const TString& workingPath)
	{
		if( mSchemeList.size() > 0 || mProcessFolder != TString::EMPTY )
		{
			assert(false);
			return false;
		}
		mProcessFolder = TStringHelper::standardizePath(processPath);

		mResourceException = false;
		bool ret = this->registerScheme( SCHEME_OF_APP_PATH, mProcessFolder, false);
		assert(ret);
		ret = this->registerScheme( SCHEME_OF_CURRENT_PATH, workingPath, false) && ret;
		assert(ret);
		ret = this->registerScheme( SCHEME_OF_CACHE_PATH, CACHE_ARCHIVE_NAME, false) && ret;
		assert(ret);
		ret = this->registerScheme( SCHEME_OF_MEDIA_PATH, mediaPath, false) && ret;
		assert(ret);
		//register file system root. note: windows disk like C:, D: are registered on need.
#if BLADE_IS_UNIX_CLASS_SYSTEM
		ret = this->registerScheme(ROOT_SCHEME, BTString("/"), false) && ret;
#endif
		mResourceException = true;
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	void		ResourceManager::readConfig(const HSTREAM& configStream)
	{
		ConfigFile file;
		if( configStream == NULL || !(configStream->getAccesMode()&IStream::AM_READ) || !file.open(configStream) )
			BLADE_EXCEPT(EXC_FILE_READ,BTString("failed to read file from stream:") + ((configStream == NULL) ? TString::EMPTY : configStream->getName()) );
		this->loadConfig(file);
	}

	//////////////////////////////////////////////////////////////////////////
	bool		ResourceManager::enableException(bool bEnable)
	{
		bool ret = mResourceException;
		mResourceException = bEnable;
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				ResourceManager::registerFileExtension(const TString& resType,const TString& extension)
	{
		if( extension == TString::EMPTY || extension.find_first_of(TEXT(":/\\.")) != TString::npos)
			BLADE_EXCEPT(EXC_INVALID_PARAM,BTString("invalid file extension.") );

		TString& type = mExtensionTypeMap[extension];
		if( type != TString::EMPTY )
		{
			if( type == resType )
				return true;
			else
			{
				assert(false);
				return false;
			}
		}
		type = resType;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		ResourceManager::registerDeviceExtension(const TString& fileDeviceType, const TString& extension)
	{
		if( extension == TString::EMPTY || extension.find_first_of(TEXT(":/\\.")) != TString::npos)
			BLADE_EXCEPT(EXC_INVALID_PARAM, BTString("invalid file extension.") );

		if( !FileDeviceFactory::getSingleton().isClassRegistered(fileDeviceType) )
			BLADE_EXCEPT(EXC_INVALID_PARAM, BTString("invalid archive type.") );

		TString& type = mArchiveExtensionMap[extension];
		if( type != TString::EMPTY )
		{
			if( type == fileDeviceType )
				return true;
			else
			{
				assert(false);
				return false;
			}
		}
		type = fileDeviceType;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		ResourceManager::addSearchPath(const TString& resType, const TString& path, bool recursive/* = false*/)
	{
		if( path == TString::EMPTY )
			BLADE_EXCEPT(EXC_INVALID_PARAM, BTString("invalid file path.") );

		SearchPathList& list = mTypePathMap[resType];
		std::pair<SearchPathList::iterator,bool> ret = list.insert( std::make_pair(TStringHelper::standardizePath(path) + TEXT('/'), recursive) );

		//if path already exist and is not an recursive search path, set to recursive
		if( !ret.second && recursive )
		{
			ret.first->second = recursive;
			return true;
		}
		return ret.second;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				ResourceManager::loadResource(const TString& resType,const TString& path,
		IResource::IListener* listener/* = NULL*/,const TString& serialType/* = TString::EMPTY*/, const ParamList* params/* = NULL*/)
	{
		LOCATOR locator;
		//check if the resource is already loaded
		HRESOURCE hRes = this->getResource(path, locator, resType);

		if( hRes != NULL )
		{
			//been unloaded, so RefCount is 1, probably in mReleasingQueue, but it will increase right now
			//we don't need to do anything.
			//if( hRes.refcount() == 1 )
			if(listener != NULL )
			{
				listener->postLoad(hRes);
				listener->onReady();
				assert(hRes.refcount() > 1);
			}
			return true;
		}

		//check if the resource is being loaded now, or in the ready queue
		if( mListeners.check(resType, locator.mFullPath, listener) )
			return true;

		if( hRes == NULL )//load
		{
			IResource* resource = BLADE_FACTORY_CREATE(IResource, resType);
			IResourceManager::setResourceSource(resource, locator.mFullPath );
			hRes.bind( resource );
		}

		TString ResourceSerializerType = serialType;
		if( ResourceSerializerType == TString::EMPTY )
			ResourceSerializerType = resType;
		ISerializer* resLoader = BLADE_FACTORY_CREATE(ISerializer, ResourceSerializerType );

		return this->loadResourceImpl(hRes, resLoader, locator, listener, params, RLM_ASYN);
	}

	//////////////////////////////////////////////////////////////////////////
	bool				ResourceManager::loadResource(const TString& path,
		IResource::IListener* listener/* = NULL*/, const ParamList* params/* = NULL*/)
	{
		TString resType;
		this->getResourceTypeByPath(path,resType);

		return this->loadResource(resType,path,listener, TString::EMPTY, params);
	}

	//////////////////////////////////////////////////////////////////////////
	HRESOURCE			ResourceManager::loadResourceSync(const TString& resType, const TString& path, const TString& serialType, const ParamList* params/* = NULL*/)
	{
		//early out to prevent creating the stream
		LOCATOR locator;
		HRESOURCE hRes = this->getResource(path, locator, resType);
		if( hRes != NULL )
			return hRes;

		HSTREAM stream = locator.locate();
		return this->loadResourceStreamSyncImpl(stream, path, resType, serialType, params != NULL ? *params : ParamList() );
	}

	//////////////////////////////////////////////////////////////////////////
	HRESOURCE		ResourceManager::loadResourceSync(const TString& path,
		const TString& serialType/* = TString::EMPTY*/, const ParamList* params/* = NULL*/)
	{
		TString resType;
		this->getResourceTypeByPath(path, resType);

		return this->loadResourceSync(resType, path, serialType, params);
	}

	//////////////////////////////////////////////////////////////////////////
	HRESOURCE		ResourceManager::loadResourceSync(const HSTREAM& stream,
		const TString& resType/* = TString::EMPTY*/,const TString& serialType/* = TString::EMPTY*/, const ParamList* params/* = NULL*/)
	{
		return this->loadResourceStreamSyncImpl(stream, TString::EMPTY, resType, serialType, params != NULL ? *params : ParamList());
	}

	//////////////////////////////////////////////////////////////////////////
	HSTREAM		ResourceManager::loadStream(const TString& path, IStream::EAccessMode mode/* = IStream::AM_READ*/)
	{
		//use locator to find stream in alternative paths
		TString			resourceType;
		this->getResourceTypeByPath(path, resourceType);
		LOCATOR locator;
		this->getResourceLocator(path, locator, resourceType);
		(int&)mode &= ~IStream::AM_OVERWRITE;
		return locator.locate();
	}

	//////////////////////////////////////////////////////////////////////////
	HSTREAM		ResourceManager::openStream(const TString& path, bool createFolder, IStream::EAccessMode mode/* = IStream::AM_READWRITE*/)
	{
		return this->loadOrCreateStreamSync(path, createFolder, mode);
	}

	//////////////////////////////////////////////////////////////////////////
	bool		ResourceManager::reloadResourceSync(HRESOURCE& hRes,const TString& path, const ParamList* params/* = NULL*/)
	{
		if (!BLADE_TS_CHECK(TS_MAIN_SYNC))
		{
			//note: is not safe to reload a resource directly in any thread:
			//the background IO task may also reloading it at the same time. (this conflict is hard to resolve especially with cascaded loading - loading in halfway)
			//TODO: use ResourceInstance (containing IResource*) so that resource can be replaced directly,
			//in that way, reload/reloadSync can create another copy, without affecting resource being used,
			//later in main sync state's update (ResourceManager::update), the old resource can be replaced directly by the new copy.
			assert(false && "reload resource sync can only be called at TS_MAIN_SYNC state");
			return false;
		}

		if( hRes == NULL )
		{
			if(mResourceException)
				BLADE_EXCEPT(EXC_INVALID_OPERATION,BTString("invalid resource handle.") );
			else
				return false;
		}

		const HRESOURCE& existRes = this->getResource(hRes->getSource(), hRes->getType());
		if(existRes == NULL || existRes != hRes )
		{
			if( mResourceException )
				BLADE_EXCEPT(EXCE_STATE_ERROR, BTString("resource not managed."));
			else
				return false;
		}

		ISerializer* loader = BLADE_FACTORY_CREATE(ISerializer, hRes->getType());
		LOCATOR locator;
		this->getResourceLocator(path, locator, hRes->getType() );
		bool ret = this->loadResourceImpl(hRes, loader, locator, NULL, params, RLM_SYNC|RLM_RELOAD);

		if (ret && loader->isLoaded(existRes) )
		{
			loader->reprocessResource(hRes);
		}
		IResourceManager::setResourceFlag(existRes, RF_READY);

		BLADE_DELETE loader;
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				ResourceManager::reloadResource(HRESOURCE& hRes,const TString& path,
		IResource::IListener* listener/* = NULL*/, const ParamList* params/* = NULL*/)
	{
		if( hRes == NULL)
		{
			if( mResourceException)
				BLADE_EXCEPT(EXC_INVALID_OPERATION,BTString("invalid resource handle.") );
			else
				return false;
		}

		const HRESOURCE& existRes = this->getResource(hRes->getSource(), hRes->getType());
		if(existRes == NULL || existRes != hRes )
		{
			if( mResourceException )
				BLADE_EXCEPT(EXCE_STATE_ERROR, BTString("resource not managed."));
			else
				return false;
		}

		LOCATOR locator;
		this->getResourceLocator(path, locator, hRes->getType() );

		//maybe its unsafe to reload a resource being used. TODO: create a copy
		//even being reloaded later in IO thread, its not safe.

		//check if the resource is being reloaded now
		if( mListeners.check(hRes->getType(), locator.mFullPath, listener) )
			return true;

		ISerializer* loader = BLADE_FACTORY_CREATE(ISerializer, hRes->getType() );
		return this->loadResourceImpl(hRes, loader, locator, listener, params, RLM_ASYN|RLM_RELOAD);
	}

	//////////////////////////////////////////////////////////////////////////
	const HRESOURCE&	ResourceManager::createResource(const TString& resType,const TString& path, ParamList& params)
	{
		mLoadedResources.getLock().lock();
		ResourceGroup& group = mLoadedResources[ resType ];
		ScopedLock sl( group.getLock() );
		mLoadedResources.getLock().unlock();

		//convert path to real path
		LOCATOR locator;
		this->getResourceLocator(path, locator, resType);

		std::pair<ResourceGroup::iterator,bool> result = group.insert( std::make_pair(locator.mFullPath, HRESOURCE()) );
		//already exist?
		if( !result.second )
		{
			BLADE_EXCEPT(EXC_REDUPLICATE,
				BTString("resource with the type \"") + resType
				+ BTString("\" and path \"") + path 
				+ BTString("\" already exist.")
				);
			//return result.first->second;
		}

		//TODO: check loading queue and ready queue?
		IResource* resource = BLADE_FACTORY_CREATE(IResource,resType);
		IResourceManager::setResourceSource(resource, locator.mFullPath);

		ISerializer* resLoader = BLADE_FACTORY_CREATE(ISerializer,resType );

		//creating not involves disk IO,so directly create it.
		if( resLoader->createResource(resource, params) )
		{
			BLADE_DELETE resLoader;
			HRESOURCE& res = result.first->second;
			res.bind( resource );
			return res;
		}
		else
		{
			TStringStream ss;
			ss.format(TEXT("creating resource failed. type:\"%s\",path:\"%s\". "), resType.c_str(), path.c_str() );
			BLADE_LOG(Error, ss.getContent());
			BLADE_DELETE resLoader;
			group.erase(result.first);
			return HRESOURCE::EMPTY;
		}

	}

	//////////////////////////////////////////////////////////////////////////
	const HRESOURCE&	ResourceManager::createResource(const TString& path, ParamList& params)
	{
		TString resType;
		this->getResourceTypeByPath(path,resType);

		return this->createResource(resType,path,params);
	}

	//////////////////////////////////////////////////////////////////////////
	bool		ResourceManager::saveResourceSync(const HRESOURCE& hRes, const HSTREAM& stream, const TString& serialType/* = TString::EMPTY*/)
	{
		BLADE_TS_VERIFY( TS_MAIN_SYNC );

		if( hRes == NULL )
			return false;

		TString ResourceSerializerType = serialType;
		if( ResourceSerializerType == TString::EMPTY )
			ResourceSerializerType = hRes->getType();

		ISerializer* resLoader = BLADE_FACTORY_CREATE(ISerializer, ResourceSerializerType );

		bool ret = resLoader->saveResource(hRes,stream);
		BLADE_DELETE resLoader;

		if(!ret)
			BLADE_LOG(Error, TEXT("save resource failed - \"") + hRes->getSource() << TEXT("\"."));

		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				ResourceManager::unloadResource(HRESOURCE& hResource)
	{
		if( hResource == NULL )
			return false;

		if( BLADE_TS_CHECK(TS_MAIN_SYNC) )
		{
			if( !mSyncResource.empty() )
			{
				LOCATOR locator;
				this->getResourceLocator(hResource->getSource(), locator, hResource->getType());
				if( mSyncResource.removeResource(hResource))
					return true;
			}
		}
		else
		{
			ScopedLock lock(mSyncResource.getLock());
			if( !mSyncResource.empty() )
			{
				LOCATOR locator;
				this->getResourceLocator(hResource->getSource(), locator, hResource->getType());
				if( mSyncResource.removeResource(hResource))
					return true;
			}
		}

		const TString& type = hResource->getType();
		ResourceTypeGroup::iterator i = mLoadedResources.find(type);
		if( i == mLoadedResources.end() )
		{
			TString source = hResource->getSource();
			hResource.clear();
			BLADE_EXCEPT(EXC_NEXIST,BTString("cannot unload resource - \"") + source + BTString("\", resource not managed.") );
		}
		else 
		{
			ResourceGroup& group = i->second;
			ResourceGroup::iterator n = group.find( hResource->getSource() );

			if( n == group.end() )
			{
				TString source = hResource->getSource();
				hResource.clear();
				BLADE_EXCEPT(EXC_NEXIST,BTString("cannot unload resource - \"") + source + BTString("\", resource not managed.") );
			}

			if( n->second != hResource )
			{
				TString source = hResource->getSource();
				BLADE_EXCEPT(EXC_UNKNOWN,BTString("resource information corrupted - \"") + source + BTString("\".") );
			}
			assert( hResource.refcount() > 1 );
			hResource.clear();

			if( n->second.refcount() == 1 )
			{
				//delay delete:
				//if a resource is unloaded in a frame, and later at somewhere else right in the same frame it is loaded,
				//the former unloaded resource is returned directly instead of reloading it.
				//that is a right behavior to return a cached resource

				//remove resource directly, especially on framework termination
				if( BLADE_TS_CHECK(TS_MAIN_SYNC) )
				{
					group.erase(n);
					if(group.size() == 0)
						mLoadedResources.erase(i);
				}
				else
				{
					//delay unload resource in main sync state (mReleasingQueue)
					IResourceManager::setResourceFlag(n->second, RF_RELEASING);
					mReleasingQueue.pushSafe(std::make_pair(i, n));
				}
			}
			else
			{
				//check resource is being reloaded asynchronously. if Yes, remove it from async queue
				if (n->second.refcount() == 2 && IResourceManager::getResourceFlag(n->second)== RF_RELOADING)
				{
					IResourceManager::setResourceFlag(n->second, RF_RELEASING);
					mReleasingQueue.pushSafe(std::make_pair(i, n));
				}
			}
			return true;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void				ResourceManager::removeAllResources()
	{
		//cleanup/delete resource & loader pointer in queue
		//clean up un-finished works
		assert( !mBackgroundTask.isWorking() );
		LOADING_WORK* work = NULL;
		do
		{
			mReadyQueue.popSafe(work);
			BLADE_DELETE work;
			work = NULL;
		}while( work != NULL );

		do
		{
			mLoadingQueue.popSafe(work);
			BLADE_DELETE work;
			work = NULL;
		}while( work != NULL );

		assert( mLoadedResources.size() == 0 );
		//TODO: dump resources that are not unloaded

		mLoadedResources.clear();
		mReleasingQueue.clear();

		//note: mBackgroundTask is auto removed.
		ProcessTask::removeAllProcessTasks();
	}

	//////////////////////////////////////////////////////////////////////////
	bool				ResourceManager::registerScheme(const TString& name, const TString& path, bool createFolder)
	{
		BLADE_TS_VERIFY(TS_MAIN_SYNC);

		SchemeMap::iterator sliter = mSchemeList.insert(SchemeMap::value_type(name, (ResourceScheme*)NULL)).first;
		ResourceScheme*& emptySlot = sliter->second;
		if( emptySlot != NULL )
		{
			if( mResourceException )
				BLADE_EXCEPT(EXC_REDUPLICATE,BTString("a resource scheme named \"")+name+BTString("\" already exist.") );
			else
			{
				BLADE_DETAIL_LOG(Warning, BTString("a resource scheme named \"") << name << BTString("\" already exist."));
				return false;
			}
		}

		index_t colonPos = path.find(TEXT(':'));
		TString	cascadeSchemeName;
		ResourceScheme* existScheme = NULL;
		if( colonPos != 1 && colonPos != TString::npos )
		{
			cascadeSchemeName = path.substr_nocopy(0,colonPos);
			existScheme = this->getScheme( cascadeSchemeName );
			if( existScheme == NULL )
			{
				mSchemeList.erase(sliter);
				if( mResourceException )
					BLADE_EXCEPT(EXC_NEXIST,BTString("scheme name not found: \"")+cascadeSchemeName+TEXT("\".") );
				else
				{
					BLADE_DETAIL_LOG(Warning, BTString("scheme name not found \"") + cascadeSchemeName + TEXT("\"."));
					return false;
				}
			}
		}

		HFILEDEVICE hFolder;
		if( existScheme != NULL )
		{
			index_t pos = path.substr_nocopy(colonPos+1).find_first_not_of( BTString("/\\") );
			//cannot be all "//" with no sub path
			assert( pos != TString::npos );
			const TString subPath = path.substr_nocopy(colonPos+1+pos);

			if( createFolder)
				existScheme->getDeviceFolder()->createDirectory( subPath );
			hFolder.bind( BLADE_NEW SubFileDevice(existScheme->getDeviceFolder(), path.substr(colonPos+1+pos) ) );
		}
		else
		{
			if( createFolder )
			{
				TString realPath;
				this->getFullResourcePath(path, realPath);

				//absolute path
				if( TStringHelper::isAbsolutePath(realPath) )
				{
					TString rootPath;
					TString subPath;
					index_t rootSlash = path.find_first_of( TEXT("\\/") );
					if( rootSlash != TString::npos )
					{
						rootPath = path.substr_nocopy(0, rootSlash);
						subPath = path.substr_nocopy(rootSlash+1);

						HFILEDEVICE hParentFolder = this->loadFolder(rootPath);
						if( hParentFolder != NULL )
							hParentFolder->createDirectory(subPath, true);
					}
				}
				else
				//relative path
				{
					HFILEDEVICE hCurrentWorkingDir = this->loadFolder(BTString("cwd:"));
					hCurrentWorkingDir->createDirectory(realPath, true);
				}
			}

			hFolder = this->loadFolderImpl(path, TString::EMPTY, name + BTString(":") == path || path == BTString("/") );

			if( hFolder == NULL && !TStringHelper::isAbsolutePath(path) )
			{
				TString testpath = mProcessFolder + BTString("/") + path;
				testpath = TStringHelper::standardizePath(testpath);
				hFolder = this->loadFolder(testpath);
			}
		}

		if(hFolder != NULL)
		{
			emptySlot = BLADE_NEW ResourceScheme(name,hFolder);
			return true;
		}
		else
		{
			mSchemeList.erase(sliter);
			return false;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	 bool		ResourceManager::unRegisterScheme(const TString& name)
	{
		SchemeMap::iterator i = mSchemeList.find( name );
		if( i == mSchemeList.end() )
			return false;
		else
		{
			BLADE_DELETE i->second;
			mSchemeList.erase(i);
			return true;
		}
	}

	 //////////////////////////////////////////////////////////////////////////
	 TString	ResourceManager::schemePathToRealPath(const TString& schemePath)
	 {
		 TString RealPath;

		 TString schemeSubPath;
		 ResourceScheme* scheme = NULL;
		 index_t colonPos = schemePath.find( TEXT(':') );
		 if( colonPos != TString::npos && colonPos >= 2 )
		 {
			 scheme = this->getSchemeByPath(schemePath,schemeSubPath);
			 if( scheme != NULL )
				 scheme->getDeviceFolder()->getFullPath(schemeSubPath,RealPath);
			 else
				 RealPath = schemePath;
		 }
		 else 
			 RealPath = schemePath;

		 return TStringHelper::standardizePath(RealPath);
	 }

	//////////////////////////////////////////////////////////////////////////
	ITask*		ResourceManager::update()
	{
		const ulong updateTimeLimit = 1u; //ulong(33.333f / 5.0f);
		mTimer->reset();

		//put sync resources into loaded resources first
		//same resource of different instances may exist in sync resources & async ready queue
		//since sync resource is already in use, use it and drop the async resource in ready queue
		TempVector<IResource*> syncCache;
		syncCache.reserve(32);
		while (!mSyncResource.empty())
		{
			const HRESOURCE& hRes = mSyncResource.front();
			HRESOURCE& resSlot = mLoadedResources[hRes->getType()][hRes->getSource()];
			assert(resSlot == NULL);
			IResourceManager::setResourceFlag(hRes, RF_SYNC);
			resSlot = hRes;
			mSyncResource.pop();
			syncCache.push_back(hRes);
		}

		while( !mReadyQueue.empty() && mTimer->getMilliseconds() < updateTimeLimit )
		{
			mTimer->update();

			LOADING_WORK* work = NULL;
			if( mReadyQueue.try_pop(work) )
			{
				const HRESOURCE& resource = work->mResource;
				assert( resource != NULL );
				if (IResourceManager::getResourceFlag(resource) == RF_RELEASING)
				{
					assert(resource.refcount() == 1);
					BLADE_DELETE work;
					continue;
				}

				IResourceManager::setResourceFlag(resource, uint16(work->mResult ? RF_READY : RF_FAILED) );

				const TString& type = resource->getType();
				const TString& source = resource->getSource();
				HRESOURCE hResource;
				bool bSamePath = work->mLocator.mFullPath == resource->getSource();

				//loading failed
				if( !work->mResult )
				{
					if( work->mReloading )
						BLADE_LOG(Error, TEXT("reloading resource failed - \"") << resource->getSource() << TEXT("\".") );
					else
						BLADE_LOG(Error, TEXT("loading resource failed - \"") << resource->getSource() << TEXT("\".") );

					//cannot delete it here, because the type and source is used by reference,
					//which may come from the resource object
					//resource.clear();

					//clear old resource
					if( !bSamePath )
						mLoadedResources[type].erase( source );

					//update resource path
					IResourceManager::setResourceSource(resource, work->mLocator.mFullPath );

					//reloading fail: use original resource?
					if( work->mReloading )
					{
						ResourceGroup& group = mLoadedResources[type];
						HRESOURCE& hRes = group[ source ];
						assert( hRes == NULL || hRes == resource);
						hRes = hResource = resource;
					}
					//hResource = HRESOURCE::EMPTY;
				}
				//loading OK
				else
				{
					if (!work->mProcessed)
					{
						const ITask::Type& processTaskType = work->mResource->getProcessTaskType();
						if (processTaskType != ITask::NO_TYPE)
						{
							const HTASK& task = ProcessTask::getProcessTask(processTaskType, mReadyQueue);
							static_cast<ProcessTask*>(task)->addWork(work);
							continue;
						}

						if (work->mReloading)
						{
							work->mLoader->reprocessResource(resource);

							//clear old resource. note it's not used yet since reloading resource doesn't make a copy
							if (!bSamePath)
								mLoadedResources[type].erase(source);

							//update resource path
							IResourceManager::setResourceSource(resource, work->mLocator.mFullPath);
						}
						else
							work->mLoader->postProcessResource(resource);
						//set processed for safety. it'll soon get deleted anyway
						work->mProcessed = true;
					}
					else if (work->mReloading)
					{
						if (!bSamePath)
							mLoadedResources[type].erase(source);

						//update resource path
						IResourceManager::setResourceSource(resource, work->mLocator.mFullPath);
					}

					ResourceGroup& group = mLoadedResources[type];
					HRESOURCE& hRes = group[ source ];
					if (hRes != NULL)
					{
						//prefer sync resource to async resources
						if (IResourceManager::getResourceFlag(hRes) == RF_SYNC)
							hResource = hRes;
						else
						{
							assert(IResourceManager::getResourceFlag(resource) == RF_RELOADING && hRes == resource);
							IResourceManager::setResourceFlag(resource, RF_READY);
							hResource = resource;
						}
					}
					else
						hRes = hResource = resource;
				}

				ListenerTypeGroup::iterator i = mListeners.find(type);
				if( i != mListeners.end() )
				{
					ListenerBySource& listeners = i->second;
					ListenerBySource::iterator n = listeners.find(source);
					if( n != listeners.end() )
					{
						ResourceListenerSet& set = n->second;
						for( ResourceListenerSet::iterator iter = set.begin(); iter != set.end(); ++iter)
						{
							ListenerRef* lr = *iter;
							if(lr->getListener() != NULL )
							{
								lr->getListener()->postLoad(hResource);

								//result notify
								if( work->mResult )
									lr->getListener()->onReady();
								else
									lr->getListener()->onFailed();
							}
						}

						listeners.erase(n);
						if( listeners.size() == 0 )
							mListeners.erase(i);
					}
				}

				hResource.clear();

				//resource not kept in postLoad(), or unloaded in onReady(), so unload it
				if (resource.refcount() == 2)	//resource registery + work->mResource
					this->unloadResource(work->mResource);
				BLADE_DELETE work;
			}//try pop

		}//while readyQueue

		for (size_t i = 0; i < syncCache.size(); ++i)
			IResourceManager::setResourceFlag(syncCache[i], RF_READY);

		//unload resources
		while( mReleasingQueue.size() > 0 && mTimer->getMilliseconds() < updateTimeLimit )
		{
			//timer->update();
			ResourceInfoPair pair;
			if( mReleasingQueue.popSafe(pair) )
			{
				if( pair.second->second.refcount() == 1 )
				{
					pair.first->second.erase( pair.second );
					if( pair.first->second.size() == 0 )
						mLoadedResources.erase( pair.first );
				}
			}
		}

		if( !mBackgroundTask.isWorking() && mBackgroundTask.needWorking() )
			return &mBackgroundTask;
		else
			return NULL;
	}


	//////////////////////////////////////////////////////////////////////////
	bool				ResourceManager::isLoadingResources() const
	{
		return mBackgroundTask.isWorking() || mBackgroundTask.needWorking();
	}

	//////////////////////////////////////////////////////////////////////////
	const TString&		ResourceManager::getSchemePath(const TString& schemeName) const
	{
		ResourceScheme* scheme = this->getScheme(schemeName);
		if( scheme != NULL )
			return scheme->getDeviceFolder()->getName();
		else
			return TString::EMPTY;
	}

	/************************************************************************/
	/* custom method                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	HFILEDEVICE			ResourceManager::loadFolderImpl(const TString& pathname, const TString& type/* = TString::EMPTY*/, bool skipScheme/* = false*/)
	{
		TString RealPathName = this->schemePathToRealPath(pathname);
		BLADE_LOG(Information, TEXT("ResourceManager: loading folder \"") + pathname + TEXT("\"."));

		HFILEDEVICE hFolder;
		TString archiveType = type;
		TString schemeSubPath;
		ResourceScheme* scheme = (!skipScheme) ? this->getSchemeByPath(pathname, schemeSubPath) : NULL;

		if ((archiveType == TString::EMPTY))
		{
			if (scheme != NULL && scheme->getDeviceFolder()->getType() != IFileDevice::DEFAULT_FILE_TYPE)
				archiveType = scheme->getDeviceFolder()->getType();
			else if (!this->getArchiveType(RealPathName, archiveType) || archiveType == TString::EMPTY)
				archiveType = IFileDevice::DEFAULT_FILE_TYPE;
			BLADE_LOG(Information, TEXT("folder type not specified, guessing type as \"") + archiveType + TEXT("\"."));
		}

		if (scheme != NULL && scheme->getDeviceFolder()->getType() == archiveType)
		{
			if (schemeSubPath == TString::EMPTY)
				return scheme->getDeviceFolder();
			else
			{
				if (scheme->existDirectory(schemeSubPath))
					hFolder.bind(BLADE_NEW SubFileDevice(scheme->getDeviceFolder(), schemeSubPath));
				return hFolder;
			}
		}

		hFolder.bind(BLADE_FACTORY_CREATE(IFileDevice, archiveType));
		TString  path = RealPathName;

		//Drive letter :/
		if (path.size() == 2 && std::isalpha(path[0]) && path[1] == TEXT(':'))
			path += TEXT("/");

		//resolve the archive's sub path in the implementation of IFileDevice
		//i.e. "C:\media.zip\texture", the sub-path "texture" is to be resolved by the ZipArchive
		if (hFolder != NULL && hFolder->load(path))
		{
			return hFolder;
		}
		else
		{
			if (mResourceException)
				BLADE_EXCEPT(EXC_FILE_READ, BTString("Failed to load archive ") + path);
			else
			{
				BLADE_LOG(Warning, BTString("Failed to load archive ") + path);
				hFolder.clear();
				return hFolder;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	typedef TempVector<size_t> DotPosList;
	bool				ResourceManager::getArchiveType(const TString& path,TString& type, TString* name /*= NULL*/, TString* subPath/* = NULL*/) const
	{
		//note this(directory name contains DOT ):
		//1. C:/dir.ext/subdir  (a folder)
		//2. C:/pack.zip/subdir (a file)
		//3. C:/pack.ext/pack.zip (a folder with a file)

		//the multi dot is not supported : C:/dir1.ext/dir2.ext/subdir
		//so it should be split first
		TStringTokenizer paths;
		paths.tokenize(path, TEXT("\\/") );
		size_t dotCount = 0;
		index_t dotPos = TString::npos;

		DotPosList dotPosList;
		dotPosList.resize(paths.size());

		for( size_t i = 0; i < paths.size(); ++i)
		{
			if( paths[i] != BTString("..") && ( dotPos = paths[i].rfind('.') ) != TString::npos && dotPos != 0 && dotPos != paths[i].size()-1 )
				++dotCount;
			else
				dotPos = TString::npos;
			dotPosList[i] = dotPos;
		}

		if( dotCount != 0)
		{
			//there are maybe many dots. try every one, find the first match
			for(size_t i = 0; i < dotPosList.size(); ++i)
			{
				size_t pos = dotPosList[i];
				if( pos != INVALID_INDEX)
				{
					ExtensionMap::const_iterator iter = mArchiveExtensionMap.find( paths[i].substr_nocopy(pos+1)  );
					if( iter != mArchiveExtensionMap.end() )
					{
						type = iter->second;

						//get archive name
						if( name != NULL )
						{
							TStringConcat concat(NULL);
							for(size_t j = 0; j <= i; ++j)
							{
								concat += paths[j];
								if( j != i )
									concat += BTString("/");
							}
							*name = concat;
						}

						if( subPath != NULL )
						{
							TStringConcat concat(NULL);
							for(size_t j = i+1; j < dotPosList.size(); ++j)
							{
								concat += paths[j];
								if( j != dotPosList.size()-1 )
									concat += BTString("/");
							}
							*subPath = concat;
						}
						return true;
					}
					else
						type = TString::EMPTY;
				}
			}
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				ResourceManager::loadResourceImpl(const HRESOURCE& hResource, ISerializer* loader,
		const LOCATOR& locator, IResource::IListener* listener,
		const ParamList* params/* = NULL*/, int loadmethod/* = RLM_ASYN*/)
	{
		if( listener != NULL )
			listener->preLoad();

		if( loadmethod & RLM_ASYN )//asynchronous load/reload
		{
			LOADING_WORK* work = BLADE_NEW LOADING_WORK(HSERIALIZER(loader), hResource, locator, params);

			IResourceManager::setResourceFlag(hResource, uint16((loadmethod & RLM_RELOAD) ? RF_RELOADING : RF_UNDEFINED));
			mLoadingQueue.pushSafe(work);
			return true;
		}
		else if( loadmethod & RLM_RELOAD )//synchronous reload
		{
			HSTREAM stream = (!(hResource->getUserFlag()&IResource::UF_PHONY)) ? locator.locate() : HSTREAM::EMPTY;
			if( stream == NULL && !(hResource->getUserFlag()&IResource::UF_PHONY) )
				return false;

			ParamList list;
			return loader->reloadResourceSync(hResource, stream, params != NULL ? *params : list);
		}
		else//synchronous load
		{
			HSTREAM stream = (!(hResource->getUserFlag()&IResource::UF_PHONY)) ? locator.locate() : HSTREAM::EMPTY;
			if( stream == NULL && !(hResource->getUserFlag()&IResource::UF_PHONY))
				return false;

			ParamList list;
			return loader->loadResourceSync(hResource, stream, params != NULL ? *params : list);
		}

	}

	//////////////////////////////////////////////////////////////////////////
	HRESOURCE			ResourceManager::loadResourceStreamSyncImpl(const HSTREAM& stream, const TString& path,
		const TString& resType, const TString& serialType, const ParamList& params)
	{
		TString resourceType;
		TString resourcePath = path;
		if (path.empty())
		{
			if ((stream == NULL || !stream->isValid()))
				return HRESOURCE::EMPTY;
			else
				resourcePath = stream->getName();
		}

		if (resType == TString::EMPTY)
			this->getResourceTypeByPath(resourcePath, resourceType);
		else
			resourceType = resType;

		assert(resourceType != TString::EMPTY);
		if (resourceType.empty())
		{
			BLADE_LOG(Error, BTString("unable to load resource, type not found: ") << stream->getName());
			return HRESOURCE::EMPTY;
		}

		LOCATOR locator;
		HRESOURCE hRes = this->getResource(resourcePath, locator, resourceType);
		if (hRes != NULL)
			return hRes;

		IResource* resource = BLADE_FACTORY_CREATE(IResource, resourceType);
		hRes.bind(resource);
		if (stream == NULL && !(hRes->getUserFlag()&IResource::UF_PHONY))
			return HRESOURCE::EMPTY;
		
		if (locator.mFullPath.empty())
			locator.mFullPath = TStringHelper::standardizePath(resourcePath);
		IResourceManager::setResourceSource(resource, locator.mFullPath);

		TString loaderType = serialType;
		if (loaderType == TString::EMPTY)
			loaderType = resourceType;
		ISerializer* resLoader = BLADE_FACTORY_CREATE(ISerializer, loaderType);

		if (resLoader->loadResourceSync(resource, stream, params) && resLoader->isLoaded(resource))
			resLoader->postProcessResource(resource);
		else
		{
			BLADE_DELETE resLoader;
			return HRESOURCE::EMPTY;
		}
		BLADE_DELETE resLoader;

		//we need to return resource immediately so it's hard to operate the loading/ready queue
		if (!BLADE_TS_CHECK(TS_MAIN_SYNC))
		{
			ScopedLock sl(mSyncResource.getLock());
			mSyncResource.addResource(hRes);
		}
		else
			mLoadedResources[resourceType][locator.mFullPath] = hRes;
		return hRes;
	}

	//////////////////////////////////////////////////////////////////////////
	ResourceScheme*		ResourceManager::getScheme(const TString& name) const
	{
		SchemeMap::const_iterator i = mSchemeList.find( name );
		if( i == mSchemeList.end() )
			return NULL;
		else
			return i->second;
	}

	//////////////////////////////////////////////////////////////////////////
	void				ResourceManager::loadConfig(ConfigFile& config)
	{
		const CFGFILE_SECTION* section = config.getSection( BTString("Resource Scheme") );

		bool bException = mResourceException;
		mResourceException = false;
		
		if( section != NULL )
		{
			size_t preCount = mSchemeList.size();
			bool	AllTried = false;
			do
			{
				size_t	curCount = mSchemeList.size();
				for( size_t i = 0; i < section->getEntryCount(); ++i)
				{
					const CFGFILE_ENTRY& entry = section->getEntry(i);
					if( !this->registerScheme(entry.getName(), entry.getValue(), false) )
					{
						if( AllTried )
							mResourceException = bException;
					}
				}

				//no more available
				if( curCount == mSchemeList.size() )
					AllTried = true;
			}while( mSchemeList.size() - preCount != section->getEntryCount() );
		}

		mResourceException = bException;
	}

	//////////////////////////////////////////////////////////////////////////
	void				ResourceManager::removeResource(const TString& name)
	{
		mLoadedResources.erase( name );
	}

	//////////////////////////////////////////////////////////////////////////
	HSTREAM				ResourceManager::loadOrCreateStreamSync(const TString& path, bool createFolder, IStream::EAccessMode mode)
	{
		ResourceScheme* scheme;
		TString			schemeSubPath;
		scheme = this->getSchemeByPath(path, schemeSubPath);
		return scheme->loadOrCreateFile(schemeSubPath, createFolder, mode);
	}

	//////////////////////////////////////////////////////////////////////////
	ResourceScheme*		ResourceManager::getSchemeByPath(const TString& path, TString& schemeSubPath) const
	{
		ResourceScheme* scheme = NULL;
		index_t			colonPos = 0;

#if BLADE_IS_WINDOWS_CLASS_SYSTEM
		//local disk path i.e. "C:"
		bool diskPrefix = path.size() >= 2 && TCharTraits::is_alpha(path[0]) && path[1] == TEXT(':');
		//hack for cygwin
		bool cygwinHack = diskPrefix ? false : ( path.size() >= 12 && TStringHelper::isStartWith(path, BTString("/cygdrive/")) 
			&& TCharTraits::is_alpha(path[10]) && path[11] == TEXT('/') );
		if( diskPrefix || cygwinHack )
		{
			TString localdiskScheme;
			TString driverLetter;
			index_t subPathBegin;
			if( diskPrefix )
			{
				localdiskScheme = TStringHelper::getUpperCase( path.substr(0,1) );
				driverLetter = path.substr(0,2);
				subPathBegin = 3;
			}
			else//cygwin path
			{
				localdiskScheme = TStringHelper::getUpperCase( path.substr(10,1) );
				driverLetter = path.substr(10,1) + BTString(":");
				subPathBegin = 12;
			}

			scheme = this->getScheme( localdiskScheme );
			if( scheme == NULL )
			{
				bool raiseException = mResourceException;
				mResourceException = false;
				bool ret = const_cast<ResourceManager*>(this)->registerScheme(localdiskScheme, driverLetter, false);
				mResourceException = raiseException;
				if( !ret )
				{
					if( mResourceException )
						BLADE_EXCEPT(EXC_NEXIST,BTString("loading \"")+driverLetter+BTString("\" failed when loading \"")+path+BTString("\" ."));
					else
					{
						BLADE_DETAIL_LOG(Warning, BTString("loading \"") << driverLetter << BTString("\" failed when loading \"") << path << BTString("\" ."));
						return NULL;
					}
				}
				scheme = this->getScheme(localdiskScheme);
			}
			schemeSubPath = path.substr(subPathBegin);
		}
		else
#elif BLADE_IS_UNIX_CLASS_SYSTEM
		TString stdPath = TStringHelper::standardizePath(path);
		bool isRoot = stdPath.size() > 1 && stdPath[0] == TEXT('/');
		if( isRoot )
		{
			scheme = this->getScheme( ROOT_SCHEME );
			schemeSubPath = path.substr(1);
		}
#endif
		if( scheme == NULL )
		{
			//format scheme:path/file
			if( ( colonPos = path.find(TEXT(":")) ) != INVALID_INDEX )
			{
				TString schemeName = path.substr(0, colonPos);
#if BLADE_IS_WINDOWS_CLASS_SYSTEM
				if (colonPos == 1)
					TStringHelper::toUpperCase(schemeName);
#endif
				scheme = this->getScheme(schemeName);
				if( scheme == NULL )
				{
					if( mResourceException )
						BLADE_EXCEPT(EXC_NEXIST,BTString("scheme \"")+schemeName+BTString("\" not found when loading \"")+path+BTString("\"."));
					else
					{
						BLADE_DETAIL_LOG(Warning, BTString("scheme \"") << schemeName << BTString("\" not found when loading \"") << path << BTString("\"."));
						return NULL;
					}
				}

				//skip the leading '/' of sub path
				while( colonPos < path.size()-1 && (path[colonPos+1] == TEXT('/') || path[colonPos+1] == TEXT('\\')) )
					colonPos++;
				schemeSubPath = path.substr(colonPos+1);
			}
			//only path,using relative path
			else
			{
				scheme = this->getScheme( SCHEME_OF_CURRENT_PATH );
				if (scheme != NULL)
				{
					schemeSubPath = TStringHelper::getRelativePath(scheme->getDeviceFolder()->getName(), path);

					//relative path doesn't exist, try as archive file
					if (!scheme->existDirectory(schemeSubPath) && !scheme->existFile(schemeSubPath))
					{
						TString archiveType;
						TString subPath;
						TString archiveName;
						bool isArchive = this->getArchiveType(path, archiveType, &archiveName, &subPath) && archiveType != TString::EMPTY;
						if (isArchive)
						{
							for (SchemeMap::const_iterator i = mSchemeList.begin(); i != mSchemeList.end(); ++i)
							{
								if (i->second != NULL && i->second->getDeviceFolder()->getName() == archiveName)
								{
									schemeSubPath = subPath;
									return i->second;
								}
							}
						}
					}
				}
				else
					schemeSubPath = path;
			}

			//remove any \...\ or repeated \\ // in path
			schemeSubPath = TStringHelper::standardizePath(schemeSubPath);
		}
		return scheme;
	}

	//////////////////////////////////////////////////////////////////////////
	void				ResourceManager::getFullResourcePath(const TString& path, TString& outFullPath, const TString& resType/* = TString::EMPTY*/) const
	{
		LOCATOR locator;
		this->getResourceLocator(path, locator, resType);
		outFullPath = locator.mFullPath;
	}

	//////////////////////////////////////////////////////////////////////////
	void				ResourceManager::getResourceLocator(const TString& path, LOCATOR& outLocator, const TString& resType/* = TString::EMPTY*/) const
	{
		TString schemeSubPath;
		bool except = mResourceException;
		mResourceException = (resType == TString::EMPTY) && mResourceException;

		ResourceScheme* scheme = this->getSchemeByPath(path, schemeSubPath);

		//file not exist, find in alternative paths
		if( (scheme == NULL || !scheme->existFile(schemeSubPath) )
			&& resType != TString::EMPTY )
		{
			ResourceScheme* altScheme = NULL;
			TString altSubPath;
			TypePathMap::const_iterator iter = mTypePathMap.find( resType );
			if( iter != mTypePathMap.end() )
			{
				const SearchPathList& list = iter->second;
				for(SearchPathList::const_iterator i = list.begin(); i != list.end(); ++i)
				{
					bool recursive = i->second;
					const TString& root = i->first;
					altScheme = this->getSchemeByPath( root + path , altSubPath);
					if( altScheme == NULL )
						continue;

					if( altScheme->existFile(altSubPath) )
					{
						scheme = altScheme;
						break;
					}
					else if( recursive )
					{
						TStringParam subPathList;
						altScheme = this->getSchemeByPath( root, altSubPath);
						altScheme->getDeviceFolder()->findFile( subPathList, altSubPath+BTString("/")+path, IFileDevice::FF_FILE|IFileDevice::FF_RECURSIVE );
						if( subPathList.size() > 0 )
						{
							scheme = altScheme;
							altSubPath = subPathList[0];	//return first found
							break;
						}
					}
				}//for

				if( altScheme == scheme )
					schemeSubPath = altSubPath;
			}
		}
		mResourceException = except;

		assert( scheme != NULL );
		outLocator.mScheme = scheme;
		outLocator.mSchemeSubPath = schemeSubPath;
		scheme->getDeviceFolder()->getFullPath(schemeSubPath, outLocator.mFullPath);
	}

	//////////////////////////////////////////////////////////////////////////
	bool				ResourceManager::getResourceTypeByPath(const TString& path,TString& outResourceType) const
	{
		TString dir,file,base,ext;
		TStringHelper::getFilePathInfo(path,dir,file,base,ext);

		ExtensionMap::const_iterator i = mExtensionTypeMap.find(ext);

		//extension not registered
		if( i == mExtensionTypeMap.end() )
			return false;
		outResourceType = i->second;

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	HRESOURCE			ResourceManager::getResource(const LOCATOR& locator, const TString& resType) const
	{
		//don't need lock on main sync mode
		//besides loadResourceSync may be recursively called on cascade serializer
		//but Blade::Lock is not recurisve
		if (BLADE_TS_CHECK(TS_MAIN_SYNC))
		{
			const HRESOURCE& hRes = mSyncResource.findResource(locator.mFullPath, resType);
			if (hRes != NULL)
				return hRes;
		}
		else
		{
			ScopedLock syncLock(mSyncResource.getLock());
			const HRESOURCE& hRes = mSyncResource.findResource(locator.mFullPath, resType);
			if (hRes != NULL)
				return hRes;
		}

		ResourceTypeGroup::const_iterator n = mLoadedResources.find(resType);
		if (n == mLoadedResources.end())
			return HRESOURCE::EMPTY;

		const ResourceGroup& group = n->second;
		ResourceGroup::const_iterator i = group.find(locator.mFullPath);
		if (i == group.end())
			return HRESOURCE::EMPTY;
		else
			return i->second;
	}

	//////////////////////////////////////////////////////////////////////////
	HRESOURCE			ResourceManager::getResource(const TString& path, LOCATOR& outLocator, const TString& type/* = TString::EMPTY*/) const
	{
		TString resType;
		if (type == TString::EMPTY)
			this->getResourceTypeByPath(path, resType);
		else
			resType = type;

		this->getResourceLocator(path, outLocator, resType);
		return this->getResource(outLocator, resType);
	}
	
}//namespace Blade