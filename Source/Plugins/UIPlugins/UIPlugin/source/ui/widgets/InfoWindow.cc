/********************************************************************
	created:	2013/01/13
	filename: 	InfoWindow.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include "stdafx.h"
#include "InfoWindow.h"

namespace Blade
{
	const TString InfoWindow::WIDGET_TYPE = BTString("InfoWidget");

	//////////////////////////////////////////////////////////////////////////
	InfoWindow::InfoWindow()
		:DockWindowBase(WIDGET_TYPE)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	InfoWindow::~InfoWindow()
	{

	}
}