/********************************************************************
	created:	2013/01/13
	filename: 	InfoWindow.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_InfoWindow_h__
#define __Blade_InfoWindow_h__
#include <ui/DockWindow.h>

namespace Blade
{
	class InfoWindow : public DockWindowBase<IUIWidgetLeaf>, public Allocatable
	{
	public:
		static const TString WIDGET_TYPE;
	public:
		using Allocatable::operator new;
		using Allocatable::operator delete;
		using Allocatable::operator new[];
		using Allocatable::operator delete[];
	public:
		InfoWindow();
		~InfoWindow();
	};

	
}//namespace Blade

#endif //  __Blade_InfoWindow_h__