/********************************************************************
	created:	2015/08/17
	filename: 	BPKExplorer.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_BPKExplorer_h__
#define __Blade_BPKExplorer_h__
#include <interface/IConfig.h>
#include <interface/public/IEditor.h>
#include <interface/public/ui/IMenu.h>

namespace Blade
{

	class BPKExplorer : public IEditor, public Singleton<BPKExplorer>
	{
	public:
		BPKExplorer();
		~BPKExplorer();

		/************************************************************************/
		/* IEditor interface                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		virtual void			initEditor();

		/*
		@describe
		@param
		@return
		*/
		virtual void			shutdownEditor();

		/************************************************************************/
		/* custom method                                                                     */
		/************************************************************************/
		/** @brief  */
		IMenu*			getContextMenu()	{return mContextMenu;}
		/** @brief  */
		const HCONFIG&	getNaviInputConfig(){return mNavigationConfig;}

	protected:

		IMenu*	mContextMenu;
		HCONFIG	mNavigationConfig;
	};
	
}//namespace Blade


#endif // __Blade_BPKExplorer_h__
