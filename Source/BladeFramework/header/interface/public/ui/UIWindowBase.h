/********************************************************************
	created:	2013/11/17
	filename: 	UIWindowBase.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_UIWindowBase_h__
#define __Blade_UIWindowBase_h__
#include <BladeFramework.h>
#include "IUIWindow.h"

namespace Blade
{
	class BLADE_FRAMEWORK_API UIWindowBase : public IUIWindow
	{
	public:
		UIWindowBase(const TString& type);
		virtual ~UIWindowBase();

		/************************************************************************/
		/* IUIWindow interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual const TString&	getWindowType() const	{return mType;}

		//////////////////////////////////////////////////////////////////////////
		//all the fields below should be available right after this IUIWindow instance is created
		//////////////////////////////////////////////////////////////////////////
		/** @brief  */
		virtual IconIndex		getIcon() const		{return mIcon;}

		/** @brief used by framework */
		virtual void			setFrameworkData(void* data) { mFrameworkData = data; }

		/** @brief used by framework */
		virtual void*			getFrameworkData() const { return mFrameworkData; }

		/** @brief  */
		virtual void			setUserData(void* data) { mUserData = data; }

		/** @brief  */
		virtual void*			getUserData() const { return mUserData; }

		/** @brief  */
		virtual IWindow*		getWindow() const { return mWindow; }

		/** @brief  */
		virtual bool			setUpdater(IUIWindow::IUpdater* updater)
		{
			if (mUpdater == NULL || updater == NULL)
			{
				mUpdater = updater;
				return true;
			}
			return false;
		}

		/** @brief  */
		virtual IUIWindow::IUpdater* getUpdater() const { return mUpdater; }

		/** @brief  */
		virtual IKeyboard*		getKeyboard() const { return mKeyboard; }

		/** @brief  */
		virtual IMouse*			getMouse() const	{ return mMouse; }

	protected:
		/** @brief internal helper */
		bool	init(uintptr_t windowImpl, IconIndex icon, const TString& caption, bool createMouse, bool createKeyboard);

		TString			mType;
		IconIndex		mIcon;
		IWindow*		mWindow;
		IKeyboard*		mKeyboard;
		IMouse*			mMouse;
		IUIWindow::IUpdater*mUpdater;
		void*			mFrameworkData;
		void*			mUserData;
	};
	
}//namespace Blade

#endif //  __Blade_UIWindowBase_h__