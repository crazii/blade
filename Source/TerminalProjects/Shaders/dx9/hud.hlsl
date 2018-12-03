//!BladeShaderHeader
//![Shader]
//!Profiles=2_0

#include "../../../Plugins/SubsystemPlugins/BladeGraphics/header/GraphicsShaderShared.inl"

struct VSOutput
{
	float4 pos : POSITION;
	float2 uv : TEXCOORD0;
	float4 color : TEXCOORD1;
};

VSOutput BladeVSMain(
	float4 pos	: POSITION,
	uniform float4 pixelSize : _INSTANCE_,
	uniform float4 color : _INSTANCE_,
	uniform float4 viewSize : VIEW_SIZE,
	uniform float4 projectedPos : PROJECTED_POS
	)
{
	VSOutput output;
	output.color = color;
	output.uv.xy = (pos.xy + float2(1,1))*0.5;

	output.pos = projectedPos;
	output.pos.xy += (pos.xy*pixelSize.xy*viewSize.zw);

	return output;
}

void BladeFSMain(
	in VSOutput input,
	uniform float4 texutreCount : SAMPLER_COUNT,
	uniform sampler2D hudDiffuse[BLADE_MAX_DYNAMIC_TEXTURE_COUNT],

	out float4 outColor : COLOR0
	)
{
	outColor = float4(0,0,0,0);
#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS	//editor only shader. currently only windows 
	for(int i = 0; i < texutreCount.x; ++i)
	{
		float4 color = tex2D(hudDiffuse[i], input.uv.xy);
		outColor = lerp(outColor, color, color.a);
	}
#endif
	outColor *= input.color;
}
