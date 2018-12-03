//!BladeShaderHeader
//![Shader]
//!Profiles=3_0
//![SubShaders]
//!final = ENABLE_BLOOM
//!final_nobloom
//!downscale2luminance = [BladeVSMain BladeFSDownscaleLum]
//!downscale = [BladeVSMain BladeFSDownscale]
//!adaptluminance = [BladeVSMainAdaptLuminance BladeFSAdaptLuminance]
//!bright = [BladeVSMain BladeFSBright]
//!bloom = [BladeVSBloom BladeFSBloom]

#include "../inc/common.hlsl"

static const float4 LUMINENCE_FACTOR  = float4(0.27f, 0.67f, 0.06f, 0.0f);
static const float FUDGE = 0.000001f;

//TODO: set to uniform to tune result
static const float MIDDLE_GREY = 0.75f;
static const float L_WHITE = 1.5f;

uniform float4 brightLimiter = float4(0.6f, 0.6f, 0.6f, 0.0f);
uniform float exposureScale = 1.0f;

uniform sampler2D inputBuffer : RENDERBUFFER_INPUT0;
uniform float4 inputUVSpace : RENDERBUFFER_UVSPACE0;
uniform float4 inputBufferSize : RENDERBUFFER_SIZE0;

uniform sampler2D luminenceBuffer : RENDERBUFFER_INPUT1;
uniform float4 luminanceUVSpace : RENDERBUFFER_UVSPACE1;
uniform float4 luminanceBufferSize : RENDERBUFFER_SIZE1;

uniform sampler2D bloomBuffer : RENDERBUFFER_INPUT2;
uniform float4 bloomUVSpace[3] : RENDERBUFFER_UVSPACE2;
uniform float4 bloomBufferSize : RENDERBUFFER_SIZE2;

/** Tone mapping function 
@note Only affects rgb, not alpha
@param inColour The HDR colour
@param lum The scene lumninence 
@returns Tone mapped colour
*/
float4 toneMap(float4 inColour, float lum)
{
#if 0	//Reinhard

	// From Reinhard et al
	// "Photographic Tone Reproduction for Digital Images"
	
	// Initial luminence scaling (equation 2)
    inColour.rgb *= MIDDLE_GREY / (FUDGE + lum);

	// Control white out (equation 4 nom)
    inColour.rgb *= (float3(1,1,1) + inColour.rgb / L_WHITE);

	// Final mapping (equation 4 denom)
	inColour.rgb /= (float3(1, 1, 1) + inColour.rgb);

	return inColour;
#else
	//ACES (Academy Color Encoding System)
	//https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
	const float A = 2.51f;
	const float B = 0.03f;
	const float C = 2.43f;
	const float D = 0.59f;
	const float E = 0.14f;

	lum *= (1-exposureScale);

	lum = 2 / (max(0.1f, 1 + 10 * lerp(0.2f, lum, 0.5f)));
	inColour *= lum;
	return (inColour * (A * inColour + B)) / (inColour * (C * inColour + D) + E);
#endif
}


struct BladeVSOutput
{
	float4	pos : POSITION;
	float2	uv	: TEXCOORD0;
	float2	uv2	: TEXCOORD1;
};

BladeVSOutput BladeVSMain(
	float4 pos		: POSITION
)
{
	BladeVSOutput output;
	pos.zw = float2(0.5, 1);
	output.pos = pos;
	float2 uv = Blade_Clip2UV(pos).xy;

	output.uv = Blade_TransformUVSpace(uv, inputUVSpace);
	//clamp sub view/texture to prevent linear bleeding
	output.uv2 = Blade_TransformUVSpace(uv, bloomUVSpace);
	return output;
}


//Downsample to half size using bilinear sampling, and convert to greyscale
float4 BladeFSDownscaleLum(BladeVSOutput input) : COLOR
{
    float4 color = tex2D(inputBuffer, input.uv);

	// Adjust the accumulated amount by lum factor
	// Cannot use float3's here because it generates dependent texture errors because of swizzle
	float lum = dot(color, LUMINENCE_FACTOR);
	// take average of 9 samples
	return lum;
}

//Downsample to half size using bilinear sampling
float4 BladeFSDownscale(BladeVSOutput input) : COLOR
{
	return tex2D(inputBuffer, input.uv);
}


BladeVSOutput BladeVSMainAdaptLuminance(
	float4 pos		: POSITION
	,uniform float4 curLumUVSpace[2] : RENDERBUFFER_UVSPACE1
)
{
	BladeVSOutput output;
	pos.zw = float2(0.5, 1);
	output.pos = pos;

	//note: current lum buffer may not be 2x2, it may be larger
	//need recaluate uv using abs uv space
	output.uv = Blade_TransformUVSpace(float2(0.5, 0.5), curLumUVSpace[1]);
	output.uv2 = output.uv;
	return output;
}

//Adapt luminance to prevent flickering
float4 BladeFSAdaptLuminance(BladeVSOutput input
	,uniform sampler2D lastLumBuffer : RENDERBUFFER_INPUT0
	,uniform sampler2D curLumBuffer : RENDERBUFFER_INPUT1
	,uniform float4 time : TIME
	) : COLOR
{
	float delta = time.y;
	float lastLum = tex2D(lastLumBuffer, float2(0.5f, 0.5f)).r;
    float currentLum = tex2D(curLumBuffer, input.uv).r;
    
    // Adapt the luminance using Pattanaik's technique
	// http://www.coretechniques.info/PostFX.zip
    const float fTau = 0.5f;
    float adaptedLum = lastLum + (currentLum - lastLum) * (1 - exp(-delta * fTau));

    return adaptedLum;
}

