/********************************************************************
	created:	2010/08/22
	filename: 	IParaState.h
	author:		Crazii
	purpose:	parallel state data interface
*********************************************************************/
#ifndef __Blade_IParaState_h__
#define __Blade_IParaState_h__
#include <Handle.h>
#include <BladeFramework.h>
#include <parallel/ParallelPriority.h>
#include <utility/Mask.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	class IElement;
	class ParaStateQueue;
	class ParaStateGroup;

	//def the data as fix sized block for memory optimization
	typedef struct SParallelDataBlock : public Allocatable
	{
	public:
		ParaStateGroup*		mGroup;
		IElement*			mOwner;
		void*				mData;
		const void*			mInput;
		ParaStateQueue*		mQueue;
		Lock				mSyncLock;
		Mask				mFlag;
		int16				mInputPriority;
		int16				mPriority;

		enum EStateFlag
		{
			SF_MANAGED	= 0x01,		//auto-created internal data
			SF_ENQUEUED = 0x02,
			SF_CHANGED	= 0x04,		//changed directly
		};

		/** @brief  */
		inline SParallelDataBlock(ParaStateQueue* queue, IElement* owner, void* data)
		{
			mInput = NULL;
			mOwner = owner;
			mQueue = queue;
			mGroup = NULL;
			mPriority = (int16)PP_MIDDLE;
			mInputPriority = (int16)PP_LOWEST;
			mData = data;
		}

		/** @brief  */
		inline bool	isChanged() const
		{
			return mFlag.checkBits(SF_CHANGED );
		}

		/** @brief  */
		inline void	setChanged(bool changed)
		{
			if(changed)
				mFlag.raiseBits( SF_CHANGED );
			else
				mFlag.clearBits( SF_CHANGED );
		}

		/** @brief  */
		inline bool	isEnqueued() const
		{
			return mFlag.checkBits( SF_ENQUEUED );
		}

		/** @brief  */
		inline void	setEnqueued(bool queued)
		{
			if( queued )
				mFlag.raiseBits( SF_ENQUEUED );
			else
				mFlag.clearBits( SF_ENQUEUED );
		}

		/** @brief thread safe en-queue */
		inline bool	safeEnqueue()
		{
			return mFlag.safeRaiseBits(SF_ENQUEUED);
		}

		/** @brief  */
		inline void	resetFlags()
		{
			mFlag &= SF_MANAGED;
		}

	}PARA_DATA_BLOCK;

	class IParaState
	{
	public:
		inline IParaState()
		{
			mParaData = BLADE_NEW PARA_DATA_BLOCK(NULL, NULL, NULL);
		}

		virtual ~IParaState()
		{
			BLADE_DELETE mParaData;
			mParaData = NULL;
		}

		/** @brief get the state's priority, lower number means higher priority */
		inline int16 getPriority() const
		{
			return this->getInternalData()->mPriority;
		}
		
		/** @brief get the state's input linkage is priority */
		inline int16 getInputPriority() const
		{
			return this->getInternalData()->mInputPriority;
		}

		/** @brief  */
		inline  IElement* getOwner() const
		{
			return this->getInternalData()->mOwner;
		}

		/** @brief  */
		inline ParaStateQueue*	getQueue() const
		{
			return this->getInternalData()->mQueue;
		}

		/** @brief  */
		inline ParaStateGroup* getGroup() const
		{
			return this->getInternalData()->mGroup;
		}

		/** @brief  */
		inline void setPriority(int16 priority)
		{
			//we cannot change priority during run/update
			BLADE_TS_VERIFY(TS_MAIN_SYNC);
			this->getInternalData()->mPriority = priority;
		}

		/** @brief  */
		inline void setOwner(IElement* owner)
		{
			this->getInternalData()->mOwner = owner;
		}

		/** @brief  */
		inline void setQueue(ParaStateQueue* queue)
		{
			this->getInternalData()->mQueue = queue;
		}

		/** @brief  */
		inline void	setGroup(ParaStateGroup* group)
		{
			this->getInternalData()->mGroup = group;
		}

		/** @brief get state */
		virtual const void*		getStateData() const { return this->getInternalData()->mData;}

#if BLADE_DEBUG
		/** @brief debug id for type match check */
		virtual const std::type_info&getIdentifier() const = 0;
#endif

		/** @brief  */
		virtual void			onNotified(const void* sourceStateData, int16 sourcePriority) = 0;

		/** @brief  final update */
		virtual void			update() = 0;

		/** @brief update directly */
		virtual void			update(const IParaState* source) = 0;

	protected:

		/** @brief  */
		inline PARA_DATA_BLOCK*	getInternalData()				{return mParaData;}

		/** @brief  */
		inline const PARA_DATA_BLOCK*	getInternalData() const	{return mParaData;}

		PARA_DATA_BLOCK*	mParaData;
	};//class IParaState
	
}//namespace Blade


#endif //__Blade_IParaState_h__