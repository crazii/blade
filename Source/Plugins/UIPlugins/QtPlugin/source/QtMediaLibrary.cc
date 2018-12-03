/********************************************************************
	created:	2016/07/26
	filename: 	QtMediaLibrary.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <QtPluginPCH.h>
#include <QtMediaLibrary.h>
#include <QtIconManager.h>

namespace Blade
{
	static const int INIT_HEIGHT = 320;
	static const int TYPES_WIDTH = 120;
	static const int FILES_WIDTH = 480;
	static const int PREVIEW_SIZE = 240;

	class QtMediaTypeItem : public QListWidgetItem, public TempAllocatable
	{
	public:
		QtMediaTypeItem(const TString& type, index_t index) :QListWidgetItem(TString2QString(BTString2Lang(type))),mType(type),mIndex(index) {}

		TString mType;
		index_t mIndex;
	};

	class QtMediaFileItem : public QListWidgetItem, public TempAllocatable
	{
	public:
		QtMediaFileItem(const QIcon& icon, const IMediaLibrary::MediaFile* file) :QListWidgetItem(icon, TString2QString(file->mName)), mFile(file){}
		const IMediaLibrary::MediaFile* mFile;
	};

	//////////////////////////////////////////////////////////////////////////
	QtMediaLibrary::QtMediaLibrary()
		:mSplitter(this)
		,mReceiver(NULL)
		,mLastPos(std::numeric_limits<int>::min(), std::numeric_limits<int>::min())
	{
		this->setWindowTitle(TString2QString(BTLang(BLANG_MEDIALIBRARY)));
		this->setWindowFlags(Qt::Tool|Qt::WindowStaysOnTopHint);
		this->resize(TYPES_WIDTH+FILES_WIDTH+PREVIEW_SIZE, INIT_HEIGHT);

		mSplitter.addWidget(&mMediaType);
		mSplitter.addWidget(&mMediaFile);
		mSplitter.addWidget(&mPreview);
		mSplitter.setCollapsible(0, false);
		mSplitter.setCollapsible(1, false);
		mSplitter.setCollapsible(2, true);
		QList<int> sz;
		sz.append(TYPES_WIDTH);
		sz.append(FILES_WIDTH);
		sz.append(PREVIEW_SIZE);
		mSplitter.setSizes(sz);

		QVBoxLayout* layout = QT_NEW QVBoxLayout();
		this->setLayout(layout);

		layout->addWidget(&mSplitter, Qt::AlignTop);

		QHBoxLayout* buttonLayout = QT_NEW QHBoxLayout();
		layout->addLayout(buttonLayout);

		buttonLayout->addStretch(1080);
		buttonLayout->addWidget(&mCancel);
		buttonLayout->addWidget(&mOK);
		mCancel.setText(TString2QString(BTLang(BLANG_CANCEL)));
		mOK.setText(TString2QString(BTLang(BLANG_OK)));

		QObject::connect(&mCancel, &QPushButton::clicked, this, &QtMediaLibrary::reject);
		QObject::connect(&mOK, &QPushButton::clicked, this, &QtMediaLibrary::accept);

		QObject::connect(&mMediaType, &QListWidget::itemClicked, this, &QtMediaLibrary::onTypeChange);
		QObject::connect(&mMediaFile, &QListWidget::itemClicked, this, &QtMediaLibrary::onFileSelect);

		mMediaType.setViewMode(QListView::ListMode);
		mMediaType.setResizeMode(QListView::Adjust);
		mMediaType.setSelectionMode(QAbstractItemView::SingleSelection);
		mMediaType.setSelectionBehavior(QAbstractItemView::SelectRows);

		mMediaFile.setViewMode(QListView::IconMode);
		mMediaFile.setResizeMode(QListView::Adjust);
		mMediaFile.setSelectionMode(QAbstractItemView::SingleSelection);
		mMediaFile.setSelectionBehavior(QAbstractItemView::SelectItems);
		mMediaFile.setWordWrap(true);
		mMediaFile.setUniformItemSizes(true);
	}

	//////////////////////////////////////////////////////////////////////////
	QtMediaLibrary::~QtMediaLibrary()
	{
		for (TypedIconList::iterator i = mIcons.begin(); i != mIcons.end(); ++i)
		{
			IconList& iconList = i->second;
			for (IconList::iterator j = iconList.begin(); j != iconList.end(); ++j)
				BLADE_DELETE *j;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool			QtMediaLibrary::initialize()
	{
		size_t types_count = IMediaLibrary::getSingleton().getMediaTypesCount();

		for (size_t i = 0; i < types_count; ++i)
		{
			const IMediaLibrary::MEDIA_TYPE& type = IMediaLibrary::getSingleton().getMediaTypeByIndex(i);

			IconList& iconList = mIcons[type.mType];

			assert(type.mPreviewer != NULL);

			if (type.mSharedIcon)
			{
				//generate only one icon, that's good enough
				const IMediaLibrary::MediaFile* file = IMediaLibrary::getSingleton().getMediaFile(i, 0);
				assert(file != NULL);

				QtIcon* icon = BLADE_NEW QtIcon();
				QtIconManager::getSingleton().fillIcon(icon, file->mPreview);
				iconList.push_back(icon);
			}
			else
			{
				//generate icon for all files
				size_t fileCount = IMediaLibrary::getSingleton().getMediaFileCount(i);
				for (size_t n = 0; n < fileCount; ++n)
				{
					const IMediaLibrary::MediaFile* file = IMediaLibrary::getSingleton().getMediaFile(i, n);
					assert(file != NULL);

					QtIcon* icon = BLADE_NEW QtIcon();
					QtIconManager::getSingleton().fillIcon(icon, file->mPreview);
					iconList.push_back(icon);
				}
			}
		}

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			QtMediaLibrary::showMediaLibrary(const TString& type, const Blade::TString& curFile,
		IMediaLibrary::ISelectionReceiver& receiver, bool bShowDefaultOnly/* = false*/)
	{
		size_t types_count = IMediaLibrary::getSingleton().getMediaTypesCount();
		mTypeFilter = type;
		mOK.setEnabled(false);
		mReceiver = &receiver;
		mFile = curFile;

		if (bShowDefaultOnly)
		{
			QtMediaTypeItem* typeItem = BLADE_NEW QtMediaTypeItem(type, 0);
			typeItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			mMediaType.addItem(typeItem);
		}
		else
		{
			const TString& defaultTypeName = BTString2Lang(type);
			for (size_t i = 0; i < types_count; ++i)
			{
				const IMediaLibrary::MEDIA_TYPE& type = IMediaLibrary::getSingleton().getMediaTypeByIndex(i);
				//add the type string to type list
				QtMediaTypeItem* typeItem = BLADE_NEW QtMediaTypeItem(type.mType, i);

				bool enable = type.mType == mTypeFilter;
				typeItem->setFlags(enable ? Qt::ItemIsEnabled | Qt::ItemIsSelectable : Qt::ItemIsSelectable);
				mMediaType.addItem(typeItem);

				//show the initial selected one
				if(enable)
				{
					mMediaType.setItemSelected(typeItem, true);
					this->onTypeChange(typeItem);
				}
			}
		}

		if(mLastPos.x() != std::numeric_limits<int>::min() && mLastPos.y() != std::numeric_limits<int>::min())
			this->move(mLastPos);

		if(QApplication::activeModalWidget() != NULL)
		{
			this->setModal(true);
			this->exec();
		}
		else
		{
			this->setModal(false);
			this->show();
			this->activateWindow();
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void QtMediaLibrary::clear()
	{
		mTypeFilter = TString::EMPTY;
		mFile = TString::EMPTY;
		mMediaType.clear();
		mMediaFile.clear();
		mReceiver = NULL;
		mLastPos = this->pos();
	}

	//////////////////////////////////////////////////////////////////////////
	void QtMediaLibrary::onTypeChange(QListWidgetItem* qitem)
	{
		mMediaFile.clear();

		QtMediaTypeItem* _item = static_cast<QtMediaTypeItem*>(qitem);
		const TString& type = _item->mType;
		bool enable = type == mTypeFilter;
		bool hasSel = false;

		//add all files of the first type into the file list
		const IMediaLibrary::MEDIA_TYPE& mediaType = IMediaLibrary::getSingleton().getMediaType(type);
		size_t fileCount = IMediaLibrary::getSingleton().getMediaFileCount(mediaType.mIndex);

		const IconList& iconList = mIcons[type];
		assert(iconList.size() >= 1);
		assert(mediaType.mSharedIcon || iconList.size() == fileCount);

		for (size_t i = 0; i < fileCount; ++i)
		{
			const IMediaLibrary::MediaFile* file = IMediaLibrary::getSingleton().getMediaFile(type, i);
			assert(file != NULL);
			IconList::const_iterator iconIter = iconList.begin();
			if (!mediaType.mSharedIcon)
				std::advance(iconIter, i);

			QIcon* icon = *iconIter;
			QtMediaFileItem* fileItem = BLADE_NEW QtMediaFileItem(*icon, file);

			fileItem->setFlags(enable ? Qt::ItemIsEnabled | Qt::ItemIsSelectable : Qt::ItemIsSelectable);
			mMediaFile.addItem(fileItem);
			if(file->mSchemePath == mFile)
			{
				mMediaFile.setItemSelected(fileItem, true);
				this->onFileSelect(fileItem);
				hasSel = true;
			}
		}

		if(!hasSel)
		{
			QPixmap pixelMap;
			mPreview.setPixmap(pixelMap);
			mOK.setEnabled(false);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void QtMediaLibrary::onFileSelect(QListWidgetItem* qitem)
	{
		QtMediaFileItem* _item = static_cast<QtMediaFileItem*>(qitem);
		const IMediaLibrary::MediaFile* file = _item->mFile;
		bool enable = (_item->flags()&Qt::ItemIsEnabled) != 0;

		QList<QListWidgetItem*> selType = mMediaType.selectedItems();
		if (selType.count() != 1)
			return;
		QtMediaTypeItem* typeItem = static_cast<QtMediaTypeItem*>(selType[0]);

		//preview
		//const IMediaLibrary::MEDIA_TYPE& mediaType = IMediaLibrary::getSingleton().getMediaType(typeItem->mType);
		//HIMAGE hPreview = mediaType.mPreviewer->generatePreview(file->mSchemePath, -1, IP_TEMPORARY);
		HIMAGE hPreview = file->mPreview;

		Handle<QImage> qimg = toQImage(hPreview);
		QPixmap pixelMap;
		pixelMap.convertFromImage(*qimg);
		mPreview.setPixmap(pixelMap.scaled(PREVIEW_SIZE, PREVIEW_SIZE));

		mOK.setEnabled(enable);
	}

	//////////////////////////////////////////////////////////////////////////
	void	QtMediaLibrary::accept()
	{
		QList<QListWidgetItem*> selType = mMediaType.selectedItems();
		if (selType.count() != 1)
			return;
		QtMediaTypeItem* typeItem = static_cast<QtMediaTypeItem*>(selType[0]);
		index_t mediaTypeIndex = IMediaLibrary::getSingleton().getMediaType(typeItem->mType).mIndex;

		QList<QListWidgetItem*> selFile = mMediaFile.selectedItems();
		if (selFile.count() != 1)
			return;
		QtMediaFileItem* fileItem = static_cast<QtMediaFileItem*>(selFile[0]);
		const IMediaLibrary::MediaFile* file = fileItem->mFile;

		//call back
		mReceiver->setSelectedTarget(mediaTypeIndex, *file);

		//clean up
		this->clear();
		this->setParent(NULL);
		QDialog::accept();
	}

	//////////////////////////////////////////////////////////////////////////
	void	QtMediaLibrary::reject()
	{
		//QDialog::accept() will deactivate and call reject
		if(mReceiver != NULL)
			mReceiver->canceled();
		this->clear();
		this->setParent(NULL);
		QDialog::reject();
	}
	
}//namespace Blade