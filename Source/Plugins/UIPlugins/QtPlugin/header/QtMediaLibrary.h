/********************************************************************
	created:	2016/07/26
	filename: 	QtMediaLibrary.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_QtMediaLibrary_h__
#define __Blade_QtMediaLibrary_h__
#include <interface/public/ui/IUIMediaLibrary.h>

namespace Blade
{
	class QtIcon;

	class QtMediaLibrary : public IUIMediaLibrary, public QDialog, public Singleton<QtMediaLibrary>
	{
	public:
		QtMediaLibrary();
		~QtMediaLibrary();

		/************************************************************************/
		/* IUIMediaLibrary interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual bool			initialize();

		/** @brief  */
		virtual bool			showMediaLibrary(const TString& type, const Blade::TString& curFile,
			IMediaLibrary::ISelectionReceiver& receiver,
			bool bShowDefaultOnly = false);

		/** @brief QWidgets cannot exist after QApplication destructed, this happens in debug mode, when all DLLs are not unloaded for memory debug */
		virtual bool			shutdown()
		{
			//QSplitter will delete children, but we don't need that
			mMediaType.setParent(NULL);
			mMediaFile.setParent(NULL);
			mPreview.setParent(NULL);
			this->setParent(NULL);
			return true;
		}

		/** @brief  */
		void clear();

	protected:
		/** @brief  */
		void onTypeChange(QListWidgetItem* qitem);
		/** @brief  */
		void onFileSelect(QListWidgetItem* qitem);
		/** @brief  */
		virtual void	accept();
		virtual void	reject();
		virtual bool	event(QEvent* evt)
		{
			if(evt->type() == QEvent::WindowDeactivate || evt->type() == QEvent::ApplicationDeactivate)
			{
				this->reject();
				return true;
			}
			return QDialog::event(evt);
		}

		typedef List<QtIcon*>			IconList;
		typedef TStringMap<IconList>	TypedIconList;

		QSplitter	mSplitter;
		QListWidget	mMediaType;
		QListWidget	mMediaFile;
		QPushButton mOK;
		QPushButton	mCancel;
		QLabel		mPreview;
		QPoint		mLastPos;	//accept/reject won't record last pos

		TypedIconList mIcons;

		//temporary
		TString		mTypeFilter;
		TString		mFile;
		IMediaLibrary::ISelectionReceiver* mReceiver;
	};
	
}//namespace Blade


#endif // __Blade_QtMediaLibrary_h__