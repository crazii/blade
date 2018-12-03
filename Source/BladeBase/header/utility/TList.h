/********************************************************************
	created:	2017/05/21
	filename: 	TList.h
	author:		Crazii
	
	purpose:	Simple List(array/vector) for common use. this class is public.
				note List<T>/Vector<T> in BladeContainer.h should be private and not
				used in public(i.e. public class members, function parameters),
				while this class is free to use.
*********************************************************************/
#ifndef __Blade_TList_h__
#define __Blade_TList_h__
#include <new>
#if BLADE_STDCPP >= BLADE_STDCPP11
#include <type_traits>
#endif
#include <BladeTypes.h>
#include <memory/SizeAllocator.h>
#include <memory/TempAllocator.h>
#include <BladeException.h>
#include <utility/String.h>

namespace Blade
{
	template<typename T, typename A = VectorAllocator<T>, typename BA = Allocatable>
	class TList : public BA
	{
	public:
		typedef T* iterator;
		typedef const T * const_iterator;

		TList()
		{
			mData = NULL;	//don't pre-allocate
			mCount = 0;
			mCap = 0;
		}

		explicit TList(size_t count)
		{
			mData = NULL;
			mCount = 0;
			mCap = 0;
			this->reserve(count);
			mCount = count;
			for (size_t i = 0; i < mCount; ++i)
				new (&mData[i]) T();
		}

		TList(size_t count, const T& e)
		{
			mData = NULL;
			mCount = 0;
			mCap = 0;
			this->reserve(count);
			mCount = count;
			for (size_t i = 0; i < mCount; ++i)
				new (&mData[i]) T(e);
		}

		TList(const TList& src)
		{
			mData = NULL;
			mCount = 0;
			mCap = 0;

			this->reserve(src.size());
			mCount = src.size();
			for (size_t i = 0; i < mCount; ++i)
				new (&mData[i]) T(src.at(i));
		}

		template<typename OT, typename OA, typename OBA>
		TList(const TList<OT, OA, OBA>& src)
		{
			mData = NULL;
			mCount = 0;
			mCap = 0;

			this->reserve(src.size());
			mCount = src.size();
			for (size_t i = 0; i < mCount; ++i)
				new (&mData[i]) T(src.at(i));
		}

		~TList()
		{
			this->clear();
		}

		TList& operator=(const TList& rhs)
		{
			size_t count = mCount;
			if (count > rhs.size())
			{
				for (size_t i = rhs.size(); i < count; ++i)
					mData[i].~T();
				mCount = rhs.size();
				this->reallocate(rhs.capacity());
			}
			else if (count < rhs.size())
			{
				this->reallocate(rhs.capacity());
				mCount = rhs.size();
				for (size_t i = count; i < rhs.size(); ++i)
					new (&mData[i]) T();
			}

			for (size_t i = 0; i < mCount; ++i)
				mData[i] = rhs.at(i);
			return *this;
		}

		template<typename OT, typename OA, typename OBA>
		TList& operator=(const TList<OT, OA, OBA>& rhs)
		{
			size_t count = mCount;
			if (count > rhs.size())
			{
				for (size_t i = rhs.size(); i < count; ++i)
					mData[i].~T();
				mCount = rhs.size();
				this->reallocate(rhs.capacity());
			}
			else if (count < rhs.size())
			{
				this->reallocate(rhs.capacity());
				mCount = rhs.size();
				for (size_t i = count; i < rhs.size(); ++i)
					new (&mData[i]) T();
			}

			for (size_t i = 0; i < mCount; ++i)
				mData[i] = rhs.at(i);
			return *this;
		}

		/** @brief  */
		bool operator==(const TList& rhs) const
		{
			if (this->size() != rhs.size())
				return false;
#if BLADE_STDCPP >= BLADE_STDCPP11	//element are memset to 0 to fill padding gaps. so memcpy is OK. @see push_back()
			if (std::is_trivially_copyable<T>::value)
				return std::memcmp(mData, rhs.mData, sizeof(T)*mCount) == 0;
			else
#endif
			{
				for (size_t i = 0; i < mCount; ++i)
				{
					if (mData[i] != rhs.mData[i])
						return false;
				}
				return true;
			}
		}

		/** @brief  */
		size_t	size() const
		{
			return mCount;
		}

		/** @brief  */
		size_t	capacity() const
		{
			return mCap;
		}

		/** @brief  */
		bool	empty() const
		{
			return mData == NULL;
		}

		/** @brief  */
		iterator begin()
		{
			return mData;
		}
		/** @brief  */
		iterator end()
		{
			return mData + mCount;
		}

		/** @brief  */
		const_iterator begin() const
		{
			return mData;
		}

		/** @brief  */
		const_iterator end() const
		{
			return mData + mCount;
		}

