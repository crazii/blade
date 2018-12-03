/********************************************************************
	created:	2013/11/22
	filename: 	DataBinding.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_DataBinding_h__
#define __Blade_DataBinding_h__
#include "Bindable.h"
#include "String.h"

namespace Blade
{
	template<typename T, typename V>
	struct TBindingDesc
	{
		typedef const V& (T::*GETTER)(index_t) const;
		typedef bool (T::*SETTER)(index_t, const V&);
		typedef V T::*MPTR;
		typedef void (T::*EVENT)(void*);
	};

	namespace DataBindingUtil
	{
		template<typename V>
		struct DataBindGetterImplBase
		{
			static Bindable* getDataBind(const void* bounddata)
			{
				V* pv = (V*)bounddata;
				//note: use static_cast: type V must inherit Bindable
				return static_cast<Bindable*>(pv);
			}

			static void getData(const V* data, void* outData)
			{
				*(V*)outData = *data;
			}

			static void setData(V* data, const void* outData)
			{
				*data = *(V*)outData;
			}
		};

		template<typename V>
		struct DataBindGetterImplBase<V*>
		{
			static Bindable* getDataBind(const void* bounddata)
			{
				V** ppv = (V**)bounddata;
				//note: use static_cast: type V must inherit Bindable
				return static_cast<Bindable*>(*ppv);
			}

			static void getData(V const* const* data, void* outData)
			{
				*(V**)outData = *data;
			}

			static void setData(V** data, const void* outData)
			{
				*data = *(const V**)outData;
			}
		};

		template<typename V, size_t N>
		struct DataBindGetterImplBase<V[N]>
		{
			static Bindable* getDataBind(const void* bounddata)
			{
				return NULL;
			}

			static void getData(V const(*data)[N], void* outData)
			{
				for (size_t i = 0; i < N; ++i)
					((V*)outData)[i] = (*data)[i];
			}

			static void setData(V(*data)[N], const void* outData)
			{
				for (size_t i = 0; i < N; ++i)
					(*data)[i] = ((const V*)outData)[i];
			}
		};

		template<typename V, bool isbase>
		struct DataBindGetterImpl
		{
			static Bindable* getDataBind(const void* /*bounddata*/)
			{
				return NULL;
			}
		};

		template<typename V>
		struct DataBindGetterImpl<V, true>
		{
			static Bindable* getDataBind(const void* bounddata)
			{
				return DataBindGetterImplBase<V>::getDataBind(bounddata);
			}
		};

		/************************************************************************/
		/*                                                                      */
		/************************************************************************/
		//note: the data binding here is basic and simple: without complicated cases (compound & collection)
		typedef void*		(*GET_FUNC)(Bindable* ptr, const char* pmdata);
		typedef bool		(*GETDATA_FUNC)(Bindable* ptr, const char* pmdata, void* outData, size_t bytes);
		typedef Bindable*	(*GETBIND_FUNC)(const void* bounddata);
		typedef bool		(*SET_FUNC)(Bindable* ptr, const char* pmdata, const void* data, size_t bytes);
		typedef bool		(*CMP_FUNC)(const void* data1, const void* data2, size_t bytes);
		typedef Bindable*	(*REBIND_FUNC)(Bindable* ptr);
		typedef void		(*EVENT_FUNC)(Bindable* ptr, const char* pmEvent, void* eventData);

		//this structure uses too much memory (use memory for every instance)
		//use virtual interface, so that per type v-table is used and save memory.
		//now use static structure object (BINDINGFUNC()), same memory as v-table.
		struct BindingFunc
		{
			REBIND_FUNC		FnRebind;
			GET_FUNC		FnGetPtr;
			GETDATA_FUNC	FnGetData;
			GETBIND_FUNC	FnGetBind;
			SET_FUNC		FnSet;
			CMP_FUNC		FnCmp;
			EVENT_FUNC		FnEvent;
		};

		template<typename T, typename V>
		static bool			getData(Bindable* ptr, const char* memberPtr, void* outData, size_t bytes)
		{
			if( bytes == sizeof(V) )
			{
				typedef V T::*PMData;
				PMData pdata = NULL;
				std::memcpy(&pdata, memberPtr, sizeof(PMData) );
				DataBindGetterImplBase<V>::getData(&(((T*)ptr)->*pdata), outData);
				return true;
			}
			else
			{
				assert(false);
				return false;
			}
		}

		/** @brief  */
		template<typename V>
		static Bindable*	getDataBind(const void* bounddata)
		{
			return DataBindGetterImpl<V, IsBindable<V>::value >::getDataBind(bounddata);
		}

		/** @brief  */
		template<typename T, typename V>
		static bool	setData(Bindable* ptr, const char* memberPtr, const void* data, size_t bytes)
		{
			if( bytes == sizeof(V) )
			{
				typedef V T::*PMData;
				PMData pdata = NULL;
				std::memcpy(&pdata, memberPtr, sizeof(PMData) );
				DataBindGetterImplBase<V>::setData(&(((T*)ptr)->*pdata), data);
				return true;
			}
			else
			{
				assert(false);
				return false;
			}
		}

		/** @brief  */
		template<typename T, typename V>
		static void*		getDataPtr(Bindable* ptr, const char* memberPtr)
		{
			typedef V T::*PMData;
			PMData pdata = NULL;
			std::memcpy(&pdata, memberPtr, sizeof(PMData));
			V& data = ((T*)ptr)->*pdata;
			return &data;
		}

		/** @brief  */
		template<typename V>
		static bool	compareData(const void* data1, const void* data2, size_t bytes)
		{
			if( bytes == sizeof(V) )
			{
				const V& lhs = *(const V*)data1;
				const V& rhs = *(const V*)data2;
				return lhs == rhs;
			}
			else
			{
				assert(false);
				return false;
			}
		}

		template<typename T>
		void		invokeEvent(Bindable* ptr, const char* eventPtr, void* eventData)
		{
			typedef void (T::*event_t)(void*);
			event_t pEvent;
			std::memcpy(&pEvent, eventPtr, sizeof(pEvent) );
			if( pEvent != NULL )
				(((T*)ptr)->*pEvent)(eventData);
		}

		/** @brief  */
		template<typename T>
		static Bindable* rebindPtr(Bindable* ptr)
		{
#if BLADE_DEBUG
			assert( dynamic_cast<T*>(ptr) != NULL );
#endif
			return (T*)static_cast<T*>(ptr);
		}


		template<typename T, typename V>
		struct DefBindingFunc : public BindingFunc
		{
			DefBindingFunc()
			{
#if BLADE_STDCPP >= BLADE_STDCPP11 && !BLADE_DEBUG	//bindable in debug edition has virtual functions
				//no longer needed because we use operator= instead of memcpy
				//static_assert( std::is_trivially_copyable<V>::value, "data must be trivially copyable" );
#endif
				FnRebind = &rebindPtr<T>;
				FnGetPtr = &getDataPtr<T,V>;
				FnGetData = &getData<T,V>;
				FnGetBind = (!IsBindable<V>::value) ? NULL : &getDataBind<V>;
				FnSet = &setData<T,V>;
				FnCmp = &compareData<V>;
				FnEvent = &invokeEvent<T>;
			}
			static const DefBindingFunc* BINDINGFUNC()
			{
				return &msFunc;
			}

			static DefBindingFunc msFunc;
		};

		template<typename T, typename V>
		DefBindingFunc<T, V> DefBindingFunc<T, V>::msFunc;

		/************************************************************************/
		/* getter setter                                                                     */
		/************************************************************************/
		template<typename T, typename V>
		static bool			getter_getData(Bindable* ptr, const char* getterPtr, void* outData, size_t bytes)
		{
			if (bytes == sizeof(V))
			{
				typedef const V& (T::*PMGetter)(index_t) const;
				PMGetter pgetter = NULL;
				std::memcpy(&pgetter, getterPtr, sizeof(PMGetter));
				*(V*)outData = (((T*)ptr)->*pgetter)(0);
				return true;
			}
			else
			{
				assert(false);
				return false;
			}
		}

		template<typename T, typename V>
		static void*		getter_getDataPtr(Bindable* ptr, const char* getterPtr)
		{
			typedef const V& (T::*PMGetter)(index_t) const;
			PMGetter pgetter = NULL;
			std::memcpy(&pgetter, getterPtr, sizeof(PMGetter));
			return const_cast<V*> (&(((T*)ptr)->*pgetter)(0));
		}

		/** @brief  */
		template<typename T, typename V>
		static bool	setter_setData(Bindable* ptr, const char* setterPtr, const void* data, size_t bytes)
		{
			if (bytes == sizeof(V))
			{
				typedef bool (T::*PMSetter)(index_t, const V&);
				PMSetter psetter = NULL;
				std::memcpy(&psetter, setterPtr, sizeof(PMSetter));
				if (psetter != NULL)
					return (((T*)ptr)->*psetter)(0, *(const V*)data);
				return false;
			}
			else
			{
				assert(false);
				return false;
			}
		}

		template<typename T, typename V>
		struct GetterSetterBindingFunc : public BindingFunc
		{
			GetterSetterBindingFunc()
			{
				FnRebind = &rebindPtr<T>;
				FnGetPtr = &getter_getDataPtr<T, V>;
				FnGetData = &getter_getData<T, V>;
				FnGetBind = (!IsBindable<V>::value) ? NULL : &getDataBind<V>;
				FnSet = &setter_setData<T, V>;
				FnCmp = &compareData<V>;
				FnEvent = &invokeEvent<T>;
			}
			static const GetterSetterBindingFunc* BINDINGFUNC()
			{
				return &msFunc;
			}
			static GetterSetterBindingFunc msFunc;
		};
		template<typename T, typename V>
		GetterSetterBindingFunc<T, V> GetterSetterBindingFunc<T, V>::msFunc;

	}//namespace DataBindingUtil

	class BLADE_BASE_API DataBinding : public NonAllocatable
	{
	protected:
		typedef DataBindingUtil::BindingFunc BindingFunc;
		static const size_t MAX_SIZE = MAX_POINTER2MEMBER_SIZE;

		Bindable*	mPtr;
		const BindingFunc*mFunc;
		size_t		mTypeMask;
		char		mMember[MAX_SIZE];
		///event binding to target: this is different from common delegate
		///a common delegate is valid at any time, but a binding event is only valid when it is bound to a target,
		///so that the same target can handle its own data change event
		char		mEvent[MAX_SIZE];
		char		mSetter[MAX_SIZE];
	public:
		static const DataBinding EMPTY;

		/** @brief  */
		inline DataBinding()
		{
#if BLADE_STDCPP >= BLADE_STDCPP11
			static_assert( std::is_trivially_copyable<DataBinding>::value, "data binding structure should be trivially copyable" );
#endif
			mPtr = NULL;
			mFunc = NULL;
			mTypeMask = 0;
			std::memset(mMember, 0, sizeof(mMember));
			std::memset(mEvent, 0, sizeof(mEvent));
			std::memset(mSetter, 0, sizeof(mSetter));
		}

		/** @brief  */
		template<typename T, typename V>
		inline DataBinding(T* ptr, V T::*pmdata, void (T::*pmEvent)(void*) = NULL)
		{
			mPtr = ptr;
			mFunc = DataBindingUtil::DefBindingFunc<T, V>::BINDINGFUNC();
			mTypeMask = 0;
			//fill any gap to make operator == work
			std::memset(mMember, 0, sizeof(mMember) );
			std::memset(mEvent, 0, sizeof(mEvent) );
			std::memset(mSetter, 0, sizeof(mSetter));
			static_assert( sizeof(pmdata) <= MAX_SIZE, "size error");
			static_assert( sizeof(pmEvent) <= MAX_SIZE, "size error");
			std::memcpy(mMember, &pmdata, sizeof(pmdata));
			std::memcpy(mEvent, &pmEvent, sizeof(pmEvent));
		}

		/** @brief  */
		template<typename T, typename V>
		inline DataBinding(T* ptr, const V& (T::*getter)(index_t) const, bool (T::*setter)(index_t, const V&) = NULL, void (T::*pmEvent)(void*) = NULL)
		{
			mPtr = ptr;
			mFunc = DataBindingUtil::GetterSetterBindingFunc<T, V>::BINDINGFUNC();
			mTypeMask = 1;
			//fill any gap to make operator == work
			std::memset(mMember, 0, sizeof(mMember));
			std::memset(mEvent, 0, sizeof(mEvent));
			std::memset(mSetter, 0, sizeof(mSetter));
			static_assert(sizeof(getter) <= MAX_SIZE, "size error");
			static_assert(sizeof(setter) <= MAX_SIZE, "size error");
			//save getter
			std::memcpy(mMember, &getter, sizeof(getter));
			//save event
			std::memcpy(mEvent, &pmEvent, sizeof(pmEvent));
			//save setter
			std::memcpy(mSetter, &setter, sizeof(setter));
		}

		/** @brief  */
		inline bool		operator==(const DataBinding& rhs) const
		{
			return std::memcmp(this, &rhs, sizeof(DataBinding) ) == 0;
		}

		/** @brief  */
		inline bool		isEmpty() const
		{
			return *this == EMPTY;
		}

		/** @brief whether the bound data(getData()) is bindable too */
		inline bool		isDataBinable() const
		{
			return mFunc->FnGetBind !=  NULL;
		}

		/** @brief the the bound data as bindable data */
		inline Bindable*getBinableData() const
		{
			if( this->isBound() && mFunc->FnGetBind != NULL )
				return mFunc->FnGetBind(mFunc->FnGetPtr(mPtr, mMember));
			else
				return NULL;
		}

		/** @brief  */
		inline void*		getDataPtr() const
		{
			if( this->isBound() )
				return mFunc->FnGetPtr(mPtr, mMember);
			else
			{
				assert(false);
				return NULL;
			}
		}

		/** @brief  */
		inline bool			getData(void* pdata, size_t bytes) const
		{
			if( this->isBound() )
				return mFunc->FnGetData(mPtr, mMember, pdata, bytes);
			else
			{
				assert(false);
				return false;
			}
		}

		/** @brief  */
		inline bool		setData(const void* data, size_t bytes, bool callEvt = true) const
		{
			bool ret = false;
			if( this->isBound() )
			{
				if (mTypeMask == 0)
				{
					ret = mFunc->FnSet(mPtr, mMember, data, bytes);
					if (ret && callEvt)
					{
						void* targetData = mFunc->FnGetPtr(mPtr, mMember);
						mFunc->FnEvent(mPtr, mEvent, targetData);
					}
					assert(ret);
				}
				else //setter
					ret = mFunc->FnSet(mPtr, mSetter, data, bytes);
			}
			return ret;
		}

		/** @brief  */
		inline bool		compareData(const void* data, size_t bytes) const
		{
			if( this->isBound() )
				return mFunc->FnCmp(mFunc->FnGetPtr(mPtr, mMember), data, bytes);
			else
			{
				assert(false);
				return false;
			}
		}

		/** @brief  */
		inline void callEvent() const
		{
			if (this->isBound())
			{
				void* targetData = mFunc->FnGetPtr(mPtr, mMember);
				mFunc->FnEvent(mPtr, mEvent, targetData);
			}
		}

		/** @brief  */
		Bindable*		getTarget() const
		{
			return mPtr;
		}

		/** @brief  */
		bool			isBound() const
		{
			return mPtr != NULL;
		}

		/** @brief  */
		inline bool		rebind(Bindable* target)
		{
			if( *this == EMPTY )
				return false;
			if( target == NULL )
			{
				mPtr = NULL;
				return true;
			}
			else
			{
				mPtr = mFunc->FnRebind(target);
				assert(mPtr != NULL);
				return mPtr != NULL;
			}
		}
	};
	
}//namespace Blade

#endif //  __Blade_DataBinding_h__