/********************************************************************
	created:	2016/06/06
	filename: 	IUIToolPanel.h
	author:		Crazii
	
	purpose:	place holder for tool boxes (IUIToolBox)
*********************************************************************/
#ifndef __Blade_IUIToolPanel_h__
#define __Blade_IUIToolPanel_h__
#include <interface/public/ui/IUIWidget.h>

namespace Blade
{
	class IUIToolBox;

	class IUIToolPanel : public IUIWidgetLeaf
	{
	public:
		virtual ~IUIToolPanel() {}

		/** @brief  */
		virtual bool	setupCategory(const TString& defCategory, IconIndex defIcon, const TString* categories, IconIndex* icons, size_t count) = 0;

		/** @brief add tool box with category info. if categories are empty, default category will be used, and
		the toolbox will always visible when switch between categories */
		virtual bool	addToolBox(const TStringParam& categories, IUIToolBox* toolbox, bool allowMove = true) = 0;

		/** @brief  */
		virtual bool	switchCategory(const TString& category) = 0;

		/** @brief  */
		virtual const TString& getDefaultCategory() const = 0;
	};

}//namespace Blade


#endif // __Blade_IUIToolPanel_h__