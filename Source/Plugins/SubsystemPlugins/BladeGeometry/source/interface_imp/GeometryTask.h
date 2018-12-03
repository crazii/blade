/********************************************************************
	created:	2010/05/07
	filename: 	GeometryTask.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_GeometryTask_h__
#define __Blade_GeometryTask_h__
#include <TaskBase.h>
#include <interface/public/geometry/GeomTypes.h>

namespace Blade
{
	class GeometryUpdater;
	class ParaStateQueue;

	class GeometryTask : public TaskBase, public Allocatable
	{
	public:
		static const ITask::Type TYPE;
	public:
		GeometryTask(const TString& name,GeometryUpdater& updater, ParaStateQueue& queue,ITask::ETaskPriority priority = ITask::TP_NORMAL);
		~GeometryTask();

		/************************************************************************/
		/* ITask interface                                                                     */
		/************************************************************************/
		/*
		@describe get task main device type on which it depends,such as CPU or GPU or HDD,etc.
		@param 
		@return 
		@remark this is useful for parallel optimizing \n
		for example,a physics task maybe on CPU,or on other device like GPU (PhysX)
		*/
		virtual const Type&	getType() const;

		/*
		@describe run the ITask
		@param 
		@return 
		*/
		virtual void			run();

		/*
		@describe update the task: synchronize data
		@param 
		@return 
		*/
		virtual void			update();

		/*
		@describe
		@param
		@return
		*/
		virtual void			onAccepted();

		/*
		@describe
		@param
		@return
		*/
		virtual void			onRemoved();

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/

		/*
		@describe
		@param
		@return
		*/
		GEOM_UPDATE_METHOD	getUpdateMethod() const;

		/*
		@describe
		@param
		@return
		*/
		void				setUpdateMethod(GEOM_UPDATE_METHOD method);

		/*
		@describe
		@param
		@return
		*/
		void				updateData();

	protected:
		GEOM_UPDATE_METHOD		mMethod;

		ParaStateQueue&			mQueue;
		GeometryUpdater&		mUpdater;
	private:
		GeometryTask&		operator=(const GeometryTask&);
	};//class GeometryTask
	
}//namespace Blade



#endif //__Blade_GeometryTask_h__