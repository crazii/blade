/********************************************************************
	created:	2010/08/15
	filename: 	SubResourceState.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_ResourceState_h__
#define __Blade_ResourceState_h__
#include <interface/public/IResource.h>

namespace Blade
{
	namespace Impl
	{
		class ListenerSet;
		class SubResourceStateList;
	}//namespace Impl

	
	class BLADE_FRAMEWORK_API ResourceState : public IResource::IListener
	{
	public:
		enum ELoadState
		{
			LS_UNLOADED,
			LS_LOADING,
			LS_LOADED,
			LS_FAILED,
			LS_NEED_DISPATCH,
		};
	public:
		ResourceState();
		virtual ~ResourceState();
		ResourceState(const ResourceState& src);
		ResourceState& operator=(const ResourceState& rhs);

		/************************************************************************/
		/* IResource::IListener interface                                                                     */
		/************************************************************************/
		/**
		@describe 
		@param 
		@return return false to break listener dispatching (dispatchReadyState)
		*/
		virtual bool	onReady();

		/**
		@describe 
		@param 
		@return return false to break listener dispatching (dispatchFailState)
		*/
		virtual bool	onFailed();

		/************************************************************************/
		/* custom method                                                                     */
		/************************************************************************/
		/**
		@describe
		@param
		@return
		*/
		bool			load();

		/**
		@describe 
		@param 
		@return 
		*/
		bool			loadSync();

		/**
		@describe reload resource if already loaded, otherwise perform a normal loading
		@param 
		@return 
		*/
		bool			reload();

		/**
		@describe 
		@param 
		@return 
		*/
		void			unload();

		/**
		@describe
		@param
		@return
		*/
		bool			addListener(IResource::IListener* pListener);

		/**
		@describe
		@param
		@return
		*/
		bool			removeListener(IResource::IListener* pListener);


		/** @brief  */
		inline bool		isEnabled() const			{return mEnable != 0;}

		/** @brief  */
		inline void		setEnable(bool enable)		{mEnable = enable ? 1u : 0;}

		/** @brief  */
		inline ELoadState	getLoadState() const	{return (ELoadState)mLoadState;}
		inline bool			isLoading() const		{return this->getLoadState() == LS_LOADING;}
		inline bool			isLoaded() const		{return this->getLoadState() == LS_LOADED;}


		/**
		@describe 
		@param 
		@return 
		*/
		virtual void				generateParams(ParamList& /*params*/) const	{}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const HRESOURCE&	getResource() const = 0;

		template <typename T>
		inline T*					getTypedResource() const	{return static_cast<T*>(this->getResource());}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool				refreshLoadState() = 0;

	protected:

		/**
		@describe notify all listeners that resource loading is finished
		@param
		@return
		@remark dispatch will call all listeners' postLoad notification
		*/
		void			dispatchReadyState();

		/**
		@describe 
		@param 
		@return 
		*/
		void			dispatchFailState();

		/**
		@describe 
		@param 
		@return 
		*/
		void			dispatchUnloadedState();

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void	unloadImpl() = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual	bool	loadImpl(const ParamList& params) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool	loadSyncImpl(const ParamList& params) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool	reloadImpl(const ParamList& params) = 0;


	private:
		LazyPimpl<Impl::ListenerSet>	mListeners;

	protected:
		uint8				mLoadState;
		uint8				mEnable;
	};

	
	class BLADE_FRAMEWORK_API SubResourceState : public ResourceState
	{
	public:
		SubResourceState();
		virtual ~SubResourceState() = 0;

		/************************************************************************/
		/* IResource::IListener interface                                                                     */
		/************************************************************************/
		/**
		@describe 
		@param 
		@return 
		*/
		virtual void	postLoad(const HRESOURCE& resource);

		/************************************************************************/
		/* custom method                                                                     */
		/************************************************************************/
		/**
		@describe 
		@param 
		@return 
		*/
		void			setResourcePath(const TString& path,const TString& stype = TString::EMPTY);

		/**
		@describe 
		@param 
		@return 
		*/
		void			setResourceInfo(const TString& type,const TString& path,const TString& serializer = TString::EMPTY);

		/**
		@describe 
		@param 
		@return 
		*/
		void			setManualResource(bool manual,const HRESOURCE& resource = HRESOURCE::EMPTY);

		/** @brief  */
		inline const TString&	getResourceType() const {return mResourceType;}

		/** @brief  */
		inline const TString&	getResourcePath() const	{return mResourcePath;}

		/** @brief  */
		inline const TString&	getSerializerType() const {return mSerializerType;}

		/** @brief  */
		inline bool		isManualResource() const	{return mManual;}

		/************************************************************************/
		/* ResourceState overrides                                                                     */
		/************************************************************************/
		/**
		@describe 
		@param 
		@return 
		*/
		virtual const HRESOURCE&	getResource() const	{return mResource;}

		/**
		@describe
		@param
		@return
		*/
		virtual bool	refreshLoadState();

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void	unloadImpl();

		/**
		@describe
		@param
		@return
		*/
		virtual	bool	loadImpl(const ParamList& params);

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool	loadSyncImpl(const ParamList& params);

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool	reloadImpl(const ParamList& params);

	protected:
		HRESOURCE				mResource;
		TString					mResourceType;
		TString					mResourcePath;
		TString					mSerializerType;
		bool					mManual;
	};//class SubResourceState


	class BLADE_FRAMEWORK_API ResourceStateGroup : public SubResourceState
	{
	public:
		ResourceStateGroup();
		ResourceStateGroup(const ResourceStateGroup& src);
		ResourceStateGroup& operator=(const ResourceStateGroup& rhs);
		virtual ~ResourceStateGroup() = 0;
	protected:
		/************************************************************************/
		/* IResource::Listener interface                                                                     */
		/************************************************************************/
		/**
		@describe 
		@param 
		@return 
		*/
		virtual void	onUnload();

		/**
		@describe called when one of sub state is ready
		@param
		@return return false to break listener dispatching (dispatchReadyState)
		*/
		virtual bool	onReady();

		/**
		@describe
		@param
		@return return false to break listener dispatching (dispatchFailState)
		*/
		virtual bool	onFailed() { return this->onReady(); }

	public:
		/************************************************************************/
		/* ResourceState overrides                                                                     */
		/************************************************************************/
		/**
		@describe
		@param
		@return
		*/
		virtual bool	loadImpl(const ParamList& params);

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool	loadSyncImpl(const ParamList& params);

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool	reloadImpl(const ParamList& params);

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void	unloadImpl();

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const HRESOURCE&	getResource() const	{return HRESOURCE::EMPTY;}

		/**
		@describe
		@param
		@return
		*/
		virtual bool	refreshLoadState();

	protected:

		/**
		@describe called on all sub states are ready
		@param
		@return
		*/
		virtual void	onAllReady()	{}

		/**
		@describe 
		@param
		@return
		*/
		virtual void	onAllUnloaded() {}


		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/**
		@describe
		@param
		@return
		*/
		bool			addSubState(ResourceState* state);

		/**
		@describe
		@param
		@return
		*/
		bool			removeSubState(ResourceState* state);

		/**
		@describe 
		@param 
		@return 
		*/
		size_t			getSubStateCount() const;

		/**
		@describe 
		@param 
		@return 
		*/
		ResourceState*	getSubState(index_t index) const;

		/**
		@describe
		@param
		@return
		*/
		void			clearSubStates();
		/** @brief  */
		void			clearAndDeleteSubStates();

		/**
		@describe 
		@param 
		@return 
		*/
		void			recalcLoadedCount();

	private:
		LazyPimpl<Impl::SubResourceStateList>	mSubStates;
		size_t				mLoadedCount;
	};//class ResourceStateGroup
	
}//namespace Blade


#endif //__Blade_ResourceState_h__