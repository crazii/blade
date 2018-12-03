/********************************************************************
	created:	2012/08/22
	filename: 	AllocatorBase.h
	author:		Crazii
	purpose:	remove allocator base from BladeMemory.h
*********************************************************************/
#ifndef __Blade_AllocatorBase_h__
#define __Blade_AllocatorBase_h__

namespace Blade
{

	template < typename T >
	struct AllocatorBase
	{
		typedef T value_type;
	};

	template< typename T >
	struct AllocatorBase< const T >
	{
		typedef T value_type;
	};

}//namespace Blade

#endif //  __Blade_AllocatorBase_h__

