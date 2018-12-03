/********************************************************************
	created:	2016/6/2
	filename: 	IUIWindow.h
	author:		Crazii
	purpose:	child window that present main content
*********************************************************************/
#ifndef __Blade_IUIWindow_h__
#define __Blade_IUIWindow_h__
#include <BladeFramework.h>
#include <Factory.h>
#include "UITypes.h"

namespace Blade
{
	class IWindow;
	class IUIToolBox;
	class IMouse;
	class IGraphicsView;

	class CameraActor;

	class BLADE_FRAMEWORK_API IUIWindow
	{
	public:
		class BLADE_FRAMEWORK_API IUpdater
		{
		public:
			virtual ~IUpdater() {}

			/** @brief setup window related params after window created or settings changed */
			virtual void setupWindow(IUIWindow* window) = 0;
		};

	public:
		/** @brief  */
		virtual const TString&	getWindowType() const = 0;

		/** @brief  */
		virtual bool			initWindow(uintptr_t parentWindowImpl, IconIndex icon, const TString& caption = TString::EMPTY) = 0;

		//////////////////////////////////////////////////////////////////////////
		//all the fields below should be available right after this IUIWindow instance is initialized
		//////////////////////////////////////////////////////////////////////////
		/** @brief  */
		virtual IconIndex		getIcon() const = 0;

		/** @brief used by framework */
		virtual void			setFrameworkData(void* data) = 0;

		/** @brief used by framework */
		virtual void*			getFrameworkData() const = 0;

		/** @brief  */
		virtual void			setUserData(void* data) = 0;

		/** @brief  */
		virtual void*			getUserData() const = 0;

		/** @brief get implementation defined window. this may be unrelated to platform related window system. i.e. windows drawn by render device */
		virtual IWindow*		getWindow() const = 0;

		/** @brief  */
		virtual IUIToolBox*		createToolBox(const TString& name, ICONSIZE is) = 0;

		/** @brief  */
		virtual IUIToolBox*		getToolBox(const TString& name) const = 0;

		/** @brief  */
		virtual bool			setUpdater(IUpdater* updater) = 0;

		/** @brief  */
		virtual IUIWindow::IUpdater* getUpdater() const = 0;

		/** @brief optional: get window keyboard */
		virtual IKeyboard*		getKeyboard() const { return NULL; }

		/** @brief optional: get window mouse */
		virtual IMouse*			getMouse() const { return NULL; }

		/** @brief get the view port camera */
		virtual CameraActor*	getCamera() const { return NULL; }

		/** @brief get the view port view */
		virtual IGraphicsView*	getView() const { return NULL; }

	};//class IUIWindow

	extern template class BLADE_FRAMEWORK_API Factory<IUIWindow>;
	typedef Factory<IUIWindow> UIWindowFactory;
	
}//namespace Blade

#endif//__Blade_IUIWindow_h__