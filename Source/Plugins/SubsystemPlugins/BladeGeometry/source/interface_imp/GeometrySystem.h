/********************************************************************
	created:	2010/05/07
	filename: 	GeometrySystem.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_GeometrySystem_h__
#define __Blade_GeometrySystem_h__
#include <StaticHandle.h>
#include <utility/BladeContainer.h>
#include <BladeGeometry.h>
#include <interface/public/geometry/IGeometryService.h>
#include "GeometryScene.h"

namespace Blade
{
	class GeometrySystem : public ISubsystem, public IGeometryService,public Singleton<GeometrySystem>
	{
	public:
		static const TString GEOMETRY_SYSTEM_NAME;
	public:
		GeometrySystem();
		~GeometrySystem();

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

		/************************************************************************/
		/* IGeometryService interface                                                                     */
		/************************************************************************/
		/**
		@describe 
		@param
		@return
		*/
		virtual IGeometryScene*		createGeometryScene(const TString& name)
		{
			return static_cast<IGeometryScene*>(this->createScene(name));
		}

		/*
		@describe
		@param
		@return
		*/
		virtual IGeometryScene*		getGeometryScene(const TString& name) const;
		/*
		@describe
		@param
		@return
		*/
		virtual GEOM_UPDATE_METHOD	getUpdateMethod() const;

		/*
		@describe
		@param
		@return
		*/
		virtual void				setUpdateMethod(GEOM_UPDATE_METHOD method);

		/*
		@describe update the geometry if update method is manual otherwise,do not call it
		@param
		@return
		*/
		virtual void				updateScenes();

	protected:
		typedef	StaticHandle<GeometryScene>	SceneHandle;
		typedef	TStringMap<SceneHandle>	SceneList;


		GEOM_UPDATE_METHOD	mUpdateMethod;

		SceneList			mSceneList;

	};
	
}//namespace Blade


#endif //__Blade_GeometrySystem_h__