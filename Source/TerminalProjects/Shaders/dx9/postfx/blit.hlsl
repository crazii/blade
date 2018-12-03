//!BladeShaderHeader
//![Shader]
//!Profiles=2_0,2_a,3_0
//![SubShaders]
//!color
//!depth = [BladeVSMain BladeBlitDepthFSMain]
//!depth_linearize = [BladeVSMain BladeBlitDepthFSMain],LINEARIZE_DEPTH
//!depth2color_linearize = [BladeVSMain BladeBlitDepthFSMain],LINEARIZE_DEPTH,COLOR_OUTPUT

#include "../inc/common.hlsl"

struct BladeVSOutput
{
	float4	pos : POSITION;
	float2	uv	: TEXCOORD0;
#if defined(LINEARIZE_DEPTH)
	float4	cpos : TEXCOORD1;
#endif
};

BladeVSOutput BladeVSMain(
	float4 pos		: POSITION
	,uniform float4 inputUVSpace : RENDERBUFFER_UVSPACE0
)
{
	BladeVSOutput output;
	pos.zw = float2(0.5, 1);
	output.pos = pos;
	output.uv = Blade_TransformUVSpace(Blade_Clip2UV(pos.xy), inputUVSpace);
#if defined(LINEARIZE_DEPTH)
	output.cpos = pos;
#endif
	return output;
}

float4 BladeFSMain(
	BladeVSOutput input
	, uniform sampler2D inputBuffer : RENDERBUFFER_INPUT0
	
) : COLOR0
{
	return tex2D(inputBuffer, input.uv);
}

void BladeBlitDepthFSMain(
	BladeVSOutput input
	,uniform sampler2D inputBuffer : RENDERBUFFER_INPUT0
#if defined(LINEARIZE_DEPTH)
	,uniform float4x4 inv_proj_matrix : INV_PROJ_MATRIX
#endif
	,out float4 color : COLOR0
#if !defined(COLOR_OUTPUT)
	,out float depth : DEPTH
#endif
)
{
	float zBuffer = tex2D(inputBuffer, input.uv).r;

#	if defined(LINEARIZE_DEPTH)
	float4 cpos = float4(input.cpos.xy, Blade_ZBuffer2NDC(zBuffer), 1);
	float4 vpos = mul(cpos, inv_proj_matrix);
	vpos /= vpos.w;
	zBuffer = abs(vpos.z);
#	endif

#if !defined(COLOR_OUTPUT)
	depth = zBuffer;
	color = 0;
#else
	color = zBuffer;
#endif
}
