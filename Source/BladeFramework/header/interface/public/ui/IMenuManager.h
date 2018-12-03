/********************************************************************
	created:	2011/04/11
	filename: 	IMenuManager.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_IMenuManager_h__
#define __Blade_IMenuManager_h__
#include <interface/InterfaceSingleton.h>

namespace Blade
{
	class IMenu;
	class IUIWindow;

	class IMenuManager : public InterfaceSingleton<IMenuManager>
	{
	public:
		virtual ~IMenuManager()		{}

		/** @brief  */
		virtual IMenu*	addRootMenu(const TString& name,tchar accessKey = tchar()) = 0;

		/** @brief  */
		virtual size_t	getRootMenuCount() const = 0;

		/** @brief  */
		virtual IMenu*	getRootMenu(index_t index) const = 0;

		/** @brief  */
		virtual IMenu*	getRootMenu(const TString& name) const = 0;

		/** @brief  */
		virtual IMenu*	findMenu(const TString& name) const = 0;

		/** @brief  */
		virtual IMenu*	createExtraMenu(const TString& name) = 0;

		/** @brief  */
		virtual bool	destroyExtraMenu(IMenu* menu) = 0;

		/** @brief  */
		virtual IMenu*	getExtraMenu(const TString& name) = 0;

		/** @brief popup a menu for a window in window coordinates  */
		virtual bool	popupMenu(IMenu* menu, int x, int y, IUIWindow* window) = 0;
	};
	
	extern template class BLADE_FRAMEWORK_API Factory<IMenuManager>;

}//namespace Blade



#endif // __Blade_IMenuManager_h__