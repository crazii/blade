/********************************************************************
created:	2013/03/09
filename: 	EditorSystem.h
author:		Crazii

purpose:	system for editor framework
			it's currently almost empty
*********************************************************************/
#ifndef __Blade_EditorSystem_h__
#define __Blade_EditorSystem_h__
#include <StaticHandle.h>
#include <utility/BladeContainer.h>
#include <interface/public/ISubsystem.h>
#include "EditorScene.h"

namespace Blade
{

	class EditorSystem : public ISubsystem, public Singleton<EditorSystem>
	{
	public:
		static const TString EDITOR_SYSTEM_NAME;
	public:
		EditorSystem();
		~EditorSystem();

		/************************************************************************/
		/* ISubsystem interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual const TString&	getName();


		/*
		@describe this would be called when added to Framework
		@param 
		@return 
		*/
		virtual void		install();

		/*
		@describe
		@param
		@return
		*/
		virtual void		uninstall();

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void		initialize();

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void		update();

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void		shutdown();

		/*
		@describe 
		@param 
		@return 
		*/
		virtual IScene*		createScene(const TString& name);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void		destroyScene(const TString& name);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual IScene*		getScene(const TString& name) const;

	protected:
		typedef StaticHandle<EditorScene>	SceneHandle;
		typedef TStringMap<SceneHandle>		SceneList;

		SceneList		mScenes;
	};//class EditorSystem

	
}//namespace Blade

#endif//__Blade_EditorSystem_h__