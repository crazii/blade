/********************************************************************
	created:	2010/04/09
	filename: 	TaskBase.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_TaskBase_h__
#define __Blade_TaskBase_h__
#include <BladeFramework.h>
#include <interface/public/ITask.h>

namespace Blade
{
	class BLADE_FRAMEWORK_API TaskBase : public ITask
	{
	public:
		TaskBase(const TString& name, ETaskPriority priority = TP_NORMAL, uint32 priorityID = 0);
		virtual ~TaskBase();

		/************************************************************************/
		/* ITask class spec                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual	const TString&	getName() const;

		/**
		@describe get the ITask's priority level
		@param 
		@return 
		*/
		virtual ETaskPriority	getPriority() const;

		/**
		@describe priority id with the same level,higher means high priority
		@param
		@return
		*/
		virtual uint32			getPriorityID() const;

	protected:
		TString			mName;
		uint32			mPriorityID;
		ETaskPriority	mPriority;
	};

	
}//namespace Blade


#endif //__Blade_TaskBase_h__