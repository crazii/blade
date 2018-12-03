/********************************************************************
	created:	2016/06/10
	filename: 	UITooltip.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_UITooltip_h__
#define __Blade_UITooltip_h__
#include <Singleton.h>
#include <afxcmn.h>

class CUIToolTip : public MFCToolTip, public Blade::Singleton<CUIToolTip>
{
public:
	using Blade::Singleton<CUIToolTip>::operator new;
	using Blade::Singleton<CUIToolTip>::operator delete;
	using Blade::Singleton<CUIToolTip>::operator new[];
	using Blade::Singleton<CUIToolTip>::operator delete[];
};

#endif // __Blade_UITooltip_h__