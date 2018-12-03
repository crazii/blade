/********************************************************************
	created:	2013/12/03
	filename: 	Bindable.h
	author:		Crazii
	purpose:	data binding base class for bindable target, used only for static cast
*********************************************************************/
#ifndef __Blade_Bindable_h__
#define __Blade_Bindable_h__
#include <BladeBase.h>

namespace Blade
{
	//this empty class is used for static casting between multiple inheritances
	class Bindable
	{
#if BLADE_DEBUG //for dynamic cast check in debug runtime
	public:
		inline virtual ~Bindable()	{}
#endif
	};

	template<typename T>
	struct BindableType
	{
		typedef typename std::remove_pointer<T>::type type;	//TODO: decay
	};

	template<typename T>
	struct IsBindable
	{
		enum { value = std::is_base_of<Bindable, typename BindableType<T>::type>::value };
	};
	
}//namespace Blade

#endif //  __Blade_Bindable_h__