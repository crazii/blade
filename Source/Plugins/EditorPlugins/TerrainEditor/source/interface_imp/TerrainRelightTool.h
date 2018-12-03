/********************************************************************
	created:	2011/05/25
	filename: 	TerrainRelightTool.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_TerrainRelightTool_h__
#define __Blade_TerrainRelightTool_h__
#include "TerrainToolBase.h"
#include <memory/BladeMemory.h>
#include <interface/IFramework.h>
#include "TerrainRelightTask.h"

namespace Blade
{

	class TerrainRelightTool : public TerrainCommandBaseTool , public IFramework::IMainLoopUpdater, public StaticAllocatable
	{
	public:
		TerrainRelightTool(IconIndex icon,HOTKEY hotkey);
		~TerrainRelightTool();

		/************************************************************************/
		/*                                                                      */
		/************************************************************************/
		/** @brief  */
		virtual void	update();

		/************************************************************************/
		/* EditorCommandTool interface                                                                     */
		/************************************************************************/
		/*
		@describe this method will be called if tool type is immediate type
		@param 
		@return 
		*/
		virtual bool	doCommand();


		/************************************************************************/
		/*                                                                      */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		void						onTaskDone();

	protected:
		TerrainRelightTask	mBackgroundTask;
		RefCount			mUpdateCount;
		uint64				mUpdateID;
		bool				mTaskRunning;
	};
	

}//namespace Blade




#endif // __Blade_TerrainRelightTool_h__