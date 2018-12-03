/********************************************************************
	created:	2016/09/24
	filename: 	QtProgressWindow.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_QtProgressWindow_h__
#define __Blade_QtProgressWindow_h__
#include <interface/public/ISerializable.h>

namespace Blade
{
	class QtProgressWindow : public QWidget, public TempAllocatable
	{
	public:
		static const int AMPLIFIER = 100*1000;
	public:
		QtProgressWindow(QWidget* parent, const TString& title);
		~QtProgressWindow();

		/** @brief  */
		void setProgress(Blade::scalar percent)
		{
			mProgress.setValue((int)(percent*AMPLIFIER));
			mProgress.update();
		}

	protected:
		QLabel mText;
		QProgressBar mProgress;
	};//QtProgressWindow

	class QtProgress : public IProgressCallback, public Allocatable
	{
	public:
		QtProgress() { mProgressWindow = NULL; }

		/** @brief  */
		virtual void onNotify(Blade::scalar percent)
		{
			if (mProgressWindow == NULL)
				return;

			mProgressWindow->setProgress(percent);

			if (percent >= 1.0f)
			{
				((QWidget*)mProgressWindow->parent())->setEnabled(true);
				BLADE_DELETE mProgressWindow;
				mProgressWindow = NULL;
			}

			//process message to prevent no response
			IWindowMessagePump::getSingleton().processMessage();
		}

		/** @brief  */
		void reset(QWidget* parent, const TString& title)
		{
			if (mProgressWindow != NULL)
				BLADE_DELETE mProgressWindow;
			mProgressWindow = BLADE_NEW QtProgressWindow(parent, title);
			mProgressWindow->update();
		}

	protected:
		QtProgressWindow* mProgressWindow;
	};//class QtProgress
	
}//namespace Blade


#endif // __Blade_QtProgressWindow_h__