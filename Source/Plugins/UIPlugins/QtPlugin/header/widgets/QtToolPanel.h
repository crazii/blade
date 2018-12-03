/********************************************************************
	created:	2016/7/26
	filename: 	QtToolPanel.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_QtToolPanel_h__
#define __Blade_QtToolPanel_h__
#include <interface/public/ui/widgets/IUIToolPanel.h>
#include <widgets/QtWidgetBase.h>

namespace Blade
{
	class QtToolPanel : public QtWidgetBase<IUIToolPanel>, public Allocatable
	{
	public:
		static const TString WIDGET_TYPE;
	public:
		QtToolPanel();
		~QtToolPanel();

		/** @brief  */
		virtual bool	setupCategory(const TString& defCategory, IconIndex defIcon, const TString* categories, IconIndex* icons, size_t count);

		/** @brief add tool box with category info. if categories are empty, default category will be used, and
		the toolbox will always visible when switch between categories */
		virtual bool	addToolBox(const TStringParam& categories, IUIToolBox* toolbox, bool allowMove = true);

		/** @brief  */
		virtual bool	switchCategory(const TString& category);

		/** @brief  */
		virtual const TString& getDefaultCategory() const {return mDefCategory;}

	protected:
		/** @brief  */
		void onSwitchCategory(int index);

		/** @brief  */
		virtual void resizeEvent(QResizeEvent* evt);
		virtual void showEvent(QShowEvent* evt);
		virtual void hideEvent(QHideEvent* evt);
		virtual void visibilityUpdate();

		typedef List<QToolButton*> ToolButtonList;
		struct ToolInfo
		{
			IUIToolBox*		toolBox;
			ToolButtonList	buttons;
			bool			allowMove;
		};
		typedef List<ToolInfo> ToolList;

		struct Category
		{
			TString name;
			IconIndex icon;
			ToolList tools;
			bool operator==(const Category& rhs) const {return name == rhs.name;}
		};

		typedef List<Category> CategoryList;

		QWidget	mContent;
		TString	mDefCategory;
		CategoryList mCategoryList;
		QTimer	mTimer;
		index_t mCateIndex;
		int16	mButtonSize;
		int8	mVisibleUpdate;
	};
	
}//namespace Blade


#endif//__Blade_QtToolPanel_h__