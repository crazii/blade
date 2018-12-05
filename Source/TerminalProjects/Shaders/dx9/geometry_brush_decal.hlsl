//!BladeShaderHeader
//![Shader]
//!Profiles=2_0

#include "inc/material_color.hlsl"

struct VSOutput
{
	float4 pos : POSITION;
	float2 uv : TEXCOORD0;
};

VSOutput BladeVSMain(
	float4 pos	: POSITION,
	uniform float4x4 wvp_matrix : WORLD_VIEWPROJ_MATRIX,
	uniform float4x4 texprojection_matrix : TEX_PROJECTION_VP_MATRIX0
	)
{
	VSOutput output;
	output.pos = mul(pos, wvp_matrix);
	float4 projectedPos = mul(pos, texprojection_matrix);
	float2 uv = projectedPos.xy/projectedPos.w;
	uv = uv * 0.5 + 0.5;
	output.uv = uv;
	return output;
}

void BladeFSMain(
	in VSOutput input,
	uniform sampler2D brushDiffuse,

	out float4 outColor : COLOR0
	)
{
	outColor = float4(diffuse_color.rgb, tex2D(brushDiffuse, input.uv.xy).r*0.5);
}
