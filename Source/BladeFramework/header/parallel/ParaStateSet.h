/********************************************************************
	created:	2011/04/09
	filename: 	ParaStateSet.h
	author:		Crazii
	purpose:	parallel state set for one element
*********************************************************************/
#ifndef __Blade_ParaStateSet_h__
#define __Blade_ParaStateSet_h__
#include <BladeFramework.h>
#include <utility/String.h>
#include <parallel/ParallelPriority.h>
#include <utility/TList.h>

namespace Blade
{
	class IElement;
	class IParaState;
	class ParaStateQueue;

	class ParaStateSet : public NonCopyable, public NonAssignable
	{
	public:
		struct  SOpAdapter : public NonAssignable
		{
		protected:
			IParaState*&	mPtrRef;
			IElement*		mOwner;
			ParaStateQueue* mQueue;
			int16			mPriority;
		public:
			inline SOpAdapter(IParaState*& src, IElement* owner, ParaStateQueue* queue, int16 priority)
				:mPtrRef(src), mOwner(owner), mQueue(queue), mPriority(priority)
			{
				assert(mPtrRef == NULL && mQueue != NULL && mOwner != NULL);
			}

			/** @brief  */
			BLADE_FRAMEWORK_API void operator=(IParaState& objRef);
		};


	public:
		ParaStateSet(IElement* owner, int16 priority) :mOwner(owner), mQueue(NULL),mPriority(priority) { mImpl.reserve(4); }
		~ParaStateSet() {}

		/** @brief  */
		IElement*				getOwner() const { return mOwner; }
		void					setOwner(IElement* owner) { mOwner = owner; }

		/** @brief  */
		ParaStateQueue*			getQueue() const { return mQueue; }
		void					setQueue(ParaStateQueue* queue) { mQueue = queue; }

		/** @brief  */
		int16					getPriority() const { return mPriority; }
		void					setPriority(int16 priority) { mPriority = priority; }

		/** @brief  */
		size_t					size() const { return mImpl.size(); }
		/** @brief  */
		const NamedType<IParaState*>&	at(size_t i) const { return mImpl.at(i); }

		/** @brief  */
		inline SOpAdapter		operator[](const TString& name)
		{
			IParaState*& stateRef = mImpl[name];
			return SOpAdapter(stateRef, this->getOwner(), this->getQueue(), this->getPriority());
		}

		/** @brief  */
		IParaState*				operator[](const TString& name) const
		{
			return mImpl[name];
		}

	private:
		typedef TNamedList<IParaState*> ParaStateSetImpl;

		IElement*			mOwner;
		ParaStateQueue*		mQueue;
		int16				mPriority;
		ParaStateSetImpl	mImpl;
	};//class ParaStateSet

}//namespace Blade



#endif // __Blade_ParaStateSet_h__