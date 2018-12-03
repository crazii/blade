/********************************************************************
	created:	2016/07/23
	filename: 	QtPluginPCH.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_QtPluginPCH_h__
#define __Blade_QtPluginPCH_h__


/*IMPORANT NOTE:
because Qt delete children on destruction
if you don't want new(), you should declare children after parent, when has window casacades,
so that children will destructed before parent and detach from parent.
C++'s principle (construct object before use) guarantees this, but with array objects, you should be careful.
i.e.
class Demo
{
public:
	Demo() :mSplitter(this), mList(&mSplitter) 
	{
		for(int i = 0; i < 4; ++i)
			mLables[i].setParent(&mSplitter);
	}

protected:
	//QLable	mLabels[4];	//not easily working
	QSplitter	mSplitter;	//parent 
	QListView	mList;		//child
	QLable		mLabels[4];	//children
};
*/

#include <QtWidgets>

#ifdef INFINITE	//MS Windows macro imported by Qt
#undef INFINITE
#endif

#include <BladePCH.h>
#include <ExceptionEx.h>
#include <interface/IResourceManager.h>
#include <interface/public/window/IWindowEventHandler.h>
#include <interface/public/window/IWindowService.h>
#include <interface/public/ui/IConfigDialog.h>
#include <interface/public/ui/IUIService.h>
#include <interface/public/ui/UITypes.h>
#include <BladeBase_blang.h>
#include <BladeFramework_blang.h>

#include <QtHelper.h>
#include <UIPlugin_blang.h>

#endif // __Blade_QtPluginPCH_h__
