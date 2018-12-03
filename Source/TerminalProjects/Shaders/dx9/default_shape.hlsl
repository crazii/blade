//!BladeShaderHeader
//![Shader]
//!Profiles=2_0,3_0

#include "inc/material_color.hlsl"

void BladeVSMain(	
	float4 pos	: POSITION,
	uniform float4x4 wvp_matrix : WORLD_VIEWPROJ_MATRIX,
	out	float4 outPos : POSITION,
	out float outDepth : TEXCOORD
	)
{
	outPos = mul(pos,wvp_matrix);
}

void BladeFSMain(
	in float4 pos : POSITION,

	out float4 outColor : COLOR
	)
{
	outColor = diffuse_color;
}
