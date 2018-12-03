/********************************************************************
	created:	2010/03/29
	filename: 	BladeContainer.h
	author:		Crazii
	
	purpose:	customization of STL containers, main for allocators
*********************************************************************/
#ifndef __Blade_BladeContainer_h__
#define __Blade_BladeContainer_h__
#include <map>
#include <set>
#include <list>
#include <vector>
#include <queue>
#include <stack>

#include <memory>
#include <functional>
#include <algorithm>

#include "String.h"
#include <memory/SizeAllocator.h>
#include <memory/TempAllocator.h>
#include <memory/StaticAllocator.h>
#include <Handle.h>

namespace Blade
{
	/************************************************************************/
	/* Pooled Containers                                                                     */
	/************************************************************************/
	//map
	template <typename KeyT, typename T , typename Cmp = std::less<KeyT> >
	class Map : public std::map<KeyT,T,Cmp,SizeAllocator< std::pair<const KeyT,T> > >
	{};

	//set
	template <typename T,typename Cmp = std::less<T> >
	class Set : public std::set<T,Cmp,SizeAllocator<T> >
	{};

	//list
	template <typename T>
	class List : public std::list<T,SizeAllocator<T> >
	{
	public:
		typedef std::list<T,SizeAllocator<T> > base_type;

		List() {}

		template <class InputIterator>
		List(InputIterator first, InputIterator last,
			const typename base_type::allocator_type& alloc = base_type::allocator_type())
			:base_type(first, last, alloc)	{}
	};

	template <typename T>
	class Vector : public std::vector<T, VectorAllocator<T> >
	{
	public:
		typedef std::vector<T, VectorAllocator<T> > base_type;
		Vector()	{}
		Vector(size_t count) :base_type(count)				{}
		Vector(size_t count,const T& val) :base_type(count,val)	{}

		template <class InputIterator>
		Vector(InputIterator first, InputIterator last,
			const typename base_type::allocator_type& alloc = base_type::allocator_type())
			: base_type(first, last, alloc)	{}
	};

	template<typename T>
	class Deque : public std::deque< T,SizeAllocator<T> >	{};

	template <typename T>
	class Queue : public std::queue< T, Deque<T> >
	{};

	//////////////////////////////////////////////////////////////////////////
	//thread safe concurrent queue
	template<typename T, typename A = SizeAllocator<T> >
	class ConQueue
	{
	public:
		ConQueue(){}
		~ConQueue(){}


		/** @brief std compatible: NOT thread safe */
		inline size_t	size() const
		{
			return mQueue.size();
		}

		/** @brief std compatible: NOT thread safe */
		inline bool	empty() const
		{
			return mQueue.empty();
		}

		/** @brief std compatible: NOT thread safe */
		inline const T&	front() const
		{
			return mQueue.front();
		}

		/** @brief std compatible: NOT thread safe */
		inline T& front()
		{
			return mQueue.front();
		}

		/** @brief std compatible: NOT thread safe */
		inline const T&	back() const
		{
			return mQueue.back();
		}

		/** @brief std compatible: NOT thread safe */
		inline T& back()
		{
			return mQueue.back();
		}

		/** @brief std compatible: NOT thread safe */
		inline void pop()
		{
			return mQueue.pop();
		}

		/** @brief std compatible: NOT thread safe */
		inline void	push(const T& val)
		{
			mQueue.push_back(val);
		}

		/** @brief custom method */
		inline bool	popSafe(T& out)
		{
			ScopedLock _lock(mLock);
			if( mQueue.size() != 0 )
			{
				out = mQueue.front();
				mQueue.pop_front();
				return true;
			}
			return false;
		}

		/** @brief  */
		inline void pushSafe(const T& val)
		{
			ScopedLock _lock(mLock);
			mQueue.push_back(val);
		}

		/** @brief custom method */
		inline bool	try_pop(T& out)
		{
			if( mLock.tryLock() )
			{
				if( mQueue.size() != 0 )
				{
					out = mQueue.front();
					mQueue.pop_front();
					mLock.unlock();
					return true;
				}
				mLock.unlock();
				return false;
			}
			return false;
		}

		/** @brief custom method */
		inline bool	try_push(const T& val)
		{
			if( mLock.tryLock() )
			{
				mQueue.push(val);
				mLock.unlock();
				return true;
			}
			return false;
		}

		/** @brief  */
		inline Lock&	getLock()
		{
			return mLock;
		}

		/** @brief  */
		inline bool try_lock()
		{
			return mLock.tryLock();
		}

		/** @brief  */
		inline void	clear()
		{
			ScopedLock _lock(mLock);
			while(!mQueue.empty())
				mQueue.pop();
		}

		/** @brief  */
		inline bool eraseSafe(const T& elem)
		{
			ScopedLock _lock(mLock);
			bool ret = false;
			typename QueueType::iterator i = std::find( mQueue.begin(), mQueue.end(), elem);
			if( i != mQueue.end() )
			{
				mQueue.erase( i );
				ret = true;
			}
			return ret;
		}

	protected:
		typedef std::deque<T, A>	QueueType;

		Lock		mLock;
		QueueType	mQueue;
	};



	/************************************************************************/
	/* static Containers                                                                     */
	/************************************************************************/
	//map
	template <typename KeyT,typename T ,typename Cmp = std::less<KeyT> >
	class StaticMap : public std::map<KeyT,T,Cmp,StaticAllocator< std::pair<const KeyT,T> > >
	{};

	//set
	template <typename T,typename Cmp = std::less<T> >
	class StaticSet : public std::set<T,Cmp,StaticAllocator<T> >
	{};

	//list
	template <typename T>
	class StaticList : public std::list<T,StaticAllocator<T> >
	{
	public:
		typedef std::list<T,StaticAllocator<T> > base_type;
		StaticList() {}

		template <class InputIterator>
		StaticList(InputIterator first, InputIterator last,
			const typename base_type::allocator_type& alloc = base_type::allocator_type())
			:base_type(first, last, alloc)	{}
	};

	//static vector
	template <typename T>
	class StaticVector : public std::vector<T,StaticAllocator<T> >
	{
	public:
		typedef std::vector<T,StaticAllocator<T> > base_type;

		StaticVector()	{}
		StaticVector(size_t count) :std::vector<T,StaticAllocator<T> >(count)					{}
		StaticVector(size_t count,const T& val) :std::vector<T,StaticAllocator<T> >(count,val)	{}

		template <class InputIterator>
		StaticVector(InputIterator first, InputIterator last,
			const typename base_type::allocator_type& alloc = base_type::allocator_type())
			:base_type(first, last, alloc)	{}
	};

