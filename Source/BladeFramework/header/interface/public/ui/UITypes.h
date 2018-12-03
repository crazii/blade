/********************************************************************
	created:	2016/6/2
	filename: 	UITypes.h
	author:		Crazii
	purpose:	move editor basic types to UI interface module
*********************************************************************/
#ifndef __Blade_UITypes_h__
#define __Blade_UITypes_h__
#include <BladeTypes.h>
#include <interface/public/input/IKeyboard.h>
#include <utility/String.h>
#include <math/Vector2i.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	typedef uint16 IconIndex;
	static const IconIndex INVALID_ICON_INDEX = (const IconIndex)-1;

	//////////////////////////////////////////////////////////////////////////
	typedef enum EIconSize
	{
		IS_8 = 0,
		IS_16,
		IS_24,
		IS_32,

		IS_COUNT,
	}ICONSIZE;

	//////////////////////////////////////////////////////////////////////////
	typedef struct SWidgetData
	{
		TString		mType;
		TString		mName;	///unique name
		TString		mCaption;
		POINT2I		mPos;
		SIZE2I		mSize;
		uint32		mFlags;
		IconIndex	mIcon;
	}WIDGET_DATA;

	enum EWidgetDockFlag
	{
		WDF_NONE = 0x0000,		///default: dockable
		WDF_MOVABLE = 0x0001,	///can move to another place while docking
		WDF_FLOATABLE = 0x0002,	///can float
		WDF_FLOATING = 0x0004,	///is floating. otherwise is docking
		WDF_MASK = 0x000F,
	};

	enum EWidgetAlignFlag
	{
		WAF_NONE = 0,
		WAF_VTOP = 0x0010,
		WAF_VCENTER = 0x0020,
		WAF_VBOTTOM = 0x0030,
		WAF_HLEFT = WAF_VTOP << 2,
		WAF_HCENTER = WAF_VCENTER << 2,
		WAF_HRIGHT = WAF_VBOTTOM << 2,
		WAF_MASK = 0x00F0,
	};

	enum EWidgetMiscFlag
	{
		WMF_VISIBLE = 0x0100,	///visible
		WMF_FIXED = 0x0200,	///sized by absolute value
		WMF_ACTIVE = 0x0008,	///is active/selected
	};

}//namespace Blade

#endif//__Blade_UITypes_h__