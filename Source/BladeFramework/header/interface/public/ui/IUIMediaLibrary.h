/********************************************************************
	created:	2016/06/09
	filename: 	IUIMediaLibrary.h
	author:		Crazii
	
	purpose:	public media library ui interface, for user interface only
				media library ui should be independent, unrelated to any other ui
*********************************************************************/
#ifndef __Blade_IUIMediaLibrary_h__
#define __Blade_IUIMediaLibrary_h__
#include <interface/InterfaceSingleton.h>
#include <interface/IMediaLibrary.h>

namespace Blade
{
	class IUIMediaLibrary : public InterfaceSingleton<IUIMediaLibrary>
	{
	public:

		/** @brief  */
		virtual bool			initialize() = 0;

		/** @brief  */
		virtual bool			showMediaLibrary(const TString& type, const Blade::TString& curFile,
			IMediaLibrary::ISelectionReceiver& receiver,
			bool bShowDefaultOnly = false) = 0;

		/** @brief  */
		virtual bool			shutdown() = 0;
	};
	
}//namespace Blade


#endif // __Blade_IUIMediaLibrary_h__