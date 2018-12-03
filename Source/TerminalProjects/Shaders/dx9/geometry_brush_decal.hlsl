//!BladeShaderHeader
//![Shader]
//!Profiles=2_0

#include "inc/material_color.hlsl"

void BladeVSMain(	
	float4 pos	: POSITION,
	uniform float4x4 wvp_matrix : WORLD_VIEWPROJ_MATRIX,
	uniform float4x4 texprojection_matrix : TEX_PROJECTION_VP_MATRIX0,
	out	float4 outPos : POSITION,
	out float2 outUV : TEXCOORD0
	)
{
	outPos = mul(pos, wvp_matrix);
	float4 projectedPos = mul(pos, texprojection_matrix);
	outUV.xy = projectedPos.xy/projectedPos.w;
	outUV.xy = outUV.xy*float2(0.5,0.5) + float2(0.5,0.5);
}

void BladeFSMain(
	in float4 pos : POSITION,
	in float2 uv  : TEXCOORD0,
	uniform sampler2D brushDiffuse,

	out float4 outColor : COLOR0
	)
{
	outColor = float4(diffuse_color.rgb, tex2D(brushDiffuse, uv.xy).r*0.5);
}
