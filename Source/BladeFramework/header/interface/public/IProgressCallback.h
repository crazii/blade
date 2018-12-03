/********************************************************************
	created:	2016/6/2
	filename: 	IProgressCallback.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_IProgressCallback_h__
#define __Blade_IProgressCallback_h__
#include <utility/BiRef.h>
#include <BladeFramework.h>

namespace Blade
{

	/************************************************************************/
	/* sync callback interface (mainly for loading progress)                                                                    */
	/************************************************************************/
	class IProgressCallback : public BiRef
	{
	public:
		virtual ~IProgressCallback() {}	//make compiler happy

		/** @brief  */
		virtual void onNotify(scalar percent) = 0;

	protected:
		using BiRef::setRef;
		using BiRef::detach;
		friend class ProgressNotifier;
	};//class ISyncCallback

	namespace Impl
	{
		class ProgressNotifierImpl;
	}//namespace Impl

	 /** @brief  */
	class BLADE_FRAMEWORK_API ProgressNotifier : public BiRef, public NonCopyable
	{
	public:
		ProgressNotifier();
		ProgressNotifier(IProgressCallback* rhs);
		~ProgressNotifier();

		/** @brief  */
		inline ProgressNotifier& operator=(IProgressCallback* rhs)
		{
			if (mRef != rhs)
			{
				if (mRef != NULL)
					static_cast<IProgressCallback*>(mRef)->detach();
				mRef = rhs;
				if (rhs)
					rhs->setRef(this);
			}
			return *this;
		}

		/** @brief  */
		void	beginStep(scalar rate) const;

		/** @brief progress on current step*/
		void	onStep(size_t index, size_t count) const;

		/** @brief progress on current step */
		void	onNotify(scalar rate) const;

		/** @brief  */
		inline void advance(scalar rate) const
		{
			this->beginStep(rate);
			this->onNotify(1.0f);
		}

		static const ProgressNotifier EMPTY;

	protected:
		mutable scalar	mPrecent;
	private:
		mutable Pimpl<Impl::ProgressNotifierImpl>	mImpl;
		ProgressNotifier(bool);
		ProgressNotifier(const ProgressNotifier&);
		ProgressNotifier& operator=(const ProgressNotifier&);
	};// class ProgressNotifier
	
}//namespace Blade

#endif//__Blade_IProgressCallback_h__