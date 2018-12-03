/********************************************************************
	created:	2011/05/26
	filename: 	LogicWorldManager.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <interface/public/graphics/IGraphicsType.h>
#include "LogicWorldManager.h"

namespace Blade
{
	/////////////////////////////////////////////////////////////////////////
	LogicWorldManager::LogicWorldManager()
	{

	}

	/////////////////////////////////////////////////////////////////////////
	LogicWorldManager::~LogicWorldManager()
	{

	}

	/************************************************************************/
	/* ILogicWorldManager interface                                                                     */
	/************************************************************************/

	/////////////////////////////////////////////////////////////////////////
	ILogicWorld*		LogicWorldManager::createWorld(const TString& worldName, const TString& worldType/* = BTString("Default") */)
	{
		ILogicWorld*& world = mGameWorlds[worldName];
		if( world != NULL )
			BLADE_EXCEPT(EXC_REDUPLICATE,BTString("a world with the same name \"") + worldName + BTString("\" already exists."));
		world = BLADE_FACTORY_CREATE(ILogicWorld, worldType);
		world->setupWorldName(worldName);
		return world;
	}

	/////////////////////////////////////////////////////////////////////////
	bool			LogicWorldManager::destroyWorld(const TString& worldName)
	{
		WorldList::iterator i = mGameWorlds.find(worldName);
		if( i == mGameWorlds.end() )
			return false;
		else
		{
			BLADE_DELETE i->second;
			mGameWorlds.erase(i);
			return true;
		}
	}

	/////////////////////////////////////////////////////////////////////////
	ILogicWorld*		LogicWorldManager::getWorld(const TString& worldName) const
	{
		WorldList::const_iterator i = mGameWorlds.find(worldName);
		if( i == mGameWorlds.end() )
			return NULL;
		else
			return i->second;
	}

}//namespace Blade