		/** @brief  */
		const T&	operator[](size_t i) const
		{
			assert(i < mCount);
			return mData[i];
		}

		/** @brief  */
		T&	operator[](size_t i)
		{
			assert(i < mCount);
			return mData[i];
		}

		/** @brief  */
		const T&	at(size_t i) const
		{
			assert(i < mCount);
			BLADE_EXCEPT_IF(i >= mCount, EXC_OUT_OF_RANGE, BTString("subscript out of range."));
			return mData[i];
		}

		/** @brief  */
		T&	at(size_t i)
		{
			assert(i < mCount);
			BLADE_EXCEPT_IF(i >= mCount, EXC_OUT_OF_RANGE, BTString("subscript out of range."));
			return mData[i];
		}

		/** @brief  */
		index_t		find(const T& val) const
		{
			for (size_t i = 0; i < mCount; ++i)
			{
				if (mData[i] == val)
					return i;
			}
			return INVALID_INDEX;
		}

		/** @brief  */
		void		push_back(const T& e)
		{
			if (mCount >= mCap || mData == NULL)
			{
				assert(mCount == mCap);
				assert(mData != NULL || (mCount == 0));
				this->reserve(mCap+mCap/2+2);
			}

#if BLADE_STDCPP >= BLADE_STDCPP11
			if (std::is_trivially_copyable<T>::value)
				std::memset(&mData[mCount], 0, sizeof(T));
#endif
			assert(mCount < mCap);
			new (&mData[mCount]) T(e);
			++mCount;
		}

		/** @brief  */
		iterator insert(iterator pos, const T& e)
		{
			if (pos < this->begin() || pos > this->end())
			{
				assert(false);
				return this->end();
			}

			//after reallocation pos may be invalid,
			//use index
			ptrdiff_t index = pos - this->begin();

			//handle re-allocation
			this->resize(this->size()+1);

			pos = this->begin() + index;

			if (pos != this->end())
			{
				iterator i = this->end();
				--i;
				for (; i != pos; --i)
				{
#if BLADE_STDCPP >= BLADE_STDCPP11
					*i = std::move(*(i - 1));
#else
					*i = *(i - 1);
#endif
				}
				*i = e;
				return i;
			}
			else
			{
				this->back() = e;
				iterator i = this->end();
				return --i;
			}
		}

		/** @brief  */
		const T&		back() const 
		{
			assert(mCount >= 1);
			BLADE_EXCEPT_IF(mCount < 1, EXC_OUT_OF_RANGE, BTString("reference empty list."));

			return mData[mCount - 1];
		}

		/** @brief  */
		T&		back()
		{
			assert(mCount >= 1);
			BLADE_EXCEPT_IF(mCount < 1, EXC_OUT_OF_RANGE, BTString("reference empty list."));

			return mData[mCount - 1];
		}

		/** @brief  */
		void	pop_back()
		{
			assert(mCount >= 1);
			BLADE_EXCEPT_IF(mCount < 1, EXC_OUT_OF_RANGE, BTString("pop empty list."));
			if (mCount >= 1)
			{
				mData[mCount - 1].~T();
				--mCount;
				if (mCount < mCap / 2 && mCap > 2)
					this->shrink_to_fit();
			}
		}

		/** @brief  */
		void	resize(size_t count)
		{
			if (count == 0)
			{
				this->clear();
				return;
			}

			if (count > mCap)
			{
				assert(mCap != 0 || mData == NULL);
				assert(mCap == 0 || mData != NULL);
				size_t newCap = mCap + mCap / 2 + 2;
				newCap = newCap < count ? count : newCap;
				this->reserve(newCap);
			}
			assert(count <= mCap);

			for (size_t i = mCount; i < count; ++i)
				new (&mData[i]) T();
			for (size_t i = count; i < mCount; ++i)
				mData[i].~T();

			mCount = count;
		}

		/** @brief  */
		void	reserve(size_t count)
		{
			if (count > mCap)
				this->reallocate(count);
		}

		/** @brief  */
		void	shrink_to_fit()
		{
			this->reallocate(mCount);
		}

		/** @brief */
		void	erase(size_t i)
		{
			assert(i < mCount);
			BLADE_EXCEPT_IF(i >= mCount, EXC_OUT_OF_RANGE, BTString("index out of range."));

#if BLADE_STDCPP >= BLADE_STDCPP11
			if(std::is_move_assignable<T>::value)
			{
				for (; i < mCount - 1; ++i)
					mData[i] = std::move(mData[i + 1]);
				this->pop_back();
			}
			else
#endif
			{
				for (; i < mCount - 1; ++i)
					mData[i] = mData[i + 1];
				this->pop_back();
			}
		}

		void	erase(iterator i)
		{
			assert(i >= mData && i < mData + mCount);
			return this->erase(size_t(i - mData));
		}

