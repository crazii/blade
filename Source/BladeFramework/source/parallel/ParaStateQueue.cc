/********************************************************************
	created:	2012/11/24
	filename: 	ParaStateQueue.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <interface/IPlatformManager.h>
#include <parallel/ParaStateQueue.h>


#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(disable:4127)	//conditional expression is constant
#endif


namespace Blade
{

	/************************************************************************/
	/* ParaStateQueue                                                                     */
	/************************************************************************/
	namespace Impl
	{
		class ParaStateQueueImpl : public Allocatable
		{
		public:
			typedef TempConQueue<IParaState*> QueueType;
			typedef Vector< QueueType > QueueList;
		public:
			/** @brief  */
			ParaStateQueueImpl()
			{
				if( msConcurrentCount == 0 || msEmptyMask == 0)
				{
					msConcurrentCount = std::min<size_t>(32u, IPlatformManager::getSingleton().getCPUInfo().mProcessorCount);
					assert(msConcurrentCount != 0 );
					for(size_t i = 0; i < msConcurrentCount; ++i)
						msEmptyMask.raiseBitAtIndex(i);
				}
				mList.resize( msConcurrentCount );
			}

			/** @brief  */
			~ParaStateQueueImpl()
			{

			}

			/** @brief  */
			void		push(IParaState* state)
			{
				if(state == NULL )
					return;
				size_t i = 0;
				while(true)
				{
					if( mList[i].try_push(state) )
						break;
					else if( ++i >= msConcurrentCount )
						i = 0;
				}
			}

			/** @brief  */
			IParaState*	pop()
			{
				size_t i = 0;
				Mask emptyMask;

				IParaState* ret = NULL;
				while(true)
				{
					QueueType& queue = mList[i];
					if( queue.try_lock() )
					{
						if( !queue.empty() )
						{
							ret= queue.front();
							queue.pop();
						}
						else
							emptyMask.raiseBitAtIndex(i);
						queue.unlock();
					}

					if( ret != NULL || emptyMask == msEmptyMask )
						break;

					do
					{
						if( ++i >= msConcurrentCount )
							i = 0;
					}while( emptyMask.checkBitAtIndex(i) );//do

				}//while
				return ret;
			}

			/** @brief  */
			bool	remove(IParaState* state)
			{
				for(size_t i = 0; i < mList.size(); ++i)
				{
					QueueType& queue = mList[i];
					if( queue.eraseSafe(state) )
						return true;
				}
				return false;
			}

		protected:
			QueueList	mList;

			static size_t	msConcurrentCount;
			static Mask		msEmptyMask;
		};
		size_t	ParaStateQueueImpl::msConcurrentCount = 0;
		Mask	ParaStateQueueImpl::msEmptyMask;
	}//namespace Impl

	using namespace Impl;

	//////////////////////////////////////////////////////////////////////////
	ParaStateQueue::ParaStateQueue()
		:mQueue( BLADE_NEW ParaStateQueueImpl() )
	{

	}

	//////////////////////////////////////////////////////////////////////////
	ParaStateQueue::~ParaStateQueue()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void		ParaStateQueue::push(IParaState* state)
	{
		mQueue->push(state);
	}

	//////////////////////////////////////////////////////////////////////////
	IParaState*	ParaStateQueue::pop()
	{
		return mQueue->pop();
	}
	
	//////////////////////////////////////////////////////////////////////////
	bool		ParaStateQueue::remove(IParaState* state)
	{
		return state != NULL && mQueue->remove(state);
	}

	//////////////////////////////////////////////////////////////////////////
	void		ParaStateQueue::update()
	{
		IParaState* state;
		while( (state = this->pop() ) != NULL )
		{
			state->update();
		}
	}

	
}//namespace Blade