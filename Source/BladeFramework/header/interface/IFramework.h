/********************************************************************
	created:	2010/04/02
	filename: 	IFramework.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_IFramework_h__
#define __Blade_IFramework_h__
#include <BladeFramework.h>
#include <interface/InterfaceSingleton.h>
#include <ExceptionEx.h>
#include <interface/public/ISubsystem.h>
#include <interface/IStage.h>

namespace Blade
{
	class IFramework : public IStageManager, public InterfaceSingleton<IFramework>
	{
	public:
		class IMainLoopUpdater
		{
		public:
			virtual void	update() = 0;
		};
	public:
		virtual ~IFramework()	{}

		/**
		@describe initial phase, prepare shared data etc.
		@param 
		@return 
		@note framework should initialize before all plug-in installation
		*/
		virtual void		initialize() = 0;

		/**
		@describe final phase, clear all data
		@param 
		@return 
		*/
		virtual void		shutdown() = 0;

		/**
		@describe 
		@param 
		@return 
		@note if the framework is already started, the new added\n
		subsystem will be initialized instantly
		*/
		virtual void		addSubSystem(ISubsystem* Subsystem) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual ISubsystem*	addSubSystem(const TString& SubsysClassName) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual ISubsystem*	getSubSystem(const TString& SubsysName) const = 0;

		/**
		@describe initialize all added subsystems and prepare to run
		@param 
		@return 
		*/
		virtual void		start() = 0;

		/**
		@describe update the framework once
		@param 
		@return 
		*/
		virtual void		update() = 0;

		/**
		@describe shutdown all subsystems and uninstall them
		@param 
		@return 
		*/
		virtual void		terminate() = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool		addUpdater(IMainLoopUpdater* updater) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool		removeUpdater(IMainLoopUpdater* updater) = 0;

	};

	extern template class BLADE_FRAMEWORK_API Factory<IFramework>;
	
}//namespace Blade


#endif //__Blade_IFramework_h__