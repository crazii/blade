/********************************************************************
	created:	2009/02/11
	filename: 	Functors.h
	author:		Crazii
	
	purpose:	some STL compatible functors,used in precompiled header
*********************************************************************/
#ifndef __Blade_Functors_h__
#define __Blade_Functors_h__
#include <functional>

namespace Blade
{
	/**
	@describe std functional extension ,storing the result
	*/
	template<typename T>
	struct FnMemFunBool : public std::mem_fun_t<bool,T>
	{
		typedef typename std::mem_fun_t<bool,T> base;

		FnMemFunBool( bool (T::*pFun)())
			:base(pFun)
			,mResult(true)
		{}

		void operator()(T *ptr) const
		{
			if( !base::operator()(ptr) )
				mResult = false;
		}
		bool mResult;
	};

	/**
	@describe std functional extension ,storing the bool result
	result will be false if any one is false.
	*/
	template< typename T,typename ArgT>
	struct FnMemFun1Bool : public std::mem_fun1_t<bool,T,ArgT>
	{
		typedef typename std::mem_fun1_t<bool,T,ArgT> base;

		FnMemFun1Bool( bool (T::*pFun)(ArgT) )
			:base(pFun)
			,mResult(true)
		{}

		bool operator()(T *ptr,ArgT arg) const
		{
			if( !base::operator()(ptr,arg) )
				mResult = false;
			return mResult;
		}
		mutable bool mResult;
	};

	template<typename FN>
	struct FnBinder2ndBool : public std::binder2nd<FN>
	{
	public:
		typedef typename FN::second_argument_type arg_type;

		FnBinder2ndBool(const FN& fn,const arg_type& arg)
			: std::binder2nd<FN>(fn,arg)
		{
		}

		bool getResult()
		{
			return this->op.mResult;
		}
	};

	namespace Fn
	{
		
		template< typename T,typename ArgT >
		FnMemFun1Bool<T,ArgT> BoolMemFun1( bool (T::*pFun)(ArgT) )
		{
			return FnMemFun1Bool<T,ArgT>(pFun);
		}

		template< typename T >
		FnMemFunBool<T> BoolMemFun( bool (T::*pFun)(void) )
		{
			return FnMemFunBool<T>(pFun);
		}

		template< typename T, typename FN>
		FnBinder2ndBool<FN> BoolBind2nd( const FN& fn,const T& arg )
		{
			typename FN::second_argument_type val(arg);
			return FnBinder2ndBool<FN>(fn, val);
		}
		
	}//namespace Fn

}//namespace Blade

#endif // __Blade_Functors_h__