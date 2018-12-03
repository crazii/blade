/********************************************************************
	created:	2009/02/08
	created:	8:2:2009   15:26
	filename: 	String.h
	author:		Crazii
	
	purpose:	this is a self defined String (template) class
*********************************************************************/
#ifndef __Blade_String_h__
#define __Blade_String_h__
#include <Pimpl.h>
#include <memory/SizeAllocator.h>
#include <memory/TempAllocator.h>


#if BLADE_COMPILER_GNU_EXTENSION

#	if BLADE_PLATFORM == BLADE_PLATFORM_ANDROID

//simple towlower, only for ASCII version
#		define towlowerASCII(wc) (((L'A' <= (wc))&&((wc) <= L'Z')) ? ((wc) - L'A' + L'a') : (wc))
namespace
{
#if BLADE_COMPILER != BLADE_COMPILER_CLANG
	inline int wcscasecmp(const wchar_t* lhs, const wchar_t* rhs)
	{
		assert(lhs != NULL && rhs != NULL);
		wchar_t cmp;
		while ((cmp = towlowerASCII(*lhs) - towlowerASCII(*rhs)) == 0)
		{
			if (0 == *lhs)
				return 0;
			++lhs;
			++rhs;
		}
		return (int)cmp;
	}
#endif
}//anonymous namespace
#		undef towlowerASCII

#		if !_GLIBCXX_USE_C99
extern long long wcstoll(const wchar_t* begwstr, wchar_t** endwstr, int base);
extern unsigned long long wcstoull(const wchar_t* begwstr, wchar_t** endwstr, int base);
#		endif

#	endif//BLADE_PLATFORM

#	define snwprintf		swprintf

#elif BLADE_COMPILER == BLADE_COMPILER_MSVC

#	define strcasecmp		stricmp
#	define wcscasecmp		wcsicmp
#	define isnan			_isnan
#	define strtoll			_strtoi64
#	define strtoull			_strtoui64
#	define wcstoll			_wcstoi64
#	define wcstoull			_wcstoui64
#	define snprintf			_snprintf
#	define snwprintf		_snwprintf

#endif

#if BLADE_STDCPP < BLADE_STDCPP11
namespace std
{
#if (defined(_MSC_VER) && (_MSC_VER < 1800/*before VS2013*/))
	BLADE_BASE_API int vsscanf(const char* strbuffer, const char* fmt, va_list varg_ptr);
#endif
#if (defined(_MSC_VER) && (_MSC_VER < 1800/*before VS2013*/)) || (BLADE_COMPILER_GNU_EXTENSION && !_GLIBCXX_HAVE_VSWSCANF)
	BLADE_BASE_API int vswscanf(const wchar_t* strbuffer, const wchar_t* fmt, va_list varg_ptr);
#endif
}
#endif//BLADE_STDCPP < BLADE_STDCPP11

