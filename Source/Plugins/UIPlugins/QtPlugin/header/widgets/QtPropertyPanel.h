/********************************************************************
	created:	2016/7/27
	filename: 	QtPropertyPanel.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_QtPropertyPanel_h__
#define __Blade_QtPropertyPanel_h__
#include <interface/public/ui/widgets/IUIPropertyGrid.h>
#include <databinding/IDataSourceDataBase.h>
#include <widgets/QtWidgetBase.h>
#include <QtPropertyGrid.h>

namespace Blade
{
	class QtPropertyPanel : public QtWidgetBase<IUIPropertyGrid>, public Allocatable
	{
	public:
		static const TString WIDGET_TYPE;
	public:
		QtPropertyPanel();
		~QtPropertyPanel();

		/************************************************************************/
		/* IUIPropertyGrid interface                                                                     */
		/************************************************************************/
		/** @brief */
		virtual bool	bindEntity(const IEntity* entity);

		/** @brief  */
		virtual const IEntity* getBoundEntity() const { return mTargetEntity; }

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		void	clear();

		/** @brief  */
		void	onBindProperty(const Event& data);

	protected:
		/** @brief  */
		virtual void showEvent(QShowEvent* evt)
		{
			if (mTargetEntity != NULL)
			{
				mRootConfig->updateData(true);
				mPropGrid.bindConfigs(mRootConfig);
			}
			QtWidgetBase<IUIPropertyGrid>::showEvent(evt);
		}

		typedef List<DataBinder*> BinderList;

		QtPropertyGrid	mPropGrid;
		const IEntity*	mTargetEntity;
		HCONFIG			mRootConfig;
		BinderList		mConfigBinder;	//long time binding
	};
	
}//namespace Blade

#endif//__Blade_QtPropertyPanel_h__