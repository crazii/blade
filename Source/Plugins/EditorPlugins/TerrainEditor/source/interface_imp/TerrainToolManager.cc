/********************************************************************
	created:	2011/05/26
	filename: 	TerrainToolManager.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <interface/IEditorToolManager.h>
#include <interface/IEditorFramework.h>
#include <interface/public/ui/IIconManager.h>

#include "TerrainToolManager.h"
//terrain tools
#include "TerrainDeformTool.h"
#include "TerrainSplatTool.h"
#include "TerrainRelightTool.h"

namespace Blade
{

	//////////////////////////////////////////////////////////////////////////
	TerrainToolManager::TerrainToolManager()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	TerrainToolManager::~TerrainToolManager()
	{
		//shutdown properly
		assert(mBrushTools.empty());
		assert(mCmdTools.empty());
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void	TerrainToolManager::initalize()
	{
		IIconManager& iconMan = IIconManager::getSingleton();
		IEditorToolManager& toolMan = IEditorToolManager::getSingleton();
		//add brush tools
		if(mBrushTools.size() == 0 )
		{
			IconIndex icon = iconMan.loadSingleIconImage(IEditorFramework::EDITOR_ICON_PATH + BTString("terrain_deform.png"));
			TerrainBrushBaseTool* brushTool = BLADE_NEW TerrainDeformTool(icon,KC_UNDEFINED);
			toolMan.addTool(brushTool);
			mBrushTools.push_back( brushTool );

			icon = iconMan.loadSingleIconImage(IEditorFramework::EDITOR_ICON_PATH + BTString("terrain_splat.png"));
			brushTool= BLADE_NEW TerrainSplatTool(icon,KC_UNDEFINED);
			toolMan.addTool(brushTool);
			mBrushTools.push_back(brushTool);

			for(TerrainBrushList::iterator i = mBrushTools.begin(); i != mBrushTools.end(); ++i)
				IEditorFramework::getSingleton().getFrameworkNotification()->addFileTypeTool( WORLD_FILE_TYPE, *i );
		}

		//add command tools
		if( mCmdTools.size() == 0)
		{
			IconIndex icon = iconMan.loadSingleIconImage(IEditorFramework::EDITOR_ICON_PATH + BTString("terrain_relight.png"));
			TerrainCommandBaseTool* cmdTool = BLADE_NEW TerrainRelightTool(icon,KC_UNDEFINED);
			toolMan.addTool(cmdTool);
			mCmdTools.push_back(cmdTool);

			for(TerrainCommandList::iterator i = mCmdTools.begin(); i != mCmdTools.end(); ++i)
				IEditorFramework::getSingleton().getFrameworkNotification()->addFileTypeTool( WORLD_FILE_TYPE, *i );
		}
	}
	
	//////////////////////////////////////////////////////////////////////////
	void	TerrainToolManager::shutdown()
	{
		for(size_t i = 0; i < mBrushTools.size(); ++i )
			BLADE_DELETE mBrushTools[i];

		for(size_t i = 0; i < mCmdTools.size(); ++i )
			BLADE_DELETE mCmdTools[i];

		mBrushTools.clear();
		mCmdTools.clear();
	}

}//namespace Blade
