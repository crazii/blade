/********************************************************************
	created:	2018/12/20
	filename: 	ParallelAnimation.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_ParallelAnimation_h__
#define __Blade_ParallelAnimation_h__
#include <interface/IGraphicsUpdater.h>
#include <interface/ITaskManager.h>
#include <interface/ITimeService.h>
#include "Model.h"

namespace Blade
{
	class ParallelAnimation : public IGraphicsUpdatable, public Allocatable
	{
	public:
		static const size_t MAX_PARALLEL_COUNT = 4;

		struct DelayedAnimation
		{
			Model*	model;
			bool	updateBounding;
		};

	public:
		ParallelAnimation()
		{
			mUpdateStateMask.raiseBitAtIndex(SS_PRE_RENDER);
		}
		~ParallelAnimation()
		{

		}

		/** @brief  */
		inline void addDelayedAnimation(Model* model, bool updateBounding)
		{
			DelayedAnimation da{ model, updateBounding };
			mAnimationList.push_back(da);
		}

		struct AnimationTask : public IDispatchableTask
		{
			DelayedAnimation* list;
			size_t count;
			uint64 loopID;
			scalar time;
			scalar frameTime;

			virtual ~AnimationTask() {}
			virtual const TString& getName() const
			{
#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable:4640)//construction of local static object is not thread-safe
#endif
				static const TString TYPE = BTString("Animation task"); return TYPE;
#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(pop)
#endif
			}
			virtual void run()
			{
				for (size_t i = 0; i < count; ++i)
					list[i].model->updateSkinnedAnimation(loopID, time, frameTime, list[i].updateBounding);
			}
		};

		/** @brief  */
		virtual void update(SSTATE /*state*/)
		{
			const TimeSource& time = ITimeService::getSingleton().getTimeSource();

			if (mAnimationList.size() < MAX_PARALLEL_COUNT)
			{
				for (size_t i = 0; i < mAnimationList.size(); ++i)
				{
					DelayedAnimation& da = mAnimationList[i];
					da.model->updateSkinnedAnimation(time.getLoopID(), time.getTime(), da.updateBounding);
				}
			}
			else
			{
				size_t taskCount = MAX_PARALLEL_COUNT;
				size_t countPerTask = mAnimationList.size() / taskCount;
				size_t remainder = mAnimationList.size() % taskCount;
				assert(countPerTask >= 1);
				size_t offset = 0;

				uint64 loopID = time.getLoopID();
				scalar elapsedTime = time.getTime();
				scalar frameTime = time.getTimeThisLoop();

				DispatchableTaskList<AnimationTask> tasks(taskCount);
				for (size_t i = 0; i < taskCount; ++i)
				{
					tasks[i].list = &mAnimationList[offset];
					offset += countPerTask;

					tasks[i].count = countPerTask;
					tasks[i].loopID = loopID;
					tasks[i].time = elapsedTime;
					tasks[i].frameTime = frameTime;

					//last task remainder
					if (i == taskCount - 1)
						tasks[i].count += remainder;
				}
				IDispatchableTask::Handles handles;
				ITaskManager::getSingleton().dispatchTasks(tasks.getTasks(), tasks.size(), &handles);
				handles.sync();
			}

			mAnimationList.clear();
		}
	protected:
		TempVector<DelayedAnimation>	mAnimationList;
	};

	
}//namespace Blade

#endif //__Blade_ParallelAnimation_h__