namespace Blade
{

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable:4996)
#endif

	/************************************************************************/
	/* BasicString template                                                                      */
	/************************************************************************/
	template<typename Char_Type,typename Traits_Type,typename Char_Alloc>
	class BasicString
	{
	public:
		typedef BasicString<Char_Type,Traits_Type,Char_Alloc> this_type;
		typedef Char_Type		char_type;
		typedef Traits_Type		traits_type;

		typedef typename Char_Alloc::size_type		size_type;
		typedef typename Char_Alloc::value_type		alloc_type;

		typedef typename traits_type::concat_type	concat_type;
		enum EConstInit
		{
			NONE_CONST = 0,
			STATIC_CONST,
			CONST_HINT,		//const string
			CONST_TEMP,		//const string but need allocation on copy because data is not persistent

			CONST_END,
			CONST_START = STATIC_CONST,
		};

		/** @brief  */
		BLADE_ALWAYS_INLINE BasicString(void) :mCPtr( Traits_Type::null() ),mSize(0),mCapacity(0), mConstHint(NONE_CONST){}

		/** @brief CONST construction */
		BLADE_ALWAYS_INLINE explicit BasicString(const char_type* source,size_t source_size, EConstInit hint)
			:mCPtr(source),mSize(source_size),mCapacity(0),mConstHint((uint8)hint)
		{
			assert(hint >= CONST_START && hint < CONST_END);
		}

		/** @brief  */
		inline explicit BasicString(const char_type* source, size_t source_size)
		{
			if (source != NULL)
			{
				mSize = source_size;
				mCapacity = this->evaluate_capacity();
				this->allocate_buffer();
				Traits_Type::copy(mPtr, source, mSize);
				mPtr[mSize] = char_type();
			}
			else
			{
				mCPtr = Traits_Type::null();
				mSize = 0;
				mCapacity = 0;
			}
			mConstHint = NONE_CONST;
		}

		/** @brief  */
		inline explicit BasicString(const char_type* source)
		{
			if( source != NULL )
			{
				mSize = Traits_Type::length(source);
				mCapacity = this->evaluate_capacity();
				this->allocate_buffer();
				Traits_Type::copy(mPtr,source,mSize+1);
			}
			else
			{
				mCPtr = Traits_Type::null();
				mSize = 0;
				mCapacity = 0;
			}
			mConstHint = NONE_CONST;
		}

		/** @brief operator + */
		inline BasicString(const concat_type& src);

		/** @brief  */
		BLADE_ALWAYS_INLINE BasicString(const this_type& source)
		{
			mSize = source.mSize;

			if ((source.is_const() && source.mConstHint != CONST_TEMP) || source.mPtr == Traits_Type::null())
			{
				mCapacity = 0;
				mCPtr = source.mCPtr;
				mConstHint = CONST_HINT;
			}
			else
			{
				mCapacity = this->evaluate_capacity();
				this->allocate_buffer();
				Traits_Type::copy(mPtr, source.mCPtr, mSize);
				mPtr[mSize] = char_type();
				mConstHint = NONE_CONST;
			}
		}

		/** @brief  */
		template<typename A>
		BLADE_ALWAYS_INLINE BasicString(const BasicString<Char_Type, Traits_Type, A>& source)
		{
			mSize = source.mSize;

			if( (source.is_const() && source.mConstHint != CONST_TEMP) || source.mPtr == Traits_Type::null() )
			{
				mCapacity = 0;
				mCPtr = source.mCPtr;
				mConstHint = CONST_HINT;
			}
			else
			{
				mCapacity = this->evaluate_capacity();
				this->allocate_buffer();
				Traits_Type::copy(mPtr, source.mCPtr, mSize);
				mPtr[mSize] = char_type();
				mConstHint = NONE_CONST;
			}
		}

		/** @brief dector */
		BLADE_ALWAYS_INLINE ~BasicString(void)
		{
			this->tidy_buffer();
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE int	compare(const this_type& rhs) const
		{
			if (mCPtr == rhs.mCPtr)
				return int(mSize - rhs.mSize);
			else
			{
				size_t len = mSize <= rhs.mSize ? mSize : rhs.mSize;
				return Traits_Type::compare(mCPtr,rhs.mCPtr,len);
			}
		}

		//////////////////////////////////////////////////////////////////////////
		BLADE_ALWAYS_INLINE int compare_no_case(const this_type& rhs) const
		{
			if( mCPtr == rhs.mCPtr)
				return int(mSize - rhs.mSize);
			else
				return Traits_Type::compare_no_case(mCPtr, mSize, rhs.mCPtr, rhs.mSize);
		}

		//////////////////////////////////////////////////////////////////////////
		/** @brief  */
		BLADE_ALWAYS_INLINE void	swap(this_type& target)
		{
			std::swap(mPtr,target.mPtr);
			std::swap(mCapacity,target.mCapacity);
			std::swap(mSize,target.mSize);
		}

		//////////////////////////////////////////////////////////////////////////
		//operators


		/** @brief  */
		BLADE_ALWAYS_INLINE bool operator==(const this_type& rhs) const
		{
			if (this == &rhs)
				return true;
			else if (mSize != rhs.mSize)
				return false;
			else if (mCPtr == rhs.mCPtr)
				return true;
			else
				return Traits_Type::compare(mCPtr,rhs.mCPtr,mSize) == 0;
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE bool operator !=(const this_type&rhs) const
		{
			return !( *this == rhs);
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE bool operator==(const char_type* rhs) const
		{
			if( mCPtr == rhs)
				return true;

			if( rhs == NULL )
			{
				if( mSize == 0)
					return true;
				else
					return false;
			}

			size_t len = Traits_Type::length(rhs);
			if( mSize != len )
				return false;
			else
				return Traits_Type::compare(mCPtr,rhs,mSize) == 0;
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE bool operator!=(const char_type* rhs) const
		{
			return !( *this == rhs );
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE bool operator <(const this_type& rhs) const
		{
			if( mCPtr == rhs.mCPtr)
				return false;

			size_t len = mSize < rhs.mSize ? mSize : rhs.mSize;
			int cmp = Traits_Type::compare(mCPtr,rhs.mCPtr,len);
			return cmp < 0 || (cmp == 0 && mSize < rhs.mSize);
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE bool operator <(const char_type* rhs) const
		{
			if( mCPtr == rhs)
				return false;

			if( rhs == NULL )
			{
				return false;
			}

			size_t chlen = Traits_Type::length(rhs);
			size_t len = mSize < chlen ? mSize : chlen;
			int cmp = Traits_Type::compare(mCPtr, rhs, len);
			return cmp < 0 || (cmp == 0 && mSize < chlen);
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE bool operator >(const this_type& rhs) const
		{
			if( mCPtr == rhs.mCPtr)
				return false;

			size_t len = mSize < rhs.mSize ? mSize : rhs.mSize;
			int cmp = Traits_Type::compare(mCPtr,rhs.mCPtr,len);
			return cmp > 0 || (cmp == 0 && mSize > rhs.mSize);
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE bool operator >(const char_type* rhs) const
		{
			if( mCPtr == rhs )
				return false;

			if( rhs == NULL )
			{
				if( mSize == 0)
					return false;
				else
					return true;
			}

			size_t chlen = Traits_Type::length(rhs);
			size_t len = mSize < chlen ? mSize : chlen;
			int cmp = Traits_Type::compare(mCPtr,rhs,len);
			return cmp > 0 || (cmp == 0 && mSize > chlen);
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE bool operator <=(const this_type& rhs) const
		{
			if( mCPtr == rhs.mCPtr)
				return false;

			size_t len = mSize < rhs.mSize ? mSize : rhs.mSize;
			int cmp = Traits_Type::compare(mCPtr,rhs.mCPtr,len);
			return cmp < 0 || (cmp == 0 && mSize <= rhs.mSize);
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE bool operator <=(const char_type* rhs) const
		{
			if( mCPtr == rhs)
				return false;

			if( rhs == NULL )
			{
				return false;
			}

			size_t chlen = Traits_Type::length(rhs);
			size_t len = mSize < chlen ? mSize : chlen;
			int cmp = Traits_Type::compare(mCPtr, rhs, len);
			return cmp < 0 || (cmp == 0 && mSize <= chlen);
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE bool operator >=(const this_type& rhs) const
		{
			if( mCPtr == rhs.mCPtr)
				return false;

			size_t len = mSize < rhs.mSize ? mSize : rhs.mSize;
			int cmp = Traits_Type::compare(mCPtr,rhs.mCPtr,len);
			return cmp > 0 || (cmp == 0 && mSize >= rhs.mSize);
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE bool operator >=(const char_type* rhs) const
		{
			if( mCPtr == rhs )
				return false;

			if( rhs == NULL )
			{
				if( mSize == 0)
					return false;
				else
					return true;
			}

			size_t chlen = Traits_Type::length(rhs);
			size_t len = mSize < chlen ? mSize : chlen;
			int cmp = Traits_Type::compare(mCPtr,rhs,len);
			return cmp > 0 || (cmp == 0 && mSize >= chlen);
		}

		/** @brief  */
		inline this_type& operator=(const this_type& rhs)
		{
			if (this == &rhs)
				return *this;
			else if (rhs.is_const() && (rhs.mConstHint != CONST_TEMP || rhs.mSize == 0))
			{
				this->tidy_buffer();

				mSize = rhs.mSize;
				mCapacity = rhs.mCapacity;
				mCPtr = rhs.mCPtr;
				mConstHint = rhs.mConstHint;
			}
			else
			{
				if (!this->is_const() && /*rhs.is_const() &&*/
					rhs.mCPtr >= mCPtr && rhs.mCPtr <= mCPtr + mSize)//CONST_TEMP may comes from self
				{
					assert(mSize >= rhs.mSize);
					assert(mCapacity != 0);
					mSize = rhs.mSize;

					size_t capacity = this->evaluate_capacity();
					if (capacity != mCapacity)
					{
						char_type* p = mPtr;
						size_t cap = mCapacity;
						mCapacity = capacity;
						this->allocate_buffer();
						Traits_Type::copy(mPtr, rhs.mCPtr, mSize);
						mAllocator.deallocate(reinterpret_cast<typename Char_Alloc::pointer>(p), cap);
					}
					else
						Traits_Type::move(mPtr, rhs.mCPtr, mSize);

					mPtr[mSize] = char_type();
					mConstHint = NONE_CONST;
				}
				else
				{
					mSize = rhs.mSize;
					size_t capacity = this->evaluate_capacity();
					if (capacity != mCapacity)
					{
						this->tidy_buffer();
						mCapacity = capacity;
						this->allocate_buffer();
					}
					Traits_Type::copy(mPtr, rhs.mCPtr, mSize);
					mPtr[mSize] = char_type();
					mConstHint = NONE_CONST;
				}
			}
			return *this;
		}

		/** @brief  */
		template<typename A>
		inline this_type& operator=(const BasicString<Char_Type, Traits_Type, A>& rhs)
		{
			if (this == (void*)&rhs)
				return *this;
			else if( rhs.is_const() && (rhs.mConstHint != CONST_TEMP || rhs.mSize == 0) )
			{
				this->tidy_buffer();

				mSize = rhs.mSize;
				mCapacity = rhs.mCapacity;
				mCPtr = rhs.mCPtr;
				mConstHint = rhs.mConstHint;
			}
			else
			{
				if ( !this->is_const() && /*rhs.is_const() &&*/
					rhs.mCPtr >= mCPtr && rhs.mCPtr <= mCPtr + mSize )//CONST_TEMP may comes from self
				{
					assert(mSize >= rhs.mSize);
					assert(mCapacity != 0);
					mSize = rhs.mSize;

					size_t capacity = this->evaluate_capacity();				
					if (capacity != mCapacity)
					{
						char_type* p = mPtr;
						size_t cap = mCapacity;
						mCapacity = capacity;
						this->allocate_buffer();
						Traits_Type::copy(mPtr, rhs.mCPtr, mSize);
						mAllocator.deallocate(reinterpret_cast<typename Char_Alloc::pointer>(p), cap);
					}
					else
						Traits_Type::move(mPtr, rhs.mCPtr, mSize);

					mPtr[mSize] = char_type();
					mConstHint = NONE_CONST;
				}
				else
				{
					mSize = rhs.mSize;
					size_t capacity = this->evaluate_capacity();
					if (capacity != mCapacity)
					{
						this->tidy_buffer();
						mCapacity = capacity;
						this->allocate_buffer();
					}
					Traits_Type::copy(mPtr, rhs.mCPtr, mSize);
					mPtr[mSize] = char_type();
					mConstHint = NONE_CONST;
				}
			}
			return *this;
		}

		/** @brief  */
		inline this_type& operator=(const char_type* rhs)
		{
			if( rhs == NULL )
			{
				this->tidy_buffer();
				mCapacity = mSize = 0;
				mCPtr = Traits_Type::null();
				mConstHint = NONE_CONST;
				return *this;
			}

			size_t len = Traits_Type::length(rhs);
			mSize = len;
			size_t capacity = this->evaluate_capacity();

			if( capacity != mCapacity )
			{
				this->tidy_buffer();
				mCapacity = capacity;
				this->allocate_buffer();
			}
			Traits_Type::copy(mPtr,rhs,mSize+1);
			mConstHint = NONE_CONST;
			return *this;
		}

		/** @brief  */
		inline this_type& operator=(const concat_type& rhs);

		/** @brief  */
		inline this_type& operator+=(const concat_type& rhs);

		/** @brief  */
		inline this_type& operator+=(const this_type& rhs)
		{
			size_t pos = mSize;
			mSize += rhs.mSize;
			size_t new_capacity = this->evaluate_capacity();
			if( new_capacity > mCapacity )
			{
				char_type* newptr = reinterpret_cast<char_type*>( mAllocator.allocate(new_capacity) );
				Traits_Type::copy(newptr,mCPtr,pos );
				this->tidy_buffer();
				mCapacity = new_capacity;
				mPtr = newptr;
			}
			Traits_Type::copy(mPtr+pos,rhs.mCPtr,rhs.mSize+1);
			return *this;
		}

		/** @brief  */
		inline this_type& operator+=(const char_type* rhs)
		{
			if( rhs == NULL )
				return *this;

			size_t pos = mSize;
			size_t len = Traits_Type::length(rhs);
			mSize += len;
			size_t new_capacity = this->evaluate_capacity();
			if( new_capacity > mCapacity )
			{
				char_type* newptr = reinterpret_cast<char_type*>( mAllocator.allocate(new_capacity) );
				Traits_Type::copy(newptr,mCPtr,pos );
				this->tidy_buffer();
				mCapacity = new_capacity;
				mPtr = newptr;
			}
			Traits_Type::copy(mPtr+pos,rhs,len);
			*(mPtr+pos+len) = char_type();
			return *this;
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE const char_type&	operator[](index_t index) const
		{
#if BLADE_DEBUG
			if( !(index <= mSize) )
			{
				assert(false);
				return *Traits_Type::null();
			}
#endif
			return mCPtr[index];
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE char_type&			operator[](index_t index)
		{
			if( !(index <= mSize) )
			{
				assert(false);
				static char_type c = char_type();
				return c;
			}

			this->ensure_writable();

			return mPtr[index];
		}

		/** @brief  */
		inline this_type	substr(index_t offset = 0,index_t lenth = npos) const
		{
			if( lenth > mSize - offset )
				lenth = (size_t)(ptrdiff_t(mSize - offset) < 0 ? 0 : mSize - offset);

			if( lenth == 0 )
				return this_type(NULL);

			this_type subconst = this_type(mCPtr+offset, lenth, CONST_HINT);

			subconst.ensure_writable();
			return subconst;
		}

		/** @brief get substring without copy data buffer.use it with extreme care:
		if original string is destructed, the returned substring becomes invalid
		*/
		inline const this_type	substr_nocopy(index_t offset = 0, index_t lenth = npos) const
		{
			if( lenth > mSize - offset )
				lenth = (size_t)(ptrdiff_t(mSize - offset) < 0 ? 0 : mSize - offset);
			return lenth != 0 && mCPtr != NULL ? this_type(mCPtr+offset,lenth, CONST_HINT) : this_type(NULL, 0, CONST_HINT);
		}

		/** @brief  */
		inline index_t		find(const char_type* ptr,index_t offset,index_t count) const
		{
			//null string matches
			if( count == 0 && offset <= mSize )
				return offset;

			if( offset < mSize)
			{
				//number of elements left to find
				size_type left;
				left = mSize - offset;

				//if left count is lesser count,no possible match
				if( count <= left )
				{
					const char_type* buffer = mCPtr + offset;
					left -= count - 1;

					//find the first element
					const char_type* nextfind = Traits_Type::find(buffer, left, *ptr) + 1;

					while( nextfind != (const char_type*)(NULL) + 1 )
					{
						//we can find the first element,then try match the whole string
						const index_t leftcompare = count-1;
						const char_type* leftptr = ptr +1;

						if (Traits_Type::compare(nextfind, leftptr, leftcompare) == 0)
							return index_t(nextfind - mCPtr -1);	// found a match

						//recalculate elements left
						left -= nextfind - buffer;
						//trying start with the next element
						buffer = nextfind;
						nextfind = Traits_Type::find(buffer, left, *ptr) + 1;
					}
				}
			}

			return INVALID_INDEX;	// no match
		}

		/** @brief  */
		inline index_t		find(const char_type *ptr, index_t offset = 0) const
		{
			return this->find(ptr, offset, Traits_Type::length(ptr) );
		}

		/** @brief  */
		inline index_t		find(const this_type& sub, index_t offset = 0) const
		{
			return this->find(sub.c_str(), offset, sub.size() );
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE index_t		find(const char_type& ch, index_t offset = 0) const
		{
			if( offset >= mSize )
				return INVALID_INDEX;

			const char_type* chf = Traits_Type::find(mCPtr+offset,mSize-offset,ch);

			if( chf == NULL )
				return INVALID_INDEX;
			else
				return index_t(chf - mCPtr);
		}

		/** @brief  */
		inline index_t		rfind(const char_type* ptr,index_t offset,index_t count) const
		{
			if( /*mSize - offset < count ||*/count == 0)
				return INVALID_INDEX;

			const char_type* findstart;
			if( offset + count >= mSize || offset + count < count )
				findstart = mCPtr + mSize - count;
			else
				findstart = mCPtr + offset;
			index_t leftcompare = count -1;
			const char_type* leftptr = ptr + 1;
			for(; findstart >= mCPtr; --findstart )
			{
				if( *findstart == *ptr )
				{
					if( Traits_Type::compare(findstart+1,leftptr,leftcompare) == 0 )
						return (index_t)(findstart - mCPtr);
				}
			}
			return INVALID_INDEX;
		}

		/** @brief  */
		inline index_t		rfind(const char_type* ptr,index_t offset = npos ) const
		{
			return this->rfind(ptr,offset,Traits_Type::length(ptr) );
		}

		/** @brief  */
		inline index_t		rfind(const this_type& sub,index_t offset = npos) const
		{
			return this->rfind(sub.c_str(),offset,sub.size());
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE index_t		rfind(const char_type& ch,index_t offset = npos) const
		{
			if( offset >= mSize )
				offset = mSize - 1;

			for( index_t pos = offset; (int)pos >= 0; --pos)
			{
				if( mCPtr[pos] == ch )
					return pos;
			}
			return INVALID_INDEX;
		}

		/** @brief  */
		inline index_t		find_first_of(char_type ch,index_t offset) const
		{
			return this->find(ch,offset);
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE index_t		find_first_of(const char_type* chset,index_t offset,index_t count) const
		{
			if( count == 0 || offset >= mSize )
				return INVALID_INDEX;

			for( index_t pos = offset; pos <  mSize; ++pos )
			{
				const char_type& element = mCPtr[pos];
				if( Traits_Type::find(chset,count,element) != NULL )
					return pos;
			}
			return INVALID_INDEX;
		}

		/** @brief  */
		inline index_t		find_first_of(const char_type* chset,index_t offset = 0) const
		{
			return this->find_first_of(chset,offset,Traits_Type::length(chset) );
		}

		/** @brief  */
		inline index_t		find_first_of(const this_type& chset,index_t offset = 0 ) const
		{
			return this->find_first_of(chset.c_str(),offset,chset.size() );
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE index_t		find_last_of(const char_type* chset,index_t offset,index_t count) const
		{
			if( count == 0 )
				return INVALID_INDEX;

			if( offset >= mSize )
				offset = mSize - 1;

			for(index_t pos = offset; (int)pos >= 0; --pos )
			{
				const char_type& element = mCPtr[pos];
				if( Traits_Type::find(chset,count,element) != NULL )
					return pos;
			}
			return INVALID_INDEX;
		}

		/** @brief  */
		inline index_t		find_last_of(const char_type* chset,index_t offset = npos) const
		{
			return this->find_last_of(chset,offset,Traits_Type::length(chset) );
		}

		/** @brief  */
		inline index_t		find_last_of(const this_type& chset,index_t offset = npos) const
		{
			return this->find_last_of(chset.c_str(),offset,chset.size());
		}

		/** @brief  */
		inline index_t		find_last_of(const char_type& ch,index_t offset = npos) const
		{
			return this->rfind(ch,offset);
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE index_t		find_first_not_of(const char_type* chset,index_t offset,index_t count) const
		{
			if( count == 0 || offset >= mSize )
				return INVALID_INDEX;

			for( index_t pos = offset; pos <  mSize; ++pos )
			{
				const char_type& element = mCPtr[pos];
				if( Traits_Type::find(chset,count,element) == NULL )
					return pos;
			}
			return INVALID_INDEX;
		}

		/** @brief  */
		inline index_t		find_first_not_of(const char_type* chset,index_t offset = 0) const
		{
			return this->find_first_not_of(chset,offset,Traits_Type::length(chset) );
		}

		/** @brief  */
		inline index_t		find_first_not_of(const this_type& chset,index_t offset = 0 ) const
		{
			return this->find_first_not_of(chset.c_str(),offset,chset.size() );
		}

		/** @brief  */
		inline index_t		find_first_not_of(const char_type& ch,index_t offset = 0) const
		{
			return this->find_first_not_of(&ch,offset,1);
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE index_t		find_last_not_of(const char_type* chset,index_t offset,index_t count) const
		{
			if( count == 0 )
				return INVALID_INDEX;

			if( offset >= mSize )
				offset = mSize - 1;

			for(index_t pos = offset; (int)pos >= 0; --pos )
			{
				const char_type& element = mCPtr[pos];
				if( Traits_Type::find(chset,count,element) == NULL )
					return pos;
			}
			return INVALID_INDEX;
		}

		/** @brief  */
		inline index_t		find_last_not_of(const char_type* chset,index_t offset = npos) const
		{
			return this->find_last_not_of(chset,offset,Traits_Type::length(chset) );
		}

		/** @brief  */
		inline index_t		find_last_not_of(const this_type& chset,index_t offset = npos) const
		{
			return this->find_last_not_of(chset.c_str(),offset,chset.size());
		}

		/** @brief  */
		inline index_t		find_last_not_of(const char_type& ch,index_t offset = npos) const
		{
			return this->find_last_not_of(&ch,offset,1);
		}

		/** @brief  */
		template<typename predicate>
		BLADE_ALWAYS_INLINE index_t		find_if(predicate pred, index_t offset = 0, size_t count = npos) const
		{
			size_t max = (size_t)(ptrdiff_t(mSize - offset) < 0 ? 0 : mSize - offset);
			if (count > max)
				count = max;
			count += offset;

			for (index_t i = offset; i < count; ++i)
			{
				if (pred(mCPtr[i]))
					return i;
			}
			return INVALID_INDEX;
		}

		/** @brief  */
		template<typename predicate>
		BLADE_ALWAYS_INLINE index_t		find_if_not(predicate pred, index_t offset = 0, size_t count = npos) const
		{
			size_t max = (size_t)(ptrdiff_t(mSize - offset) < 0 ? 0 : mSize - offset);
			if (count > max)
				count = max;
			count += offset;

			for (index_t i = offset; i < count; ++i)
			{
				if (!pred(mCPtr[i]))
					return i;
			}
			return INVALID_INDEX;
		}

		/** @brief  */
		template<typename predicate>
		BLADE_ALWAYS_INLINE index_t		rfind_if(predicate pred, index_t offset = npos) const
		{
			if (offset >= mSize)
				offset = mSize - 1;

			for (indexdiff_t i = (indexdiff_t)offset; i >= 0; --i)
			{
				if (pred(mCPtr[i]))
					return i;
			}
			return INVALID_INDEX;
		}

		/** @brief  */
		template<typename predicate>
		BLADE_ALWAYS_INLINE index_t		rfind_if_not(predicate pred, index_t offset = npos) const
		{
			if (offset >= mSize)
				offset = mSize - 1;

			for (indexdiff_t i = (indexdiff_t)offset; i >= 0; --i)
			{
				if (!pred(mCPtr[i]))
					return i;
			}
			return INVALID_INDEX;
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE size_t		capacity() const
		{
			return mCapacity*sizeof(alloc_type);
		}


		/** @brief  */
		inline bool		empty() const
		{
			return this->size() == 0;
		}

		/** @brief  */
		inline size_type size(void) const
		{
			return mSize;
		}

		/** @brief  */
		inline size_type length(void) const
		{
			return mSize;
		}

		/** @brief  */
		inline const char_type* c_str(void) const
		{
			return mCPtr;
		}

		/** @brief  */
		inline const char_type* data(void) const
		{
			return mCPtr;
		}

		/** @brief  */
		inline void	resize(size_t n, char_type c)
		{
			if( mSize == n )
				return;

			size_t oldSize = mSize;
			mSize = n;
			bool append = mSize > oldSize;
			size_t capacity = this->evaluate_capacity();
			if( capacity != mCapacity )
			{
				char_type* newptr = reinterpret_cast<char_type*>( mAllocator.allocate(capacity) );
				size_t copySize = append ? oldSize : mSize;
				traits_type::copy(newptr, mPtr, copySize);
				this->tidy_buffer();
				mCapacity = capacity;
				mPtr = newptr;
			}
			if(append)
				traits_type::assign(mPtr+oldSize, mSize-oldSize, c);
			mPtr[mSize] = char_type();
		}

		/** @brief  */
		inline void	resize(size_t n)
		{
			return this->resize(n, char_type() );
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE void ensure_writable()
		{
			if (this->is_const())
			{
				//copy on write
				const char_type* constBuffer = mCPtr;
				size_t capacity = this->evaluate_capacity();
				if (capacity > mCapacity)
				{
					mCapacity = capacity;
					this->allocate_buffer();
				}
				Traits_Type::copy(mPtr, constBuffer, mSize);
				mPtr[mSize] = char_type();
			}
		}
		
		/** @brief make allocation on next copy */
		BLADE_ALWAYS_INLINE void make_const_temp()
		{
			if (this->is_const() && mConstHint == CONST_HINT)
				mConstHint = CONST_TEMP;
		}
	protected:

		/** @brief  */
		inline bool is_const() const
		{
			return mCapacity == 0; 
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE size_t evaluate_capacity()
		{
			return ( (mSize+1)*sizeof(char_type) + sizeof(alloc_type) -1 )/sizeof(alloc_type);
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE void allocate_buffer()
		{
			mPtr = reinterpret_cast<char_type*>( mAllocator.allocate( mCapacity ) );
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE void tidy_buffer()
		{
			if( !this->is_const() )
				mAllocator.deallocate( reinterpret_cast<typename Char_Alloc::pointer>(mPtr),mCapacity);
			mPtr = NULL;
		}

		union
		{
			const char_type*	mCPtr;
			char_type*			mPtr;
		};
		size_type	mSize;
		size_t		mCapacity;
		Char_Alloc	mAllocator;
		uint8		mConstHint;

		template<typename, typename, typename> friend class BasicString;
	public:
		/** @brief empty value */
		//add class to avoid weird conflicts (known form max SDK)
		static const this_type	EMPTY;

		//for more STL compatibility
		static	const index_t	npos;
	};//class BasicString<char_type,Traits_Type,Char_Alloc>

	/************************************************************************/
	/* default allocator                                                                     */
	/************************************************************************/
	struct StringAllocGranularity
	{
		char buffer[16];
	};
	class StringAllocator : public SizeAllocator< StringAllocGranularity >
	{
	public:
		static const size_t ALLOC_SIZE = sizeof(StringAllocGranularity);
	};

	/************************************************************************/
	/* Char_Traits template                                                                     */
	/************************************************************************/
	template< typename T >
	class Char_Traits
	{
	public:
		typedef void* char_type;
		typedef void* int_type;
		typedef void pos_type;
		typedef void off_type;
		typedef void state_type;
#if BLADE_STDCPP == BLADE_STDCPP98
		typedef const char_type& input_type;
		typedef const int_type& input_int_type;
#else
		typedef char_type		input_type;
		typedef int_type		input_int_type;
#endif

		/** @brief check if equal */
		static inline constexpr bool eq(input_type lhs,input_type rhs) noexcept
		{
			return lhs == rhs;
		}

		/** @brief check if left less than right */
		static inline constexpr bool lt(input_type lhs,input_type rhs) noexcept
		{
			return lhs < rhs;
		}

		/** @brief  */
		static inline constexpr int_type eof() noexcept
		{
			return 0;
		}

		/** @brief  */
		static inline constexpr int_type not_eof(input_int_type c) noexcept
		{
			return 0;
		}

		/** @brief  */
		static inline constexpr char_type to_char_type (input_int_type c) noexcept
		{
			return (char_type)c;
		}

		/** @brief  */
		static inline constexpr int_type to_int_type (input_type c) noexcept
		{
			return (int_type)c;
		}

		/** @brief  */
		static inline constexpr bool eq_int_type (input_int_type x, input_int_type y) noexcept
		{
			return x == y;
		}

		/** @brief assign an element */
		static inline void assign(char_type& lhs,const char_type& rhs) noexcept
		{
			lhs = rhs;
		}

		/** @brief assign size * ch to [ch_seq, ...) */
		static inline char_type* assign(char_type *ch_seq,
			size_t size, char_type ch)
		{
			char_type *_Next = ch_seq;
			for (; 0 < size; --size, ++_Next)
				assign(*_Next, ch);
			return (ch_seq);
		}

		/** @brief  compare [lhs, lhs + size) with [rhs, ...) */
		static inline int compare(const char_type *lhs,const char_type *rhs, size_t size)
		{
			for (; 0 < size; --size, ++lhs, ++rhs)
				if (!eq(*lhs, *rhs))
					return (lt(*lhs, *rhs) ? -1 : +1);
			return 0;
		}

		/** @brief copy [lhs, lhs + size) to [rhs, ...) */
		static inline char_type* copy(char_type *lhs,const char_type *rhs, size_t size)
		{
			char_type *next = lhs;
			for (; 0 < size; --size, ++next, ++rhs)
				assign(*next, *rhs);
			return lhs;
		}

		/** @brief  look for ch in [ch_seq, ch_seq + size) */
		static inline const char_type* find(const char_type *ch_seq,size_t size, const char_type& ch)
		{
			for (; 0 < size; --size, ++ch_seq)
				if (eq(*ch_seq, ch))
					return (ch_seq);
			return NULL;
		}

		/** @brief find length of null-terminated sequence */
		static inline size_t length(const char_type* ch_seq)
		{
			size_t size;
			for (size = 0; !eq(*ch_seq, char_type()); ++ch_seq)
				++size;
			return size;
		}

		/** @brief move [lhs, lhs + size) to [rhs, ...) */
		static inline char_type* move(char_type *lhs,const char_type *rhs, size_t size)
		{
			char_type *next = lhs;
			if (rhs < next && next < rhs + size)
				for (next += size, rhs += size; 0 < size; --size)
					assign(*--next, *--rhs);
			else
				for (; 0 < size; --size, ++next, ++rhs)
					assign(*next, *rhs);
			return NULL;
		}

		//none-std extension
		typedef void concat_type;

		/** @brief  */
		static inline bool	is_alpha(const char_type c)
		{
			return false;
		}

		/** @brief  */
		static inline int32	to_i32(const char_type* sstart, char_type** send, int base)
		{
			return 0;
		}

		/** @brief  */
		static inline uint32 to_u32(const char_type* sstart, char_type** send, int base)
		{
			return 0U;
		}

		/** @brief  */
		static inline int64 to_i64(const char_type* sstart, char_type** send, int base)
		{
			return int64(0);
		}

		/** @brief  */
		static inline uint64 to_u64(const char_type* sstart, char_type** send, int base)
		{
			return uint64(0);
		}

		/** @brief  */
		static inline double to_double(const char_type* sstart, char_type** send)
		{
			return 0.0;
		}

		/** @brief  */
		static inline float	to_float(const char_type* sstart, char_type** send)
		{
			return 0.0f;
		}

		/** @brief  */
		static inline int	from_i32(char_type* dest, size_t count, int32 v)
		{
			return 0;
		}

		/** @brief  */
		static inline int from_u32(char_type* dest, size_t count, uint32 v)
		{
			return 0;
		}

		/** @brief  */
		static inline int from_i64(char_type* dest, size_t count, int64 v)
		{
			return 0;
		}

		/** @brief  */
		static inline int from_u64(char_type* dest, size_t count, uint64 v)
		{
			return 0;
		}

		/** @brief  */
		static inline int from_double(char_type* dest, size_t count, double v)
		{
			return 0;
		}

		/** @brief  */
		static inline int	from_float(char_type* dest, size_t count, float v)
		{
			return 0;
		}

		/** @brief  */
		static inline const char_type* null() noexcept
		{
			static char_type _null[1];
			return _null;
		}

		/** @brief  */
		static inline int compare_no_case(const char_type* lhs, size_t ls, const char_type* rhs, size_t rs)
		{
			return 0;
		}

		/** @brief  */
		static inline char_type* toupper(char_type* src)
		{
			return src;
		}

		/** @brief  */
		static inline char_type* tolower(char_type* src)
		{
			return src;
		}

	private:
		Char_Traits() {};
		~Char_Traits() {};
	};//class Char_Traits<char_type>

	class StringConcat;
	class WStringConcat;

	template<> class Char_Traits<char>
	{
	public:
		//std compatible typedefs
		typedef char char_type;
		typedef int int_type;
#if BLADE_STDCPP == BLADE_STDCPP98
		typedef const char_type& input_type;
		typedef const int_type& input_int_type;
#else
		typedef char_type		input_type;
		typedef int_type		input_int_type;
#endif
		//none-std extension
		typedef StringConcat concat_type;

		/** @brief check if equal */ 
		static inline constexpr bool eq(input_type lhs,input_type rhs) noexcept
		{
			return lhs == rhs;
		}

		/** @brief check if left less than right */
		static inline constexpr bool lt(input_type lhs, input_type rhs) noexcept
		{
			return (unsigned char)lhs < (unsigned char)rhs;
		}

		/** @brief  */
		static inline constexpr int_type eof() noexcept
		{
			return EOF;
		}

		/** @brief  */
		static inline constexpr int_type not_eof(input_int_type c) noexcept
		{
			return (c != eof() ? c : !c );
		}

		/** @brief  */
		static inline constexpr char_type to_char_type(input_int_type c) noexcept
		{
			return (char_type)c;
		}

		/** @brief  */
		static inline constexpr int_type to_int_type (input_type c) noexcept
		{
			return (unsigned char)c;
		}

		/** @brief  */
		static constexpr bool eq_int_type (input_int_type x, input_int_type y) noexcept
		{
			return x == y;
		}

		/** @brief assign an element */
		static inline void assign(char_type& lhs,const char_type& rhs) noexcept
		{
			lhs = rhs;
		}

		/** @brief assign size * ch to [ch_seq, ...) */
		static inline char_type* assign(char_type *ch_seq, size_t size, char_type ch)
		{
			return (char_type *)std::memset(ch_seq, ch, size);
		}

		/** @brief compare [lhs, lhs + size) with [rhs, ...) */
		static inline int compare(const char_type *lhs, const char_type *rhs,size_t size)
		{
			return std::memcmp(lhs, rhs, size);
		}

		/** @brief copy [lhs, lhs + size) to [rhs, ...) */
		static inline char_type* copy(char_type *lhs,const char_type *rhs,size_t size)
		{
			return (char_type*)std::memcpy(lhs,rhs, size);
		}

		/** @brief look for ch in [ch_seq, ch_seq + size) */
		static inline const char_type* find(const char_type *ch_seq, size_t size,const char_type& ch)
		{
			return (const char_type *)std::memchr(ch_seq, ch, size);
		}

		/** @brief find length of null-terminated String */
		static inline size_t length(const char_type *ch_seq)
		{
			return std::strlen(ch_seq);
		}

		/** @brief move [lhs, lhs + size) to [rhs, ...) */
		static inline char_type* move(char_type *lhs, const char_type *rhs,
			size_t size)
		{
			return (char_type*)std::memmove(lhs,rhs, size);
		}

		/** @brief  */
		static inline bool	is_alpha(const char_type c)
		{
			return std::isalpha((int)c) != 0;
		}

		/** @brief  */
		static inline int32	to_i32(const char_type* sstart, char_type** send, int base)
		{
			return (int)std::strtol(sstart, send, base);
		}

		/** @brief  */
		static inline uint32 to_u32(const char_type* sstart, char_type** send, int base)
		{
			return (uint)std::strtoul(sstart, send, base);;
		}

		/** @brief  */
		static inline int64 to_i64(const char_type* sstart, char_type** send, int base)
		{
			return (int64)strtoll(sstart, send, base);
		}

		/** @brief  */
		static inline uint64 to_u64(const char_type* sstart, char_type** send, int base)
		{
			return (uint64)strtoull(sstart, send, base);
		}

		/** @brief  */
		static inline double to_double(const char_type* sstart, char_type** send)
		{
			return std::strtod(sstart, send);
		}

		/** @brief  */
		static inline float to_float(const char_type* sstart, char_type** send)
		{
			return (float)std::strtod(sstart, send);
		}

		/** @brief  */
		static inline int	from_i32(char_type* dest, size_t count, int32 v)
		{
			return snprintf(dest, count, "%d", v);
		}

		/** @brief  */
		static inline int from_u32(char_type* dest, size_t count, uint32 v)
		{
			return snprintf(dest, count, "%u", v);
		}

		/** @brief  */
		static inline int from_i64(char_type* dest, size_t count, int64 v)
		{
			return snprintf(dest, count, "%lld", v);
		}

		/** @brief  */
		static inline int from_u64(char_type* dest, size_t count, uint64 v)
		{
			return snprintf(dest, count, "%llu", v);
		}

		/** @brief  */
		static inline int from_double(char_type* dest, size_t count, double v)
		{
			return snprintf(dest, count, "%.3f", v);
		}

		/** @brief  */
		static inline int	from_float(char_type* dest, size_t count, float v)
		{
			return snprintf(dest, count, "%.3f", v);
		}

		/** @brief  */
		static inline constexpr const char_type* null() noexcept
		{
			return "";
		}

		/** @brief  */
		static int inline compare_no_case(const char_type* lhs, size_t ls, const char_type* rhs, size_t rs)
		{
#define towlowerASCII(c) ((('A' <= (c))&&((c) <= L'Z')) ? ((c) - L'A' + L'a') : (c))
			assert(lhs != NULL && rhs != NULL);
			int cmp;
			while ((cmp = towlowerASCII(*lhs) - towlowerASCII(*rhs)) == 0)
			{
				if (0 == *lhs)
					return 0;

				--ls;
				--rs;
				if (ls == 0)
					return int(ls - rs);
				if (rs == 0)
					return int(rs - ls);

				++lhs;
				++rhs;
			}
			return cmp;
#undef towlowerASCII
		}

		/** @brief  */
		static inline char_type* toupper(char_type* src)
		{
			char_type* dest = src;
			while(dest != NULL && *dest != '\0' )
			{
				*dest = (char_type)std::toupper(*dest);
				++dest;
			}
			return src;
		}

		/** @brief  */
		static inline char_type* tolower(char_type* src)
		{
			char_type* dest = src;
			while(dest != NULL && *dest != '\0' )
			{
				*dest = (char_type)std::tolower(*dest);
				++dest;
			}
			return src;
		}

	private:
		Char_Traits() {};
		~Char_Traits<char>() {};
	};//class Char_Traits<char>


	template<> class Char_Traits<wchar_t>
	{
	public:
		typedef wchar_t char_type;
		typedef std::wint_t int_type;
#if BLADE_STDCPP == BLADE_STDCPP98
		typedef const char_type& input_type;
		typedef const int_type& input_int_type;
#else
		typedef char_type		input_type;
		typedef int_type		input_int_type;
#endif
		//none-std extension
		typedef WStringConcat concat_type;

		/** @brief check if equal */
		static inline constexpr bool eq(input_type lhs,input_type rhs) noexcept
		{
			return lhs == rhs;
		}

		/** @brief check if left less than right */
		static inline constexpr bool lt(input_type lhs,input_type rhs) noexcept
		{
			return lhs < rhs;
		}

		/** @brief  */
		static inline constexpr int_type eof() noexcept
		{
			return WEOF;
		}

		/** @brief  */
		static inline constexpr int_type not_eof(input_int_type c) noexcept
		{
			return (c != eof() ? c : (int_type)!c );
		}

		/** @brief  */
		static inline constexpr char_type to_char_type (input_int_type c) noexcept
		{
			return c;
		}

		/** @brief  */
		static inline constexpr int_type to_int_type (input_type c) noexcept
		{
			return c;
		}

		/** @brief  */
		static inline constexpr bool eq_int_type (input_int_type x, input_int_type y) noexcept
		{
			return x == y;
		}

		/** @brief assign size * ch to [ch_seq, ...) */
		static inline char_type* assign(char_type *ch_seq, size_t size, char_type ch)
		{
			return ((char_type *)std::wmemset(ch_seq, ch, size));
		}

		/** @brief assign an element */
		static inline void assign(char_type& lhs,const char_type& rhs) noexcept
		{
			lhs = rhs;
		}

		/** @brief compare [lhs, lhs + size) with [rhs, ...) */
		static inline int compare(const char_type *lhs, const char_type *rhs,size_t size)
		{
			return std::wmemcmp(lhs, rhs, size);
		}

		/** @brief copy [lhs, lhs + size) to [rhs, ...) */
		static inline char_type* copy(char_type *lhs,const char_type *rhs,size_t size)
		{
			return (char_type*)std::wmemcpy(lhs,rhs, size);
		}

		/** @brief look for ch in [ch_seq, ch_seq + size) */
		static inline const char_type* find(const char_type *ch_seq, size_t size,const char_type& ch)
		{
			return ((const char_type *)std::wmemchr(ch_seq, ch, size));
		}

		/** @brief find length of null-terminated String */
		static inline size_t length(const char_type *ch_seq)
		{
			return std::wcslen(ch_seq);
		}

		/** @brief move [lhs, lhs + size) to [rhs, ...) */
		static inline char_type* move(char_type *lhs,const char_type *rhs,
			size_t size)
		{
			return (char_type*)std::wmemmove(lhs, rhs, size);
		}

		/** @brief  */
		static inline bool	is_alpha(const char_type c)
		{
			return std::iswalpha((wint_t)c) != 0;
		}

		/** @brief  */
		static inline int32	to_i32(const char_type* sstart, char_type** send, int base)
		{
			return (int)std::wcstol(sstart, send, base);
		}

		/** @brief  */
		static inline uint32 to_u32(const char_type* sstart, char_type** send, int base)
		{
			return (uint)std::wcstoul(sstart, send, base);;
		}

		/** @brief  */
		static inline int64 to_i64(const char_type* sstart, char_type** send, int base)
		{
			return (int64)wcstoll(sstart, send, base);
		}

		/** @brief  */
		static inline uint64 to_u64(const char_type* sstart, char_type** send, int base)
		{
			return (uint64)wcstoull(sstart, send, base);
		}

		/** @brief  */
		static inline double to_double(const char_type* sstart, char_type** send)
		{
			return std::wcstod(sstart, send);
		}

		/** @brief  */
		static inline float to_float(const char_type* sstart, char_type** send)
		{
			return (float)std::wcstod(sstart, send);
		}

		/** @brief  */
		static inline int	from_i32(char_type* dest, size_t count, int32 v)
		{
			return snwprintf(dest, count, L"%d", v);
		}

		/** @brief  */
		static inline int from_u32(char_type* dest, size_t count, uint32 v)
		{
			return snwprintf(dest, count, L"%u", v);
		}

		/** @brief  */
		static inline int from_i64(char_type* dest, size_t count, int64 v)
		{
			return snwprintf(dest, count, L"%lld", v);
		}

		/** @brief  */
		static inline int from_u64(char_type* dest, size_t count, uint64 v)
		{
			return snwprintf(dest, count, L"%llu", v);
		}

		/** @brief  */
		static inline int from_double(char_type* dest, size_t count, double v)
		{
			return snwprintf(dest, count, L"%.3f", v);
		}

		/** @brief  */
		static inline int	from_float(char_type* dest, size_t count, float v)
		{
			return snwprintf(dest, count, L"%.3f", v);
		}

		/** @brief  */
		static inline constexpr const char_type* null() noexcept
		{
			return L"";
		}

		/** @brief  */
		static inline int compare_no_case(const char_type* lhs, size_t ls, const char_type* rhs, size_t rs)
		{
#define towlowerASCII(wc) (((L'A' <= (wc))&&((wc) <= L'Z')) ? ((wc) - L'A' + L'a') : (wc))
			assert(lhs != NULL && rhs != NULL);
			int cmp;
			while ((cmp = towlowerASCII(*lhs) - towlowerASCII(*rhs)) == 0)
			{
				if (0 == *lhs)
					return 0;

				--ls;
				--rs;
				if (ls == 0)
					return int(ls - rs);
				if (rs == 0)
					return int(ls - rs);

				++lhs;
				++rhs;
			}
			return cmp;
#undef towlowerASCII
		}

		/** @brief  */
		static inline char_type* toupper(char_type* src)
		{
			char_type* dest = src;
			while(dest != NULL && *dest != '\0' )
			{
				*dest = std::towupper(*dest);
				++dest;
			}
			return src;
		}

		/** @brief  */
		static inline char_type* tolower(char_type* src)
		{
			char_type* dest = src;
			while(dest != NULL && *dest != '\0' )
			{
				*dest = std::towlower(*dest);
				++dest;
			}
			return src;
		}

	private:
		Char_Traits()	{};
		~Char_Traits<wchar_t>() {};
	};//class Char_Traits<wchar_t>

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(pop)
#endif

	//static member definition
	template <typename char_type,typename Traits_Type,typename Char_Alloc>
	const BasicString<char_type,Traits_Type,Char_Alloc>
		BasicString<char_type,Traits_Type,Char_Alloc>::EMPTY;

	template <typename char_type,typename Traits_Type,typename Char_Alloc>
	const index_t
		BasicString<char_type,Traits_Type,Char_Alloc>::npos = INVALID_INDEX;

	typedef BasicString<char,		Char_Traits<char>,		StringAllocator>	String;
	typedef BasicString<wchar_t,	Char_Traits<wchar_t>,	StringAllocator>	WString;

	/*
	extern template class BLADE_BASE_API BasicString<char,		Char_Traits<char>,		StringAllocator>;
	extern template class BLADE_BASE_API BasicString<wchar_t,	Char_Traits<wchar_t>,	StringAllocator>;
	*/

	namespace Impl
	{
		class StringConcatImpl;
		class WStringConcatImpl;
		///add wrapper to resolve ambiguous overload, 
		///i.e. ParamValue::operator int() wil match operator+(char)
		///but ParamValue::operator const TString&() will also match operator(const TString&)
		///that is OK for MSVC but according to the C++ standard, that will cause ambiguous overload.
		class Char
		{
		public:
			Char(char c) :mChar(c) {}
			operator char() const  {return mChar;}
		protected:
			char	mChar;
		};
		class WChar
		{
		public:
			WChar(wchar wc) :mWChar(wc) {}
			operator wchar() const  {return mWChar;}
		protected:
			wchar	mWChar;
		};
	}

	/************************************************************************/
	/** concatenate types (operator+ proxy)                                                                    */
	/** simple helper for string operator +
	usually you don't need to explicitly use it, TString str = a + b + c will auto match it.

	TStringConcat helper = TStringHelper::fromInt(...) + ...;	
	where TStringHelper::fromInt() is an temporary object
	THIS IS OK according to C++03. (12.2.3)

	"When an implementation introduces a temporary object of a class that has a non-trivial constructor (12.1), 
	it shall ensure that a constructor is called for the temporary object. Similarly, the destructor shall be called for a temporary with a non-trivial destructor (12.4). 
	Temporary objects are destroyed as the last step in evaluating the full-expression (1.9) that (lexically) contains the point where they were created. 
	This is true even if that evaluation ends in throwing an exception."
	
	But it may have corrupted data for some compiler not conformed to the standard.(i.e. older MSVC,VS2010)
	and it will be destructed after this statement, making object 'helper' holding invalid data.

	note: use TStringConcat across scope may have invalid input because temporary objects will destruct out side scope.
	TStringConcat helper;
	for(...)
	{
		helper += TStringHelper::fromInt(...)  + ...;	//this is incorrect.
	}
	*/
	/************************************************************************/
	class BLADE_BASE_API StringConcat : public NonAllocatable
	{
	public:
		typedef String string_type;
		typedef string_type::traits_type traits_type;
		typedef StringConcat concat_type;

		StringConcat(const char* origin);

		StringConcat(const string_type& origin);

		StringConcat& operator+(const string_type& rhs);

		StringConcat& operator+(const char* rhs);

		StringConcat& operator+(const Impl::Char rhs);

		StringConcat operator+(const StringConcat& rhs);

		StringConcat& operator+=(const string_type& rhs);

		StringConcat& operator+=(const char* rhs);

		StringConcat& operator+=(const Impl::Char rhs);

		BLADE_BASE_API friend StringConcat operator+(const Impl::Char lhs, const StringConcat& rhs);

		StringConcat& operator+=(const StringConcat& rhs);

		StringConcat(const StringConcat& src);

		~StringConcat();

		/** @brief sub object count */
		size_t	size() const;

		/** @brief total size in bytes */
		size_t	total_bytes() const;

		/** @brief  */
		const string_type& at(index_t index) const;

		/** @brief  */
		inline bool operator==(const string_type& rhs) const
		{
			string_type lhs(*this);
			return lhs == rhs;
		}

	private:
		Pimpl<Impl::StringConcatImpl>	mImpl;
	};//class StringConcat

	inline StringConcat operator+(const String& lhs,const String& rhs)
	{
		return StringConcat(lhs) + rhs;
	}
	inline StringConcat operator+(const String& lhs,const char* rhs)
	{
		return StringConcat(lhs) + rhs;
	}
	inline StringConcat operator+(const char* lhs,const String& rhs)
	{
		return StringConcat(lhs)+rhs;
	}
	inline StringConcat operator+(const String& lhs, const Impl::Char rhs)
	{
		return StringConcat(lhs) + rhs;
	}
	inline StringConcat operator+(const Impl::Char lhs, const String& rhs)
	{
		return lhs + StringConcat(rhs);
	}
	inline bool operator==(const String& lhs, const StringConcat& rhs)
	{
		return rhs == lhs;
	}


	class BLADE_BASE_API WStringConcat : public NonAllocatable
	{
	public:
		typedef WString string_type;
		typedef string_type::traits_type traits_type;
		typedef WStringConcat concat_type;

		WStringConcat(const wchar* origin);

		WStringConcat(const string_type& origin);

		WStringConcat& operator+(const string_type& rhs);

		WStringConcat& operator+(const wchar* rhs);

		WStringConcat& operator+(const Impl::WChar rhs);

		WStringConcat operator+(const WStringConcat& rhs);

		WStringConcat& operator+=(const string_type& rhs);

		WStringConcat& operator+=(const wchar* rhs);

		WStringConcat& operator+=(const Impl::WChar rhs);

		BLADE_BASE_API friend WStringConcat operator+(const Impl::WChar lhs, const WStringConcat& rhs);

		WStringConcat& operator+=(const WStringConcat& rhs);

		WStringConcat(const WStringConcat& src);

		~WStringConcat();

		/** @brief sub object count */
		size_t	size() const;

		/** @brief total size in bytes */
		size_t	total_bytes() const;

		/** @brief  */
		const string_type& at(index_t index) const;

		/** @brief  */
		inline bool operator==(const string_type& rhs) const
		{
			string_type lhs(*this);
			return lhs == rhs;
		}

	private:
		Pimpl<Impl::WStringConcatImpl>	mImpl;
	};//class WStringConcat

	inline WStringConcat operator+(const WString& lhs,const WString& rhs)
	{
		return WStringConcat(lhs) + rhs;
	}
	inline WStringConcat operator+(const WString& lhs,const wchar* rhs)
	{
		return WStringConcat(lhs) + rhs;
	}
	inline WStringConcat operator+(const wchar* lhs,const WString& rhs)
	{
		return WStringConcat(lhs) + rhs;
	}
	inline WStringConcat operator+(const WString& lhs, const Impl::WChar rhs)
	{
		return WStringConcat(lhs) + rhs;
	}
	inline WStringConcat operator+(const Impl::WChar lhs, const WString& rhs)
	{
		return lhs + WStringConcat(rhs);
	}
	inline bool operator==(const WString& lhs, const WStringConcat& rhs)
	{
		return rhs == lhs;
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	///used for utility::Basic_String constructor(source,source_size) optimization
#define BTString(_naked_str)	Blade::TString(TEXT(_naked_str),	sizeof(TEXT(_naked_str))/sizeof(Blade::tchar)-1,	Blade::TString::STATIC_CONST)
#define BString(_naked_str)		Blade::String(_naked_str,			sizeof(_naked_str)/sizeof(char)-1,					Blade::String::STATIC_CONST)
#define BWString(_naked_str)	Blade::WString(L##_naked_str,		sizeof(L##_naked_str)/sizeof(wchar_t)-1,			Blade::WString::STATIC_CONST)

	///construction from a const-tchar variable,this is barely used,
	///and be care that the _tvar's real size is known by compiler,that means
	///_tvar MUST NOT have been transferred as a function's argument,otherwise sizeof(_tvar) is just sizeof(tchar*)
#define BVString(_tvar)			Blade::String(_tvar ,				sizeof(_tvar)/sizeof(char)-1,						Blade::String::STATIC_CONST)
#define BVWString(_tvar)		Blade::WString(_tvar ,				sizeof(_tvar)/sizeof(wchar)-1,						Blade::WString::STATIC_CONST)
#define BVTString(_tvar)		Blade::TString(_tvar ,				sizeof(_tvar)/sizeof(Blade::tchar)-1,				Blade::TString::STATIC_CONST)

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	template<typename Char_Type,typename Traits_Type,typename Char_Alloc>
	inline BasicString<Char_Type, Traits_Type, Char_Alloc>::BasicString(const concat_type& src)
		:mSize(0)
	{
		mSize = src.total_bytes();

		if( mSize == 0 )
		{
			mCPtr = Traits_Type::null();
			mCapacity = 0;
			return;
		}

		mCapacity = this->evaluate_capacity();
		this->allocate_buffer();

		char_type* buffer = mPtr;
		for( size_t i = 0; i < src.size(); ++i)
		{
			const char_type* srcBuffer = src.at(i).c_str();
			const size_t srcSize = src.at(i).size();
			Traits_Type::copy(buffer, srcBuffer, srcSize);
			buffer += srcSize;
		}
		mPtr[mSize] = char_type();
	}

	//////////////////////////////////////////////////////////////////////////
	template<typename Char_Type,typename Traits_Type,typename Char_Alloc>
	inline BasicString<Char_Type, Traits_Type, Char_Alloc>&
		BasicString<Char_Type, Traits_Type, Char_Alloc>::operator=(const concat_type& rhs)
	{
		size_t totalSize = rhs.total_bytes();

		if( totalSize == 0 || rhs.size() == 0)
		{
			*this = EMPTY;
			return *this;
		}

		mSize = totalSize;
		size_t capcacity = this->evaluate_capacity();

		//bug!: if this contained is rhs ( this = this + "A" + B + ... )
		//tidy buffer may cause rhs data element invalid.
		//tidy_buffer();
		//mCapacity = capcacity;
		//this->allocate_buffer();
		char_type* buffer = reinterpret_cast<char_type*>( mAllocator.allocate( capcacity ) );

		{
			char_type* data = buffer;
			for( size_t i = 0; i < rhs.size(); ++i)
			{
				const char_type* srcBuffer = rhs.at(i).c_str();
				const size_t srcSize = rhs.at(i).size();
				Traits_Type::copy(data, srcBuffer, srcSize);
				data += srcSize;
			}
		}

		buffer[mSize] = char_type();
		this->tidy_buffer();
		mCapacity = capcacity;
		mPtr = buffer;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	template<typename Char_Type,typename Traits_Type,typename Char_Alloc>
	inline BasicString<Char_Type, Traits_Type, Char_Alloc>&
		BasicString<Char_Type, Traits_Type, Char_Alloc>::operator+=(const concat_type& rhs)
	{
		size_t totalSize = rhs.total_bytes();

		if( totalSize == 0 )
			return *this;

		size_t oldSize = mSize;
		mSize += totalSize;
		size_t new_capacity = this->evaluate_capacity();
		char_type* newptr = reinterpret_cast<char_type*>( mAllocator.allocate(new_capacity) );
		char_type* oldptr = mPtr;
		size_t old_capacity = mCapacity;
		Traits_Type::copy(newptr,mCPtr,oldSize);

		mCapacity = new_capacity;
		mPtr = newptr;

		char_type* buffer = mPtr+oldSize;
		for( size_t i = 0; i < rhs.size(); ++i)
		{
			const char_type* srcBuffer = rhs.at(i).c_str();
			const size_t srcSize = rhs.at(i).size();
			Traits_Type::copy(buffer, srcBuffer, srcSize);
			buffer += srcSize;
		}

		mPtr[mSize] = char_type();

		if( old_capacity != 0 )
			mAllocator.deallocate( reinterpret_cast<typename Char_Alloc::pointer>(oldptr),old_capacity);
		return *this;
	}

	inline bool operator==(const char* lhs,const String& rhs)
	{
		return rhs == lhs;
	}

	inline bool operator==(const wchar* lhs,const WString& rhs)
	{
		return rhs == lhs;
	}

	inline bool operator!=(const char* lhs,const String& rhs)
	{
		return !(rhs == lhs);
	}

	inline bool operator!=(const wchar* lhs,const WString& rhs)
	{
		return !(rhs == lhs);
	}


	inline bool operator<(const char* lhs,const String& rhs)
	{
		return rhs > lhs;
	}

	inline bool operator<(const wchar* lhs,const WString& rhs)
	{
		return rhs > lhs;
	}

	inline bool operator>(const char* lhs,const String& rhs)
	{
		return rhs < lhs;
	}

	inline bool operator>(const wchar* lhs,const WString& rhs)
	{
		return rhs < lhs;
	}


	/************************************************************************/
	/* string fast comparator                                                                     */
	/************************************************************************/
	/* @note this is faster but doesn't sort in alphabet order.*/
	template<typename char_type, typename AllocT>
	struct FnBasicStringFastLess
	{
		typedef BasicString<char_type,Char_Traits<char_type>, AllocT> string_type;

		inline bool operator()(const string_type& _Left,const string_type& _Right ) const
		{
			if( _Left.size() == _Right.size() )
				return _Left.compare(_Right) < 0;
			else return( _Left.size() < _Right.size() );
		}
	};

	struct FnStringFastLess
	{
		inline bool operator()(const String& _Left,const String& _Right) const
		{
			return FnStringFastLess::compare(_Left, _Right);
		}

		static inline bool compare(const String& _Left,const String& _Right)
		{
			if( _Left.size() == _Right.size() )
				return _Left.compare(_Right) < 0;
			else return( _Left.size() < _Right.size() );
		}

		static inline int icompare(const String& _Left, const String& _Right)
		{
			if (_Left.size() == _Right.size())
				return _Left.compare(_Right);
			else return int(_Left.size() - _Right.size());
		}
	};


	struct FnWStringFastLess
	{
		inline bool operator()(const WString& _Left,const WString& _Right) const
		{
			return FnWStringFastLess::compare(_Left, _Right);
		}

		static inline bool compare(const WString& _Left,const WString& _Right)
		{
			if( _Left.size() == _Right.size() )
				return _Left.compare(_Right) < 0;
			else return( _Left.size() < _Right.size() );
		}

		static inline int icompare(const WString& _Left, const WString& _Right)
		{
			if (_Left.size() == _Right.size())
				return _Left.compare(_Right);
			else return int(_Left.size() - _Right.size());
		}
	};

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#	pragma warning(push)
#	pragma warning(disable:4996)	//'stricmp': The POSIX name for this item is deprecated. 
#endif
	struct FnStringAlphabetNoCaseLess
	{
		bool operator()(const String& lhs, const String& rhs) const
		{
			return compare(lhs, rhs);
		}

		static inline bool compare(const String& lhs, const String& rhs)
		{
			return lhs.compare_no_case(rhs) < 0;
		}
	};

	struct FnWStringAlphabetNoCaseLess
	{
		bool operator()(const WString& lhs, const WString& rhs) const
		{
			return compare(lhs, rhs);
		}

		static inline bool compare(const WString& lhs, const WString& rhs)
		{
			return lhs.compare_no_case(rhs) < 0;
		}
	};
#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#	pragma warning(pop)
#endif


	/************************************************************************/
	/* naked string cmp                                                                     */
	/************************************************************************/
	struct FnCharAlphabetLess
	{
		bool operator()(const char* lhs, const char* rhs) const
		{
			return compare(lhs, rhs);
		}

		static inline bool compare(const char* lhs, const char* rhs)
		{
			return ::strcmp(lhs, rhs) < 0;
		}
	};

	struct FnWCharAlphabetLess
	{
		bool operator()(const wchar* lhs, const wchar* rhs) const
		{
			return compare(lhs, rhs);
		}

		static inline bool compare(const wchar* lhs, const wchar* rhs)
		{
			return ::wcscmp(lhs, rhs) < 0;
		}
	};

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#	pragma warning(push)
#	pragma warning(disable:4996)	//'stricmp': The POSIX name for this item is deprecated. 
#endif
	struct FnCharAlphabetNoCaseLess
	{
		bool operator()(const char* lhs, const char* rhs) const
		{
			return compare(lhs, rhs);
		}

		static inline bool compare(const char* lhs, const char* rhs)
		{
			return strcasecmp(lhs, rhs) < 0;
		}
	};

	struct FnWCharAlphabetNoCaseLess
	{
		bool operator()(const wchar* lhs, const wchar* rhs) const
		{
			return compare(lhs, rhs);
		}

		static inline bool compare(const wchar* lhs, const wchar* rhs)
		{
			return wcscasecmp(lhs, rhs) < 0;
		}
	};
#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#	pragma warning(pop)
#endif

#if BLADE_UNICODE
	typedef WString TString;
	typedef WStringConcat TStringConcat;
	typedef Char_Traits<wchar> TCharTraits;

	typedef FnWStringFastLess FnTStringFastLess;
	typedef FnWStringAlphabetNoCaseLess FnTStringNCLess;

	typedef FnWCharAlphabetLess FnTCharLess;
	typedef FnWCharAlphabetNoCaseLess FnTCharNCLess;
#else
	typedef StringConcat TStringConcat;
	typedef String TString;
	typedef Char_Traits<char> TCharTraits;

	typedef FnStringFastLess FnTStringFastLess;
	typedef FnStringAlphabetNoCaseLess FnTStringNCLess;

	typedef FnCharAlphabetLess FnTCharLess;
	typedef FnCharAlphabetNoCaseLess FnTCharNCLess;
#endif

	/************************************************************************/
	/* string with temporary allocation, used internally                                                                    */
	/************************************************************************/
	class TempStringAllocator : public TempAllocator< StringAllocGranularity >
	{
	public:
		static const size_t ALLOC_SIZE = sizeof(StringAllocGranularity);
	};
	typedef BasicString<char, Char_Traits<char>, TempStringAllocator >			TmpString;
	typedef BasicString<wchar_t, Char_Traits<wchar_t>, TempStringAllocator >	TmpWString;
	typedef BasicString<tchar, Char_Traits<tchar>, TempStringAllocator >		TmpTString;

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	namespace StringConverter
	{
		BLADE_BASE_API WString		StringToWString(const char* mbcString, size_t len = 0);
		inline WString				StringToWString(const String& mbString)
		{
			return StringToWString(mbString.c_str(), mbString.size());
		}

		BLADE_BASE_API String		WStringToString(const wchar* wcString, size_t len = 0);
		inline String				WStringToString(const WString& wString)
		{
			return WStringToString(wString.c_str(), wString.size());
		}

		inline String		TStringToString(const tchar* tcString, size_t len = 0)
		{
#if BLADE_UNICODE
			return WStringToString(tcString, len);
#else
			BLADE_UNREFERENCED(len);
			return String(tcString);
#endif
		}
		inline String				TStringToString(const TString& tString)
		{
#if BLADE_UNICODE
			return WStringToString(tString.c_str(), tString.size());
#else
			return tString;
#endif
		}

		inline WString		TStringToWString(const tchar* tcString, size_t len = 0)
		{
#if BLADE_UNICODE
			BLADE_UNREFERENCED(len);
			return WString(tcString);
#else
			return StringToWString(tcString, len);
#endif
		}
		inline WString				TStringToWString(const TString& tString)
		{
#if BLADE_UNICODE
			return tString;
#else
			return StringToWString(tString.c_str(), tString.size());
#endif
		}

		inline TString		StringToTString(const char* mbcString, size_t len = 0)
		{
#if BLADE_UNICODE
			return StringToWString(mbcString, len);
#else
			BLADE_UNREFERENCED(len);
			return String(mbcString);
#endif
		}
		inline TString				StringToTString(const String& mbString)
		{
#if BLADE_UNICODE
			return StringToWString(mbString.c_str(), mbString.size());
#else
			return mbString;
#endif
		}

		inline TString		WStringToTString(const wchar* wcString, size_t len = 0)
		{
#if BLADE_UNICODE
			BLADE_UNREFERENCED(len);
			return TString(wcString);
#else
			return WStringToString(wcString, len);
#endif
		}
		inline TString		WStringToTString(const WString& wString)
		{
#if BLADE_UNICODE
			return wString;
#else
			return WStringToString(wString.c_str(), wString.size());
#endif
		}

		BLADE_BASE_API	String		StringToUTF8String(const char* mbcString, size_t len = 0);
		inline String				StringToUTF8String(const String& mbString)
		{
			return StringToUTF8String(mbString.c_str(), mbString.size());
		}

		BLADE_BASE_API	String		WStringToUTF8String(const wchar* wcString, size_t len = 0);
		inline String				WStringToUTF8String(const WString& wString)
		{
			return WStringToUTF8String(wString.c_str(), wString.size());
		}

		inline BLADE_BASE_API String TStringToUTF8String(const tchar* tcString, size_t len = 0)
		{
#if BLADE_UNICODE
			return StringConverter::WStringToUTF8String(tcString, len);
#else
			return StringConverter::StringToUTF8String(tcString, len);
#endif
		}

		inline String				TStringToUTF8String(const TString& tString)
		{
			return					TStringToUTF8String(tString.c_str(), tString.size());
		}

		BLADE_BASE_API	String		UTF8StringToString(const char* utf8String, size_t bytes = 0);
		inline String				UTF8StringToString(const String& utf8String)
		{
			return StringToUTF8String(utf8String.c_str(), utf8String.size());
		}

		BLADE_BASE_API	WString		UTF8StringToWString(const char* utf8String, size_t bytes = 0);
		inline WString				UTF8StringToWString(const String& utf8String)
		{
			return UTF8StringToWString(utf8String.c_str(), utf8String.size());
		}

		inline BLADE_BASE_API TString UTF8StringToTString(const char* utf8String, size_t bytes = 0)
		{
#if BLADE_UNICODE
			return StringConverter::UTF8StringToWString(utf8String, bytes);
#else
			return StringConverter::UTF8StringToString(utf8String, bytes);
#endif
		}
		inline TString				UTF8StringToTString(const String& utf8String)
		{
			return UTF8StringToTString(utf8String.c_str(), utf8String.size());
		}
	}//namespace StringConverter



#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#	define BLADE_TFUNCTION	Blade::TString( TEXT(__FUNCTION__), sizeof(TEXT(__FUNCTION__))/sizeof(Blade::tchar)-1, Blade::TString::CONST_HINT)
#	define BLADE_TFILE		Blade::TString( TEXT(__FILE__), sizeof(TEXT(__FILE__))/sizeof(Blade::tchar)-1, Blade::TString::CONST_HINT)
#elif BLADE_COMPILER_GNU_EXTENSION

	struct BLADE_BASE_API TLiteralHelper
	{
		TString mString;
		TLiteralHelper(const String& str);
		operator const TString&() const	{return mString;}
	};

#	define BLADE_TFUNCTION	TLiteralHelper( BVString(BLADE_FUNCTION) )
#	define BLADE_TFILE		TLiteralHelper( BVString(__FILE__) )
#else
#error not implemented.
#endif


}//namespace Blade

#endif // __Blade_String_h__