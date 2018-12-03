/********************************************************************
	created:	2016/6/8
	filename: 	IUILogView.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_IUILogView_h__
#define __Blade_IUILogView_h__
#include <interface/public/ui/IUIWidget.h>
#include <interface/ILog.h>

namespace Blade
{
	class IUILogView : public IUIWidgetLeaf
	{
	public:
		virtual ~IUILogView() {}

		/** @brief enable/disable log redirection. default: true */
		virtual bool enableLogRedirection(bool enable = true) = 0;

	};//class IUILogView
	
}//namespace Blade


#endif//__Blade_IUILogView_h__