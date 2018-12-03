/********************************************************************
	created:	2013/05/19
	filename: 	ToolApplication.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_ToolApplication_h__
#define __Blade_ToolApplication_h__
#include <Application.h>
#include <ToolLog.h>
#include <interface/ILog.h>

namespace Blade
{
	class BLADE_APPLICATION_API ToolApplication : public Application
	{
	public:
		ToolApplication();
		virtual ~ToolApplication();

		/** @brief  */
		bool			hasError() const;

		/** @brief  */
		void			setError(bool error);

	protected:

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


		/*
		@describe 
		@param 
		@return 
		*/
		virtual void	initPlugins() = 0;

	protected:
		//backup CWD on app startup, tool app will change it to its own exec path
		TString	mCurrentWorkingDir;
	};//class ToolApplication
	
}//namespace Blade

#endif //  __Blade_ToolApplication_h__