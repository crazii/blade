/********************************************************************
	created:	2010/05/07
	filename: 	GeometryScene.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_GeometryScene_h__
#define __Blade_GeometryScene_h__
#include <BladeGeometry.h>
#include <interface/public/ITask.h>
#include <interface/public/geometry/IGeometryScene.h>
#include <parallel/ParaStateQueue.h>
#include "GeometryUpdater.h"


namespace Blade
{
	class GeometryScene : public IGeometryScene, public Allocatable
	{
	public:
		static const TString GEOMETRY_SCENE_TYPE;
	public:
		GeometryScene();
		~GeometryScene();

		/************************************************************************/
		/* ISerializable interface                                                                     */
		/************************************************************************/
		/** @brief called after loading data */
		virtual void			postProcess(const ProgressNotifier& notifier);

		/************************************************************************/
		/* IScene Interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual const TString&	getSceneType() const;

		///*
		//@describe 
		//@param 
		//@return 
		//*/
		//virtual const TString&	getName() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			getAvailableElementTypes(ElementTypeList& elemList) const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual IElement*		createElement(const TString& type);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			onAttachToStage(IStage* pstage);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			onDetachFromStage(IStage* pstage);

		/************************************************************************/
		/* IGeometryScene interface                                                                     */
		/************************************************************************/
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
		virtual void				update();

	protected:
		GEOM_UPDATE_METHOD		mUpdateMethod;
		HTASK					mGeomTask;

		ParaStateQueue			mStateQueue;
		GeometryUpdater			mUpdater;
	};//class GeometryScene
	
}//namespace Blade


#endif //__Blade_GeometryScene_h__