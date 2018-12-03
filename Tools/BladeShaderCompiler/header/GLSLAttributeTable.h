/********************************************************************
	created:	2015/01/01
	filename: 	GLSLAttributeTable.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_GLSLAttributeTable_h__
#define __Blade_GLSLAttributeTable_h__
#include <graphics/GLES/public/GLESAttributeNames.h>

namespace Blade
{
	//semantics that need to renamed
	static EAttribSemantic semantics[] = 
	{
		EAttrSemPosition,
		EAttrSemPosition1,
		EAttrSemPosition2,
		EAttrSemPosition3,
		EAttrSemNormal,
		EAttrSemNormal1,
		EAttrSemNormal2,
		EAttrSemNormal3,
		EAttrSemColor0,
		EAttrSemColor1,
		EAttrSemColor2,
		EAttrSemColor3,
		EAttrSemTex0,
		EAttrSemTex1,
		EAttrSemTex2,
		EAttrSemTex3,
		EAttrSemTex4,
		EAttrSemTex5,
		EAttrSemTex6,
		EAttrSemTex7,
		EAttrSemTangent,
		EAttrSemTangent1,
		EAttrSemTangent2,
		EAttrSemTangent3,
		EAttrSemBinormal,
		EAttrSemBinormal1,
		EAttrSemBinormal2,
		EAttrSemBinormal3,
		EAttrSemBlendWeight,
		EAttrSemBlendWeight1,
		EAttrSemBlendWeight2,
		EAttrSemBlendWeight3,
		EAttrSemBlendIndices,
		EAttrSemBlendIndices1,
		EAttrSemBlendIndices2,
		EAttrSemBlendIndices3,
	};

	//names to be changed to
	static const char* variableNames[] = 
	{
		GLES_Attributes[VU_POSITION][0].c_str(),
		GLES_Attributes[VU_POSITION][1].c_str(),
		GLES_Attributes[VU_POSITION][2].c_str(),
		GLES_Attributes[VU_POSITION][3].c_str(),

		GLES_Attributes[VU_NORMAL][0].c_str(),
		GLES_Attributes[VU_NORMAL][1].c_str(),
		GLES_Attributes[VU_NORMAL][2].c_str(),
		GLES_Attributes[VU_NORMAL][3].c_str(),

		GLES_Attributes[VU_COLOR][0].c_str(),
		GLES_Attributes[VU_COLOR][1].c_str(),
		GLES_Attributes[VU_COLOR][2].c_str(),
		GLES_Attributes[VU_COLOR][3].c_str(),

		GLES_Attributes[VU_TEXTURE_COORDINATES][0].c_str(),
		GLES_Attributes[VU_TEXTURE_COORDINATES][1].c_str(),
		GLES_Attributes[VU_TEXTURE_COORDINATES][2].c_str(),
		GLES_Attributes[VU_TEXTURE_COORDINATES][3].c_str(),
		GLES_Attributes[VU_TEXTURE_COORDINATES][4].c_str(),
		GLES_Attributes[VU_TEXTURE_COORDINATES][5].c_str(),
		GLES_Attributes[VU_TEXTURE_COORDINATES][6].c_str(),
		GLES_Attributes[VU_TEXTURE_COORDINATES][7].c_str(),

		GLES_Attributes[VU_TANGENT][0].c_str(),
		GLES_Attributes[VU_TANGENT][1].c_str(),
		GLES_Attributes[VU_TANGENT][2].c_str(),
		GLES_Attributes[VU_TANGENT][3].c_str(),

		GLES_Attributes[VU_BINORMAL][0].c_str(),
		GLES_Attributes[VU_BINORMAL][1].c_str(),
		GLES_Attributes[VU_BINORMAL][2].c_str(),
		GLES_Attributes[VU_BINORMAL][3].c_str(),

		GLES_Attributes[VU_BLEND_WEIGHTS][0].c_str(),
		GLES_Attributes[VU_BLEND_WEIGHTS][1].c_str(),
		GLES_Attributes[VU_BLEND_WEIGHTS][2].c_str(),
		GLES_Attributes[VU_BLEND_WEIGHTS][3].c_str(),

		GLES_Attributes[VU_BLEND_INDICES][0].c_str(),
		GLES_Attributes[VU_BLEND_INDICES][1].c_str(),
		GLES_Attributes[VU_BLEND_INDICES][2].c_str(),
		GLES_Attributes[VU_BLEND_INDICES][3].c_str(),
	};
	static_assert( _countof(semantics) == _countof(variableNames), "array mismatch");

}//namespace Blade


#endif // __Blade_GLSLAttributeTable_h__