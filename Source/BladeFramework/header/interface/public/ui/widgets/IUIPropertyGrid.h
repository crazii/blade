/********************************************************************
	created:	2016/6/8
	filename: 	IUIPropertyGrid.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_IUIPropertyGrid_h__
#define __Blade_IUIPropertyGrid_h__
#include <interface/public/ui/IUIWidget.h>

namespace Blade
{
	class IEntity;

	class IUIPropertyGrid : public IUIWidgetLeaf
	{
	public:
		virtual ~IUIPropertyGrid() {}
		
		/** @brief */
		virtual bool	bindEntity(const IEntity* entity) = 0;

		/** @brief  */
		virtual const IEntity* getBoundEntity() const = 0;
	};
	
}//namespace Blade


#endif//__Blade_IUIPropertyGrid_h__