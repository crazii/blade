//!BladeShaderHeader
//![Shader]
//!Profiles=2_0

//gizmo helper shader wich perform ring_circle line clip (back culling)

#include "inc/material_color.hlsl"

void BladeVSMain(	
	float4 pos	: POSITION,
	uniform float4x4 wvp_matrix : WORLD_VIEWPROJ_MATRIX,
	uniform float4x4 wv_matrix : WORLD_VIEW_MATRIX,
	out	float4 outPos : POSITION,
	out float3 outViewNormal : TEXCOORD		//view space normal
	)
{
	outPos = mul(pos,wvp_matrix);
	float3 normal = pos.xyz;
	outViewNormal = normalize( mul(normal, (float3x3)wv_matrix) );
}

float4 BladeFSMain(
	in float4 pos : POSITION,
	in float3 viewNormal : TEXCOORD
	) :COLOR0
{
	clip( viewNormal.z );
	//diffuse_color is a built-in semantic for per-instance diffuse
	return diffuse_color;
}