	/************************************************************************/
	/* string map using fast comparator and Pool allocator                                                                */
	/************************************************************************/
	template< typename T, typename Alloc = SizeAllocator< std::pair<const TString, T> > >
	class TStringMap : public std::map<TString, T, FnTStringFastLess ,Alloc>
	{
	public:
		TStringMap() {}

		template <typename OtherAlloc>
		TStringMap( const TStringMap<T,OtherAlloc>& src )
		{
			for(typename TStringMap<T,OtherAlloc>::const_iterator it = src.begin(); it != src.end(); ++it )
			{
				this->insert( std::make_pair(it->first,it->second) );
			}
		}

		template <typename OtherAlloc>
		TStringMap&	operator=( const TStringMap<T,OtherAlloc>& src )
		{
			this->clear();
			for(typename TStringMap<T,OtherAlloc>::const_iterator it = src.begin(); it != src.end(); ++it )
			{
				this->insert( std::make_pair(it->first,it->second) );
			}
			return *this;
		}

	};

	typedef SizeAllocator< std::pair< const TString,void*> > TStringPointerPairAllocator;
	typedef StaticAllocator< std::pair< const TString,void*> > StaticTStringPointerPairAllocator;
	typedef TempAllocator< std::pair< const TString,void*> > TempTStringPointerPairAllocator;

	template<typename T, typename Alloc = TStringPointerPairAllocator >
	class TStringPointerMap : public TStringMap<T*,Alloc>
	{};

	/************************************************************************/
	/* string map using fast comparator and static allocator                                                                */
	/************************************************************************/
	template< typename T >
	class StaticTStringMap : public TStringMap<T,StaticAllocator< std::pair< const TString,T> > >
	{
	};

	/************************************************************************/
	/* string set using fast comparator and Pool allocator                                                                */
	/************************************************************************/
	class TStringSet : public std::set< TString ,FnTStringFastLess,SizeAllocator<TString> >
	{
	public:
		TStringSet() {}

		template <typename OtherAlloc>
		TStringSet(const std::set<TString,OtherAlloc>& src )
		{
			for(typename std::set<TString,OtherAlloc>::const_iterator it = src.begin(); it != src.end(); ++it )
			{
				this->insert( *it );
			}
		}

		template <typename OtherAlloc>
		TStringSet&	operator=(const std::set<TString,OtherAlloc>& src)
		{
			this->clear();
			for(typename std::set<TString,OtherAlloc>::const_iterator it = src.begin(); it != src.end(); ++it )
			{
				this->insert( *it );
			}
			return *this;
		}
	};

	/************************************************************************/
	/* string set using fast comparator and static allocator                                                                     */
	/************************************************************************/
	class StaticTStringSet : public std::set< TString ,FnTStringFastLess,StaticAllocator<TString> >
	{
	public:
		StaticTStringSet() {}

		template <typename OtherAlloc>
		StaticTStringSet(const std::set<TString,OtherAlloc>& src )
		{
			for(typename std::set<TString,OtherAlloc>::const_iterator it = src.begin(); it != src.end(); ++it )
			{
				this->insert( *it );
			}
		}

		template <typename OtherAlloc>
		StaticTStringSet&	operator=(const std::set<TString,OtherAlloc>& src)
		{
			this->clear();
			for(typename std::set<TString,OtherAlloc>::const_iterator it = src.begin(); it != src.end(); ++it )
			{
				this->insert( *it );
			}
			return *this;
		}
	};





	/************************************************************************/
	/* temporary containers                                                                     */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	template< typename IterT >
	class TempIterator : public IterT
	{
	public:
		inline TempIterator(const IterT& src)
			:IterT(src),mEmpty(false)		{}
		inline TempIterator(const TempIterator& src)
			:IterT(src),mEmpty(src.mEmpty)	{}

		inline TempIterator() :mEmpty(true)		{}

		TempIterator& operator=(const TempIterator& rhs)
		{
			(IterT&)*this = (const IterT&)rhs;
			this->mEmpty = rhs.mEmpty;
			return *this;
		}

		///helper for convert iterator => const_iterator
		///if you got compiling errors, it's probably type mismatch
		///maybe you're constructing / assigning an iterator from that of a totally different type
		template<typename U>
		inline TempIterator(const TempIterator<U>& src)
			:IterT( static_cast<const U&>(src) ),mEmpty(src.mEmpty)	{}

		template<typename U>
		TempIterator& operator=(const TempIterator<U>& rhs)
		{
			(IterT&)(*this) = (const IterT&) (const U&)rhs;
			this->mEmpty = rhs.mEmpty;
			return *this;
		}

		inline TempIterator& operator++()
		{
			return (TempIterator&)(++((IterT&)*this));
		}

		inline TempIterator operator++(int)
		{
			TempIterator Tmp = *this;
			++*this;
			return Tmp;
		}

		inline TempIterator& operator--()
		{
			return (TempIterator&)(--((IterT&)*this));
		}

		inline TempIterator operator--(int)
		{
			TempIterator Tmp = *this;
			--*this;
			return Tmp;
		}

		inline bool	empty() const	{return mEmpty;}
	protected:
		bool	mEmpty;
		template<typename O>
		friend class TempIterator;

		template<typename OIterT>
		friend bool operator==(const TempIterator<OIterT>& lhs,const TempIterator<OIterT>& rhs);

		template<typename OIterT>
		friend bool operator!=(const TempIterator<OIterT>& lhs,const TempIterator<OIterT>& rhs);

		template<typename IterT1,typename IterT2>
		friend bool operator==(const TempIterator<IterT1>& lhs,const TempIterator<IterT2>& rhs);
		template<typename IterT1,typename IterT2>
		friend bool operator!=(const TempIterator<IterT1>& lhs,const TempIterator<IterT2>& rhs);
	};

	template <typename OIterT>
	inline bool	operator==(const TempIterator<OIterT>& lhs,const TempIterator<OIterT>& rhs)
	{
		return (lhs.empty() && rhs.empty()) || (!lhs.empty() && !rhs.empty() && (const OIterT&)lhs == (const OIterT&)rhs);
	}

	template <typename OIterT>
	inline bool	operator!=(const TempIterator<OIterT>& lhs,const TempIterator<OIterT>& rhs)
	{
		return !(lhs == rhs);
	}

	template <typename IterT1,typename IterT2>
	inline bool	operator==(const TempIterator<IterT1>& lhs,const TempIterator<IterT2>& rhs)
	{
		return (lhs.empty() && rhs.empty()) || (!lhs.empty() && !rhs.empty() && (const IterT1&)lhs == (const IterT2&)rhs);
	}

