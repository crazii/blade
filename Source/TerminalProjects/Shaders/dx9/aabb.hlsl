//!BladeShaderHeader
//![Shader]
//!Profiles=2_0

struct AABBVSOutput
{
	float4 Pos	: POSITION;
	half4 Color : COLOR0;
};

//vertex shader
AABBVSOutput BladeVSMain(
	float4 pos		: POSITION,
	
	//instance data
	float3 min		: TEXCOORD0,
	float3 max		: TEXCOORD1,
	half4 color		: COLOR0,
	
	//engine senmatic auto param
	uniform float4x4 vp_matrix : VIEWPROJ_MATRIX
	)
{
	float3 worldPos = (min+max)*float3(0.5,0.5,0.5);
	float3 worldSize = (max-min);

	float4 p;
	p.xyz = pos.xyz*worldSize + worldPos;
	p.w = 1;

	AABBVSOutput o;
	o.Pos = mul(p, vp_matrix);
	o.Color = color;
	return o;
}

//fragment shader
void BladeFSMain(
	in AABBVSOutput vsin,
	out half4 outColor : COLOR0
	)
{
	outColor = half4(vsin.Color.rgb,1);
}
