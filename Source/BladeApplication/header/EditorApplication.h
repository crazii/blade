/********************************************************************
	created:	2010/05/23
	filename: 	EditorApplication.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_EditorApplication_h__
#define __Blade_EditorApplication_h__
#include <BladeApplication.h>
#include <Singleton.h>
#include <utility/String.h>
#include <interface/public/ui/IUIMainWindow.h>
#include <interface/public/ui/IIconManager.h>
#include <Application.h>

namespace Blade
{
	class BLADE_APPLICATION_API EditorApplication : public Application, public Singleton<EditorApplication>
	{
	public:
		static const TString TYPE;
	public:
		EditorApplication();
		virtual ~EditorApplication();

	protected:
		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/

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

		/************************************************************************/
		/*                                                                      */
		/************************************************************************/

		/*
		@describe
		@param
		@return
		*/
		virtual IUIMainWindow*	createEditorUI();

	protected:
		TString		mEditorConfigFile;
		bool		mInited;
	};//class EditorApplication
	
}//namespace Blade


#endif //__Blade_EditorApplication_h__