	template <typename IterT1,typename IterT2>
	inline bool	operator!=(const TempIterator<IterT1>& lhs,const TempIterator<IterT2>& rhs)
	{
		return !(lhs == rhs);
	}

	template<typename IterT>
	class TempRandomAccessIterator : public TempIterator<IterT>
	{
	public:
		typedef IterT IterType;
		typedef typename IterType::difference_type difference_type;
		typedef typename IterType::reference reference;
		typedef TempRandomAccessIterator this_type;

		TempRandomAccessIterator()
			:TempIterator<IterT>()							{}

		TempRandomAccessIterator(const IterType& src)
			:TempIterator<IterT>(src)						{}

		TempRandomAccessIterator(const TempIterator<IterT>& src)
			:TempIterator<IterT>(src)						{}


		inline this_type& operator+=(difference_type Off)
		{
			if( Off != 0 )
			{
				assert( !this->empty() );
				(IterType&)(*this) += Off;
			}
			return *this;
		}

		inline this_type operator+(difference_type Off) const
		{
			this_type Tmp = *this;
			return Tmp += Off;
		}

		inline this_type& operator-=(difference_type Off)
		{
			return *this += -Off;
		}

		inline this_type operator-(difference_type Off) const
		{
			this_type Tmp = *this;
			return Tmp -= Off;
		}

		inline difference_type operator-(const this_type& Right) const
		{
			if( this->empty() )
			{
				assert( Right.empty() );
				return 0;
			}
			else
				return ((IterType&)*this) - Right;
		}

		inline this_type& operator++()
		{
			return (this_type&)(++((IterType&)*this));
		}

		inline this_type operator++(int)
		{
			this_type Tmp = *this;
			++*this;
			return Tmp;
		}

		inline this_type& operator--()
		{
			return (this_type&)(--((IterType&)*this));
		}

		inline this_type operator--(int)
		{
			this_type Tmp = *this;
			--*this;
			return Tmp;
		}

		inline reference operator[](difference_type Off) const
		{
			assert( !this->empty() );
			return (*(*this + Off));
		}
	};


	//////////////////////////////////////////////////////////////////////////
	template <typename ConT ,template<class IterT> class TempIterT = TempIterator, typename CA = TempAllocatable >
	class TempContainerBase
	{
	protected:
		class ContainerImpType : public ConT, public CA
		{
		public:
			ContainerImpType()	{}

			template<typename InputIterator>
			inline ContainerImpType(InputIterator _begin, InputIterator _end)
				:ConT(_begin, _end)
			{
			}
		};
	public:
		typedef TempIterT<typename ConT::iterator> iterator;
		typedef TempIterT<typename ConT::const_iterator> const_iterator;
		typedef TempIterT<typename ConT::reverse_iterator> reverse_iterator;
		typedef TempIterT<typename ConT::const_reverse_iterator> const_reverse_iterator;
		typedef typename ConT::allocator_type allocator_type;

		typedef typename ConT::value_type value_type;
		typedef typename ConT::size_type size_type;
		typedef typename ConT::reference reference;
		typedef typename ConT::const_reference const_reference;
		typedef typename ConT::difference_type difference_type;

#if BLADE_DEBUG
		inline TempContainerBase()	:mImp(NULL),mFile(NULL),mLine(0)							{}
		inline TempContainerBase(const char* file,int line)	:mImp(NULL),mFile(file),mLine(line)	{}		
#else
		inline TempContainerBase()	:mImp(NULL)													{}
#endif

		inline TempContainerBase(const TempContainerBase& src)
		{
#if BLADE_DEBUG
			mFile = NULL;
			mLine = 0;
#endif
			if( src.mImp != NULL && !src.mImp->empty() )
				this->mImp = BLADE_NEW ContainerImpType(*src.mImp);
			else
				this->mImp = NULL;
		}

		template<typename InputIterator>
		inline TempContainerBase(InputIterator _begin, InputIterator _end)
			:mImp(NULL)
		{
#if BLADE_DEBUG
			mFile = NULL;
			mLine = 0;
#endif
			if( _begin != _end )
				mImp = BLADE_NEW ContainerImpType(_begin, _end);
		}

		inline ~TempContainerBase()			{this->clear();}

		inline TempContainerBase& operator=(const TempContainerBase& rhs)
		{
			if( rhs.empty() )
				this->clear();
			else if( this->mImp == NULL )
				this->mImp = BLADE_NEW ContainerImpType(*rhs.mImp);
			else
				*this->mImp = *rhs.mImp;
			return *this;
		}

		/** @brief  */
		inline iterator	begin()
		{
			if( this->mImp == NULL )
				return iterator();
			else
				return iterator(this->mImp->begin());
		}

		/** @brief  */
		inline const_iterator begin() const
		{
			if( this->mImp == NULL )
				return const_iterator();
			else
				return const_iterator( this->mImp->begin());
		}

		/** @brief  */
		inline iterator	end()
		{
			if( this->mImp == NULL )
				return iterator();
			else
				return iterator(this->mImp->end());
		}

		/** @brief  */
		inline const_iterator end() const
		{
			if( this->mImp == NULL )
				return const_iterator();
			else
				return const_iterator( this->mImp->end() );
		}

		/** @brief  */
		inline reverse_iterator	rbegin()
		{
			if( this->mImp == NULL )
				return reverse_iterator();
			else
				return reverse_iterator(this->mImp->rbegin());
		}

		/** @brief  */
		inline const_reverse_iterator	rbegin() const
		{
			if( this->mImp == NULL )
				return const_reverse_iterator();
			else
				return const_reverse_iterator(this->mImp->rbegin());
		}

		/** @brief  */
		inline reverse_iterator rend()
		{
			if( this->mImp == NULL )
				return reverse_iterator();
			else
				return reverse_iterator(this->mImp->rend());
		}

		/** @brief  */
		inline const_reverse_iterator rend() const
		{
			if( this->mImp == NULL )
				return const_reverse_iterator();
			else
				return const_reverse_iterator(this->mImp->rend());
		}

		/** @brief  */
		inline bool	empty() const
		{
			return this->mImp == NULL || this->mImp->empty();
		}

		/** @brief  */
		inline size_t	size() const
		{
			return this->mImp == NULL ? 0 : this->mImp->size();
		}

		/** @brief  */
		inline void	clear()
		{
			if( this->mImp )
			{
				BLADE_DELETE this->mImp;
				this->mImp = NULL;
			}
		}

		/** @brief  */
		inline size_type max_size()
		{
			return allocator_type().max_size();
		}

		/** @brief  */
		void	swap(TempContainerBase& Right)
		{
			std::swap(this->mImp,Right.mImp);
		}

