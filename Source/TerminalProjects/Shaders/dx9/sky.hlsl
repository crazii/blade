//!BladeShaderHeader
//![Shader]
//!Profiles=2_0
//![SubShaders]
//!skysphere=[BladeVSMain BladeFSMain]
//!skybox=[BladeBoxVSMain BladeBoxFSMain]

#include "inc/common.hlsl"

struct SkySphereVSOutput
{
	float4 pos : POSITION;
	float2 uv : TEXCOORD1;
};

SkySphereVSOutput BladeVSMain(
	float4 pos	: POSITION
	,float2 uv	: TEXCOORD0
	, uniform float4 eye_position : EYE_POS	//eye pos in world space
	,uniform float4x4 vp_matrix : VIEWPROJ_MATRIX
	)
{
	SkySphereVSOutput output;
	output.pos = mul(pos + float4(eye_position.xyz,0), vp_matrix);
	output.uv = uv;
	return output;
}

void BladeFSMain(
	in SkySphereVSOutput input
	,uniform sampler2D skyDiffuse

	,out float4 outColor : COLOR0
#if BLADE_PROFILE == profile_3_0
	,out float4 outNormal : COLOR1
#endif
	)
{
	outColor = tex2D(skyDiffuse, input.uv.xy);
#if BLADE_PROFILE == profile_3_0
	outNormal = float4(0,0,0,0);
#endif
}


struct SkyBoxVSOutput
{
	float4 pos : POSITION;
	float3 dir : TEXCOORD0;
};

SkyBoxVSOutput BladeBoxVSMain(
	float4 pos	: POSITION
	, uniform float4 eye_position : EYE_POS	//eye pos in world space
	, uniform float4x4 inv_viewporj_matrix : INV_VIEWPROJ_MATRIX
)
{
	SkyBoxVSOutput output;
	output.pos = float4(pos.xy, 0.999, 1);
	float4 unProjectedPos = mul(pos, inv_viewporj_matrix);
	float3 worldPos = unProjectedPos.xyz / unProjectedPos.w;

	output.dir = normalize(worldPos - eye_position.xyz);
	return output;
}

void BladeBoxFSMain(
	SkyBoxVSOutput input
	, uniform samplerCUBE skyDiffuse

	, out float4 outColor : COLOR0
#if BLADE_PROFILE == profile_3_0
	//, out float4 outNormal : COLOR1
#endif
)
{
	outColor = texCUBE(skyDiffuse, input.dir);
#if BLADE_PROFILE == profile_3_0
	//outNormal = float4(0, 0, 0, 0);
#endif
}