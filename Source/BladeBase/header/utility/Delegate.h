/********************************************************************
	created:	2011/04/21
	filename: 	Delegate.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_Delegate_h__
#define __Blade_Delegate_h__
#include <Pimpl.h>

namespace Blade
{
	///21/Nov/2013: use pointer to static template function instead of virtual interface
	///remove virtual calls can hardly increase runtime performance for this case
	///so it is almost the same as the former virtual one, but may boost compiling process
	namespace DelegateUtil
	{
		typedef void (*FN_MEMBER_INVOKE)(uintptr_t ptr, const char* mfunc, void* data);
		typedef void (*FN_COMMON_INVOKE)(const char* func, void* data);

		template <typename T>
		static void	MemberInvoke(uintptr_t ptr, const char* mfunc, void* data)
		{
			BLADE_UNREFERENCED(data);
			assert(data == NULL);
			typedef void (T::*TMemberFn)();
			TMemberFn TFn = NULL;
			std::memcpy(&TFn, mfunc, sizeof(TFn) );
			ptr &= ~0x1;
			( ((T*)ptr)->*TFn )();
		}

		template <typename T>
		static void	MemberInvokePointer(uintptr_t ptr, const char* mfunc, void* data)
		{
			typedef void (T::*TMemberFn)(void* data);
			TMemberFn TFn = NULL;
			std::memcpy(&TFn, mfunc, sizeof(TFn) );
			ptr &= ~0x1;
			( ((T*)ptr)->*TFn )( data );
		}

		template <typename T, typename V>
		static void	MemberInvoke(uintptr_t ptr, const char* mfunc, void* data)
		{
			typedef void (T::*TMemberFn)(V& data);
			TMemberFn TFn = NULL;
			std::memcpy(&TFn, mfunc, sizeof(TFn) );
			ptr &= ~0x1;
			( ((T*)ptr)->*TFn )( *(V*)(void*)data );
		}

		static void CommonInvoke(const char* func, void* data)
		{
			BLADE_UNREFERENCED(data);
			assert(data == NULL);
			void(*CommonFn)(void) = NULL;
			std::memcpy(&CommonFn, func, sizeof(CommonFn) );
			(*CommonFn)();
		}

		static void CommonInvokePointer(const char* func, void* data)
		{
			void(*CommonFnVP)(void*) = NULL;
			std::memcpy(&CommonFnVP, func, sizeof(CommonFnVP) );
			(*CommonFnVP)(data);
		}

		template<typename V>
		static void CommonInvoke(const char* func, void* data)
		{
			void (*CommonFnV)(V& data) = NULL;
			std::memcpy(&CommonFnV, func, sizeof(CommonFnV) );
			(*CommonFnV)( *(V*)(void*)data );
		}
	}

	//another ref:http://www.codeproject.com/Articles/11015/The-Impossibly-Fast-C-Delegates
	//which don't use byte buffer to store pointer to member,
	//but need pointer to member as a TEMPLATE ARGUMENT (explicit) (no auto construction)
	class Delegate
	{
		//note: pointer to member function is not normal pointer, so it cannot be cast to normal function pointer
		//but it is POD type (C++03 3.9-10) and can be copied
		//TODO: check different compilers for the compatibility of member function of virtual/multiple inheritance
		//virtual/single/multiple inheritance: are they the same size for the same compiler?
		//there's table: http://www.codeproject.com/Articles/7150/Member-Function-Pointers-and-the-Fastest-Possible
		/*
		Compiler			Options		Single	Multi	Virtual	Unknown
			MSVC	 					4		8		12		16
			MSVC			/vmg		16#		16#		16#		16
			MSVC			/vmg /vmm	8#		8#		--		8
			Intel_IA32	 				4		8		12		16
			Intel_IA32		/vmg /vmm	4		8		--		8
			Intel_Itanium	 			8		12		16		20
			G++	 						8		8		8		8
			Comeau	 					8		8		8		8
			DMC	 						4		4		4		4
			BCC32	 					12		12		12		12
			BCC32			/Vmd		4		8		12		12
			WCL386	 					12		12		12		12
			CodeWarrior	 				12		12		12		12
			XLC	 						20		20		20		20
			DMC	small					2		2		2		2
			medium						4		4		4		4
			WCL	small					6		6		6		6
			compact						6		6		6		6
			medium						8		8		8		8
			large						8		8		8		8
		*/
		//we need compiler option to keep at least single/multi inheritance are the same.
		//or use char [max] and memcpy to copy pointer data.
		//TODO: use unknown (forward decl type) instead of MAX_SIZE, but is still a kind of hack
		//maybe to use a constant large enough size.
		static const size_t MAX_SIZE = MAX_POINTER2MEMBER_SIZE;
		uintptr_t		mPtr;		//pointer to object
		char			mFunctionRaw[MAX_SIZE];	//pointer to member function (raw data) TODO: alignment?
		
		union
		{
			DelegateUtil::FN_MEMBER_INVOKE	mMemberInvoker;	//pointer to invoker function: actually template function with type info
			DelegateUtil::FN_COMMON_INVOKE	mInvoker;
		};

		/** @brief default EMPTY data */
		BLADE_BASE_API static const char ZERO[MAX_SIZE];

	public:
		explicit Delegate(void (*func)()) :mPtr(0),mInvoker( &DelegateUtil::CommonInvoke )
		{
			std::memset(mFunctionRaw,0,MAX_SIZE);
			std::memcpy(mFunctionRaw, &func, sizeof(func) );
		}
		explicit Delegate(void (*funcV)(void* data)) :mPtr(0x1),mInvoker( &DelegateUtil::CommonInvokePointer )
		{
			std::memset(mFunctionRaw,0,MAX_SIZE);
			std::memcpy(mFunctionRaw, &funcV, sizeof(funcV) );
		}
		template<typename V>
		explicit Delegate(void (*funcV)(V& data)) :mPtr(0x1),mInvoker( &DelegateUtil::CommonInvoke<V> )
		{
			std::memset(mFunctionRaw, 0, MAX_SIZE);
			std::memcpy(mFunctionRaw, &funcV, sizeof(funcV));
		}

		template <typename T>
		explicit Delegate(T* ptr, void (T::*func)() )
			:mPtr( (uintptr_t)ptr), mMemberInvoker( &DelegateUtil::MemberInvoke<T> )
		{
			assert( sizeof(func) <= MAX_SIZE );
			assert( (mPtr&0x1 ) == 0 ); //must have alignment
			std::memset(mFunctionRaw, 0, MAX_SIZE);
			std::memcpy(mFunctionRaw, &func, sizeof(func) );
		}
		template <typename T>
		explicit Delegate(T* ptr, void (T::*func)(void*) )
			:mPtr( (uintptr_t)ptr), mMemberInvoker( &DelegateUtil::MemberInvokePointer<T> )
		{
			assert( sizeof(func) <= MAX_SIZE );
			assert( (mPtr&0x1 ) == 0 ); //must have alignment
			mPtr |= 0x01;
			std::memset(mFunctionRaw, 0, MAX_SIZE);
			std::memcpy(mFunctionRaw, &func, sizeof(func) );
		}

		template <typename T, typename V>
		explicit Delegate(T* ptr, void (T::*funcV)(V&) )
			:mPtr( (uintptr_t)ptr), mMemberInvoker( &DelegateUtil::MemberInvoke<T,V> )
		{
			assert( sizeof(funcV) <= MAX_SIZE );
			assert( (mPtr&0x1 ) == 0 ); //must have alignment
			mPtr |= 0x01;
			std::memset(mFunctionRaw, 0, MAX_SIZE);
			std::memcpy(mFunctionRaw, &funcV, sizeof(funcV) );
		}

		Delegate() :mPtr(0), mInvoker(NULL)			{ std::memset(mFunctionRaw, 0, MAX_SIZE);}

		/** @brief  */
		~Delegate()	{}

		/** @brief  */
		inline bool	isEmpty() const					{return ::memcmp(mFunctionRaw, ZERO, MAX_SIZE) == 0;}

		/** @brief whether bound to an object, or a direct call to a function */
		inline bool	isBound() const					{return this->getBoundTarget() != NULL;}

		/** @brief  */
		inline bool hasParameter() const			{return (mPtr&0x1) != 0;}

		/** @brief  */
		inline void* getBoundTarget() const			{return (void*)(mPtr&(~0x1));}

		inline bool	equals(void* ptr) const
		{
			if (this->isBound())
				return this->getBoundTarget() == ptr;
			else
			{
				typedef void(*_PFN)(void);
				return std::memcmp(mFunctionRaw, ptr, sizeof(_PFN)) == 0;
			}
		}

		/** @brief  */
		inline bool operator<(const Delegate& rhs) const
		{
			return (this->isBound() && !rhs.isBound() )
				|| ( mPtr < rhs.mPtr || (mPtr == rhs.mPtr && ::memcmp(mFunctionRaw, rhs.mFunctionRaw, Delegate::MAX_SIZE) < 0 ));
		}

		inline void	call() const
		{
			if( !this->isEmpty() )
			{
				assert( !this->hasParameter() );
				if( this->isBound() )
					mMemberInvoker(mPtr, mFunctionRaw, NULL);
				else
					mInvoker(mFunctionRaw, NULL);
			}
		}

		inline void call(void* data) const
		{
			if (!this->isEmpty())
			{
				assert(this->hasParameter());
				if (this->isBound())
					mMemberInvoker(mPtr, mFunctionRaw, data);
				else
					mInvoker(mFunctionRaw, data);
			}
		}

		/** @brief call the delegate */
		/* @param [in] data: user-defined data for custom protocol */
		template <typename V>
		inline void call(V& data) const
		{
			return this->call( (void*)&data );
		}

	public:
		BLADE_BASE_API static const Delegate		EMPTY;
	};

	//////////////////////////////////////////////////////////////////////////
	namespace Impl
	{
		class DelegateListImpl;
	}

	class BLADE_BASE_API DelegateList : public Allocatable
	{
	public:
		DelegateList();
		~DelegateList();

		DelegateList(const DelegateList& src);
		DelegateList& operator=(const DelegateList& rhs);

		/** @brief  */
		void			push_back(const Delegate& _delegate);

		/** @brief  */
		bool			erase(index_t index);

		/** @brief  */
		const Delegate& at(index_t index) const;
		inline const Delegate& operator[](index_t index) const
		{
			return this->at(index);
		}

		/** @brief  */
		size_t			size() const;

		/** @brief call the delegate */
		/* @param [in] data: user-defined data for custom protocol */
		void			call(void* data) const;	

		/** @brief  */
		void			call() const;

		template <typename V>
		inline void call(V& data) const
		{
			this->call(&data);
		}

	private:
		LazyPimpl<Impl::DelegateListImpl>	mList;
	};
	

}//namespace Blade



#endif // __Blade_Delegate_h__