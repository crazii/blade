/********************************************************************
	created:	2013/11/24
	filename: 	DataBridge.h
	author:		Crazii
	purpose:	bridge class for data-binding & delegate 
*********************************************************************/
#ifndef __Blade_DataBridge_h__
#define __Blade_DataBridge_h__
#include "DataBinding.h"
#include "Delegate.h"

namespace Blade
{

	class BLADE_BASE_API DataBridge : public NonAllocatable
	{
	public:
		template<typename T, typename V>
		inline DataBridge(V T::* pmdata, T* ptr = NULL, void (T::*pmEvent)(void*) = NULL)
			:mBinding(ptr, pmdata, pmEvent )
		{
			assert(pmdata != NULL);
		}

		template<typename T, typename V>
		inline DataBridge(const V& (T::*getter)(index_t) const, T* ptr = NULL, bool (T::*setter)(index_t, const V&) = NULL)
			:mBinding(ptr, getter, setter) {}

		inline DataBridge(const DataBinding& binding)
			:mBinding(binding) {}

		/** @brief  */
		inline const DataBinding&	getBinding() const	{return mBinding;}

		/** @brief  */
		inline void*	getData() const					{return mBinding.getDataPtr();}

		/** @brief  */
		inline const DelegateList&	getDelegates() const{return mDelegates;}

		/** @brief  */
		inline void		addDelegate(const Delegate& _delegate)	{mDelegates.push_back(_delegate);}

		/** @brief  */
		inline bool		rebind(Bindable* ptr)
		{
			bool ret = mBinding.rebind(ptr);
			assert(ret);
			return ret;
		}

		/** @brief update target data  */
		inline bool		setValue(const void* data, size_t bytes, bool callEvent = true) const
		{
			return mBinding.setData(data, bytes, callEvent);
		}

		/** @brief get data from target to parameter 'data'  */
		inline bool		getValue(void* data, size_t bytes) const
		{
			return mBinding.getData(data, bytes);
		}

	protected:
		DataBinding		mBinding;
		DelegateList	mDelegates;
	};
	
}//namespace Blade

#endif //  __Blade_DataBridge_h__