/********************************************************************
	created:	2016/08/08
	filename: 	QtNewFileDialog.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <QtPluginPCH.h>
#include <QtNewFileDialog.h>
#include <QtIconManager.h>

namespace Blade
{
	class QtFileTypeItem : public QListWidgetItem, public TempAllocatable
	{
	public:
		QtFileTypeItem(const QIcon& icon, const QString& text) :QListWidgetItem(icon, text) {}
		QtFileTypeItem(const QString& text) :QListWidgetItem(text) {}
		index_t mIndex;
		virtual bool operator<(const QListWidgetItem &other) const { return mIndex < static_cast<const QtFileTypeItem&>(other).mIndex; }
	};

	//////////////////////////////////////////////////////////////////////////
	QtNewFileDialog::QtNewFileDialog(QWidget* widget)
		:QDialog(widget)
		,mProperties(&mSplitter)
		,mSelectedIndex(INVALID_INDEX)
	{
		this->setWindowTitle(TString2QString(BTLang("New File")));
		QVBoxLayout* layout = new QVBoxLayout();
		this->setLayout(layout);

		layout->addWidget(&mSplitter);

		mSplitter.setCollapsible(0, false);
		mSplitter.setCollapsible(1, false);
		mSplitter.addWidget(&mTypes);
		mSplitter.addWidget( mProperties.getWidget() );
		mTypes.setViewMode(QListView::ListMode);
		mTypes.setResizeMode(QListView::Adjust);
		mTypes.setSelectionMode(QAbstractItemView::SingleSelection);
		mTypes.setSelectionBehavior(QAbstractItemView::SelectRows);
		QObject::connect(&mTypes, &QListWidget::itemClicked, this, &QtNewFileDialog::onItemClick);

		QHBoxLayout* line1 = new QHBoxLayout();
		layout->addLayout(line1);

		mNameLabel.setText(TString2QString(BTLang(BLANG_NAME)));
		line1->addWidget(&mNameLabel);
		line1->addWidget(&mName);
		line1->addWidget(&mOK);
		mOK.setText(TString2QString(BTLang(BLANG_OK)));
		QObject::connect(&mOK, &QPushButton::clicked, this, &QDialog::accept);

		QHBoxLayout* line2 = QT_NEW QHBoxLayout();
		layout->addLayout(line2);

		mFolderLabel.setText(TString2QString(BTLang(BLANG_FILE_FOLDER)));
		line2->addWidget(&mFolderLabel);
		line2->addWidget(&mFolder);
		line2->addWidget(&mBrowse);
		line2->addWidget(&mCancel);
		mFolder.setEnabled(false);
		mCancel.setText(TString2QString(BTLang(BLANG_CANCEL)));
		mBrowse.setText(TString2QString(BTLang(BLANG_BROWSE)));
		QObject::connect(&mCancel, &QPushButton::clicked, this, &QDialog::reject);
		QObject::connect(&mBrowse, &QPushButton::clicked, this, &QtNewFileDialog::onBrowse);

		int width = std::max( mFolderLabel.minimumSizeHint().width(), mNameLabel.minimumSizeHint().width()) ;
		mNameLabel.setMinimumWidth(width);
		mFolderLabel.setMinimumWidth(width);
	}

	//////////////////////////////////////////////////////////////////////////
	QtNewFileDialog::~QtNewFileDialog()
	{
		//note: QSplitter will delete its children, but we don't need that
		mTypes.setParent(NULL);
		//QtPropertyGrid will delete its widget
		mProperties.getWidget()->setParent(NULL);
	}

	//////////////////////////////////////////////////////////////////////////
	void QtNewFileDialog::initialize(const IUIMainWindow::NEW_FILE_DESC* desc, size_t count)
	{
		mFileInfo.clear();
		mFileInfo.reserve(count);
		for(size_t i = 0; i < count; ++i)
			mFileInfo.push_back(desc+i);

		for(size_t i = 0; i < count; ++i)
		{
			QtFileTypeItem* qitem;
			QIcon* icon = QtIconManager::getSingleton().getIcon( desc[i].mIcon );
			if(icon != NULL)
				qitem = BLADE_NEW QtFileTypeItem(*icon, TString2QString(desc[i].mDesc));
			else
				qitem = BLADE_NEW QtFileTypeItem(TString2QString(desc[i].mDesc));

			qitem->mIndex = i;
			qitem->setFlags(desc[i].mEnable ? Qt::ItemIsEnabled|Qt::ItemIsSelectable : Qt::ItemIsSelectable);
			mTypes.addItem(qitem);
			mTypes.setItemSelected(qitem, false);
		}
		mTypes.sortItems(Qt::AscendingOrder);

		for (size_t i = 0; i < mTypes.count(); ++i)
		{
			QtFileTypeItem* _item = static_cast<QtFileTypeItem*>(mTypes.item(i));
			index_t index = _item->mIndex;

			const IUIMainWindow::NEW_FILE_DESC* curDesc = mFileInfo[index];
			if (curDesc->mEnable)
			{
				mTypes.setItemSelected(_item, true);
				//note: select won't generate click event
				this->onItemClick(_item);
				break;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void QtNewFileDialog::onItemClick(QListWidgetItem* qitem)
	{
		if(!(qitem->flags()&Qt::ItemIsEnabled))
		{
			mFolder.setText( tr("") );
			mProperties.bindConfigs( NULL );
			return;
		}

		QtFileTypeItem* _item = static_cast<QtFileTypeItem*>(qitem);
		index_t index = _item->mIndex;

		const IUIMainWindow::NEW_FILE_DESC* desc = mFileInfo[index];
		mProperties.bindConfigs( desc->mNewFileConfig );
		TString path = TStringHelper::standardizePath(desc->mDefaultPath);
		mFolder.setText(TString2QString(path));
	}

	//////////////////////////////////////////////////////////////////////////
	void QtNewFileDialog::accept()
	{
		TString name = QString2TString(mName.text());
		TString folder = QString2TString(mFolder.text());
		HFILEDEVICE hFolder(BLADE_FACTORY_CREATE(IFileDevice, IFileDevice::DEFAULT_FILE_TYPE));
		hFolder->load(folder);
		QList<QListWidgetItem*> selected = mTypes.selectedItems();

		if(selected.count() != 1)
			QMessageBox::critical(this, TString2QString(BTLang(BLANG_ERROR)), TString2QString(BTLang("Please Select a file type.")), QMessageBox::Ok);		
		else if( name.find(TEXT("<>/\\:.\"|?*")) != TString::npos || name.size() < 2 )
			QMessageBox::critical(this, TString2QString(BTLang(BLANG_ERROR)), TString2QString(BTLang("Invallid File Name.")), QMessageBox::Ok);		
		else if( folder.find(TEXT("<>/\\:.\"|?*")) != TString::npos || folder.size() < 1)
			QMessageBox::critical(this, TString2QString(BTLang(BLANG_ERROR)), TString2QString(BTLang("Invallid Folder selected.")), QMessageBox::Ok);
		else if(!hFolder->isLoaded() || hFolder->isReadOnly() )
			QMessageBox::critical(this, TString2QString(BTLang(BLANG_ERROR)), TString2QString(BTLang("Selected Folder doesn't exist or is read only.")), QMessageBox::Ok);
		else
		{
			mResultName = name;
			mResultFolder = folder;
			mSelectedIndex = static_cast<QtFileTypeItem*>(selected[0])->mIndex;
			QDialog::accept();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void QtNewFileDialog::onBrowse()
	{
		QFileDialog dialog;
		dialog.setOption(QFileDialog::DontUseNativeDialog, true);
		dialog.setAcceptMode(QFileDialog::AcceptOpen);
		dialog.setFileMode(QFileDialog::DirectoryOnly);
		dialog.setDirectory(mFolder.text());
		if( dialog.exec() )
			mFolder.setText( dialog.selectedFiles()[0] );
	}
	
}//namespace Blade