/********************************************************************
	created:	2016/07/24
	filename: 	QtSplash.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_QtSplash_h__
#define __Blade_QtSplash_h__
#include <interface/public/IStartupOutput.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	class QtSplashWindow : public QDialog
	{
	public:
		QtSplashWindow() :QDialog(NULL, Qt::FramelessWindowHint),mImage(this),mInfo(this),mDetail(this)
		{
			QPalette palette = mInfo.palette();
			palette.setColor(mInfo.foregroundRole(), Qt::black);
			mInfo.setPalette(palette);
			mDetail.setPalette(palette);
		}
		~QtSplashWindow()	{}

		/** @brief  */
		virtual void keyPressEvent(QKeyEvent* evt)
		{
			if (evt->key() == Qt::Key_Escape)
				evt->ignore();
			else
				return QDialog::keyPressEvent(evt);
		}

		/** @brief  */
		inline void show(const TString& softInfo, const TString& image)
		{
			mInfo.setText(TString2QString(softInfo));
			HSTREAM stream = IResourceManager::getSingleton().loadStream(image);
			if(stream == NULL)
				return;

			HIMAGE img = IImageManager::getSingleton().loadImage(stream, IP_TEMPORARY, PO_NONE, IMGO_TOP_DOWN);
			if(img == NULL)
				return;

			Handle<QImage> qimg = toQImage(img);
			QPixmap pm;
			pm.convertFromImage(*qimg);
			mImage.setPixmap(pm);

			this->setFixedSize(pm.width(), pm.height());
			mImage.resize(pm.width(), pm.height());

			const int PADDING = 5;

			mDetail.setText(QObject::tr(" "));
			mDetail.move(PADDING, pm.height() - mDetail.sizeHint().height() - PADDING);
			mDetail.resize(pm.width() - PADDING*2, mDetail.sizeHint().height());
			mInfo.setAlignment(Qt::AlignRight);
			mInfo.move(pm.width()-mInfo.sizeHint().width()-PADDING,pm.height()-mInfo.sizeHint().height()-mDetail.sizeHint().height() - PADDING*2);		
			QDialog::show();
		}
		/** @brief  */
		inline void setDetailText(const TString& text)
		{
			mDetail.setText(TString2QString(text));
		}
		QLabel	mImage;
		QLabel	mInfo;
		QLabel	mDetail;
	};//class QtSplashWindow

	//////////////////////////////////////////////////////////////////////////
	class QtSplash : public IStartupOutput, public Singleton<QtSplash>
	{
	public:
		QtSplash();
		~QtSplash();
		/************************************************************************/
		/* IStartupOutput interface                                                                     */
		/************************************************************************/
		/**
		@describe 
		@param 
		@return 
		*/
		virtual void initialize(const TString& softInfo);

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void show(bool bShow);

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void outputProgress(const TString& detail);

	protected:
		QtSplashWindow*	mSplash;
		TString	mInfoString;
		TString	mImagePath;
	};

	
}//namespace Blade


#endif // __Blade_QtSplash_h__