/********************************************************************
	created:	2012/11/24
	filename: 	ParaStateQueue.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_ParaStateQueue_h__
#define __Blade_ParaStateQueue_h__
#include <BladeFramework.h>
#include <interface/IParaState.h>
#include <Pimpl.h>

namespace Blade
{
	namespace Impl
	{
		class ParaStateQueueImpl;
	}//namespace Impl

	///state queue for subjects
	///this queue is accessed by multiple threads and should be thread safe
	class BLADE_FRAMEWORK_API ParaStateQueue : public Allocatable
	{
	public:
		ParaStateQueue();
		~ParaStateQueue();

		/** @brief  */
		void		push(IParaState* state);

		/** @brief  */
		IParaState*	pop();

		/** @brief  */
		bool		remove(IParaState* state);

		/** @brief  */
		void		update();

	private:
		Pimpl<Impl::ParaStateQueueImpl> mQueue;
	};

	typedef Handle<ParaStateQueue> HPARASTATEQUEUE;
	
}//namespace Blade


#endif//__Blade_ParaStateQueue_h__