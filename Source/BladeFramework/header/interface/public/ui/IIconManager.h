/********************************************************************
	created:	2010/05/23
	filename: 	IIconManager.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_IIconManager_h__
#define __Blade_IIconManager_h__
#include <interface/InterfaceSingleton.h>
#include <BladeFramework.h>
#include "UITypes.h"

namespace Blade
{

	class IIconManager : public InterfaceSingleton<IIconManager>
	{
	public:
		virtual ~IIconManager()		{}

		/** @brief  */
		virtual void		initialize(const TString& path) = 0;

		/** @brief  */
		virtual const TString& getIconPath() const = 0;

		/** @brief  */
		virtual IconIndex	loadSingleIconImage(const TString& image) = 0;

		/** @brief  */
		virtual bool		loadIconsImage(const TString& image, size_t iconSize, size_t validCount) = 0;

		/** @brief  */
		virtual IconIndex	loadIconForFileType(const TString& extension) = 0;

		/** @brief  */
		virtual size_t		getIconCount() const = 0;

		/** @brief  */
		virtual size_t		getIconSize(ICONSIZE is) const = 0;

	};//class IIconManager

	extern template class BLADE_FRAMEWORK_API Factory<IIconManager>;
	
}//namespace Blade


#endif //__Blade_IIconManager_h__