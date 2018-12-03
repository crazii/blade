/********************************************************************
	created:	2010/04/21
	filename: 	Color.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <interface/public/graphics/Color.h>
#include "ColorChannel.h"

namespace Blade
{
	static_assert(sizeof(Blade::Color::RGBA) == 4, "size/algiment error");
	static_assert(sizeof(Blade::Color::RGB) == 3, "size/algiment error");

	static const fp32 lBaseConversion = 1.0f/255.0f;

	const Color Color::WHITE	= Color(1.0f,1.0f,1.0f);
	const Color Color::BLACK	= Color(0.0f,0.0f,0.0f);
	const Color Color::GRAY		= Color(0.5f,0.5f,0.5f);
	const Color Color::RED		= Color(1.0f,0.0f,0.0f);
	const Color Color::GREEN	= Color(0.0f,1.0f,0.0f);
	const Color Color::BLUE		= Color(0.0f,0.0f,1.0f);
	const Color Color::BLACK_ALPHA	= Color(0.0f,0.0f,0.0f,1.0f);
	
	//////////////////////////////////////////////////////////////////////////
	void				Color::saturate()
	{
		if( r < 0.0f )
			r = 0.0f;
		else if( r > 1.0f )
			r = 1.0f;

		if( g < 0.0f )
			g = 0.0f;
		else if( g > 1.0f )
			g = 1.0f;

		if( b < 0.0f )
			b = 0.0f;
		else if( b > 1.0f )
			b = 1.0f;

		if( a < 0.0f )
			a = 0.0f;
		else if( a > 1.0f )
			a = 1.0f;
	}

	//////////////////////////////////////////////////////////////////////////
	Color				Color::getSaturateColor() const
	{
		Color ret = *this;
		ret.saturate();
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	Color::COLOR		Color::getRGBA() const
	{
		COLOR value;
		// Red
		value.component[0] = static_cast<uint8>(r * 255);
		// Green
		value.component[1] = static_cast<uint8>(g * 255);
		// Blue
		value.component[2] = static_cast<uint8>(b * 255);
		// Alpha
		value.component[3] = static_cast<uint8>(a * 255);
		return value;
	}

	//////////////////////////////////////////////////////////////////////////
	void				Color::setRGBA(COLOR color)
	{
		COLOR value;
		value.color = color;
		// Red
		r = value.component[0]*lBaseConversion;
		// Green
		g = value.component[1]*lBaseConversion;
		// Blue
		b = value.component[2]*lBaseConversion;
		// Alpha
		a = value.component[3]*lBaseConversion;
	}

	//////////////////////////////////////////////////////////////////////////
	Color::COLOR			Color::getARGB() const
	{
		COLOR value;
		// Alpha
		value.component[0] = static_cast<uint8>(a * 255);
		// Red
		value.component[1] = static_cast<uint8>(r * 255);
		// Green
		value.component[2] = static_cast<uint8>(g * 255);
		// Blue
		value.component[3] = static_cast<uint8>(b * 255);

		return value;
	}

	//////////////////////////////////////////////////////////////////////////
	void				Color::setARGB(COLOR color)
	{
		COLOR value;
		value.color = color;
		// Alpha
		a = value.component[0]*lBaseConversion;
		// Red
		r = value.component[1]*lBaseConversion;
		// Green
		g = value.component[2]*lBaseConversion;
		// Blue
		b = value.component[3]*lBaseConversion;
	}

	//////////////////////////////////////////////////////////////////////////
	Color::COLOR			Color::getBGRA() const
	{
		COLOR value;
		// Blue
		value.component[0] = static_cast<uint8>(b * 255);
		// Green
		value.component[1] = static_cast<uint8>(g * 255);
		// Red
		value.component[2] = static_cast<uint8>(r * 255);
		// Alpha
		value.component[3] = static_cast<uint8>(a * 255);
		return value.color;
	}

	//////////////////////////////////////////////////////////////////////////
	void				Color::setBGRA(COLOR color)
	{
		COLOR value;
		value.color = color;
		// Blue
		b = value.component[0]*lBaseConversion;
		// Green
		g = value.component[1]*lBaseConversion;
		// Red
		r = value.component[2]*lBaseConversion;
		// Alpha
		a = value.component[3]*lBaseConversion;
	}

	//////////////////////////////////////////////////////////////////////////
	Color::COLOR			Color::getABGR() const
	{
		COLOR value;
		// Alpha
		value.component[0] = static_cast<uint8>(a * 255);
		// Blue
		value.component[1] = static_cast<uint8>(b * 255);
		// Green
		value.component[2] = static_cast<uint8>(g * 255);
		// Red
		value.component[3] = static_cast<uint8>(r * 255);
		return value.color;
	}

	//////////////////////////////////////////////////////////////////////////
	void				Color::setABGR(COLOR color)
	{
		COLOR value;
		value.color = color;
		// Alpha
		a = value.component[0]*lBaseConversion;
		// Blue
		b = value.component[1]*lBaseConversion;
		// Green
		g = value.component[2]*lBaseConversion;
		// Red
		r = value.component[3]*lBaseConversion;
	}


	//////////////////////////////////////////////////////////////////////////
	Color::COLOR	Color::ARGB2RGBA(Color::COLOR argb)
	{
		COLOR ucolor;
		COLOR input;
		input.color = argb;
		ucolor.component[RGBA_A] = input.component[ARGB_A];
		ucolor.component[RGBA_R] = input.component[ARGB_R];
		ucolor.component[RGBA_G] = input.component[ARGB_G];
		ucolor.component[RGBA_B] = input.component[ARGB_B];
		return ucolor.color;
	}
	//////////////////////////////////////////////////////////////////////////
	Color::COLOR	Color::ABGR2RGBA(Color::COLOR abgr)
	{
		COLOR ucolor;
		COLOR input;
		input.color = abgr;
		ucolor.component[RGBA_A] = input.component[ABGR_A];
		ucolor.component[RGBA_B] = input.component[ABGR_B];
		ucolor.component[RGBA_G] = input.component[ABGR_G];
		ucolor.component[RGBA_R] = input.component[ABGR_R];
		return ucolor.color;
	}
	//////////////////////////////////////////////////////////////////////////
	Color::COLOR	Color::BGRA2RGBA(Color::COLOR bgra)
	{
		COLOR ucolor;
		COLOR input;
		input.color = bgra;
		ucolor.component[RGBA_B] = input.component[BGRA_B];
		ucolor.component[RGBA_G] = input.component[BGRA_G];
		ucolor.component[RGBA_R] = input.component[BGRA_R];
		ucolor.component[RGBA_A] = input.component[BGRA_A];
		return ucolor.color;
	}

	//////////////////////////////////////////////////////////////////////////
	Color::COLOR	Color::RGBA2ARGB(Color::COLOR rgba)
	{
		COLOR ucolor;
		COLOR input;
		input.color = rgba;
		ucolor.component[ARGB_R] = input.component[RGBA_R];
		ucolor.component[ARGB_G] = input.component[RGBA_G];
		ucolor.component[ARGB_B] = input.component[RGBA_B];
		ucolor.component[ARGB_A] = input.component[RGBA_A];
		return ucolor.color;
	}
	//////////////////////////////////////////////////////////////////////////
	Color::COLOR	Color::ABGR2ARGB(Color::COLOR abgr)
	{
		COLOR ucolor;
		COLOR input;
		input.color = abgr;
		ucolor.component[ARGB_A] = input.component[ABGR_A];
		ucolor.component[ARGB_B] = input.component[ABGR_B];
		ucolor.component[ARGB_G] = input.component[ABGR_G];
		ucolor.component[ARGB_R] = input.component[ABGR_R];
		return ucolor.color;
	}
	//////////////////////////////////////////////////////////////////////////
	Color::COLOR	Color::BGRA2ARGB(Color::COLOR bgra)
	{
		COLOR ucolor;
		COLOR input;
		input.color = bgra;
		ucolor.component[ARGB_B] = input.component[BGRA_B];
		ucolor.component[ARGB_G] = input.component[BGRA_G];
		ucolor.component[ARGB_R] = input.component[BGRA_R];
		ucolor.component[ARGB_A] = input.component[BGRA_A];
		return ucolor.color;
	}
	//////////////////////////////////////////////////////////////////////////
	Color::COLOR	Color::RGBA2ABGR(Color::COLOR rgba)
	{
		COLOR ucolor;
		COLOR input;
		input.color = rgba;
		ucolor.component[ABGR_R] = input.component[RGBA_R];
		ucolor.component[ABGR_G] = input.component[RGBA_G];
		ucolor.component[ABGR_B] = input.component[RGBA_B];
		ucolor.component[ABGR_A] = input.component[RGBA_A];
		return ucolor.color;
	}
	//////////////////////////////////////////////////////////////////////////
	Color::COLOR	Color::ARGB2ABGR(Color::COLOR argb)
	{
		COLOR ucolor;
		COLOR input;
		input.color = argb;
		ucolor.component[ABGR_A] = input.component[ARGB_A];
		ucolor.component[ABGR_R] = input.component[ARGB_R];
		ucolor.component[ABGR_G] = input.component[ARGB_G];
		ucolor.component[ABGR_B] = input.component[ARGB_B];
		return ucolor.color;
	}
	//////////////////////////////////////////////////////////////////////////
	Color::COLOR	Color::BGRA2ABGR(Color::COLOR bgra)
	{
		COLOR ucolor;
		COLOR input;
		input.color = bgra;
		ucolor.component[ABGR_B] = input.component[BGRA_B];
		ucolor.component[ABGR_G] = input.component[BGRA_G];
		ucolor.component[ABGR_R] = input.component[BGRA_R];
		ucolor.component[ABGR_A] = input.component[BGRA_A];
		return ucolor.color;
	}
	//////////////////////////////////////////////////////////////////////////
	Color::COLOR	Color::RGBA2BGRA(Color::COLOR rgba)
	{
		COLOR ucolor;
		COLOR input;
		input.color = rgba;
		ucolor.component[BGRA_R] = input.component[RGBA_R];
		ucolor.component[BGRA_G] = input.component[RGBA_G];
		ucolor.component[BGRA_B] = input.component[RGBA_B];
		ucolor.component[BGRA_A] = input.component[RGBA_A];
		return ucolor.color;
	}
	//////////////////////////////////////////////////////////////////////////
	Color::COLOR	Color::ARGB2BGRA(Color::COLOR argb)
	{
		COLOR ucolor;
		COLOR input;
		input.color = argb;
		ucolor.component[BGRA_A] = input.component[ARGB_A];
		ucolor.component[BGRA_R] = input.component[ARGB_R];
		ucolor.component[BGRA_G] = input.component[ARGB_G];
		ucolor.component[BGRA_B] = input.component[ARGB_B];
		return ucolor.color;
	}
	//////////////////////////////////////////////////////////////////////////
	Color::COLOR	Color::ABGR2BGRA(Color::COLOR abgr)
	{
		COLOR ucolor;
		COLOR input;
		input.color = abgr;
		ucolor.component[BGRA_A] = input.component[ABGR_A];
		ucolor.component[BGRA_B] = input.component[ABGR_B];
		ucolor.component[BGRA_G] = input.component[ABGR_G];
		ucolor.component[BGRA_R] = input.component[ABGR_R];
		return ucolor.color;
	}


}//namespace Blade