		/** @brief  */
		allocator_type get_allocator() const
		{
			return allocator_type();
		}
	protected:

		inline bool		emptyImp() const
		{
			return this->mImp == NULL;
		}

		inline void		allocateImp()
		{
#if BLADE_DEBUG
			if( this->mImp == NULL )
				this->mImp = new(mFile, mLine, (Blade::MEMINFO*)NULL ) ContainerImpType();
#else
			if( this->mImp == NULL )
				this->mImp = BLADE_NEW ContainerImpType();
#endif
		}
		ContainerImpType*	mImp;

		//memory allocation debug info
#if BLADE_DEBUG
		const char* mFile;
		int			mLine;
#endif
	};

	//////////////////////////////////////////////////////////////////////////
	template <typename ConT ,template<class IterT> class TempIterT , typename CA = TempAllocatable >
	class TempSeqContainerBase : public TempContainerBase<ConT,TempIterT,CA >
	{
	public:
		typedef TempContainerBase<ConT,TempIterT,CA > base_type;

		typedef typename base_type::iterator iterator;
		typedef typename base_type::const_iterator const_iterator;
		typedef typename base_type::reverse_iterator reverse_iterator;
		typedef typename base_type::const_reverse_iterator const_reverse_iterator;
		typedef typename base_type::allocator_type allocator_type;

		typedef typename base_type::value_type value_type;
		typedef typename base_type::size_type size_type;
		typedef typename base_type::reference reference;
		typedef typename base_type::const_reference const_reference;
		typedef typename base_type::difference_type difference_type;

	public:
		inline TempSeqContainerBase()	{}


		template<typename InputIterator>
		inline TempSeqContainerBase(InputIterator _begin, InputIterator _end)
			:base_type(_begin, _end)	{}

#if BLADE_DEBUG
		inline TempSeqContainerBase(const char* file, int  line)
			:TempContainerBase<ConT,TempIterT,CA >(file,line)	{}
#endif

		/** @brief  */
		inline iterator erase(iterator Where)
		{
			if( this->mImp != NULL )
			{
				assert( !Where.empty() );
				iterator i(this->mImp->erase(Where) );
				if (this->mImp->size() == 0)
				{
					this->clear();
					i = this->end();
				}
				return i;
			}
			else
			{
				assert( Where.empty() );
				return Where;
			}
		}

		/** @brief  */
		inline iterator erase(iterator First,iterator Last)
		{
			if( this->mImp != NULL )
			{
				assert( !First.empty() && !Last.empty() );
				iterator i(this->mImp->erase(First,Last) );
				if( this->mImp->size() == 0 )
					this->clear();

				return i;
			}
			else
			{
				assert( First.empty() && Last.empty() );
				return First;
			}
		}


		/** @brief  */
		iterator insert(iterator Where,const value_type& Val)
		{
			this->allocateImp();
			if( Where.empty() )
				Where = this->mImp->begin();
			return iterator(this->mImp->insert(Where,Val));
		}

		/** @brief  */
		template<class InputIterator>
#if BLADE_STDCPP >= BLADE_STDCPP11 || (BLADE_COMPILER == BLADE_COMPILER_MSVC && _MSC_VER >= 1700 )//VS2012
		iterator
#else
		void
#endif
			insert(iterator Where,InputIterator First,InputIterator Last)
		{
			this->allocateImp();
			if( Where.empty() )
				Where = this->mImp->begin();

#if BLADE_STDCPP >= BLADE_STDCPP11 || (BLADE_COMPILER == BLADE_COMPILER_MSVC && _MSC_VER >= 1700 )//VS2012
			return iterator( this->mImp->insert( Where, First, Last) );
#else
			return this->mImp->insert( Where, First, Last);
#endif
		}

		/** @brief  */
		inline reference front( )
		{
			assert( !this->emptyImp() );
			return this->mImp->front();
		}

		/** @brief  */
		inline const_reference front( ) const
		{
			assert( !this->emptyImp() );
			return this->mImp->front();
		}

		/** @brief  */
		inline reference back( )
		{
			assert( !this->emptyImp() );
			return this->mImp->back();
		}

		/** @brief  */
		inline const_reference back( ) const
		{
			assert( !this->emptyImp() );
			return this->mImp->back();
		}

		/** @brief  */
		template<class InputIterator>
		inline void assign(InputIterator First,InputIterator Last)
		{
			if( First != Last )
			{
				this->allocateImp();
				this->mImp->assign(First,Last);
			}
			else
				this->clear();
		}

		/** @brief  */
		inline void assign(size_type Count,const value_type& Val)
		{
			if( Count != 0 )
			{
				this->allocateImp();
				this->mImp->assign(Count,Val);
			}
			else
				this->clear();
		}

		/** @brief  */
		inline void push_back(const value_type& Val)
		{
			this->allocateImp();
			this->mImp->push_back(Val);
		}

#if BLADE_STDCPP >= BLADE_STDCPP11 || BLADE_COMPILER == BLADE_COMPILER_MSVC && _MSC_VER >= 1700
		inline void push_back(value_type&& Val)
		{
			this->allocateImp();
			this->mImp->push_back(Val);
		}
#endif

		/** @brief  */
		inline void pop_back( )
		{
			assert( !this->emptyImp() );
			this->mImp->pop_back();
			if( this->mImp->empty() )
				this->clear();
		}

		/** @brief  */
		inline void resize(size_type Newsize)
		{
			if( Newsize != 0 )
			{
				this->allocateImp();
				this->mImp->resize(Newsize);
			}
			else
				this->clear();
		}

		/** @brief  */
#if BLADE_STDCPP >= BLADE_STDCPP11
		inline void resize(size_type Newsize, const value_type& Val)
#else
		inline void resize(size_type Newsize, value_type Val)
#endif
		{
			if( Newsize != 0 )
			{
				this->allocateImp();
				this->mImp->resize(Newsize,Val);
			}
			else
				this->clear();
		}
	};

	//////////////////////////////////////////////////////////////////////////
	template <typename ConT , typename CA = TempAllocatable>
	class TempAscContainerBase : public TempContainerBase<ConT ,TempIterator , CA >
	{
	public:
		typedef TempContainerBase<ConT ,TempIterator , CA > base_type;

		typedef typename base_type::iterator iterator;
		typedef typename base_type::const_iterator const_iterator;
		typedef typename base_type::reverse_iterator reverse_iterator;
		typedef typename base_type::const_reverse_iterator const_reverse_iterator;
		typedef typename base_type::allocator_type allocator_type;

