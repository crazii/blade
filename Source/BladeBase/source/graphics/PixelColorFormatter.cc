/********************************************************************
	created:	2014/05/12
	filename: 	PixelColorFormatter.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <interface/public/graphics/PixelColorFormatter.h>
#include "ColorChannel.h"

namespace Blade
{

	//////////////////////////////////////////////////////////////////////////
	static Color::COLOR noConvert(Color::COLOR c)
	{
		return c;
	}

	//////////////////////////////////////////////////////////////////////////
	RGBAConverter::FnConvertRGBA RGBAConverter::msFunList[PO_COUNT][PO_COUNT][2] = 
	{
		//
		{
			//from				to
			{ noConvert,			noConvert, },			//NONE
			{ noConvert,			noConvert, },			//RGBA
			{ noConvert,			noConvert, },			//ARGB
			{ noConvert,			noConvert, },			//ABGR
			{ noConvert,			noConvert, },			//BGRA
		},

		//RGBA
		{
			//from				to
			{ noConvert,			noConvert, },			//NONE
			{ noConvert,			noConvert, },			//RGBA
			{ Color::ARGB2RGBA,	Color::RGBA2ARGB, },	//ARGB
			{ Color::ABGR2RGBA,	Color::RGBA2ABGR, },	//ABGR
			{ Color::BGRA2RGBA,	Color::RGBA2BGRA, },	//BGRA
		},

		//BGRA
		{
			{ noConvert,			noConvert, },			//NONE
			{ Color::RGBA2BGRA,	Color::BGRA2RGBA, },	//RGBA
			{ Color::ARGB2BGRA,	Color::BGRA2ARGB, },	//ARGB
			{ Color::ABGR2BGRA,	Color::BGRA2ABGR, },	//ABGR
			{ noConvert,			noConvert, },			//BGRA
		},

		//ARGB
		{
			{ noConvert,			noConvert, },			//NONE
			{ Color::RGBA2ARGB,	Color::ARGB2RGBA, },	//RGBA
			{ noConvert,			noConvert, },			//ARGB
			{ Color::ABGR2ARGB,	Color::ARGB2ABGR, },	//ABGR
			{ Color::BGRA2ARGB,	Color::ARGB2BGRA, },	//BGRA
		},

		//ABGR
		{
			{ noConvert,			noConvert, },			//NONE
			{ Color::RGBA2ABGR,	Color::ABGR2RGBA, },	//RGBA
			{ Color::ARGB2ABGR,	Color::ABGR2ARGB, },	//ARGB
			{ noConvert,			noConvert, },			//ABGR
			{ Color::BGRA2ABGR,	Color::ABGR2BGRA, },	//BGRA
		},
	};

	const int RGBAConverter::msIndexList[PO_COUNT][4] =
	{
		{ 0,		0,		0,		0, },
		{ RGBA_R,	RGBA_G,	RGBA_B,	RGBA_A, },//RGBA
		{ BGRA_R,	BGRA_G,	BGRA_B,	BGRA_A, },//BGRA
		{ ARGB_R,	ARGB_G,	ARGB_B,	ARGB_A, },//ARGB
		{ ABGR_R,	ABGR_G,	ABGR_B,	ABGR_A, },//ABGR
	};

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/

	
}//namespace Blade