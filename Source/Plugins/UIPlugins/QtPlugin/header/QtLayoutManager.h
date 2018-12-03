/********************************************************************
	created:	2016/07/26
	filename: 	QtLayoutManager.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_QtLayoutManager_h__
#define __Blade_QtLayoutManager_h__
#include <interface/public/ui/IUILayoutManager.h>
#include <interface/public/ui/UIWidgetLayout.h>

namespace Blade
{
	class QtLayoutManager : public IUILayoutManager, public StaticAllocatable
	{
	public:
		QtLayoutManager(QMainWindow* mainWindow);
		~QtLayoutManager();

		/** @brief  create a window, if caption not specified,use name as caption */
		virtual IUIWindow*		createWindow(IconIndex icon, const TString& wtype, const TString& caption = TString::EMPTY);

		/** @brief get the active window */
		virtual IUIWindow*		getActiveWindow() const;

		/** @brief  */
		virtual IUIWidget*	createWidget(const WIDGET_DATA& widgetData);

		/** @brief  */
		virtual IUIWidget*	getWidget(const TString& name) const
		{
			UIWidgetFinder finder(name);
			WidgetSet::const_iterator i = mWidgets.find(&finder);
			return i != mWidgets.end() ? *i : NULL;
		}

		/** @brief get top level widget count */
		virtual size_t		getWidgetCount() const {return mWidgets.size();}

		/** @brief get top level widget */
		virtual IUIWidget*	getWidget(index_t index) const
		{
			if(index >= mWidgets.size())
			{
				assert(false);
				return NULL;
			}
			WidgetSet::const_iterator i = mWidgets.begin();
			std::advance(i, (indexdiff_t)index);
			return *i;
		}

		/** @brief apply layout to default, by using widget's layout data */
		virtual bool		applyLayout(const UIWidgetLayout& layout);

		/** @brief  */
		virtual bool		loadLastLayout();

		/** @brief  */
		virtual bool		saveLayout() const;

	protected:
		typedef Set<IUIWidget*, FnWidgetNameLess> WidgetSet;

		UIWidgetLayout	mLayout;
		WidgetSet		mWidgets;
		QMainWindow*	mOwner;
	};
	
}//namespace Blade


#endif // __Blade_QtLayoutManager_h__