/********************************************************************
	created:	2009/02/14
	filename: 	ISubsystem.h
	author:		Crazii
	
	purpose:	ISubsystem abstraction for Framework
*********************************************************************/
#ifndef __Blade_ISubsystem_h__
#define __Blade_ISubsystem_h__
#include <BladeFramework.h>
#include <Handle.h>
#include <Factory.h>

namespace Blade
{
	class IScene;

	class BLADE_FRAMEWORK_API ISubsystem
	{
	public:
		virtual ~ISubsystem()	{}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const TString&	getName() = 0;

		/**
		@describe this would be called when added to Framework
		@param 
		@return 
		*/
		virtual void		install() = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual void		uninstall() = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void		initialize() = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void		update() = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void		shutdown() = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual IScene*		createScene(const TString& name) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void		destroyScene(const TString& name) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual IScene*		getScene(const TString& name) const = 0;

	};//class subSystem

	extern template class BLADE_FRAMEWORK_API Factory<ISubsystem>;
	typedef Factory<ISubsystem> SubsystemFactory;

}//namespace Blade

#endif // __Blade_ISubsystem_h__