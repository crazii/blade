/********************************************************************
	created:	2011/06/05
	filename: 	BiRef.h
	author:		Crazii
	purpose:	mutual reference
*********************************************************************/
#ifndef __Blade_BiRef_h__
#define __Blade_BiRef_h__
#include <BladeBase.h>

namespace Blade
{
	class BLADE_BASE_API BiRef
	{
	public:
		explicit BiRef(BiRef* that = NULL);
		~BiRef();

		BiRef(BiRef& src);
		BiRef&	operator=(BiRef& rhs);

		/**
		@describe 
		@param 
		@return 
		*/
		inline BiRef*		getRef() const	{return mRef;}

	protected:

		/**
		@describe 
		@param 
		@return 
		*/
		void				detach();

	protected:

		/**
		@describe 
		@param 
		@return 
		*/
		void				setRef(BiRef* Ref)	{mRef = Ref;}

		BiRef*	mRef;
	};
	

}//namespace Blade



#endif // __Blade_BiRef_h__