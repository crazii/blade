/********************************************************************
	created:	2010/04/10
	filename: 	LoadingTask.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_LoadingTask_h__
#define __Blade_LoadingTask_h__
#include <TaskBase.h>
#include <ExceptionEx.h>
#include <utility/StringList.h>
#include <StaticHandle.h>
#include <utility/BladeContainer.h>
#include <interface/ILog.h>
#include <interface/public/time/ITimeDevice.h>
#include <interface/public/file/IFileDevice.h>
#include <interface/public/IResource.h>
#include <interface/public/ISerializer.h>
#include "ResourceScheme.h"

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	// ListenerRef safety ref for listener
	//////////////////////////////////////////////////////////////////////////
	class ListenerRef : public BiRef , public TempAllocatable
	{
	public:
		ListenerRef(IResource::IListener* listener )	:BiRef(listener)	{}
		~ListenerRef()	{}

		IResource::IListener*	getListener() const	{ return static_cast<IResource::IListener*>( this->getRef() );}
	};

	class ListenerRefHandle : public Handle<ListenerRef>
	{
	public:
		ListenerRefHandle()	{}
		ListenerRefHandle(IResource::IListener* listener)	{ this->bind( BLADE_NEW ListenerRef(listener) ); }

		bool	operator<(const ListenerRefHandle& rhs) const
		{
			return mPtr->getListener() < rhs.mPtr->getListener();
		}
	};

	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////
	typedef struct SResourceLocator
	{
	public:
		TString			mFullPath;		//full path string as ID
		ResourceScheme*	mScheme;
		TString			mSchemeSubPath;	//sub path in scheme to id the file

		inline HSTREAM	locate() const
		{
			return mScheme->loadFile(mSchemeSubPath, IStream::AM_READ);
		}
	}LOCATOR;

	//////////////////////////////////////////////////////////////////////////
	// LOADING_WORK
	//////////////////////////////////////////////////////////////////////////
	class ResourceListenerSet : public TempSet< ListenerRefHandle >
	{
	public:
		void	lock()
		{
			mLock.lock();
		}
		void	unlock()
		{
			mLock.unlock();
		}
	protected:
		Lock	mLock;
	};

	typedef struct SLoadingWork : public TempAllocatable
	{
		SLoadingWork()	{}
		SLoadingWork(const HSERIALIZER& hLoader,const HRESOURCE& res, const LOCATOR& locator,
				const ParamList* params = NULL, bool reload = false)
			:mLoader(hLoader),mResource(res),mLocator(locator), mReloading(reload), mResult(false)
		{
			mProcessed = false;

			if( params != NULL )
				mParams = *params;
		}

		HSERIALIZER				mLoader;
		HRESOURCE				mResource;
		LOCATOR					mLocator;
		ParamList				mParams;
		bool					mReloading;
		bool					mResult;
		bool					mProcessed;
	}LOADING_WORK;

	//////////////////////////////////////////////////////////////////////////
	// WorkQueue
	//////////////////////////////////////////////////////////////////////////
	typedef		TempConQueue<LOADING_WORK*>		WorkQueue;
	typedef		TempList<LOADING_WORK*>			WaitQueue;

	//////////////////////////////////////////////////////////////////////////
	// LoadingTask
	//////////////////////////////////////////////////////////////////////////
	class LoadingTask : public TaskBase
	{
	public:
		static const ITask::Type TYPE;
	public:
		LoadingTask(WorkQueue& loadingQueue, WorkQueue& readyQueue)
			:TaskBase(BTString("Resource Loading Task"),TP_BACKGROUND)
			,mLoading(0)
			,mLoadingQueue(loadingQueue)
			,mReadyQueue(readyQueue)
			,mTimer(ITimeDevice::create(ITimeDevice::TP_DEFAULT))
		{}

		~LoadingTask()	{}

		/************************************************************************/
		/* ITask specs                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual const Type&	getType() const
		{
			return TYPE;
		}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual inline void run()
		{
			for(WaitQueue::iterator i = mWaitQueue.begin(); i != mWaitQueue.end(); /*++i*/)
			{
				LOADING_WORK* work = *i;

				if( work->mLoader->isLoaded(work->mResource) )
				{
					i = mWaitQueue.erase(i);
					mReadyQueue.pushSafe(work);
				}
				else
					++i;
			}

			//perform loading with a time limit
			//if there's still jobs in the queue, do it in the next/run/update
			const ulong updateTimeLimit = 2ul;	//2ms
			mTimer->reset();

			while( mLoadingQueue.size() > 0 && mTimer->getMilliseconds() < updateTimeLimit )
			{
				mTimer->update();

				LOADING_WORK* work;
				if( mLoadingQueue.try_pop(work) )
				{
					assert( work->mResource != NULL );

					//DO NOT use resource's path info, because on reloading, the path info of resource is old.
					HSTREAM stream = (!(work->mResource->getUserFlag()&IResource::UF_PHONY)) ? work->mLocator.locate() : HSTREAM::EMPTY;

					if( stream == NULL && !(work->mResource->getUserFlag()&IResource::UF_PHONY))
					{
						BLADE_LOG(Error, BTString("open file failed: \"") << work->mResource->getSource() << BTString("\".") );
						work->mResult = false;
					}
					else
					{
						if( work->mReloading )
							work->mResult = work->mLoader->reloadResource(work->mResource, stream, work->mParams);
						else
							work->mResult = work->mLoader->loadResource(work->mResource, stream, work->mParams);
					}

					if( work->mLoader->isLoaded(work->mResource) )
						mReadyQueue.pushSafe(work);
					else
						mWaitQueue.push_back(work);//loader is doing a cascaded loading, process further
				}
			}
		}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void update()
		{
			
		}

		/*
		@describe
		@param
		@return
		*/
		virtual void			onAccepted()
		{
			Lock::set(&mLoading,1);
		}

		/*
		@describe
		@param
		@return
		*/
		virtual void			onRemoved()
		{
			Lock::set(&mLoading,0);
		}

		/************************************************************************/
		/* custom                                                                     */
		/************************************************************************/
		/** @brief  */
		bool		isWorking() const
		{
			return mLoading != 0;
		}

		/** @brief  */
		bool		needWorking() const
		{
			return !mLoadingQueue.empty() || !mWaitQueue.empty();
		}

	protected:
		size_t		mLoading;
		WorkQueue&	mLoadingQueue;
		WorkQueue&	mReadyQueue;
		WaitQueue	mWaitQueue;
		StaticHandle<ITimeDevice> mTimer;
	private:
		LoadingTask&	operator=(const LoadingTask&);
	};//class LoadingTask


	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////
	class ListenerBySource : public TempTStringMap<ResourceListenerSet>
	{
	public:
		void	lock()
		{
			mLock.lock();
		}
		void	unlock()
		{
			mLock.unlock();
		}
	protected:
		Lock	mLock;
	};

	class ListenerTypeGroup : public TempTStringMap<ListenerBySource>
	{
	public:

		/** @brief return true if the resource exists (is being loaded) */
		bool				check(const TString& type,const TString& source,IResource::IListener* newListener)
		{
			bool result = true;
			mLock.lock();
			std::pair<iterator,bool> insert_ret = this->insert( std::make_pair(type , ListenerBySource()) );
			mLock.unlock();

			//new added, so not exist
			if( insert_ret.second )
				result = false;

			ListenerBySource& listeners = insert_ret.first->second;
			listeners.lock();
			std::pair<ListenerBySource::iterator,bool> ret = listeners.insert( std::make_pair(source,ResourceListenerSet()) );
			listeners.unlock();

			//listener set is newly added, so not exist
			if( result && ret.second )
				result = false;

			if( newListener != NULL )
			{
				ResourceListenerSet& set = ret.first->second;
				
				set.lock();
				set.insert( ListenerRefHandle(newListener) );
				set.unlock();
			}		
			return result;
		}

	protected:
		Lock mLock;
	};


	// uses lowest priority ID to run after main task, so that some sub system set thread id in main task and check in this task is OK.
	// i.e. IGraphicsSystem::isGraphiscContext() won't fail.
	class ProcessTask : public TaskBase, public Allocatable, public NonAssignable
	{
	public:
		ProcessTask(const Type& type, WorkQueue& readyQueue)
			:TaskBase(BTString("Specific Task"), TP_LOW, 0)
			,mType(type)
			,mReadyQueue(readyQueue)
			,mTimer(ITimeDevice::create(ITimeDevice::TP_DEFAULT))
		{
			mLoading = false;
		}

		~ProcessTask() {}

		/************************************************************************/
		/* ITask specs                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		virtual const Type&	getType() const
		{
			return mType;
		}

		/*
		@describe
		@param
		@return
		*/
		virtual inline void run()
		{
			const ulong updateTimeLimit = 2ul;	//2ms
			mTimer->reset();

			while (mProcessingQueue.size() > 0 && mTimer->getMilliseconds() < updateTimeLimit)
			{
				mTimer->update();

				LOADING_WORK* work;
				if (mProcessingQueue.try_pop(work))
				{
					if (!work->mReloading)
						work->mLoader->postProcessResource(work->mResource);
					else
						work->mLoader->reprocessResource(work->mResource);

					work->mProcessed = true;
					mReadyQueue.pushSafe(work);
				}
			}
		}

		/**
		@describe 
		@param
		@return
		*/
		virtual inline void update()
		{
			
		}

		/*
		@describe
		@param
		@return
		*/
		virtual void			onAccepted()
		{
			Lock::set(&mLoading, 1);
		}

		/*
		@describe
		@param
		@return
		*/
		virtual void			onRemoved()
		{
			Lock::set(&mLoading, 0);
		}

		/************************************************************************/
		/*                                                                      */
		/************************************************************************/
		/** @brief  */
		void			addWork(LOADING_WORK* work)
		{
			assert(work != NULL && work->mResult && !work->mProcessed);
			mProcessingQueue.pushSafe(work);
		}

		/**
		@describe
		@param
		@return
		*/
		static const HTASK&		getProcessTask(const ITask::Type& type, WorkQueue& readyQueue);

		/**
		  @describe 
		  @param
		  @return
		*/
		static void				removeAllProcessTasks();

	protected:
		typedef	Map<ITask::Type, HTASK> ProcessTaskMap;
		/** @brief  */
		static ProcessTaskMap&	getTaskMap()
		{
			static ProcessTaskMap TYPED_PROCESS_TASKS;
			return TYPED_PROCESS_TASKS;
		}

		ITask::Type	mType;
		WorkQueue	mProcessingQueue;
		WorkQueue&	mReadyQueue;
		StaticHandle<ITimeDevice> mTimer;
		size_t		mLoading;
	};//class ProcessTask
	
}//namespace Blade


#endif //__Blade_LoadingTask_h__