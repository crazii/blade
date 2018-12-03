/********************************************************************
	created:	2016/09/24
	filename: 	QtProgressWindow.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <QtPluginPCH.h>
#include <QtProgressWindow.h>

namespace Blade
{
	
	//////////////////////////////////////////////////////////////////////////
	QtProgressWindow::QtProgressWindow(QWidget* parent, const TString& title)
		:QWidget(parent)
	{
		this->setWindowFlags(this->windowFlags() | Qt::SplashScreen);
		QHBoxLayout* layout = new QHBoxLayout(this);
		layout->addWidget(&mText);
		layout->addWidget(&mProgress);

		mProgress.setOrientation(Qt::Horizontal);
		mProgress.setRange(0, AMPLIFIER);
		mProgress.reset();

		parent->setEnabled(false);

		QRect rect = parent->geometry();

		int width = rect.width() / 3;
		int height = 40;
		QSize sz(width, height);
		QPoint lefttop = rect.center();
		lefttop.rx() -= width / 2;
		lefttop.ry() -= height / 2;
		rect = QRect(lefttop, sz);

		this->setGeometry(rect);

		mText.setText(TString2QString(title));
		this->show();
	}
	
	//////////////////////////////////////////////////////////////////////////
	QtProgressWindow::~QtProgressWindow()
	{

	}

}//namespace Blade