		typedef typename base_type::value_type value_type;
		typedef typename base_type::size_type size_type;
		typedef typename base_type::reference reference;
		typedef typename base_type::const_reference const_reference;
		typedef typename base_type::difference_type difference_type;

		typedef typename ConT::key_type key_type;
		typedef typename ConT::key_compare key_compare;
		typedef typename ConT::value_compare value_compare;

		typedef typename base_type::ContainerImpType ContainerImpType;

		inline TempAscContainerBase()	{}

		template<typename InputIterator>
		inline TempAscContainerBase(InputIterator _begin, InputIterator _end)
			:base_type(_begin, _end){}
#if BLADE_DEBUG
		inline TempAscContainerBase(const char* file, int  line)
			:TempContainerBase<ConT ,TempIterator , CA >(file,line)	{}
#endif

		/** @brief  */
		inline iterator insert(iterator Where,const value_type& Val)
		{
			this->allocateImp();
			if( Where.empty() )
				Where = this->mImp->begin();
			return iterator(this->mImp->insert(Where,Val));
		}

		/** @brief  */
		inline std::pair <iterator, bool> insert(const value_type& Val)
		{
			this->allocateImp();
			return this->mImp->insert(Val);
		}

		/** @brief  */
		template<class InputIterator>
		inline void insert(InputIterator First,InputIterator Last)
		{
			for (; First != Last; ++First)
				this->insert(*First);
		}

		/** @brief  */
#if BLADE_STDCPP >= BLADE_STDCPP11
		inline iterator erase(iterator Where)
		{
			if( this->mImp != NULL )
			{
				assert( !Where.empty() );
				iterator i = this->mImp->erase(Where);
				if( this->mImp->size() == 0 )
					this->clear();

				return i;
			}
			else
			{
				assert( Where.empty() );
				return Where;
			}
		}

		/** @brief  */
		inline iterator erase(iterator First,iterator Last)
		{
			if( this->mImp != NULL )
			{
				assert( !First.empty() && !Last.empty() );

				iterator i(this->mImp->erase(First,Last) );
				if( this->mImp->size() == 0 )
					this->clear();

				return i;
			}
			else
			{
				assert( First.empty() && Last.empty() );
				return First;
			}
		}
#else
		inline void erase(iterator Where)
		{
			if( this->mImp != NULL )
			{
				assert( !Where.empty() );
				this->mImp->erase(Where);
				if( this->mImp->size() == 0 )
					this->clear();
			}
			else
				assert( Where.empty() );
		}

		/** @brief  */
		inline void erase(iterator First,iterator Last)
		{
			if( this->mImp != NULL )
			{
				assert( !First.empty() && !Last.empty() );

				this->mImp->erase(First,Last);
				if( this->mImp->size() == 0 )
					this->clear();
			}
			else
				assert( First.empty() && Last.empty() );
		}
#endif

		/** @brief  */
		inline size_type erase(const key_type& Key)
		{
			if( !this->emptyImp() )
			{
				size_type n = this->mImp->erase(Key);
				if( this->mImp->size() == 0 )
					this->clear();
				return n;
			}
			else
				return 0;
		}

		/** @brief  */
		inline size_type	count(const key_type& key) const
		{
			if( this->emptyImp() )
				return 0;
			else
				return this->mImp->count(key);
		}

		/** @brief  */
		inline std::pair <const_iterator, const_iterator> equal_range(const key_type& val) const
		{
			if( this->emptyImp() )
				return std::make_pair(this->begin(), this->end());
			else
				return this->mImp->equal_range(val);
		}

		/** @brief  */
		inline std::pair <iterator, iterator> equal_range(const key_type& val)
		{
			if( this->emptyImp() )
				return std::make_pair(this->begin(), this->end());
			else
				return this->mImp->equal_range(val);
		}

		/** @brief  */
		inline iterator find(const key_type& Key)
		{
			if( this->emptyImp() )
				return this->end();
			else
				return this->mImp->find(Key);
		}

		/** @brief  */
		inline const_iterator find(const key_type& Key) const
		{
			if( this->emptyImp() )
				return this->end();
			else
				return ( (const ContainerImpType*) this->mImp)->find(Key);
		}

		/** @brief  */
		inline key_compare key_comp() const
		{
			return ConT::key_compare();
		}

		/** @brief  */
		inline value_compare value_comp ( ) const
		{
			return ConT::value_compare();
		}

		/** @brief  */
		inline iterator lower_bound(const key_type& Key)
		{
			if( this->emptyImp())
				return this->end();
			else
				return this->mImp->lower_bound(Key);
		}

		/** @brief  */
		inline const_iterator lower_bound(const key_type& Key) const
		{
			if( this->emptyImp())
				return this->end();
			else
				return this->mImp->lower_bound(Key);
		}

		/** @brief  */
		inline iterator upper_bound(const key_type& Key)
		{
			if( this->emptyImp())
				return this->end();
			else
				return this->mImp->upper_bound(Key);
		}

		/** @brief  */
		inline const_iterator upper_bound(const key_type& Key) const
		{
			if( this->emptyImp())
				return this->end();
			else
				return this->mImp->upper_bound(Key);
		}
	};


	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	template <typename KeyT,typename T ,typename C = std::less<KeyT> , typename A = TempAllocator<KeyT>, typename CA = TempAllocatable  >
	class TempMap : public TempAscContainerBase< std::map<KeyT,T,C,A>,CA >
	{
	public:
		typedef TempAscContainerBase< std::map<KeyT,T,C,A>,CA > base_type;

		typedef typename base_type::iterator iterator;
		typedef typename base_type::const_iterator const_iterator;
		typedef typename base_type::reverse_iterator reverse_iterator;
		typedef typename base_type::const_reverse_iterator const_reverse_iterator;
		typedef typename base_type::allocator_type allocator_type;

		typedef typename base_type::value_type value_type;
		typedef typename base_type::size_type size_type;
		typedef typename base_type::reference reference;
		typedef typename base_type::const_reference const_reference;
		typedef typename base_type::difference_type difference_type;

		typedef typename base_type::ContainerImpType::key_type key_type;
		typedef typename base_type::ContainerImpType::mapped_type mapped_type;

		inline TempMap()	{}

		template<typename InputIterator>
		inline TempMap(InputIterator _begin, InputIterator _end)
			:base_type(_begin, _end){}
#if BLADE_DEBUG
		inline TempMap(const char* file, int  line)
			:TempAscContainerBase< std::map<KeyT,T,C,A>,CA >(file,line)	{}
#endif

		inline mapped_type& operator[](const key_type& Key)
		{
			this->allocateImp();
			return (*this->mImp)[Key];
		}
	};

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	template<typename T,typename C = std::less<T>, typename A = TempAllocator<T>, typename CA = TempAllocatable >
	class TempSet : public TempAscContainerBase< std::set<T,C,A>,CA >
	{
	public:
		typedef TempAscContainerBase< std::set<T,C,A>,CA > base_type;

