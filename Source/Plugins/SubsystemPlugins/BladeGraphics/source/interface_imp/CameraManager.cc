/********************************************************************
	created:	2012/04/01
	filename: 	CameraManager.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "CameraManager.h"
#include <interface/ISpace.h>
#include "../Camera.h"

namespace Blade
{

	//////////////////////////////////////////////////////////////////////////
	CameraManager::CameraManager()
		:mSpace(NULL)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	CameraManager::~CameraManager()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	ICamera*	CameraManager::getCamera(const TString& name) const
	{
		CameraNameMap::const_iterator i = mCameraMap.find(name);
		if( i != mCameraMap.end() )
			return i->second;
		else
			return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		CameraManager::addCamera(const TString& name, ICamera* camera)
	{
		if( camera == NULL )
			return false;

		bool ret = mCameraMap.insert( std::make_pair(name,camera) ).second;
		if (ret && mSpace != NULL)
			mSpace->addContent(static_cast<Camera*>(camera));
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		CameraManager::removeCamera(const TString& name)
	{
		CameraNameMap::iterator i = mCameraMap.find(name);
		if (i != mCameraMap.end())
		{
			if (mSpace != NULL)
				mSpace->removeContent(static_cast<Camera*>(i->second));
			mCameraMap.erase(i);
			return true;
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	void	CameraManager::setSpace(ISpace* space)
	{
		mSpace = space;
		for (CameraNameMap::const_iterator i = mCameraMap.begin(); i != mCameraMap.end(); ++i)
		{
			//TODO: unsafe down cast?
			Camera* cam = static_cast<Camera*>(i->second);
			if (cam->getSpace() == NULL)
				space->addContent(cam);
		}
	}
	

}//namespace Blade