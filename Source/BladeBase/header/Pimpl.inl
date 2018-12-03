/********************************************************************
	created:	2012/02/26
	filename: 	Pimpl.inl
	author:		Crazii
	purpose:	
*********************************************************************/
#include <Pimpl.h>
#include <memory/BladeMemory.h>
#include <memory>

//for one internal class A, its header file using Pimpl template(only Pimpl.h), it will only include the Pimpl.inl to its source file.
//and then the destruct() & construct() is visible.
//this is why & how this .inl file is used

//!!!please take care of MSVC C4150 warning, \n
//because it may cause error on DLL export class if other .cc/.cpp file (none-implementation)\n
//include the Pimpl.inl and compile it.

//detail: if another class B using Pimpl is included(used) by the source of class A, the compiler will also see the Pimpl<class B>::destruct() as "BLADE_DELETE B".
//but it doesn't know class B 's complete type here, thus the compiler cannot generate the right delete operator, 
//but the MSVC doesn't generate any error. instead, it genterate a (global scope) ::operator delete without dector for it.
//although the compiler may generate the right version of destruct() (call DELETE B) when compiling class B's source file,
//but the linker of MSVC may choose the first incorrect one, so when the Pimpl<B> object destructed, 
//it call ::operator delete directly(which may be incorrect) without calling the destructor.

//what will happen if compiler is GCC?

//update: 21/Dec/2013: use a traits (PimplTraits) to delete the binding object T in runtime, this avoid code conflict
//to avoid the problem above.
//tested for MSVC & GCC

namespace Blade
{

}//namespace Blade