		typedef typename base_type::iterator iterator;
		typedef typename base_type::const_iterator const_iterator;
		typedef typename base_type::reverse_iterator reverse_iterator;
		typedef typename base_type::const_reverse_iterator const_reverse_iterator;
		typedef typename base_type::allocator_type allocator_type;

		typedef typename base_type::value_type value_type;
		typedef typename base_type::size_type size_type;
		typedef typename base_type::reference reference;
		typedef typename base_type::const_reference const_reference;
		typedef typename base_type::difference_type difference_type;
	public:
		inline TempSet()	{}

		template<typename InputIterator>
		inline TempSet(InputIterator _begin, InputIterator _end)
			:base_type(_begin, _end){}
#if BLADE_DEBUG
		inline TempSet(const char* file, int  line)
			:TempAscContainerBase< std::set<T,C,A>,CA >(file,line)	{}
#endif
	};

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	template<typename T , typename A = TempAllocator<T>, typename CA = TempAllocatable >
	class TempVector : public TempSeqContainerBase< std::vector<T,A> ,TempRandomAccessIterator,CA >
	{
	public:
		typedef TempSeqContainerBase< std::vector<T,A> ,TempRandomAccessIterator,CA > base_type;

		typedef typename base_type::iterator iterator;
		typedef typename base_type::const_iterator const_iterator;
		typedef typename base_type::reverse_iterator reverse_iterator;
		typedef typename base_type::const_reverse_iterator const_reverse_iterator;
		typedef typename base_type::allocator_type allocator_type;

		typedef typename base_type::value_type value_type;
		typedef typename base_type::size_type size_type;
		typedef typename base_type::reference reference;
		typedef typename base_type::const_reference const_reference;
		typedef typename base_type::difference_type difference_type;

		inline TempVector(){}

		template<typename InputIterator>
		inline TempVector(InputIterator _begin, InputIterator _end)
			:base_type(_begin, _end)	{}
#if BLADE_DEBUG
		inline TempVector(const char* file, int  line)
			:TempSeqContainerBase< std::vector<T,A> ,TempRandomAccessIterator,CA >(file,line)	{}	
#endif

		/** @brief  */
		inline TempVector(size_t count)
		{
			if (count > 0)
			{
				this->allocateImp();
				this->mImp->resize(count);
			}
		}

		/** @brief  */
		inline TempVector(size_t count,const T& val)
		{
			if (count > 0)
			{
				this->allocateImp();
				this->mImp->resize(count, val);
			}
		}

		/** @brief  */
		inline size_type capacity() const
		{
			if( this->emptyImp() )
				return 0;
			else
				return this->mImp->capacity();
		}

		/** @brief  */
		inline void reserve(size_type Count)
		{
			if( Count == 0 )
				this->clear();
			else
			{
				this->allocateImp();
				this->mImp->reserve(Count);
			}
		}

		/** @brief  */
		inline reference at(size_type Pos)
		{
			assert( !this->emptyImp() );
			return this->mImp->at(Pos);
		}

		/** @brief  */
		inline const_reference at(size_type Pos) const
		{
			assert( !this->emptyImp() );
			return this->mImp->at(Pos);
		}

		/** @brief  */
		inline reference operator[](size_type Pos)
		{
			assert( !this->emptyImp() );
			return (*this->mImp)[Pos];
		}

		/** @brief  */
		inline const_reference operator[](size_type Pos) const
		{
			assert( !this->emptyImp() );
			return (*this->mImp)[Pos];
		}

		/** @brief  */
		iterator insert (iterator position, const value_type& val)
		{
			this->allocateImp();
			if( position == this->end() )
				return this->mImp->insert( this->mImp->end(), val );
			else
				return this->mImp->insert(position, val);
		}

		/** @brief  */	
		void insert (iterator position, size_type n, const value_type& val)
		{
			this->allocateImp();
			if( position == this->end() )
				return this->mImp->insert( this->mImp->end(), n, val );
			else
				return this->mImp->insert(position, n, val);
		}

		/** @brief  */
		template <class InputIterator>
#if BLADE_STDCPP >= BLADE_STDCPP11 
		iterator
#else
#	if defined(_MSC_VER) && _MSC_VER >=1700
		iterator
#	else
		void 
#	endif
#endif
			insert (iterator position, InputIterator first, InputIterator last)
		{
			this->allocateImp();
			if( position.empty() )
				return this->mImp->insert( this->mImp->end(), first, last );
			else
				return this->mImp->insert(position, first, last);
		}
	};

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	template<typename T, typename A = TempAllocator<T>, typename CA = TempAllocatable >
	class TempList : public TempSeqContainerBase< std::list<T,A> ,TempIterator,CA >
	{
	public:
		typedef TempList this_type;

		typedef TempSeqContainerBase< std::list<T,A> ,TempIterator,CA > base_type;

		typedef typename base_type::iterator iterator;
		typedef typename base_type::const_iterator const_iterator;
		typedef typename base_type::reverse_iterator reverse_iterator;
		typedef typename base_type::const_reverse_iterator const_reverse_iterator;
		typedef typename base_type::allocator_type allocator_type;

		typedef typename base_type::value_type value_type;
		typedef typename base_type::size_type size_type;
		typedef typename base_type::reference reference;
		typedef typename base_type::const_reference const_reference;
		typedef typename base_type::difference_type difference_type;

		inline TempList()	{}
		template <class InputIterator>
		TempList(InputIterator first, InputIterator last,
			const allocator_type& alloc = allocator_type())
			:base_type(first, last)	{}

#if BLADE_DEBUG
		inline TempList(const char* file, int  line)
			:TempSeqContainerBase< std::list<T,A> ,TempIterator,CA >(file,line)	{}
#endif

		/** @brief  */
		inline void push_front(const value_type& Val)
		{
			this->allocateImp();
			this->mImp->push_front(Val);
		}

		/** @brief  */
		inline void pop_front( )
		{
			assert( !this->emptyImp() );
			this->mImp->pop_front();
			if( this->mImp->empty() )
				this->clear();
		}


		/** @brief  */
		inline void splice(iterator Where, this_type& Right)
		{
			if( this->emptyImp() )
			{
				assert( Where.empty() );
				return;
			}
			else
			{
				assert( !Where.empty() );
				Right.allocateImp();
				this->mImp->splice(Where,*Right.mImp);
			}
		}