		/** @brief  */
		void	clear()
		{
			for (size_t i = 0; i < mCount; ++i)
				mData[i].~T();

			A a;
			a.deallocate(mData, mCount);
			mCount = 0;
			mCap = 0;
			mData = NULL;
		}

		static const TList EMPTY;

	protected:
		/** @brief  */
		void	reallocate(size_t count)
		{
			assert(count >= mCount);
			if (mCap != count)
			{
				A a;
				T* newData = a.allocate(count);
				if (newData == NULL)
					throw std::bad_alloc();

#if BLADE_STDCPP >= BLADE_STDCPP11
				if (std::is_trivially_copyable<T>::value)
					std::memcpy(newData, mData, sizeof(T)*mCount);
#if BLADE_STDCPP >= BLADE_STDCPP17
				else if (std::is_move_constructable<T>::value)
				{
					for (size_t i = 0; i < mCount; ++i)
						new (&newData[i]) T(std::move(mData[i]));
				}
#endif
				else
#endif
				{
					//copy elements
					for (size_t i = 0; i < mCount; ++i)
					{
						new (&newData[i]) T(mData[i]);
						mData[i].~T();
					}
				}
				mCap = count;
				a.deallocate(mData, mCount);
				mData = newData;
			}
		}

		T*		mData;
		size_t	mCount;
		size_t	mCap;
	};

	template<typename T, typename A, typename BA>
	const TList<T, A, BA> TList<T, A, BA>::EMPTY;

	/************************************************************************/
	/* Named List                                                                     */
	/************************************************************************/
	///named list is string key mapped list, for simply use. it enumerate all elements to find the right key
	///so it is for light weight use. 
	///if you have huge quantities of objects, using TStringMap is recommended. or you can sort&search it manually.
	template<typename T>
	struct NamedType
	{
		NamedType() {}
		NamedType(const TString& name, const T& val) :mName(name), mValue(val) {}
		inline const bool operator<(const NamedType& rhs) { return mName < rhs.mName; }
		inline const TString&	getName() const { return mName; }
		inline const T&			getValue() const { return mValue; }
		TString mName;
		T		mValue;
		//default construction of T should be the EMPTY its type.
		static inline T const& getEMPTY() {
#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable:4640)//construction of local static object is not thread-safe
#endif
			static T _EMPTY = T(); return _EMPTY; 

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(pop)
#endif
		}
	};

	template<typename T, typename A = VectorAllocator< NamedType<T> >, typename BA = Allocatable>
	class TNamedList : public TList<NamedType<T>, A, BA>
	{
	public:
		typedef TList<NamedType<T>, A, BA> Base;

		TNamedList(const TString& name = TString::EMPTY) :mName(name) {}

		/** @brief  */
		inline const TString& getName() const { return mName; }

		/** @brief  */
		inline bool operator==(const TNamedList& rhs) const { return mName == rhs.mName; }

		/** @brief  */
		const T& operator[](const index_t& index) const
		{
			if (index < this->size())
				return this->at(index).mValue;
			return NamedType<T>::getEMPTY();
		}

		/** @brief  */
		const T& operator[](const TString& name) const
		{
			for (size_t i = 0; i < this->size(); ++i)
			{
				if (this->at(i).mName == name)
					return this->at(i).mValue;
			}
			return NamedType<T>::getEMPTY();
		}

		/** @brief  */
		T&	operator[](const TString& name)
		{
			for (size_t i = 0; i < this->size(); ++i)
			{
				if (this->at(i).mName == name)
					return this->at(i).mValue;
			}
			this->Base::push_back(NamedType<T>(name, T()));
			return this->back().mValue;
		}

		/** @brief  */
		index_t		find(const TString& name) const
		{
			for (size_t i = 0; i < this->size(); ++i)
			{
				if (this->at(i).mName == name)
					return i;
			}
			return INVALID_INDEX;
		}

		using Base::find;

	protected:
		TString	mName;
	};

	/************************************************************************/
	/* Pointer list                                                                     */
	/************************************************************************/
	template<typename T>
	class TPointerList : public TList<T*, VectorAllocator<T*> >
	{
	public:
		TPointerList() {}
		~TPointerList() {}
	};

	class PointerList : public TPointerList<void>
	{
	public:
		PointerList() {}
		~PointerList() {}

		template<typename T>
		T*	getAt(size_t i)
		{
			return static_cast<T*>(this->at(i));
		}
	};

	template<typename T>
	class TPointerParam : public TList<T*, TempAllocator<T*>, TempAllocatable>
	{
	public:
		TPointerParam() {}
		~TPointerParam() {}
	};

	class PointerParam : public TPointerParam<void>
	{
	public:
		PointerParam() {}
		~PointerParam() {}

		template<typename T>
		T*	getAt(size_t i)
		{
			return static_cast<T*>(this->at(i));
		}
	};
	
}//namespace Blade



#endif // __Blade_TList_h__