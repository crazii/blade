/********************************************************************
	created:	2010/05/14
	filename: 	FixedArray.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_FixedArray_h__
#define __Blade_FixedArray_h__
#include <BladeTypes.h>

namespace Blade
{
	template<typename T ,size_t SIZE>
	class FixedArray
	{
	public:
		enum ESizeInfo
		{
			SI_ELEMCOUNT	= SIZE,
			SI_ELEMSIZE		= sizeof(T),
			SI_TOTALBYTES	= SIZE*sizeof(T),
		};

		FixedArray()	{}
		~FixedArray()	{}

		/** @brief  */
		static size_t size() { return SI_ELEMCOUNT; }

		/** @brief  */
		static size_t capacity() { return SI_ELEMCOUNT; }

		/** @brief  */
		const T&		operator[](size_t index) const
		{
			assert( index < SIZE );
			return mData[index];
		}

		/** @brief  */
		T&				operator[](size_t index)
		{
			assert( index < SIZE );
			return mData[index];
		}

		/** @brief  */
		FixedArray&		operator=(const FixedArray& rhs)
		{
			if( this != &rhs )
			{
				for(size_t i = 0; i < SIZE; ++i )
					mData[i] = rhs.mData[i];
			}
			return *this;
		}

	protected:
		T	mData[SIZE];
	};


	template<typename T,size_t SIZE>
	class FixedVector
	{
	public:
		typedef T*			iterator;
		typedef const T*	const_iterator;
	public:
		FixedVector()
			:mSize(0)		{}
		~FixedVector()		{}

		/** @brief  */
		FixedVector&		operator=(const FixedVector& rhs)
		{
			mSize = rhs.mSize;

			if( this != &rhs )
			{
				for(size_t i = 0; i < mSize; ++i )
					mData[i] = rhs.mData[i];
			}
			return *this;
		}


		/** @brief  */
		size_t			size() const
		{
			return mSize;
		}

		/** @brief  */
		size_t			capacity() const
		{
			return SIZE;
		}

		/** @brief  */
		const T&		operator[](size_t index) const
		{
			assert( index < mSize );
			return mData[index];
		}

		/** @brief  */
		T&				operator[](size_t index)
		{
			assert( index < mSize );
			return mData[index];
		}

		/** @brief  */
		iterator		begin()
		{
			return &mData[0];
		}

		/** @brief  */
		const_iterator	begin() const
		{
			return &mData[0];
		}

		/** @brief  */
		iterator		end()
		{
			assert( mSize <= SIZE );
			return &mData[mSize];
		}

		/** @brief  */
		const_iterator	end() const
		{
			assert( mSize <= SIZE );
			return &mData[mSize];
		}

		/** @brief  */
		void			push_back(const T& elem)
		{
			assert( mSize < SIZE );
			mData[mSize++] = elem;
		}

		/** @brief  */
		void			pop_back()
		{
			assert( mSize > 0 );
			--mSize;
		}

		/** @brief  */
		const T&		front() const
		{
			assert( mSize > 0 );
			return mData[0];
		}

		/** @brief  */
		const T&		back() const
		{
			assert( mSize > 0 );
			return mData[mSize-1];
		}

		/** @brief  */
		T&				front()
		{
			assert( mSize > 0 );
			return mData[0];
		}

		/** @brief  */
		T&				back()
		{
			assert( mSize > 0 );
			return mData[mSize-1];
		}

		/** @brief  */
		void			clear()
		{
			for( size_t i = 0; i < mSize; ++i )
			{
				//clear to default state
				mData[i] = T();
			}
			mSize = 0;
		}

		/** @brief  */
		void			resize(size_t count)
		{
			if( count <= SIZE )
			{
				for(size_t i = count; i < mSize; ++i)
					mData[i] = T();
				mSize = count;
			}
			else
				assert(false);
		}


	protected:
		T		mData[SIZE];
		size_t	mSize;
	};
	
}//namespace Blade


#endif //__Blade_FixedArray_h__