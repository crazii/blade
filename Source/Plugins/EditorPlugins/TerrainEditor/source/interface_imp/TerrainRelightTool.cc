/********************************************************************
	created:	2011/05/25
	filename: 	TerrainRelightTool.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "TerrainRelightTool.h"
#include "TerrainToolManager.h"
#include <interface/ITaskManager.h>
#include <interface/ITimeService.h>
#include <interface/ITerrainConfigManager.h>

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(disable:4355)
#endif

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	TerrainRelightTool::TerrainRelightTool(IconIndex icon,HOTKEY hotkey)
		:TerrainCommandBaseTool(BXLang(BLANG_REBUILD_NORMAL),icon,hotkey)
		,mBackgroundTask(this)
		,mUpdateCount(0)
		,mUpdateID(0)
		,mTaskRunning(false)
	{
		IFramework::getSingleton().addUpdater(this);
	}

	//////////////////////////////////////////////////////////////////////////
	TerrainRelightTool::~TerrainRelightTool()
	{
		IFramework::getSingleton().removeUpdater(this);
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	void				TerrainRelightTool::update()
	{
		if( mUpdateCount.count() > 0 )
		{
			size_t n = mUpdateCount.safeDecrement();

			const TerrainToolManager::TerrainEditableList&	List = TerrainToolManager::getSingleton().getEditableList();
			TerrainEditable* te = List[n];
			te->updateNormalBuffer();

			mUpdateID = ITimeService::getSingleton().getTimeSource().getLoopID();
		}
		else if( mUpdateID != 0 )
		{
			//because the normal update mode is synchronized, so it will not update immediately
			//wait a 2-3 frame to wait the normal is really updated (actually 1 frame is  enough)
			if( ITimeService::getSingleton().getTimeSource().getLoopID() - mUpdateID > 3 )
			{
				ITerrainConfigManager::getInterface().enableMultiLoading(true);
				mUpdateID = 0;
			}
		}
	}

	/************************************************************************/
	/* EditorCommandTool interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool				TerrainRelightTool::doCommand()
	{
		if( mTaskRunning )
			return false;
		//get the cached page editable
		//disable terrain dynamic loading while update normal
		ITerrainConfigManager::getInterface().enableMultiLoading(false);

		const TerrainToolManager::TerrainEditableList&	List = TerrainToolManager::getSingleton().getEditableList();
		for(size_t n = 0; n < List.size(); ++n)
		{
			TerrainEditable* te = List[n];
			mBackgroundTask.addTerrainForUpdate(te, te->getHeightBuffer(), te->getNormalBuffer());
		}

		//this should be called at the main thread (and now it is )
		ITaskManager::getSingleton().addTask(&mBackgroundTask);
		mTaskRunning = true;
		return true;
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void				TerrainRelightTool::onTaskDone()
	{
		const TerrainToolManager::TerrainEditableList&	List = TerrainToolManager::getSingleton().getEditableList();
		mUpdateCount.safeSet( List.size() );
		mTaskRunning = false;
	}


}//namespace Blade
