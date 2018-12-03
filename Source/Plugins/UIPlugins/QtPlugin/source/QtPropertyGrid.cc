/********************************************************************
	created:	2016/7/25
	filename: 	QtPropertyGrid.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <QtPluginPCH.h>
#include <QtPropertyGrid.h>
#include <QtPropertyHelper.h>
#include "propertybrowser/qttreepropertybrowser.h"

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	QtPropertyGrid::QtPropertyGrid(QWidget* parent)
		:mHelper(parent)
	{
		assert(parent != NULL);
		QtTreePropertyBrowser* browser = QT_NEW QtTreePropertyBrowser(parent);

		//browser->setHeaderVisible(false);
		browser->setResizeMode(QtTreePropertyBrowser::Interactive);

		mImp = browser;
		mHelper.initialize(mImp);
	}

	//////////////////////////////////////////////////////////////////////////
	QtPropertyGrid::~QtPropertyGrid()
	{
		//if mImp has parent, delete it will detach its parent
		//otherwise, just delete it.
		BLADE_DELETE mImp;
	}

	//////////////////////////////////////////////////////////////////////////
	QWidget*		QtPropertyGrid::getWidget()
	{
		return mImp;
	}

	//////////////////////////////////////////////////////////////////////////
	void			QtPropertyGrid::bindConfigs(IConfig* root)
	{
		mImp->clear();
		mHelper.clear();

		if(root == NULL)
			return;
		IConfig::IList* list = root->getSubConfigs();
		if(list == NULL)
			return;

		for(size_t i = 0; i < list->getCount(); ++i)
		{
			IConfig* config = list->getConfig(i);
			this->addConfig(config, NULL, NULL);
		}

		static_cast<QtTreePropertyBrowser*>(mImp)->adjustColumns(false);
	}

	//////////////////////////////////////////////////////////////////////////
	size_t			QtPropertyGrid::getHeight()
	{
		QtTreePropertyBrowser* treeBrowser = static_cast<QtTreePropertyBrowser*>(mImp);
		return treeBrowser->totalHeight();
	}
	
	//////////////////////////////////////////////////////////////////////////
	void			QtPropertyGrid::addConfig(IConfig* config, IConfig* parentConfig, QtProperty* parentProp)
	{
		QtProperty* prop = mHelper.createProperty(config, parentConfig);
		if(prop == NULL)
			return;

		if(parentProp != NULL)
			parentProp->addSubProperty(prop);
		else
			mImp->addProperty(prop);

		IConfig::IList* list = config->getSubConfigs();
		if(list == NULL)
			return;

		for(size_t i = 0; i < list->getCount(); ++i)
		{
			IConfig* subConfig = list->getConfig(i);
			this->addConfig(subConfig, config, prop);
		}
	}
	
}//namespace Blade