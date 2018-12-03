/********************************************************************
	created:	2016/7/25
	filename: 	QtPropertyGrid.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_QtPropertyGrid_h__
#define __Blade_QtPropertyGrid_h__
#include <QtPropertyHelper.h>

namespace Blade
{
	class QtPropertyGrid
	{
	public:
		QtPropertyGrid(QWidget* parent);
		~QtPropertyGrid();

		/** @brief  */
		QWidget*		getWidget();

		/** @brief  */
		void			bindConfigs(IConfig* root);

		/** @brief  */
		size_t			getHeight();

	protected:
		/** @brief  */
		void			addConfig(IConfig* config, IConfig* parentConfig, QtProperty* parentProp);

		QtAbstractPropertyBrowser*	mImp;
		QtPropertyHelper			mHelper;
	};


	
}//namespace Blade

#endif//__Blade_QtPropertyGrid_h__