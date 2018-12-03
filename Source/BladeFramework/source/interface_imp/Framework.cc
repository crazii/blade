/********************************************************************
	created:	2009/02/14
	filename: 	Framework.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "Framework.h"
#include <ExceptionEx.h>
#include <ConstDef.h>
#include <utility/Profiling.h>
#include <StagePool.h>	//not used yet.

//internal dependency...(which is not as good as designed)
#include <interface/IEventManager.h>
#include "TimeSourceManager.h"
//
#if BLADE_DEBUG
#include <interface/IResourceManager.h>
#endif

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	class StagePoolInteral : public StagePool
	{
	public:
		using StagePool::initialize;
		using StagePool::shutdown;
	};

	template class Factory<IFramework>;

	//////////////////////////////////////////////////////////////////////////
	//ctor & dector
	Framework::Framework()
		:mEventManger(NULL)
		,mTimeManager(NULL)
		,bStarted(false)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	Framework::~Framework()
	{
	}

	/************************************************************************/
	/* IFramework interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void		Framework::initialize()
	{
		mEventManger = IEventManager::getSingletonPtr();
		mTimeManager = TimeSourceManager::getSingletonPtr();

		mEventManger->registerEvent( ConstDef::EVENT_MAIN_LOOP );
		mEventManger->registerEvent( ConstDef::EVENT_LOOP_DATA_INIT );
		mEventManger->registerEvent( ConstDef::EVENT_LOOP_DATA_CLEAR );
	}

	//////////////////////////////////////////////////////////////////////////
	void		Framework::shutdown()
	{
		mEventManger->unRegisterEvent( ConstDef::EVENT_MAIN_LOOP);
		mEventManger->unRegisterEvent( ConstDef::EVENT_LOOP_DATA_INIT );
		mEventManger->unRegisterEvent( ConstDef::EVENT_LOOP_DATA_CLEAR );

		mEventManger = NULL;
		mTimeManager = NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	void		Framework::addSubSystem(ISubsystem* hSubsystem)
	{
		if( hSubsystem == NULL )
			return;

		//assert( std::find(mSubSystemList.begin(),mSubSystemList.end(),hSubsystem) == mSubSystemList.end() );
		ISubsystem* & empty = mSubSystemMap[ hSubsystem->getName() ];
		if( empty != NULL )
		{
			BLADE_EXCEPT(EXC_REDUPLICATE,BTString("A Subsystem with the same name \"")+hSubsystem->getName() + TEXT("\" already exist.") );
			return;
		}

		empty = hSubsystem;

		mSubSystemList.push_back(empty);
		hSubsystem->install();

		if( bStarted )
		{
			hSubsystem->initialize();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	ISubsystem*	Framework::addSubSystem(const TString& SubsysClassName)
	{
		if( SubsysClassName == TString::EMPTY )
			BLADE_EXCEPT(EXC_REDUPLICATE,BTString("A Subsystem Class' name should not be EMPTY") );

		if( mSubSystemDesc.insert( SubsysClassName ).second == false )
			BLADE_EXCEPT(EXC_REDUPLICATE,BTString("A Subsystem Class with the same name \"")+ SubsysClassName + TEXT("\" already exist.") );

		ISubsystem* subsystem = BLADE_FACTORY_CREATE(ISubsystem,SubsysClassName);
		this->addSubSystem(subsystem);
		return subsystem;
	}

	//////////////////////////////////////////////////////////////////////////
	ISubsystem*	Framework::getSubSystem(const TString& SubsysName) const
	{
		SubSystemMap::const_iterator it = mSubSystemMap.find( SubsysName );
		if( it == mSubSystemMap.end() )
			return NULL;
		else
			return it->second;
	}


	//////////////////////////////////////////////////////////////////////////
	void		Framework::start()
	{
		if( bStarted == false )
			bStarted = true;
		else
		{
			assert(false);
			return;
		}

		mTimeManager->getTimeSource().reset();

		for( SubSystemList::iterator iter = mSubSystemList.begin(); iter != mSubSystemList.end(); ++iter)
		{
			(*iter)->initialize();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void		Framework::update()
	{
		mTimeManager->getTimeSource().update();

		//update registered main loop updater - some updater need update before subsystems (i.e. track view UI need update animations before sync draw)
		std::for_each(mUpdaters.begin(), mUpdaters.end(), std::mem_fun(&IMainLoopUpdater::update));

		//update subsystems
		std::for_each(mSubSystemList.begin(), mSubSystemList.end(), std::mem_fun(&ISubsystem::update));

		//update stages
		for (StageMap::iterator i = mStageMap.begin(); i != mStageMap.end(); ++i)
			i->second->update();

		mEventManger->dispatchEvent(Event(ConstDef::EVENT_MAIN_LOOP));
		mEventManger->dispatchEvent(Event(ConstDef::EVENT_LOOP_DATA_CLEAR));
		FrameAccumulateProfiling::onLoopFinish();

#if BLADE_MEMORY_DEBUG	//temporary memory profiling
		{
			static scalar overtime = 0.0f;
			static const scalar THRESHOLD = 3.6f;

			if (!IResourceManager::getSingleton().isLoadingResources())
			{
				IPool::SPoolState states;
				IPool* tempPool = Memory::getTemporaryPool();
				tempPool->getPoolState(states);
				if (states.mAllocatedBytes != 0 || states.mAllocationCount != 0)
				{
					//assert(false && "temporary memory should be release on each loop.");
					overtime += mTimeManager->getTimeSource().getTimeThisLoop();
					if (overtime > THRESHOLD)
					{
						BLADE_LOG(Error, TEXT("all temporary memories should be released on each loop.") );
						overtime = 0.0f;
					}
				}
				else
					overtime = 0.0f;
			}
		}
#endif

#if 1
		//temporarily check Frame rate
		{
			static scalar overtime = 0.0f;
			static const scalar THRESHOLD = 1.2f;
			overtime += TimeSourceManager::getSingleton().getTimeSource().getTimeThisLoop();
			if( overtime > THRESHOLD )
			{
				BLADE_LOG(Warning, BTString("Frametime:") << TimeSourceManager::getSingleton().getTimeSource().getTimeThisLoop() );
				BLADE_LOG(Warning, BTString("AvgFrame:") << TimeSourceManager::getSingleton().getTimeSource().getTimePerLoop() );
				overtime = 0.0f;
			}
		}
#endif

		mEventManger->dispatchEvent(Event(ConstDef::EVENT_LOOP_DATA_INIT));
		FrameAccumulateProfiling::onLoopInit();	//TODO: move envent manager to base/foundation?
	}

	//////////////////////////////////////////////////////////////////////////
	void		Framework::terminate()
	{
		//signal data clear
		mEventManger->dispatchEvent(Event(ConstDef::EVENT_LOOP_DATA_CLEAR));

		mStageMap.clear();
		for( SubSystemList::reverse_iterator iter = mSubSystemList.rbegin(); iter != mSubSystemList.rend(); ++iter)
		{
			(*iter)->shutdown();
			(*iter)->uninstall();
		}
		mSubSystemList.clear();
		mSubSystemMap.clear();
	}

	//////////////////////////////////////////////////////////////////////////
	bool		Framework::addUpdater(IMainLoopUpdater* updater)
	{
		if( updater == NULL )
			return false;
		else
			return mUpdaters.insert(updater).second;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		Framework::removeUpdater(IMainLoopUpdater* updater)
	{
		return mUpdaters.erase(updater) == 1;
	}

	/************************************************************************/
	/* IStageManager interface                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	IStage*		Framework::createStage(const TString& name, IWindow* Window, bool autoDelete/* = true*/, bool autoScene/* = true*/, bool showWindow/* = true*/)
	{
		HSTAGE& hstage = mStageMap[name];
		if( hstage != NULL )
			BLADE_EXCEPT(EXC_REDUPLICATE,BTString("a stage with the same name already exist:")+ name );

		hstage.bind(BLADE_NEW Stage(name, Window, this, autoScene, autoDelete, showWindow));

		//stage pool intentionally for per stage physics,AI(path-finding),sound,client logics
		//it is not used currently
		//StagePoolInteral::initialize();

		if( autoScene )
		{
			for( SubSystemList::iterator iter = mSubSystemList.begin(); iter != mSubSystemList.end(); ++iter)
			{
				IScene* pscene = (*iter)->createScene( name );
				if( pscene != NULL )
					hstage->addScene(pscene);
			}

		}
		return hstage;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		Framework::destroyStage(const TString& StageName)
	{
		StageMap::iterator i = mStageMap.find( StageName );
		if( i == mStageMap.end() )
			return false;

		bool deleteScene = i->second->isSceneAutoBinded();

		//in case that the name comes from the stage itself,
		//then the stage name cannot be used any more
		const TString name = StageName;

		//the stage should be deleted before scenes, because the entities may depend on some sub system/scene data
		mStageMap.erase(i);

		if( deleteScene )
		{
			//destroy scene in reversed order
			for( SubSystemList::reverse_iterator iter = mSubSystemList.rbegin(); iter != mSubSystemList.rend(); ++iter)
			{
				(*iter)->destroyScene( name );
			}
		}

		StagePoolInteral::shutdown();
		return true;
	}


	//////////////////////////////////////////////////////////////////////////
	IStage*		Framework::getStage(const TString& StageName) const
	{
		StageMap::const_iterator i = mStageMap.find( StageName );
		if( i == mStageMap.end() )
			return NULL;
		else
			return i->second;
	}

}//namespace Blade