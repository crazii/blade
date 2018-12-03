/********************************************************************
	created:	2010/05/01
	filename: 	GameTask.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_GameTask_h__
#define __Blade_GameTask_h__
#include <TaskBase.h>
#include <interface/public/logic/ILogic.h>
#include <utility/BladeContainer.h>

namespace Blade
{
	class GameTask : public TaskBase, public Allocatable
	{
	public:
		static const ITask::Type TYPE;
	public:
		GameTask(const TString& name,ITask::ETaskPriority priority = TP_NORMAL);
		~GameTask();

		/************************************************************************/
		/* ITask class interfaces                                                                     */
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
		void					addLogic(ILogic* logic);

	protected:
		typedef	Set<ILogic*>	GameLgocSet;

		GameLgocSet		mLgoicSet;
	};//class GameTask
	
}//namespace Blade


#endif //__Blade_GameTask_h__
