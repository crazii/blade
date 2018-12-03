/********************************************************************
	created:	2016/08/08
	filename: 	QtNewFileDialog.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_QtNewFileDialog_h__
#define __Blade_QtNewFileDialog_h__
#include <QtPropertyGrid.h>
#include <interface/public/ui/IUIMainWindow.h>

namespace Blade
{
	class QtNewFileDialog : public QDialog
	{
	public:
		QtNewFileDialog(QWidget* widget);
		~QtNewFileDialog();

		/** @brief  */
		void initialize(const IUIMainWindow::NEW_FILE_DESC* desc, size_t count);

		/** @brief  */
		inline index_t getSelected() const { return mSelectedIndex;}
		/** @brief  */
		const TString& getName() const {return mResultName;}
		/** @brief  */
		const TString& getFolder() const {return mResultFolder;}

	protected:
		/** @brief  */
		void onItemClick(QListWidgetItem* qitem);
		/** @brief  */
		virtual void accept();
		/** @brief  */
		void onBrowse();

		typedef Blade::TempVector<const Blade::IUIMainWindow::NEW_FILE_DESC*>	FileInfoList;

		QSplitter	mSplitter;
		QLabel		mNameLabel;
		QLabel		mFolderLabel;
		QLineEdit	mName;
		QLineEdit	mFolder;
		QPushButton	mOK;
		QPushButton mCancel;
		QPushButton	mBrowse;
		QListWidget	mTypes;
		QtPropertyGrid	mProperties;
		FileInfoList	mFileInfo;

		TString		mResultName;
		TString		mResultFolder;
		index_t		mSelectedIndex;
	};
	
}//namespace Blade


#endif // __Blade_QtNewFileDialog_h__