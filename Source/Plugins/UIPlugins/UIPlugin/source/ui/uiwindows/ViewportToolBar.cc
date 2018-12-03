/********************************************************************
	created:	2013/04/25
	filename: 	ViewportToolbar.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include "stdafx.h"
#include "ViewportToolBar.h"
#include <ui/UIToolBox.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	ViewportToolBar::ViewportToolBar(const TString& name, size_t viewportCount, size_t existingWidth,  ::CFrameWnd* viewPort[])
		:mViewportCount(viewportCount)
	{
		for(size_t i = 0; i < mViewportCount; ++i )
		{
			mViewportToolBox[i] = BLADE_NEW UIToolBox(name, IS_16, AFX_IDW_TOOLBAR, viewPort[i], existingWidth, true, false);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	ViewportToolBar::~ViewportToolBar()
	{
		for(size_t i = 0; i < mViewportCount; ++i )
		{
			BLADE_DELETE mViewportToolBox[i];
		}
	}

	/************************************************************************/
	/* IUIToolBox interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool			ViewportToolBar::addButton(IUICommand* cmd, const TString& name, IconIndex icon, int style/* = TBS_NONE*/)
	{
		bool ret = true;
		for (size_t i = 0; i < mViewportCount; ++i)
		{
			if (!mViewportToolBox[i]->addButton(cmd, name, icon, style))
				ret = false;
		}
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			ViewportToolBar::addButton(IMenu* menu, int style/* = TBS_CHECKGROUP*/)
	{
		bool ret = true;
		for(size_t i = 0; i < mViewportCount; ++i )
		{
			if( !mViewportToolBox[i]->addButton(menu, style) )
				ret = false;
		}
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			ViewportToolBar::addTool(IConfig* config, int width/* = -1*/)
	{
		bool ret = true;
		for(size_t i = 0; i < mViewportCount; ++i )
		{
			if( !mViewportToolBox[i]->addTool(config, width) )
				ret = false;
		}
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			ViewportToolBar::addToolSet(IConfig* config, IconIndex icon)
	{
		bool ret = true;
		for (size_t i = 0; i < mViewportCount; ++i)
		{
			if (!mViewportToolBox[i]->addToolSet(config, icon))
				ret = false;
		}
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	void			ViewportToolBar::updateTools()
	{
		for(size_t i = 0; i < mViewportCount; ++i )
			mViewportToolBox[i]->updateTools();
	}

	//////////////////////////////////////////////////////////////////////////
	size_t			ViewportToolBar::getButtonCount() const
	{
		if( mViewportCount == 0 )
			return 0;
		else
			return mViewportToolBox[0]->getButtonCount();
	}

	//////////////////////////////////////////////////////////////////////////
	ICONSIZE		ViewportToolBar::getIconSize()	const
	{
		if( mViewportCount == 0 )
			return IS_16;
		else
			return mViewportToolBox[0]->getIconSize();
	}

	//////////////////////////////////////////////////////////////////////////
	bool			ViewportToolBar::setIconSize(ICONSIZE is)
	{
		bool ret = true;
		for(size_t i = 0; i < mViewportCount; ++i )
		{
			if( !mViewportToolBox[i]->setIconSize(is) )
				ret = false;
		}
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	IUIToolBox*	ViewportToolBar::getViewportToolbox(index_t index) const
	{
		if( index < mViewportCount )
			return mViewportToolBox[index];
		else
			return NULL;
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	size_t			ViewportToolBar::getWidth() const
	{
		return mViewportToolBox[0]->getWidth();
	}
	//////////////////////////////////////////////////////////////////////////
	const TString&	ViewportToolBar::getName() const
	{
		return mViewportToolBox[0]->getName();
	}

}//namespace Blade