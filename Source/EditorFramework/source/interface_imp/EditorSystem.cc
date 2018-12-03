/********************************************************************
	created:	2013/03/09
	filename: 	EditorSystem.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "EditorSystem.h"


namespace Blade
{
	const TString EditorSystem::EDITOR_SYSTEM_NAME = BTString("BladeEditorSystem");

	//////////////////////////////////////////////////////////////////////////
	EditorSystem::EditorSystem()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	EditorSystem::~EditorSystem()
	{

	}

	/************************************************************************/
	/* ISubsystem interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	const TString&	EditorSystem::getName()
	{
		return EDITOR_SYSTEM_NAME;
	}


	//////////////////////////////////////////////////////////////////////////
	void		EditorSystem::install()
	{
		
	}

	//////////////////////////////////////////////////////////////////////////
	void		EditorSystem::uninstall()
	{
		
	}

	//////////////////////////////////////////////////////////////////////////
	void		EditorSystem::initialize()
	{
		
	}

	//////////////////////////////////////////////////////////////////////////
	void		EditorSystem::update()
	{
		
		//for(SceneList::iterator i = mScenes.begin(); i != mScenes.end(); ++i)
		//{
		//	EditorScene* scene = i->second;
		//	scene->onMainLoopUpdate();
		//}
	}

	//////////////////////////////////////////////////////////////////////////
	void		EditorSystem::shutdown()
	{
		
	}

	//////////////////////////////////////////////////////////////////////////
	IScene*		EditorSystem::createScene(const TString& name)
	{
		SceneHandle& scene = mScenes[name];
		if( scene != NULL )
		{
			BLADE_EXCEPT(EXC_REDUPLICATE,BTString("cannot create scene,a scene with the same name already exist:")+name );
		}
		scene.bind( BLADE_NEW EditorScene() );

		return scene;
	}

	//////////////////////////////////////////////////////////////////////////
	void		EditorSystem::destroyScene(const TString& name)
	{
		mScenes.erase(name);
	}

	//////////////////////////////////////////////////////////////////////////
	IScene*		EditorSystem::getScene(const TString& name) const
	{
		SceneList::const_iterator i = mScenes.find(name);
		if( i == mScenes.end() )
			return NULL;
		else
			return i->second;
	}


}//namespace Blade