/********************************************************************
	created:	2010/04/22
	filename: 	VertexElement.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <interface/public/graphics/VertexElement.h>

#include <interface/public/graphics/IGraphicsResourceManager.h>
#include <interface/public/graphics/PixelColorFormatter.h>

namespace Blade
{

	/************************************************************************/
	/* VertexElement                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	static uint16 lTypeSizeMap[VET_COUNT] = 
	{
		sizeof(float),
		sizeof(float)*2,
		sizeof(float)*3,
		sizeof(float)*4,
		sizeof(Color::COLOR),
		sizeof(short)*2,
		sizeof(short)*4,
		sizeof(unsigned char)*4,
		sizeof(unsigned char)*4,
		sizeof(short)*2,
		sizeof(short)*4,
		sizeof(unsigned short)*2,
		sizeof(unsigned short)*4,
		sizeof(int32),
		sizeof(int32),
		sizeof(float)/2*2,
		sizeof(float)/2*4,
	};
	uint16				VertexElement::getSize(EVertexElementType type)
	{
		assert( type >= VET_BEGIN && type < VET_COUNT );
		return lTypeSizeMap[type];
	}

	//////////////////////////////////////////////////////////////////////////
	Color::COLOR			VertexElement::getColorValue(const Color& color,const EVertexElementType color_type)
	{
		assert( color_type == VET_COLOR );
		BLADE_UNREFERENCED(color_type);
		RGBAConverter converter = IGraphicsResourceManager::getSingleton().getGraphicsConfig().DesiredOrder;
		return converter.packColor(color);
	}

}//namespace Blade