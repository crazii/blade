/********************************************************************
	created:	2015/01/01
	filename: 	GLESAttributeNames.h
	author:		Crazii
	
	purpose:	shared attrubte name between rutnime & shader compiler tool
*********************************************************************/
#ifndef __Blade_GLESAttributeNames_h__
#define __Blade_GLESAttributeNames_h__
#include <utility/String.h>
#include <interface/public/graphics/VertexElement.h>

namespace Blade
{
	//[EVertexUsage][index]
	static const String GLES_Attributes[VU_COUNT][8] =
	{
		//VU_POSITION
		BString("blade_position0"), BString("blade_position1"), BString("blade_position2"), BString("blade_position3"), BString("blade_position4"), BString("blade_position5"), BString("blade_position6"), BString("blade_position7"),
		//VU_BLEND_WEIGHTS
		BString("blade_blendwight0"), BString("blade_blendweight1"), BString("blade_blendweight2"), BString("blade_blendweight3"), BString("blade_blendweight4"), BString("blade_blendweight5"), BString("blade_blendweight6"), BString("blade_blendweight7"),
		//VU_BLEND_INDICES
		BString("blade_blendindices0"), BString("blade_blendindices1"), BString("blade_blendindices2"), BString("blade_blendindices3"), BString("blade_blendindices4"), BString("blade_blendindices5"), BString("blade_blendindices6"), BString("blade_blendindices7"),
		//VU_NORMAL
		BString("blade_normal0"), BString("blade_normal1"), BString("blade_normal2"), BString("blade_normal3"), BString("blade_normal4"), BString("blade_normal5"), BString("blade_normal6"), BString("blade_normal7"),
		//VU_COLOR
		BString("blade_color0"), BString("blade_color1"), BString("blade_color2"), BString("blade_color3"), BString("blade_color4"), BString("blade_color5"), BString("blade_color6"), BString("blade_color7"),
		//VU_TEXTURE_COORDINATES
		BString("blade_texcoord0"), BString("blade_texcoord1"), BString("blade_texcoord2"), BString("blade_texcoord3"), BString("blade_texcoord4"), BString("blade_texcoord5"), BString("blade_texcoord6"), BString("blade_texcoord7"),
		//VU_BINORMAL
		BString("blade_binormal0"), BString("blade_binormal1"), BString("blade_binormal2"), BString("blade_binormal3"), BString("blade_binormal4"), BString("blade_binormal5"), BString("blade_binormal6"), BString("blade_binormal7"),
		//VU_TANGENT
		BString("blade_tangent0"), BString("blade_tangent1"), BString("blade_tangent2"), BString("blade_tangent3"), BString("blade_tangent4"), BString("blade_tangent5"), BString("blade_tangent6"), BString("blade_tangent7"),
	};

	//compiling switch  (defined macro)
	static String GLSL_VS_SWITCH = BString("ENABLE_VS");
	static String GLSL_FS_SWITCH = BString("ENABLE_FS");
	
}//namespace Blade


#endif // __Blade_GLESAttributeNames_h__
