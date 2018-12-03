/********************************************************************
	created:	2010/05/07
	filename: 	GeometrySystem.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "GeometrySystem.h"

namespace Blade
{
	const TString GeometrySystem::GEOMETRY_SYSTEM_NAME = BTString("BladeGeometrySystem");

	//////////////////////////////////////////////////////////////////////////
	GeometrySystem::GeometrySystem()
		:mUpdateMethod(GUM_MANUAL)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	GeometrySystem::~GeometrySystem()
	{

	}

	/************************************************************************/
	/* ISubsystem interface                                                                     */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	const TString&	GeometrySystem::getName()
	{
		return GEOMETRY_SYSTEM_NAME;
	}


	//////////////////////////////////////////////////////////////////////////
	void		GeometrySystem::install()
	{
		RegisterSingleton(GeometrySystem, IGeometryService);
	}

	//////////////////////////////////////////////////////////////////////////
	void		GeometrySystem::uninstall()
	{
		UnregisterSingleton(GeometrySystem, IGeometryService);
	}

	//////////////////////////////////////////////////////////////////////////
	void		GeometrySystem::initialize()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void		GeometrySystem::update()
	{
		
	}

	//////////////////////////////////////////////////////////////////////////
	void		GeometrySystem::shutdown()
	{
		
	}

	//////////////////////////////////////////////////////////////////////////
	IScene*		GeometrySystem::createScene(const TString& name)
	{
		SceneHandle& scene = mSceneList[name];

		if( scene != NULL )
		{
			BLADE_EXCEPT(EXC_REDUPLICATE,BTString("a scene with the same name already exist:")+name);
		}
		scene.bind( BLADE_NEW GeometryScene() );
		return scene;
	}

	//////////////////////////////////////////////////////////////////////////
	void		GeometrySystem::destroyScene(const TString& name)
	{
		mSceneList.erase(name);
	}

	//////////////////////////////////////////////////////////////////////////
	IScene*		GeometrySystem::getScene(const TString& name) const
	{
		SceneList::const_iterator i = mSceneList.find(name);
		if( i == mSceneList.end() )
			return NULL;
		else
			return i->second;
	}

	/************************************************************************/
	/* IGeometryService interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	IGeometryScene*						GeometrySystem::getGeometryScene(const TString& name) const
	{
		SceneList::const_iterator i = mSceneList.find(name);
		if( i == mSceneList.end() )
			return NULL;
		else
			return i->second;
	}

	//////////////////////////////////////////////////////////////////////////
	GEOM_UPDATE_METHOD		GeometrySystem::getUpdateMethod() const
	{
		return mUpdateMethod;
	}

	//////////////////////////////////////////////////////////////////////////
	void				GeometrySystem::setUpdateMethod(GEOM_UPDATE_METHOD method)
	{
		mUpdateMethod = method;

		for( SceneList::iterator i = mSceneList.begin(); i != mSceneList.end(); ++i )
		{
			SceneHandle& handle = i->second;
			handle->setUpdateMethod(method);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void				GeometrySystem::updateScenes()
	{
		if( mUpdateMethod == GUM_MANUAL )
		{
			for( SceneList::iterator i = mSceneList.begin(); i != mSceneList.end(); ++i )
			{
				SceneHandle& handle = i->second;
				if( handle->getUpdateMethod() == GUM_MANUAL )
					handle->update();
			}
		}
	}
	
}//namespace Blade