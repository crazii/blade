//!BladeShaderHeader
//![Shader]
//!Profiles=2_0,2_a,3_0
#include "../inc/common.hlsl"

struct MotionVSOutput
{
	float4	pos : POSITION;
	float2	uv	: TEXCOORD0;
	float4  cpos : TEXCOORD1;
};

float4x4 ReprojectionMatrix;

MotionVSOutput BladeVSMain(
	float4 pos		: POSITION
)
{
	MotionVSOutput output;
	pos.zw = float2(0.5, 1);
	output.pos = pos;
	output.cpos = pos;
	output.uv = Blade_Clip2UV(pos).xy;
	return output;
}

float4 BladeFSMain(
	MotionVSOutput input
	, uniform float4x4 inv_viewporj_matrix : INV_VIEWPROJ_MATRIX
	, uniform sampler2D depthBuffer : RENDERBUFFER_INPUT0
	, uniform float4 depthUVSpace : RENDERBUFFER_UVSPACE0
) : COLOR0
{
	float2 depthUV = Blade_TransformUVSpace(input.uv, depthUVSpace);
	float z = tex2D(depthBuffer, depthUV).r;
	float4 cpos = float4(input.cpos.xy, Blade_ZBuffer2NDC(z), 1);

	float4 prevpos = mul(cpos, ReprojectionMatrix);
	prevpos /= prevpos.w;
	return half4(cpos.xy - prevpos.xy, 0, 0);
}