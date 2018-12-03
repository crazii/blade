/********************************************************************
	created:	2011/10/23
	filename: 	LoopContainers.h
	author:		Crazii
	purpose:	the loop containers are special temporary containers
				the difference between loop container and temporary container(LoopContainer)
				is that the loop containers are auto-cleared at the beginning of each main loop( framework update)
	remark:		the MAIN_LOOP_STATE(declared in FrameworkStates.h) - "::MainLoopBegin" state, is registered by the framework,
				and the state is activated on each beginning of the main loop ( the framework 's update )
*********************************************************************/
#ifndef __Blade_LoopContainers_h__
#define __Blade_LoopContainers_h__
#include <utility/BladeContainer.h>
#include <interface/IEventManager.h>
#include <ConstDef.h>

namespace Blade
{
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class BLADE_VCLASS_EXPORT LoopContainerIterface
	{
	public:
		/** @brief  */
		virtual void onLoopClear() = 0;
		/** @brief  */
		virtual void onLoopInit() {}

		/** @brief  */
		inline void	loopClear(const Event& data)
		{
			BLADE_UNREFERENCED(data);
			this->onLoopClear();
		}

		/** @brief  */
		inline void	loopInit(const Event& data)
		{
			BLADE_UNREFERENCED(data);
			this->onLoopInit();
		}

		inline LoopContainerIterface()
		{
			IEventManager::getSingleton().addEventHandler(ConstDef::EVENT_LOOP_DATA_INIT, EventDelegate(this, &LoopContainerIterface::loopInit));
			IEventManager::getSingleton().addEventHandler(ConstDef::EVENT_LOOP_DATA_CLEAR, EventDelegate(this, &LoopContainerIterface::loopClear));
		}

		inline LoopContainerIterface(const LoopContainerIterface&)
		{
			IEventManager::getSingleton().addEventHandler(ConstDef::EVENT_LOOP_DATA_INIT, EventDelegate(this, &LoopContainerIterface::loopInit));
			IEventManager::getSingleton().addEventHandler(ConstDef::EVENT_LOOP_DATA_CLEAR, EventDelegate(this, &LoopContainerIterface::loopClear));
		}

		virtual ~LoopContainerIterface()
		{
			IEventManager::getSingleton().removeEventHandlers(ConstDef::EVENT_LOOP_DATA_INIT, this);
			IEventManager::getSingleton().removeEventHandlers(ConstDef::EVENT_LOOP_DATA_CLEAR, this);
		}
	};

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	template <typename KeyT,typename T ,typename C = std::less<KeyT> , typename A = TempAllocator<KeyT>, typename CA = TempAllocatable  >
	class LoopMap : public TempMap<KeyT, T, C, A, CA>, public LoopContainerIterface
	{
	public:
		typedef TempMap<KeyT, T, C, A, CA> base_type;

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

		/** @brief  */
		virtual void onLoopClear() { this->clear(); }

		inline LoopMap()	{}

		template<typename InputIterator>
		inline LoopMap(InputIterator _begin, InputIterator _end)
			:base_type(_begin, _end){}
#if BLADE_DEBUG
		inline LoopMap(const char* file, int  line)
			: base_type(file,line)	{}
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
	class LoopSet : public TempSet<T, C, A, CA>, public LoopContainerIterface
	{
	public:
		typedef TempSet<T, C, A, CA> base_type;

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
		/** @brief  */
		virtual void onLoopClear() { this->clear(); }

		inline LoopSet()	{}

		template<typename InputIterator>
		inline LoopSet(InputIterator _begin, InputIterator _end)
			:base_type(_begin, _end){}
#if BLADE_DEBUG
		inline LoopSet(const char* file, int  line)
			: base_type(file,line)	{}
#endif
	};

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	template<typename T , typename A = TempAllocator<T>, typename CA = TempAllocatable >
	class LoopVector : public TempVector<T, A, CA>, public LoopContainerIterface
	{
	public:
		typedef TempVector<T, A, CA> base_type;

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

		/** @brief  */
		virtual void onLoopClear() { this->clear(); }

		inline LoopVector(){}

		template<typename InputIterator>
		inline LoopVector(InputIterator _begin, InputIterator _end)
			:base_type(_begin, _end)	{}
#if BLADE_DEBUG
		inline LoopVector(const char* file, int  line)
			: base_type(file,line)	{}
#endif

		/** @brief  */
		inline LoopVector(size_t count)
		{
			if (count > 0)
			{
				this->allocateImp();
				this->mImp->resize(count);
			}
		}

