/********************************************************************
	created:	2013/04/28
	filename: 	UIViewportCommand.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_UIViewportCommand_h__
#define __Blade_UIViewportCommand_h__
#include <interface/public/ui/IUICommand.h>
#include "ViewportOption.h"

namespace Blade
{
	class UIViewportCommand : public IUICommand , public Singleton<UIViewportCommand>
	{
	public:
		/** @brief  */
		virtual void execute(ICommandUI* /*cui*/)
		{
			CUITabDialog dialog;
			dialog.setCaption( BTLang(BLANG_VIEWPORT) + BTString(" ") + BTLang(BLANG_OPTIONS) );
			dialog.addTab( BLADE_NEW LayoutPage() );
			dialog.DoModal();
		}
	};
	
}//namespace Blade

#endif//__Blade_UIViewportCommand_h__