//Downsample and perform a brightness pass
float4 BladeFSBright(BladeVSOutput input) : COLOR
{
    float4 color = tex2D(inputBuffer, input.uv);
    
	// Get average luminence
	float4 lum = tex2D(luminenceBuffer, float2(0.5f, 0.5f));

#if 1
    // Reduce bright and clamp
	color = max(float4(0.0f, 0.0f, 0.0f, 1.0f), color - brightLimiter);
		
	// Tone map result
	return toneMap(color, lum.r);
#else
	//color = toneMap(color, lum.r);

	const float minLum = brightLimiter.x;
	float clum = dot(color, LUMINENCE_FACTOR);
	color = step(minLum, clum) * max(float4(0.0f, 0.0f, 0.0f, 1.0f), color - brightLimiter);
	return toneMap(color, lum.r);
	//return color;
#endif
}

struct BloomVSOutput
{
	float4	pos : POSITION;
	float2	uv	: TEXCOORD0;
	float4	uv1	: TEXCOORD1;
	float4	uv2	: TEXCOORD2;
	float4	uv3	: TEXCOORD3;
	float4	uv4	: TEXCOORD4;
	float4	uv5	: TEXCOORD5;
	float4	uv6	: TEXCOORD6;
	float4	uv7	: TEXCOORD7;
};

BloomVSOutput BladeVSBloom(
	float4 pos : POSITION
	,uniform float2 sampleDir
	,uniform float2 sampleOffsets[14]
	,uniform float4 uvSpace[3] : RENDERBUFFER_UVSPACE0	//override bloomUVSpace since only one input, use first set
	)
{
	BloomVSOutput output;
	pos.zw = float2(0.5, 1);
	output.pos = pos;
	float2 uv = Blade_Clip2UV(pos).xy;
	uv = Blade_TransformUVSpace(uv, uvSpace[0]);

	float2 min = uvSpace[2].xy;
	float2 max = uvSpace[2].zw;

	float2 offsets[14];
    for( int i = 0; i < 14; i++ )
	{
        offsets[i] = uv + sampleOffsets[i] * sampleDir * inputBufferSize.zw;
		//rendering to sub view and sampling it back, need to clamp to sub view uv space
		offsets[i] = clamp(offsets[i], min, max);
	}
	output.uv = uv;
	
	output.uv1.xy = offsets[0];
	output.uv1.zw = offsets[1];
	output.uv2.xy = offsets[2];
	output.uv2.zw = offsets[3];
	output.uv3.xy = offsets[4];
	output.uv3.zw = offsets[5];
	output.uv4.xy = offsets[6];
	output.uv4.zw = offsets[7];

	output.uv5.xy = offsets[8];
	output.uv5.zw = offsets[9];
	output.uv6.xy = offsets[10];
	output.uv6.zw = offsets[11];
	output.uv7.xy = offsets[12];
	output.uv7.zw = offsets[13];
	return output;
}

//Gaussian bloom pass
float4 BladeFSBloom(BloomVSOutput input
					,uniform float4 sampleWeights[15]
					) : COLOR
{
	float4 accum = tex2D(inputBuffer, input.uv) * sampleWeights[14];

	accum += sampleWeights[0] * tex2D(inputBuffer, input.uv1.xy);
	accum += sampleWeights[1] * tex2D(inputBuffer, input.uv1.zw);
	accum += sampleWeights[2] * tex2D(inputBuffer, input.uv2.xy);
	accum += sampleWeights[3] * tex2D(inputBuffer, input.uv2.zw);
	accum += sampleWeights[4] * tex2D(inputBuffer, input.uv3.xy);
	accum += sampleWeights[5] * tex2D(inputBuffer, input.uv3.zw);
	accum += sampleWeights[6] * tex2D(inputBuffer, input.uv4.xy);
	accum += sampleWeights[7] * tex2D(inputBuffer, input.uv4.zw);
	accum += sampleWeights[8] * tex2D(inputBuffer, input.uv5.xy);
	accum += sampleWeights[9] * tex2D(inputBuffer, input.uv5.zw);
	accum += sampleWeights[10] * tex2D(inputBuffer, input.uv6.xy);
	accum += sampleWeights[11] * tex2D(inputBuffer, input.uv6.zw);
	accum += sampleWeights[12] * tex2D(inputBuffer, input.uv7.xy);
	accum += sampleWeights[13] * tex2D(inputBuffer, input.uv7.zw);
    return accum;
}


//Final pass with tone mapping
float4 BladeFSMain(BladeVSOutput input) : COLOR
{
	// Get main scene colour
    float4 sceneCol = tex2D(inputBuffer, input.uv);

	// Get luminence value
	float4 lum = tex2D(luminenceBuffer, float2(0.5f, 0.5f));

	// tone map this
	float4 toneMappedSceneCol = toneMap(sceneCol, lum.r);
	
#if defined(ENABLE_BLOOM)
	// Get bloom colour
    float4 bloom = tex2D(bloomBuffer, input.uv2);
#else
	float4 bloom = 0;
#endif

	// Add scene & bloom
	return float4(toneMappedSceneCol.rgb+bloom.rgb, 1.0f);
}