		/** @brief  */
		inline LoopVector(size_t count,const T& val)
		{
			if (count > 0)
			{
				this->allocateImp();
				this->mImp->resize(count, val);
			}
		}
	};

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	template<typename T, typename A = TempAllocator<T>, typename CA = TempAllocatable >
	class LoopList : public TempList<T, A, CA>, public LoopContainerIterface
	{
	public:
		typedef LoopList this_type;
		typedef TempList<T, A, CA> base_type;

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

		/** @brief  */
		virtual void onLoopClear() { this->clear(); }

		inline LoopList()	{}

		template <class InputIterator>
		LoopList(InputIterator first, InputIterator last,
			const allocator_type& alloc = allocator_type())
			:base_type(first, last)		{}

#if BLADE_DEBUG
		inline LoopList(const char* file, int  line)
			: base_type(file,line)	{}
#endif
	};

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	template<typename T, typename A = TempAllocator<T>, typename CA = TempAllocatable >
	class LoopDeque : public TempDeque<T, A, CA>, public LoopContainerIterface
	{
	public:
		typedef TempDeque<T, A, CA> base_type;

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
		/** @brief  */
		virtual void onLoopClear() { this->clear(); }

		inline LoopDeque()	{}
#if BLADE_DEBUG
		inline LoopDeque(const char* file, int  line)
			:base_type(file,line)	{}
#endif
	};

	template<typename T, typename A = TempAllocator<T>, typename CA = TempAllocatable >
	class LoopQueue : public TempQueue<T, A, CA>, public LoopContainerIterface
	{
	public:
		typedef T value_type;
		typedef LoopDeque<T, A, CA> container_type;
		typedef typename container_type::size_type size_type;
		typedef T& reference;
		typedef const T& const_reference;

		/** @brief  */
		virtual void onLoopClear() { this->clear(); }

		inline LoopQueue()	{}
#if BLADE_DEBUG
		inline LoopQueue(const char* file, int  line)
			:container_type(file,line)	{}
#endif
	};


	template<typename T, typename A = TempAllocator<T>, typename CA = TempAllocatable >
	class LoopStack : public TempStack<T, A, CA>, public LoopContainerIterface
	{
	public:
		typedef T value_type;
		typedef TempStack<T, A, CA> base_type;
		typedef typename base_type::size_type size_type;
		typedef T& reference;
		typedef const T& const_reference;

		/** @brief  */
		virtual void onLoopClear() { this->clear(); }

		inline LoopStack()	{}
#if BLADE_DEBUG
		inline LoopStack(const char* file, int  line)
			:base_type(file,line)	{}
#endif
	};

	/************************************************************************/
	/* Loop Concurrent Queue                                                                     */
	/************************************************************************/
	template<typename T, typename A = TempAllocator<T>, typename CA = TempAllocatable >
	class LoopConQueue : public TempConQueue<T, A, CA>, public LoopContainerIterface
	{
	protected:
		typedef		TempConQueue<T,A,CA>	QueueType;
	public:
		/** @brief  */
		virtual void onLoopClear() { this->clear(); }

		inline LoopConQueue(){}
#if BLADE_DEBUG
		inline LoopConQueue(const char* file, int  line)
			:QueueType(file,line)	{}	
#endif
		inline ~LoopConQueue(){}
	};

	/************************************************************************/
	/* string map using fast comparator and Loop allocator                                                                */
	/************************************************************************/
	template< typename T, typename A = TempAllocator<T> >
	class LoopTStringMap : public LoopMap<TString,T,FnTStringFastLess,A>
	{
	public:
		inline LoopTStringMap()	{}
#if BLADE_DEBUG
	inline LoopTStringMap(const char* file, int line) :LoopMap<TString,T,FnTStringFastLess,A>(file,line)	{}
#endif
	};

	/************************************************************************/
	/* string set using fast comparator and temp allocator                                                                     */
	/************************************************************************/
	class LoopTStringSet : public LoopSet< TString ,FnTStringFastLess >
	{
	public:
	
		LoopTStringSet() {}
#if BLADE_DEBUG
		LoopTStringSet(const char* file, int line)
			:LoopSet< TString ,FnTStringFastLess >(file,line)	{}	
#endif

		template <typename OtherAlloc>
		LoopTStringSet(const std::set<TString,OtherAlloc>& src )
		{
			for(typename std::set<TString,OtherAlloc>::const_iterator it = src.begin(); it != src.end(); ++it )
			{
				this->insert( *it );
			}
		}

		template <typename OtherAlloc>
		LoopTStringSet&	operator=(const std::set<TString,OtherAlloc>& src)
		{
			this->clear();
			for(typename std::set<TString,OtherAlloc>::const_iterator it = src.begin(); it != src.end(); ++it )
			{
				this->insert( *it );
			}
			return *this;
		}
	};

}//namespace Blade



#endif // __Blade_LoopContainers_h__