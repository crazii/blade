/********************************************************************
	created:	2010/04/30
	filename: 	GraphicsUpdater.h
	author:		Crazii
	purpose:	
	log:		2012/11/23 change name from GraphicsStateQueue to GraphicsUpdater
*********************************************************************/
#ifndef __Blade_GraphicsUpdater_h__
#define __Blade_GraphicsUpdater_h__
#include <utility/BladeContainer.h>
#include <utility/FunctorsExt.h>
#include <utility/FixedArray.h>
#include <interface/IGraphicsUpdater.h>


namespace Blade
{
	class GraphicsUpdater : public IGraphicsUpdater, public Allocatable
	{
	public:
		GraphicsUpdater();
		~GraphicsUpdater();

		/************************************************************************/
		/* IGraphicsUpdater interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual bool addForUpdateOnce(IGraphicsUpdatable* updatable);

		/** @brief  */
		virtual bool addForUpdate(IGraphicsUpdatable* updatable);

		/** @brief  */
		virtual bool removeFromUpdate(IGraphicsUpdatable* updatable);

		/** @brief  */
		virtual bool addForIntervalUpate(IGraphicsUpdatable* updatable, scalar interval);

		/** @brief  */
		virtual bool removeFromIntervalUpdate(IGraphicsUpdatable* updatable, scalar interval);

		/** @brief  */
		virtual void update(SSTATE state);


		class UpdateOnceList : public TempVector<IGraphicsUpdatable*>, public NonCopyable
		{
		public:
			Lock	mOnceLock;
		};//use vector: update once is very special and need to keep order.

		class UpdateList : public Set<IGraphicsUpdatable*>
		{
		public:
			Lock	mLock;
		};
	protected:
		static scalar TOLERANCE;

		typedef FixedArray<UpdateList,SS_TOTAL_COUNT>	UpdateGroupList;
		typedef FixedArray<UpdateOnceList, SS_TOTAL_COUNT> UpdateOnceGroupList;
		

		struct IntervalUpdateList
		{
			IntervalUpdateList()	:mTimer(0)	{}
			UpdateGroupList		mList;
			scalar				mTimer;
		};

		typedef Map<scalar, IntervalUpdateList, FnScalarLess<TOLERANCE> > IntervalMap;
#if BLADE_DEBUG
		UpdateList				mRegistry;
		Lock					mRegistryLock;
#endif
		UpdateGroupList			mUpdateList;
		UpdateOnceGroupList		mOnceList;

		IntervalMap				mIntervalMap;
		Lock					mIntervalLock;
	};

	
}//namespace Blade


#endif //__Blade_GraphicsUpdater_h__