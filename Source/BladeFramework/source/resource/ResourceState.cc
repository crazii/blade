/********************************************************************
	created:	2010/08/15
	filename: 	SubResourceState.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <ResourceState.h>
#include <ExceptionEx.h>
#include <interface/IPlatformManager.h>
#include <interface/IResourceManager.h>


namespace Blade
{
	namespace Impl
	{
		class ListenerSet : public Set<IResource::IListener*> ,public Allocatable {};
		class SubResourceStateList : public List<ResourceState*> , public Allocatable {};

		class TempListenerSet : public TempVector<IResource::IListener*>, public TempAllocatable {};
	}//namespace Impl
	using namespace Impl;

	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	ResourceState::ResourceState()
		:mLoadState(LS_UNLOADED)
		,mEnable(true)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	ResourceState::~ResourceState()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	ResourceState::ResourceState(const ResourceState& src)
	{
		if( src.mLoadState == LS_LOADED )
			mLoadState = LS_NEED_DISPATCH;
		else
			mLoadState = LS_UNLOADED;
		mEnable = src.mEnable;
	}

	//////////////////////////////////////////////////////////////////////////
	ResourceState&	ResourceState::operator=(const ResourceState& rhs)
	{
		if( this != &rhs )
		{
			this->unload();

			if( mListeners != NULL )
				mListeners->clear();

			if(rhs.mLoadState == LS_LOADED || rhs.mLoadState == LS_NEED_DISPATCH)
				mLoadState = LS_NEED_DISPATCH;
			else
				mLoadState = LS_UNLOADED;

			mEnable = rhs.mEnable;

			//TODO: copy listeners ?
		}
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	ResourceState::onReady()
	{
		mLoadState = LS_LOADED;
		this->dispatchReadyState();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			ResourceState::onFailed()
	{
		mLoadState = LS_FAILED;
		this->dispatchFailState();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	ResourceState::load()
	{
		if( !mEnable )
		{
			this->onReady();
			return true;
		}

		if( mLoadState == LS_UNLOADED )
		{
			mLoadState = LS_LOADING;
			ParamList params;
			this->generateParams(params);
			//note: if loadImpl directly succeeded, ReourceManager will issue a ready event, we don't need to to it
			//if( this->loadImpl(params) )
			//	this->onReady();
			this->loadImpl(params);
		}
		else if( mLoadState == LS_NEED_DISPATCH || mLoadState == LS_LOADED )
			this->onReady();

		return this->isLoaded();
	}
	//////////////////////////////////////////////////////////////////////////
	bool	ResourceState::loadSync()
	{
		BLADE_TS_VERIFY(TS_MAIN_SYNC);
		if( this->isLoading() )
		{
			assert(false);
			return false;
		}
		if( !mEnable )
		{
			this->onReady();
			return true;
		}

		if( mLoadState == LS_UNLOADED )
		{
			mLoadState = LS_LOADING;
			ParamList params;
			this->generateParams(params);
			this->loadSyncImpl(params);
		}
		else if( mLoadState == LS_NEED_DISPATCH || mLoadState == LS_LOADED )
			this->onReady();

		return this->isLoaded();
	}

	//////////////////////////////////////////////////////////////////////////
	bool	ResourceState::reload()
	{
		if( !mEnable )
		{
			this->onReady();
			return true;
		}

		//cannot reload on loading
		if( this->isLoading() )
		{
			//assert(false);
			return true;
		}
		else if( !this->isLoaded() )
			return this->load();

		//reload
		ParamList params;
		this->generateParams(params);
		bool ret = this->reloadImpl(params);
		if(ret)
			mLoadState = LS_LOADING;
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	void	ResourceState::unload()
	{
		ELoadState state = this->getLoadState();
		if( (state != LS_UNLOADED /*&& state != LS_LOADING*/) )
		{
			this->onUnload();
			this->unloadImpl();
		}
		mLoadState = LS_UNLOADED;

		//if (!this->isLoading())
		if ((state != LS_UNLOADED && state != LS_LOADING))
			this->dispatchUnloadedState();
	}

	//////////////////////////////////////////////////////////////////////////
	bool	ResourceState::addListener(IResource::IListener* pListener)
	{
		if( pListener == NULL )
			return false;
		return mListeners->insert(pListener).second;
	}


	//////////////////////////////////////////////////////////////////////////
	bool	ResourceState::removeListener(IResource::IListener* pListener)
	{
		if( pListener == NULL || mListeners == NULL )
			return false;
		return mListeners->erase(pListener) == 1;
	}

	//////////////////////////////////////////////////////////////////////////
	void	ResourceState::dispatchReadyState()
	{
		if( mListeners == NULL )
			return;

		for (ListenerSet::iterator i = mListeners->begin(); i != mListeners->end(); ++i)
		{
			bool bContinue = (*i)->onReady();
			//on ready may delete this
			if (!bContinue)
				break;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void	ResourceState::dispatchFailState()
	{
		if( mListeners == NULL )
			return;

		for (ListenerSet::iterator i = mListeners->begin(); i != mListeners->end(); ++i)
		{
			bool bContinue = (*i)->onFailed();
			//on ready may delete this
			if (!bContinue)
				break;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void	ResourceState::dispatchUnloadedState()
	{
		if( mListeners == NULL )
			return;

		for(Impl::ListenerSet::iterator i = mListeners->begin(); i != mListeners->end(); ++i )
		{
			(*i)->onUnload();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	SubResourceState::SubResourceState()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	SubResourceState::~SubResourceState()
	{
		this->unload();
	}

	/************************************************************************/
	/* IResource::IListener interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void			SubResourceState::postLoad(const HRESOURCE& resource)
	{
		mResource = resource;
	}

	/************************************************************************/
	/* custom method                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void			SubResourceState::setResourcePath(const TString& path, const TString& stype/* = TString::EMPTY*/)
	{
		bool changed = false;

		if( mResourcePath != path )
		{
			mResourcePath = path;
			changed = true;
		}
		
		if( stype != TString::EMPTY && mSerializerType != stype)
		{
			changed = true;
			mSerializerType = stype;
		}

		if( !changed )
			return;

		if( mManual && mResource != NULL )
		{
			assert(false && "try to change manual resource to non-manual?" );
			mResource.clear();
		}

		mManual = false;

		ELoadState state = this->getLoadState();
		if( state != LS_UNLOADED)
		{
			if( mResourcePath != TString::EMPTY )
			{
				this->unload();
				this->load();
			}
			else
			{
				if( mLoadState == LS_LOADED )
				{
					//hack :skip dispatching unloaded state
					mLoadState = LS_LOADING;

					this->unload();
				}
				else
				{
					this->unload();
					this->dispatchReadyState();
				}
				mLoadState = LS_LOADED;
				mManual = true;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void			SubResourceState::setResourceInfo(const TString& type,const TString& path,const TString& serializer/* = TString::EMPTY*/)
	{
		bool changed = false;

		if(mResourceType != type)
		{
			mResourceType = type;
			changed = true;
		}

		if( mResourcePath != path )
		{
			mResourcePath = path;
			changed = true;
		}

		if( serializer != TString::EMPTY && mSerializerType != serializer)
		{
			changed = true;
			mSerializerType = serializer;
		}

		if( !changed )
			return;

		mManual = false;

		ELoadState state = this->getLoadState();
		if( state != LS_UNLOADED)
		{
			this->unload();
			this->load();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool			SubResourceState::refreshLoadState()
	{
		if( mManual )
			return false;

		if( mLoadState == LS_UNLOADED )
		{
			assert(mResource == NULL);
			return false;
		}

		bool changed = false;
		if( mEnable )
		{
			if( mResource == NULL && mLoadState == LS_LOADED)
			{
				mLoadState = LS_UNLOADED;
				this->load();
				changed = true;
			}
		}
		else
		{
			if( mResource != NULL )
				assert( mLoadState == LS_LOADED );

			if( mLoadState != LS_UNLOADED)
			{
				this->unload();
				//disabled states always are loaded
				mLoadState = LS_LOADED;
				changed = true;
			}
		}
		return changed;
	}

	//////////////////////////////////////////////////////////////////////////
	void			SubResourceState::setManualResource(bool manual, const HRESOURCE& resource)
	{
		if( !mManual && this->getLoadState() != LS_UNLOADED )
		{
			bool result = true;
			if(mResource != NULL)
				IResourceManager::getSingleton().unloadResource(mResource);
			assert(result);
			BLADE_UNREFERENCED(result);

			if (this->isLoading())
			{
				this->postLoad(resource);
				this->onReady();
			}
		}

		if( mManual )
			mResource.clear();
		if( resource != NULL )
			mLoadState = LS_LOADED;

		mResource = resource;
		mManual = manual;
		if( resource != NULL )
			mResourcePath = resource->getSource();
	}

	//////////////////////////////////////////////////////////////////////////
	void	SubResourceState::unloadImpl()
	{
		//this is an atom state (not state group )
		//and this is being loaded
		if (this->isListening())
		{
			//stop receiving notify when resource is loaded
			this->cancel();
		}

		if( mResource == NULL )
			return;

		if( mManual )
		{
		}
		else if( mResource != NULL)
		{
			bool result = IResourceManager::getSingleton().unloadResource(mResource);
			assert(result);
			BLADE_UNREFERENCED(result);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool	SubResourceState::loadImpl(const ParamList& params)
	{
		if( mManual )
			return true;

		if( mResource != NULL )
		{
			assert( mLoadState != LS_LOADED );
			this->postLoad(mResource);
			this->onReady();
			return true;
		}
		else
		{
			assert( mResourcePath != TString::EMPTY );

			bool success = false;
			if( mResourceType == TString::EMPTY /*|| mSerializerType == TString::EMPTY*/ )
				success = IResourceManager::getSingleton().loadResource(mResourcePath, this, &params);
			else
				success = IResourceManager::getSingleton().loadResource(mResourceType, mResourcePath, this, mSerializerType, &params);

			if( !success )
				BLADE_EXCEPT(EXC_NEXIST,BTString("Loading resource error:") + mResourcePath );

			return false;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool SubResourceState::loadSyncImpl(const ParamList& params)
	{
		if( mManual )
			return true;

		if( mResource != NULL )
		{
			assert( mLoadState == LS_LOADED );
			this->postLoad(mResource);
			this->onReady();
			return true;
		}
		else
		{
			assert( mResourcePath != TString::EMPTY );

			if( mResourceType == TString::EMPTY /*|| mSerializerType == TString::EMPTY*/ )
				mResource = IResourceManager::getSingleton().loadResourceSync(mResourcePath, TString::EMPTY, &params);
			else
				mResource = IResourceManager::getSingleton().loadResourceSync(mResourceType, mResourcePath, mSerializerType, &params);

			if( mResource != NULL )
			{
				this->postLoad(mResource);
				this->onReady();
				return true;
			}
			else
				return false;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool	SubResourceState::reloadImpl(const ParamList& params)
	{
		if( mManual )
			return true;
		if( mResource == NULL )
			return this->loadImpl(params);

		assert(mResourcePath != TString::EMPTY );
		bool result = IResourceManager::getSingleton().reloadResource(mResource, mResourcePath, this , &params);
		if( !result )
			BLADE_EXCEPT(EXC_NEXIST,BTString("Reloading resource error:") + mResourcePath );
		return result;
	}

	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	ResourceStateGroup::ResourceStateGroup()
		:mLoadedCount(0)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	ResourceStateGroup::ResourceStateGroup(const ResourceStateGroup& /*src*/)
		:mLoadedCount(0)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	ResourceStateGroup& ResourceStateGroup::operator=(const ResourceStateGroup& rhs)
	{
		if( this != &rhs )
		{
			if( mSubStates != NULL )
				mSubStates->clear();
			mLoadedCount = 0;
		}
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	ResourceStateGroup::~ResourceStateGroup()
	{
		//unload all resource
		//this->unload();
		this->clearSubStates();
	}

	/************************************************************************/
	/* IResource::IListener interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void	ResourceStateGroup::onUnload()
	{
		this->recalcLoadedCount();
		assert( mLoadedCount <= mSubStates->size() );
		mLoadState = uint8(mLoadedCount == 0 ? LS_UNLOADED : LS_LOADING);
		this->dispatchUnloadedState();

		if (mLoadedCount == 0)
			this->onAllUnloaded();
	}

	//////////////////////////////////////////////////////////////////////////
	bool	ResourceStateGroup::onReady()
	{
		if( mLoadedCount < mSubStates->size() )
		{
			if (++mLoadedCount == mSubStates->size())
			{
				//group may have full clones from other group:
				//if this group is not yet perform loading, but other group has loaded
				//this group will also receive ready event, just ignore it otherwise there'll be redundant ready event
				if (mLoadState == LS_LOADING)
				{
					mLoadState = LS_LOADED;
					this->onAllReady();
					this->dispatchReadyState();
				}
			}
		}
		else
		{
			assert( mLoadedCount == mSubStates->size() );
			if (mLoadState == LS_LOADING || mLoadState == LS_NEED_DISPATCH)
			{
				mLoadState = LS_LOADED;
				this->onAllReady();
				this->dispatchReadyState();
			}
		}
		return true;
	}

	/************************************************************************/
	/* SubResourceState overrides                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool	ResourceStateGroup::loadImpl(const ParamList& /*params*/)
	{
		if(mSubStates != NULL && mSubStates->size() != 0 && mLoadedCount < mSubStates->size() )
		{
			bool ret = true;
			for( Impl::SubResourceStateList::iterator i = mSubStates->begin(); i != mSubStates->end(); ++i)
			{
				if( !(*i)->load() )
					ret = false;
				if (mSubStates == NULL)	//deleted on ready notification
					return true;
			}
			return ret;
		}
		else
		{
			this->onReady();
			return true;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool	ResourceStateGroup::loadSyncImpl(const ParamList& /*params*/)
	{
		if (mSubStates == NULL || mSubStates->size() == 0 || mSubStates->size() == mLoadedCount)
		{
			assert(mSubStates == NULL || mSubStates->size() == mLoadedCount);
			this->onReady();
			return true;
		}
		else
		{
			bool ret = true;
			for( Impl::SubResourceStateList::iterator i = mSubStates->begin(); i != mSubStates->end(); ++i)
			{
				if( !(*i)->loadSync() )
					ret = false;
			}
			return ret;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool	ResourceStateGroup::reloadImpl(const ParamList& /*params*/)
	{
		if(mSubStates != NULL && mSubStates->size() != 0 )
		{
			bool ret = true;
			for( Impl::SubResourceStateList::iterator i = mSubStates->begin(); i != mSubStates->end(); ++i)
			{
				if( !(*i)->reload() )
					ret = false;
			}
			return ret;
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void	ResourceStateGroup::unloadImpl()
	{
		mLoadedCount = 0;
		if( mSubStates == NULL )
			return;

		for( Impl::SubResourceStateList::iterator i = mSubStates->begin(); i != mSubStates->end(); ++i)
			(*i)->unload();
	}

	//////////////////////////////////////////////////////////////////////////
	bool	ResourceStateGroup::refreshLoadState()
	{
		if( mLoadState == LS_UNLOADED )
		{
			this->load();
			return true;
		}

		bool ret  = false;
		if( mSubStates != NULL )
		{
			for( Impl::SubResourceStateList::iterator i = mSubStates->begin(); i != mSubStates->end(); ++i)
			{
				if( (*i)->refreshLoadState() )
					ret = true;
			}
		}

		if( ret )
		{
			this->recalcLoadedCount();
			if( mLoadedCount == mSubStates->size() )
				mLoadState = LS_LOADED;
			else if( mLoadState == LS_LOADED )
				mLoadState = LS_LOADING;
		}
		return ret || mLoadState == LS_LOADING;
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool			ResourceStateGroup::addSubState(ResourceState* state)
	{
		if( state == NULL )
			return false;

		if( std::find(mSubStates->begin(), mSubStates->end(), state) != mSubStates->end() )
		{
			assert(false);
			return false;
		}

		if (state->isLoaded())
			++mLoadedCount;

		mSubStates->push_back(state);
		state->addListener(this);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			ResourceStateGroup::removeSubState(ResourceState* state)
	{
		if( state == NULL || mSubStates == NULL )
			return false;

		SubResourceStateList::iterator i = std::find( mSubStates->begin(), mSubStates->end(), state);
		if( i == mSubStates->end() )
		{
			assert(false);
			return false;
		}

		mSubStates->erase(i);
		state->removeListener(this);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t			ResourceStateGroup::getSubStateCount() const
	{
		if( mSubStates == NULL )
			return 0;
		else
			return mSubStates->size();
	}

	//////////////////////////////////////////////////////////////////////////
	ResourceState*	ResourceStateGroup::getSubState(index_t index) const
	{
		if( mSubStates == NULL || index >= mSubStates->size() )
			return NULL;

		SubResourceStateList::const_iterator i = mSubStates->begin();
		std::advance(i, index);
		return *i;
	}

	//////////////////////////////////////////////////////////////////////////
	void			ResourceStateGroup::clearSubStates()
	{
		for (Impl::SubResourceStateList::iterator i = mSubStates->begin(); i != mSubStates->end(); ++i)
		{
			IPlatformManager::prefetch<PM_WRITE>(*i);
			(*i)->removeListener(this);
		}
		mSubStates->clear();
	}

	//////////////////////////////////////////////////////////////////////////
	void			ResourceStateGroup::clearAndDeleteSubStates()
	{
		if( mSubStates == NULL )
			return;

		for( Impl::SubResourceStateList::iterator i = mSubStates->begin(); i != mSubStates->end(); ++i)
		{
			ResourceState* state = *i;
			state->removeListener(this);
			BLADE_DELETE state;
		}
		mSubStates->clear();
	}

	//////////////////////////////////////////////////////////////////////////
	void			ResourceStateGroup::recalcLoadedCount()
	{
		mLoadedCount = 0;
		if( mSubStates == NULL )
			return;

		for( Impl::SubResourceStateList::iterator i = mSubStates->begin(); i != mSubStates->end(); ++i)
		{
			if( (*i)->isLoaded() )
				++mLoadedCount;
		}

		assert( mLoadedCount <= mSubStates->size() );
		if( mSubStates->size() == mLoadedCount && mLoadedCount != 0)
			mLoadState = LS_NEED_DISPATCH;
	}

}//namespace Blade