		/** @brief  */
		inline void splice(iterator Where, this_type& Right,iterator First)
		{
			if( this->emptyImp() )
			{
				assert( Where.empty() );
				return;
			}
			else
			{
				assert( !Where.empty() );
				Right.allocateImp();
				this->mImp->splice(Where,*Right.mImp,First);
			}
		}

		/** @brief  */
		inline void splice(iterator Where, this_type& Right,iterator First,iterator Last)
		{
			if( this->emptyImp() )
			{
				assert( Where.empty() );
				return;
			}
			else
			{
				assert( !Where.empty() );
				Right.allocateImp();
				this->mImp->splice(Where,*Right.mImp,First,Last);
			}
		}

		/** @brief  */
		inline void remove(const value_type& Val)
		{
			if( !this->emptyImp() )
				this->mImp->remove(Val);
		}

		/** @brief  */
		template<class Predicate> 
		inline void remove_if(Predicate Pred)
		{
			if( !this->emptyImp() )
				this->mImp->remove_if(Pred);
		}

		/** @brief  */
		inline void unique( )
		{
			if( !this->emptyImp() )
				this->mImp->unique();
		}

		/** @brief  */
		template<class BinaryPredicate>
		inline void unique(BinaryPredicate Pred)
		{
			if( !this->emptyImp() )
				this->mImp->unique(Pred);
		}


		/** @brief  */
		inline void merge(this_type& Right)
		{
			if( !this->emptyImp())
			{
				Right.allocateImp();
				this->mImp->merge(*Right.mImp);
			}
		}

		/** @brief  */
		template<class Traits>
		inline void merge(this_type& Right, Traits Comp)
		{
			if( !this->emptyImp())
			{
				Right.allocateImp();
				this->mImp->merge(*Right.mImp,Comp);
			}
		}

		/** @brief  */
		inline void sort( )
		{
			if( !this->emptyImp())
			{
				this->mImp->sort();
			}
		}

		/** @brief  */
		template<typename Cmp> 
		inline void sort(Cmp Comp)
		{
			if( !this->emptyImp())
			{
				this->mImp->sort(Comp);
			}
		}

		/** @brief  */
		inline void reverse()
		{
			if( !this->emptyImp())
			{
				this->mImp->reverse();
			}
		}
	};

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	template<typename T, typename A = TempAllocator<T>, typename CA = TempAllocatable >
	class TempDeque : public TempSeqContainerBase< std::deque<T,A> ,TempRandomAccessIterator,CA >
	{
	public:
		typedef TempSeqContainerBase< std::deque<T,A> ,TempRandomAccessIterator,CA > base_type;

		typedef typename base_type::iterator iterator;
		typedef typename base_type::const_iterator const_iterator;
		typedef typename base_type::reverse_iterator reverse_iterator;
		typedef typename base_type::const_reverse_iterator const_reverse_iterator;
		typedef typename base_type::allocator_type allocator_type;

		typedef typename base_type::value_type value_type;
		typedef typename base_type::size_type size_type;
		typedef typename base_type::reference reference;
		typedef typename base_type::const_reference const_reference;
		typedef typename base_type::difference_type difference_type;

	public:
		inline TempDeque()	{}
#if BLADE_DEBUG
		inline TempDeque(const char* file, int  line)
			:TempSeqContainerBase< std::deque<T,A> ,TempRandomAccessIterator,CA >(file,line)	{}
#endif


		/** @brief  */
		inline void push_front(const value_type& Val)
		{
			this->allocateImp();
			this->mImp->push_front(Val);
		}

		/** @brief  */
		inline void pop_front( )
		{
			assert( !this->emptyImp() );
			this->mImp->pop_front();

			if( this->mImp->empty() )
				this->clear();
		}

		/** @brief  */
		inline reference at(size_type Pos)
		{
			assert( !this->emptyImp() );
			return this->mImp->at(Pos);
		}

		/** @brief  */
		inline const_reference at(size_type Pos) const
		{
			assert( !this->emptyImp() );
			return this->mImp->at(Pos);
		}

		/** @brief  */
		inline reference operator[](size_type Pos)
		{
			assert( !this->emptyImp() );
			return (*this->mImp)[Pos];
		}

		/** @brief  */
		inline const_reference operator[](size_type Pos) const
		{
			assert( !this->emptyImp() );
			return (*this->mImp)[Pos];
		}
	};

	template<typename T, typename A = TempAllocator<T>, typename CA = TempAllocatable >
	class TempQueue : public TempDeque<T,A,CA>
	{
	public:
		typedef T value_type;
		typedef TempDeque<T, A, CA> container_type;
		typedef typename container_type::size_type size_type;
		typedef T& reference;
		typedef const T& const_reference;

		inline TempQueue()	{}
#if BLADE_DEBUG
		inline TempQueue(const char* file, int  line)
			:container_type(file,line)	{}
#endif

		/** @brief  */
		inline void push(const T& in)
		{
			this->push_back(in);
		}

		/** @brief  */
		inline void	pop()
		{
			this->pop_front();
		}
	};

	template<typename T, typename A = TempAllocator<T>, typename CA = TempAllocatable >
	class TempStack : public TempDeque<T,A,CA >
	{
	public:
		typedef T value_type;
		typedef TempDeque<T, A, CA> container_type;
		typedef typename container_type::size_type size_type;
		typedef T& reference;
		typedef const T& const_reference;

		inline TempStack()	{}
#if BLADE_DEBUG
		inline TempStack(const char* file, int  line)
			:container_type(file,line)	{}
#endif
		/** @brief  */
		inline reference top()
		{
			return this->back();
		}

		/** @brief  */
		inline const_reference top() const
		{
			return this->back();
		}

		/** @brief  */
		inline void push(const T& val)
		{
			this->push_back(val);
		}

		/** @brief  */
		inline void	pop()
		{
			this->pop_back();
		}
	};

	/************************************************************************/
	/* Temporary Concurrent Queue                                                                     */
	/************************************************************************/
	template<typename T, typename A = TempAllocator<T>, typename CA = TempAllocatable >
	class TempConQueue : public CA
	{
	protected:
		typedef		TempDeque<T, A, CA>	QueueType;
	public:
		inline TempConQueue(){}
#if BLADE_DEBUG
		inline TempConQueue(const char* file, int  line)
			:mQueue(file,line)	{}	
#endif
		inline ~TempConQueue(){}

		/** @brief std compatible: NOT thread safe */
		inline size_t	size() const
		{
			return mQueue.size();
		}

		/** @brief std compatible: NOT thread safe */
		inline bool		empty() const
		{
			return mQueue.empty();
		}

		/** @brief std compatible: NOT thread safe */
		inline const T&	front() const
		{
			return mQueue.front();
		}

