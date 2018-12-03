/********************************************************************
	created:	2010/04/02
	filename: 	GameApplication.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_GameApplication_h__
#define __Blade_GameApplication_h__
#include <BladeApplication.h>
#include <Singleton.h>
#include <utility/String.h>
#include <Application.h>

namespace Blade
{
	class BLADE_APPLICATION_API GameApplication : public Application, public Singleton<GameApplication>
	{
	public:
		static const TString TYPE;
	public:
		GameApplication();
		virtual ~GameApplication();

	protected:
		/*
		@describe init the application
		@param none
		@return result
		@retval true init succeeded
		@retval false init failed
		@remark it will call preInitialize before init \n
		and postInitialize after init
		*/
		virtual bool	doInitialize();

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool	preInitialize();

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool	postInitialize();

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void	closeImpl();

	};//class Application

	
}//namespace Blade


#endif //__Blade_GameApplication_h__