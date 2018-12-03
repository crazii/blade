/********************************************************************
	created:	2016/07/25
	filename: 	QtConfigDialog.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_QtConfigDialog_h__
#define __Blade_QtConfigDialog_h__
#include <QtPropertyGrid.h>

namespace Blade
{
	const TString CONFIG_DIALOG_SECTION = BTString("ConfigDialog");

	class QtConfigDialog : public IConfigDialog, public QDialog, public Allocatable
	{
	public:
		enum EMode
		{
			NORMAL,
			TOPLEVEL,
			POPUP,
		};
	public:
		QtConfigDialog(QWidget* parent = NULL, Qt::WindowFlags f = Qt::WindowFlags());
		~QtConfigDialog();

		/************************************************************************/
		/* IConfigDialog interface                                                                     */
		/************************************************************************/
		/**
		@describe config via dialog(maybe no change) or canceled
		@param modal if the dialog is modal
		@return show state
		@retval true showed & confirmed successfully
		@retval false dialog not showed or canceled
		*/
		virtual bool show(const INFO& info, const HCONFIG* configs, size_t count,
			const TString& caption = BTString("Blade Config") );

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		bool addConfig(IConfig* configItem);
		/** @brief  */
		bool show(const INFO& info, const TString& caption);

		/** @brief top level mode contains a image banner and an extra check box. default: false */
		void setMode(EMode mode);

		/** @brief  */
		virtual void accept();
		virtual void reject();

	protected:
		/** @brief  */
		bool		showImpl(const INFO& info, const TString& caption);

		/** @brief  */
		virtual void hideEvent(QHideEvent* hevent);
		/** @brief  */
		virtual void showEvent(QShowEvent* evt);

		/** @brief  */
		void onTabChange(int index);

	protected:
		typedef List<IConfig*> ConfigList;

		QLabel			mBanner;
		QTabBar			mTab;
		QtPropertyGrid	mPropGrid;
		QPushButton		mOK;
		QPushButton		mCancel;
		QCheckBox		mCheckAutoShow;

		INFO			mInfo;
		ConfigList		mConfigList;
		EMode			mMode;
	};
	
}//namespace Blade


#endif // __Blade_QtConfigDialog_h__