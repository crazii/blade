/********************************************************************
	created:	2012/03/22
	filename: 	GraphicsTaskController.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_GraphicsTaskController_h__
#define __Blade_GraphicsTaskController_h__
#include "GraphicsTask.h"

namespace Blade
{
	class IRenderScene;

	class GraphicsTaskController : public IGraphicsTaskController, public Allocatable
	{
	public:
		GraphicsTaskController(IRenderScene* pScene);
		virtual ~GraphicsTaskController();

		/************************************************************************/
		/* IGraphicsTaskController interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual bool	isTaskRunNeeded(GraphicsTask* ptask,uint32& sleeptime) const;
		/** @brief  */
		virtual void	onTaskAccepted(GraphicsTask* ptask);
		/** @brief  */
		virtual void	onTaskRemoved(GraphicsTask* ptask);
		/** @brief  */
		virtual void	onTaskDestroy(GraphicsTask* ptask);
		/** @brief  */
		virtual void	updateThreadID(Thread::ID id);
		/** @brief  */
		virtual IRenderScene* getScene() const { return mScene; }

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		inline void		setSkipRunAndUpdate(bool skip)	{mSkippAll = skip;}
		/** @brief  */
		inline bool		getSkipRunAndUpdate() const		{return mSkippAll;}

	protected:
		IRenderScene*	mScene;
		bool			mSkippAll;
	};
	

}//namespace Blade


#endif //__Blade_GraphicsTaskController_h__