		/** @brief std compatible: NOT thread safe */
		inline T& front()
		{
			return mQueue.front();
		}

		/** @brief std compatible: NOT thread safe */
		inline const T&	back() const
		{
			return mQueue.back();
		}

		/** @brief std compatible: NOT thread safe */
		inline T& back()
		{
			return mQueue.back();
		}

		/** @brief std compatible: NOT thread safe */
		inline void pop()
		{
			if( mQueue.size() != 0 )
				mQueue.pop_front();
		}

		/** @brief std compatible: NOT thread safe */
		inline void	push(const T& val)
		{
			mQueue.push_back(val);
		}

		/** @brief custom method */
		inline bool	popSafe(T& out)
		{
			ScopedLock _lock(mLock);
			if( !mQueue.empty() )
			{
				out = mQueue.front();
				mQueue.pop_front();
				return true;
			}
			return false;
		}

		/** @brief custom method */
		inline void	pushSafe(const T& val)
		{
			ScopedLock _lock(mLock);
			mQueue.push_back(val);
		}

		/** @brief custom method */
		inline bool	try_pop(T& out)
		{
			if( mLock.tryLock() )
			{
				if( !mQueue.empty() )
				{
					out = mQueue.front();
					mQueue.pop_front();
				}
				mLock.unlock();
				return true;
			}
			return false;
		}

		/** @brief custom method */
		inline bool	try_push(const T& val)
		{
			if( mLock.tryLock() )
			{
				mQueue.push_back(val);
				mLock.unlock();
				return true;
			}
			return false;
		}

		/** @brief  */
		inline Lock& getLock()
		{
			return mLock;
		}

		/** @brief  */
		inline bool try_lock()
		{
			return mLock.tryLock();
		}

		/** @brief  */
		inline void unlock()
		{
			mLock.unlock();
		}

		/** @brief  */
		inline void	clear()
		{
			ScopedLock _lock(mLock);
			mQueue.clear();
		}

		/** @brief  */
		inline bool eraseSafe(const T& elem)
		{
			ScopedLock _lock(mLock);
			bool ret = false;
			typename QueueType::iterator i = std::find( mQueue.begin(), mQueue.end(), elem);
			if( i != mQueue.end() )
			{
				mQueue.erase( i );
				ret = true;
			}
			return ret;
		}

	protected:
		Lock		mLock;
		QueueType	mQueue;
	};

	/************************************************************************/
	/* string map using fast comparator and Temp allocator                                                                */
	/************************************************************************/
	template< typename T, typename A = TempAllocator<T> >
	class TempTStringMap : public TempMap<TString,T,FnTStringFastLess,A>
	{
	public:
		inline TempTStringMap()	{}
#if BLADE_DEBUG
		inline TempTStringMap(const char* file, int line) :TempMap<TString,T,FnTStringFastLess,A>(file,line)	{}
#endif
	};

	/************************************************************************/
	/* string set using fast comparator and temp allocator                                                                     */
	/************************************************************************/
	class TempTStringSet : public TempSet< TString ,FnTStringFastLess >
	{
	public:

		TempTStringSet() {}
#if BLADE_DEBUG
		TempTStringSet(const char* file, int line)
			:TempSet< TString ,FnTStringFastLess >(file,line)	{}	
#endif

		template <typename OtherAlloc>
		TempTStringSet(const std::set<TString,OtherAlloc>& src )
		{
			for(typename std::set<TString,OtherAlloc>::const_iterator it = src.begin(); it != src.end(); ++it )
			{
				this->insert( *it );
			}
		}

		template <typename OtherAlloc>
		TempTStringSet&	operator=(const std::set<TString,OtherAlloc>& src)
		{
			this->clear();
			for(typename std::set<TString,OtherAlloc>::const_iterator it = src.begin(); it != src.end(); ++it )
			{
				this->insert( *it );
			}
			return *this;
		}
	};

#if BLADE_DEBUG
#define BLADE_TEMPCONT_INIT __FILE__,__LINE__
#else
#define BLADE_TEMPCONT_INIT 
#endif

	/************************************************************************/
	/* simple object pool:                                                                     */
	/* sometimes complicated objects need pooling besides allocation/memory pooling, */
	/* memory pooling only boost memory allocation, while object pooling can avoid destruction/construction */
	/************************************************************************/
	template<typename T>
	struct TPoolNullAlloc
	{
	};
	template<typename T>
	struct TPoolDefAlloc
	{
	};

	template<typename T>
	struct TPoolNullAlloc<T*>
	{
		static T* create() { return NULL; }
	};
	template<typename T>
	struct TPoolDefAlloc<T*>
	{
		static T* create() { return BLADE_NEW T(); }
	};

	template<typename T>
	struct TPoolNullAlloc< Handle<T> >
	{
		static Handle<T> create() { return Handle<T>::EMPTY; }
	};
	template<typename T>
	struct TPoolDefAlloc< Handle<T> >
	{
		static Handle<T> create() { return Handle<T>(BLADE_NEW T()); }
	};

	namespace Impl
	{
		template<typename T, typename A>
		class TPoolBase
		{
		};
		template<typename T, typename A>
		class TPoolBase<T*, A>
		{
		public:
			typedef List<T*>	FreeList;

			~TPoolBase<T*, A>()
			{
				this->clear();
			}

			/** @brief  */
			void add(T* v)
			{
				if (v != NULL)
					mList.push_back(v);
			}

			/** @brief  */
			T*	get()
			{
				if (!mList.empty())
				{
					T* v = mList.back();
					mList.pop_back();
					return v;
				}
				return A::create();
			}

			/** @brief  */
			void clear()
			{
				for (typename FreeList::iterator i = mList.begin(); i != mList.end(); ++i)
					BLADE_DELETE *i;
				mList.clear();
			}

		protected:
			FreeList mList;
		};

		template<typename T, typename A>
		class TPoolBase<Handle<T>, A>
		{
		public:
			typedef List< Handle<T> >	FreeList;

			/** @brief  */
			void add(const Handle<T>& v)
			{
				if (v != NULL)
					mList.push_back(v);
			}

			/** @brief  */
			Handle<T>	get()
			{
				if (!mList.empty())
				{
					Handle<T> v = mList.back();
					mList.pop_back();
					return v;
				}
				return A::create();
			}

			/** @brief  */
			void clear()
			{
				mList.clear();
			}

		protected:
			FreeList mList;
		};
	}

	template<typename T, typename A = TPoolNullAlloc<T> >
	class TPool : public Impl::TPoolBase<T, A> {};
	
}//namespace Blade

#endif //__Blade_BladeContainer_h__