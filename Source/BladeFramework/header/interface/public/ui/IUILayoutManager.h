/********************************************************************
	created:	2016/6/12
	filename: 	IUILayoutManager.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_IUILayoutManager_h__
#define __Blade_IUILayoutManager_h__
#include "UITypes.h"
#include "IUIWidget.h"

namespace Blade
{
	class UIWidgetLayout;
	class IUIWindow;

	class IUILayoutManager
	{
	public:
		static const BLADE_FRAMEWORK_API TString WORKING_AREA;
	public:
		virtual ~IUILayoutManager() {}

		/** @brief  create a window, if caption not specified,use name as caption */
		virtual IUIWindow*		createWindow(IconIndex icon, const TString& wtype, const TString& caption = TString::EMPTY) = 0;

		/** @brief get the active window */
		virtual IUIWindow*		getActiveWindow() const = 0;

		/** @brief  */
		virtual IUIWidget*	createWidget(const WIDGET_DATA& widgetData) = 0;

		/** @brief  */
		virtual IUIWidget*	getWidget(const TString& name) const = 0;

		/** @brief get top level widget count */
		virtual size_t		getWidgetCount() const = 0;

		/** @brief get top level widget */
		virtual IUIWidget*	getWidget(index_t index) const = 0;

		/** @brief apply layout to default, by using widget's layout data */
		virtual bool		applyLayout(const UIWidgetLayout& layout) = 0;

		/** @brief  */
		virtual bool		loadLastLayout() = 0;

		/** @brief  */
		virtual bool		saveLayout() const = 0;

		/** @brief  */
		virtual bool		toggleWidget(IUIWidget* widget, bool visible)
		{
			return widget != NULL && (visible != widget->isVisible()) && widget->setVisible(visible);
		}

		/** @brief  */
		virtual bool		activateWidget(IUIWidget* widget, bool active)
		{
			return widget != NULL && (active != widget->isActive()) && widget->setActive(active);
		}
	};
	
}//namespace Blade


#endif//__Blade_IUILayoutManager_h__