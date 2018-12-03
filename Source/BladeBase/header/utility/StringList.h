/********************************************************************
	created:	2010/09/04
	filename: 	StringList.h
	author:		Crazii
	purpose:	string list in common use.

				string param that only for temporary use, as params or return result
				remark:		the only difference between StringList & StringParam is that StringParam is temporary and should not
				be used as none-temp objects, nor as none-temp class member ( allocated on heap ).
				StringParam is used as function parameters (both [input]param or [output]param will be ok)
*********************************************************************/
#ifndef __Blade_StringList_h__
#define __Blade_StringList_h__
#include "String.h"
#include "TList.h"

namespace Blade
{
	/************************************************************************/
	/* String List                                                                     */
	/************************************************************************/
	typedef TList<String, VectorAllocator<String> > StringList;
	typedef TList<WString, VectorAllocator<WString> > WStringList;

#if BLADE_UNICODE
	typedef WStringList TStringList;
#else
	typedef StringList TStringList;
#endif

	/************************************************************************/
	/* String Param (temporary use)                                                                     */
	/************************************************************************/
	template<typename T>
	struct StringParamTraits
	{
		typedef void ElemType;
		typedef void BaseType;
	};

	template<>
	struct StringParamTraits<String>
	{
		typedef TmpString ElemType;
		typedef TList<TmpString, TempAllocator<TmpString>, TempAllocatable> BaseType;
	};

	template<>
	struct StringParamTraits<WString>
	{
		typedef TmpWString ElemType;
		typedef TList<TmpWString, TempAllocator<TmpWString>, TempAllocatable> BaseType;
	};

	template<typename T>
	class StringParamT : public StringParamTraits<T>::BaseType
	{
	public:
		typedef typename StringParamTraits<T>::ElemType ElemType;
		typedef typename StringParamTraits<T>::BaseType BaseType;
		class StringParamHelper
		{
			ElemType* mVal;
		public:
			StringParamHelper(ElemType* val) :mVal(val) {}
			void operator=(const T& val)
			{
				assert(mVal != NULL);
				if (mVal != NULL)
					*mVal = reinterpret_cast<const ElemType&>(val);
			}
			operator const T&() const { return mVal != NULL ? reinterpret_cast<const T&>(*mVal) : T::EMPTY; }
			inline bool operator==(const T& rhs) const { return mVal != NULL ? reinterpret_cast<const T&>(*mVal) == rhs : rhs.empty(); }
			inline void swap(StringParamHelper rhs) { if (mVal != NULL && rhs.mVal != NULL) std::swap(*mVal, *(rhs.mVal)); }
		};

		using BaseType::size;
		using BaseType::capacity;
		using BaseType::empty;
		using BaseType::pop_back;
		using BaseType::clear;
		using BaseType::reserve;
		using BaseType::resize;
		using BaseType::operator=;
#if BLADE_STDCPP >= BLADE_STDCPP11
		using BaseType::BaseType;
#else
		StringParamT() {}

		template<typename OT, typename OA>
		StringParamT(const TList<OT, OA>& src)
			:BaseType(src) {}
#endif
		/** @brief  */
		index_t find(const T& e) const
		{
			return this->BaseType::find(reinterpret_cast<const ElemType&>(e));
		}

		/** @brief  */
		void				push_back(const T& val)
		{
			return this->BaseType::push_back(reinterpret_cast<const ElemType&>(val));
		}

		/** @brief  */
		const T&	back() const
		{
			return reinterpret_cast<const T&>(this->BaseType::back());
		}

		/** @brief  */
		const T&	at(index_t index) const
		{
			return reinterpret_cast<const T&>(this->BaseType::at(index));
		}

		/** @brief  */
		inline const T& operator[](index_t index) const
		{
			return this->at(index);
		}

		inline StringParamHelper operator[](index_t index)
		{
			if (index < this->size())
				return StringParamHelper(&this->BaseType::at(index));
			return StringParamHelper(NULL);
		}

		/** @brief get internal array buffer */
		inline const T*	getBuffer() const
		{
			return reinterpret_cast<const T*>(BaseType::mData);
		}
	};//class StringParamT

	typedef StringParamT<String> StringParam;
	typedef StringParamT<WString> WStringParam;

#if BLADE_UNICODE
	typedef WStringParam TStringParam;
#else
	typedef StringParam TStringParam;
#endif

}//namespace Blade

namespace std
{
	inline void swap(Blade::StringParam::StringParamHelper lhs, Blade::StringParam::StringParamHelper rhs)
	{
		lhs.swap(rhs);
	}

	inline void swap(Blade::WStringParam::StringParamHelper lhs, Blade::WStringParam::StringParamHelper rhs)
	{
		lhs.swap(rhs);
	}
}

#endif //__Blade_StringList_h__