/********************************************************************
	created:	2011/05/03
	filename: 	GameWorldData.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "GameWorldData.h"
#include <interface/public/graphics/IGraphicsScene.h>
#include <interface/public/graphics/IGraphicsSpaceCoordinator.h>
#include "GameWorld.h"

namespace Blade
{
	const TString GameWorldData::GAME_WORLD_DATA_TYPE = BTString("GameWorldData");
	const TString GameWorldData::WORLD_NAME = BTString("Name");
	const TString GameWorldData::WORLD_SART_POS = BTString("StartPosition");

	//////////////////////////////////////////////////////////////////////////
	GameWorldData::GameWorldData()
		 :mScene(NULL)
		 ,mCamera(NULL)
	{
		mStartPostition = Vector3::ZERO;
	}

	//////////////////////////////////////////////////////////////////////////
	GameWorldData::~GameWorldData()
	{
		if(mScene != NULL && mScene->getSpaceCoordinator() != NULL)
			mScene->getSpaceCoordinator()->setPositionReference(NULL);
	}

	/************************************************************************/
	/* ISerializable interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void	GameWorldData::prepareSave()
	{
		mStartPostition = this->getPosition();
	}

	//////////////////////////////////////////////////////////////////////////
	void	GameWorldData::postProcess(const ProgressNotifier& /*callback*/)
	{
		if(mScene != NULL && mScene->getSpaceCoordinator() != NULL)
			mScene->getSpaceCoordinator()->setPositionReference(this);
	}

	/************************************************************************/
	/* custom methods                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void		GameWorldData::setGraphicsScene(IGraphicsScene* scene)
	{
		mScene = scene;
		if(mScene != NULL && mScene->getSpaceCoordinator() != NULL)
			mScene->getSpaceCoordinator()->setPositionReference(this);
	}

	//////////////////////////////////////////////////////////////////////////
	void		GameWorldData::onConfigChange(const void* data)
	{
		if( data == &mStartPostition )
		{
		}
	}
}//namespace Blade
