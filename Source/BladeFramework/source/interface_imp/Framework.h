/********************************************************************
	created:	2009/02/14
	filename: 	Framework.h
	author:		Crazii
	
	purpose:	main Framework of engine
*********************************************************************/
#ifndef __Blade_Framework_h__
#define __Blade_Framework_h__
#include <BladeFramework.h>
#include <interface/IFramework.h>
#include "Stage.h"

namespace Blade
{
	class TimeSourceManager;
	class IEventManager;

	class Framework : public IFramework , public Singleton<Framework>
	{
	public:
		Framework();
		~Framework();

		/************************************************************************/
		/* IFramework interface                                                                     */
		/************************************************************************/
		/**
		@describe initial phase, prepare shared data etc.
		@param 
		@return 
		@note framework should initialize before all plug-in installation
		*/
		virtual void		initialize();

		/**
		@describe final phase, clear all data
		@param 
		@return 
		*/
		virtual void		shutdown();

		/**
		@describe 
		@param 
		@return 
		@note if the framework is already started, the new added\n
		subsystem will be initialized instantly
		*/
		virtual void		addSubSystem(ISubsystem* hSubsystem);

		/**
		@describe 
		@param 
		@return 
		*/
		virtual ISubsystem*	addSubSystem(const TString& SubsysClassName);

		/**
		@describe 
		@param 
		@return 
		*/
		virtual ISubsystem*	getSubSystem(const TString& SubsysName) const;

		/**
		@describe initialize all added subsystems and prepare to run
		@param 
		@return 
		*/
		virtual void		start();

		/**
		@describe update the framework once
		@param 
		@return 
		*/
		virtual void		update();

		/**
		@describe shutdown all subsystems and uninstall them
		@param 
		@return 
		*/
		virtual void		terminate();

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool		addUpdater(IMainLoopUpdater* updater);

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool		removeUpdater(IMainLoopUpdater* updater);


		/************************************************************************/
		/* IStageManager interface                                      */
		/************************************************************************/
		/** @copydoc IStageManager::createStage */
		virtual IStage*		createStage(const TString& name, IWindow* Window, bool autoDelete = true, bool autoScene = true, bool showWindow = true);

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool		destroyStage(const TString& StageName);

		/**
		@describe 
		@param 
		@return 
		*/
		virtual IStage*		getStage(const TString& StageName) const;

	private:

		/** @brief Subsystem Class Info */
		typedef StaticTStringSet				SubsystemClassDesc;
		/** @brief  */
		typedef	StaticTStringMap<ISubsystem*>	SubSystemMap;
		/** @brief  */
		typedef StaticVector<ISubsystem*>		SubSystemList;
		/** @brief  */
		typedef	TStringMap< HSTAGE >			StageMap;
		/** @brief  */
		typedef Set<IMainLoopUpdater*>			UpdaterSet;

		SubsystemClassDesc	mSubSystemDesc;

		SubSystemList		mSubSystemList;
		SubSystemMap		mSubSystemMap;
		StageMap			mStageMap;
		UpdaterSet			mUpdaters;

		IEventManager*		mEventManger;
		TimeSourceManager*	mTimeManager;

		/** @brief indicates whether the framework is initialized */
		bool				bStarted;
	};

}//namespace Blade



#endif // __Blade_Framework_h__