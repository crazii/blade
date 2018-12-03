//!BladeShaderHeader
//![Shader]
//!Profiles=3_0
#include "../inc/common.hlsl"

#define AA_FILTERED 0
#define AA_BORDER 1
#define AA_ALPHA 0
#define AA_CROSS 2
#define AA_GREEN_AS_LUMA 0
#define AA_AABB 1
#define AA_LOWPASS 0
#define AA_DEBUG 0
#define AA_VELOCITY_WEIGHTING 0
#define AA_YCOCG 1
#define AA_BICUBIC 0
#define AA_DYNAMIC 0

// 1 = Use tighter AABB clamp for history.
// 0 = Use simple min/max clamp.
#ifndef AA_AABB
#define AA_AABB 1
#endif

// 0 = Anti-alias the alpha channel also (not getting used currently). 
// 1 = Use alpha channel to improve quality (required for primary AA).
//     Leverages dead code removal to work in RGB instead of RGBA.
#ifndef AA_ALPHA
#define AA_ALPHA 1
#endif

// Cross distance in pixels used in depth search X pattern.
// 0 = Turn this feature off.
// 2 = Is required for standard temporal AA pass.
#ifndef AA_CROSS
#define AA_CROSS 2
#endif

// 1 = Render in blue, with green = diff between frames, red = alpha channel.
// 0 = Non-debug.
#ifndef AA_DEBUG
#define AA_DEBUG 0
#endif

// 2 = Dilate in cross pattern by 2 pixels in distance (this can be larger than 2 if required).
// 1 = Dilate history alpha using maximum of neighborhood.
//     This increases thin edge quality in motion.
//     This is only valid for AA_ALPHA == 1
// 0 = Turn off.
#ifndef AA_DILATE
#define AA_DILATE AA_ALPHA
#endif

// 1 = Use dynamic motion.
// 0 = Skip dynamic motion, currently required for half resolution passes.
#ifndef AA_DYNAMIC
#define AA_DYNAMIC 1
#endif

// 1 = Use filtered sample.
// 0 = Use center sample.
#ifndef AA_FILTERED
#define AA_FILTERED 1
#endif

// 0 = Dynamic motion based lerp value (default).
// non-zero = Use 1/LERP fixed lerp value (used for reflections).
#ifndef AA_LERP
#define AA_LERP 0
#endif

// 1 = Use extra lowpass filter for quality bump.
// 0 = Don't use.
#ifndef AA_LOWPASS
#define AA_LOWPASS 1
#endif

// 1 = Use higher quality round clamp.
// 0 = Use lower quality but faster box clamp.
#ifndef AA_ROUND
#define AA_ROUND 1
#endif

// 1 = Use extra clamp to avoid NANs
// 0 = Don't use.
#ifndef AA_NAN
#define AA_NAN 1
#endif

// Fix for lack of borders during current frame filter.
#ifndef AA_BORDER
#define AA_BORDER 0
#endif

// Force clamp on alpha.
#ifndef AA_FORCE_ALPHA_CLAMP
#define AA_FORCE_ALPHA_CLAMP 0
#endif

// Use YCoCg path.
#ifndef AA_YCOCG
#define AA_YCOCG 0
#endif

// Use green as luma.
#ifndef AA_GREEN_AS_LUMA
#define AA_GREEN_AS_LUMA AA_YCOCG
#endif

// Bicubic filter history
#ifndef AA_BICUBIC
#define AA_BICUBIC 0
#endif

// Special adjustments for DOF.
#ifndef AA_DOF
#define AA_DOF 0
#endif

// Tone map to kill fireflies
#ifndef AA_TONE
#define AA_TONE 1
#endif

#if BLADE_PROFILE < profile_5_0
float rcp(float r)
{
	return 1/r;
}
float2 rcp(float2 r)
{
	return 1/r;
}
float3 rcp(float3 r)
{
	return 1/r;
}
float4 rcp(float4 r)
{
	return 1/r;
}
#endif

float3 RGBToYCoCg(float3 RGB)
{
	float Y = dot(RGB, float3(1, 2, 1));
	float Co = dot(RGB, float3(2, 0, -2));
	float Cg = dot(RGB, float3(-1, 2, -1));

	float3 YCoCg = float3(Y, Co, Cg);
	return YCoCg;
}

float3 YCoCgToRGB(float3 YCoCg)
{
	float Y = YCoCg.x * 0.25;
	float Co = YCoCg.y * 0.25;
	float Cg = YCoCg.z * 0.25;

	float R = Y + Co - Cg;
	float G = Y + Cg;
	float B = Y - Co - Cg;

	float3 RGB = float3(R, G, B);
	return RGB;
}

// Faster but less accurate luma computation. 
// Luma includes a scaling by 4.
float Luma4(float3 Color)
{
	return (Color.g * 2.0) + (Color.r + Color.b);
}

// Optimized HDR weighting function.
float HdrWeight4(float3 Color, float Exposure)
{
	return rcp(Luma4(Color) * Exposure + 4.0);
}

float HdrWeightY(float Color, float Exposure)
{
	return rcp(Color * Exposure + 1.0);
}

float HdrWeightG(float3 Color, float Exposure)
{
	return rcp(Color.g * Exposure + 1.0);
}

float HdrWeightG_(float Color, float Exposure)
{
	return rcp(Color * Exposure + 1.0);
}


// Optimized HDR weighting function.
float HdrWeight4_(float Color, float Exposure)
{
	return rcp(Color * Exposure + 4.0);
}

// Optimized HDR weighting inverse.
float HdrWeightInv4(float3 Color, float Exposure)
{
	return 4.0 * rcp(Luma4(Color) * (-Exposure) + 1.0);
}

float HdrWeightInvG(float3 Color, float Exposure)
{
	return rcp(Color.g * (-Exposure) + 1.0);
}

float HdrWeightInvY(float Color, float Exposure)
{
	return rcp(Color * (-Exposure) + 1.0);
}

float HdrWeightInv4_(float Color, float Exposure)
{
	return 4.0 * rcp(Color * (-Exposure) + 1.0);
}

float HdrWeightInvG_(float Color, float Exposure)
{
	return rcp(Color * (-Exposure) + 1.0);
}


// This returns exposure normalized linear luma from a PerceptualLuma4().
float LinearLuma4(float Channel, float Exposure)
{
	return Channel * HdrWeightInv4_(Channel, Exposure);
}

// This returns exposure normalized linear luma from a PerceptualLuma4().
float LinearLumaG(float Channel, float Exposure)
{
	return Channel * HdrWeightInvG_(Channel, Exposure);
}


float PerceptualLuma4(float3 Color, float Exposure)
{
	float L = Luma4(Color);
	return L * HdrWeight4_(L, Exposure);
}

float PerceptualLumaG(float3 Color, float Exposure)
{
	return Color.g * HdrWeightG_(Color.g, Exposure);
}



float Luma(float3 Color)
{
#if 1
	// This seems to work better (less same luma ghost trails).
	// CCIR 601 function for luma.
	return dot(Color, float3(0.299, 0.587, 0.114));
#else
	// Rec 709 function for luma.
	return dot(Color, float3(0.2126, 0.7152, 0.0722));
#endif
}

float HighlightCompression(float Channel)
{
	return Channel * rcp(1.0 + Channel);
}

float HighlightDecompression(float Channel)
{
	return Channel * rcp(1.0 - Channel);
}

float PerceptualLuma(float3 Color, float Exposure)
{
	return sqrt(HighlightCompression(Luma(Color) * Exposure));
}

float LinearLuma(float Channel)
{
	// This returns exposure normalized linear luma from a PerceptualLuma().
	return HighlightDecompression(Channel * Channel);
}

// Intersect ray with AABB, knowing there is an intersection.
//   Dir = Ray direction.
//   Org = Start of the ray.
//   Box = Box is at {0,0,0} with this size.
// Returns distance on line segment.
float IntersectAABB(float3 Dir, float3 Org, float3 Box)
{
#if PS4_PROFILE
	// This causes flicker, it should only be used on PS4 until proper fix is in.
	if (min(min(abs(Dir.x), abs(Dir.y)), abs(Dir.z)) < (1.0 / 65536.0)) return 1.0;
#endif
	float3 RcpDir = rcp(Dir);
	float3 TNeg = (Box - Org) * RcpDir;
	float3 TPos = ((-Box) - Org) * RcpDir;
	return max(max(min(TNeg.x, TPos.x), min(TNeg.y, TPos.y)), min(TNeg.z, TPos.z));
}

float HistoryClamp(float3 History, float3 Filtered, float3 NeighborMin, float3 NeighborMax)
{
	float3 Min = min(Filtered, min(NeighborMin, NeighborMax));
	float3 Max = max(Filtered, max(NeighborMin, NeighborMax));
	float3 Avg2 = Max + Min;
	float3 Dir = Filtered - History;
	float3 Org = History - Avg2 * 0.5;
	float3 Scale = Max - Avg2 * 0.5;
	return saturate(IntersectAABB(Dir, Org, Scale));
}

float HdrWeight(float3 Color, float Exposure)
{
	return rcp(max(Luma(Color) * Exposure, 1.0));
}

float4 HdrLerp(float4 ColorA, float4 ColorB, float Blend, float Exposure)
{
	float BlendA = (1.0 - Blend) * HdrWeight(ColorA.rgb, Exposure);
	float BlendB = Blend  * HdrWeight(ColorB.rgb, Exposure);
	float RcpBlend = rcp(BlendA + BlendB);
	BlendA *= RcpBlend;
	BlendB *= RcpBlend;
	return ColorA * BlendA + ColorB * BlendB;
}

void Bicubic2DCatmullRom(in float2 UV, in float4 Size4, out float2 Sample[3], out float2 Weight[3])
{
	const float2 InvSize = Size4.zw;
	const float2 Size = Size4.xy;

	UV *= Size;

	float2 tc = floor(UV - 0.5) + 0.5;
	float2 f = UV - tc;
	float2 f2 = f * f;
	float2 f3 = f2 * f;

	float2 w0 = f2 - 0.5 * (f3 + f);
	float2 w1 = 1.5 * f3 - 2.5 * f2 + 1;
	float2 w3 = 0.5 * (f3 - f2);
	float2 w2 = 1 - w0 - w1 - w3;

	Weight[0] = w0;
	Weight[1] = w1 + w2;
	Weight[2] = w3;

	Sample[0] = tc - 1;
	Sample[1] = tc + w2 / Weight[1];
	Sample[2] = tc + 2;

	Sample[0] *= InvSize;
	Sample[1] *= InvSize;
	Sample[2] *= InvSize;
}


//unproject current VP and project to prev VP
uniform float4x4 historyMatrix;
uniform float PlusWeights[5];
uniform float SampleWeights[9];
uniform float LowpassWeights[9];

//history
uniform sampler2D historyTexture;
uniform float4 historyTexture_Size;
uniform float4 historyTexture_UVSpace;

//color
uniform sampler2D inputColor : IMAGEFX_INPUT;
uniform float4 inputColorUVSpace : IMAGEFX_INPUT_UVSPACE;
uniform float4 inputColorSize : IMAGEFX_INPUT_SIZE;

//depth
uniform sampler2D inputDepth : RENDERBUFFER_INPUT0;
uniform float4 inputDepthUVSpace : RENDERBUFFER_UVSPACE0;
uniform float4 inputDepthSize : RENDERBUFFER_SIZE0;

//motion
uniform sampler2D motionBuffer : RENDERBUFFER_INPUT1;
uniform float4 motionUVSpace : RENDERBUFFER_UVSPACE1;
uniform float4 motionBufferSize : RENDERBUFFER_SIZE1;

struct BladeVSOutput
{
	float4	pos : POSITION;
	float4	cpos : TEXCOORD0;	//clip space position
	float2	colorUV	: TEXCOORD1;
	float2	depthUV	: TEXCOORD2;
	float2	motionUV : TEXCOORD3;
};

BladeVSOutput BladeVSMain(
	float4 pos		: POSITION
)
{
	BladeVSOutput output;
	pos.zw = float2(0.5, 1);
	output.pos = pos;
	output.cpos = pos;

	float2 uv = Blade_Clip2UV(pos).xy;
	output.colorUV = Blade_TransformUVSpace(uv, inputColorUVSpace);
	output.depthUV = Blade_TransformUVSpace(uv, inputDepthUVSpace);
	output.motionUV = Blade_TransformUVSpace(uv, motionUVSpace);
	return output;
}

void BladeFSMain(
	BladeVSOutput input
	, uniform float4 viewSize : VIEW_SIZE
	, out float4 outColor0 : COLOR0
	, out float4 outColor1 : COLOR1
){
	//Crazii
	float2 colorUV = input.colorUV;
	float2 depthUV = input.depthUV;
	float2 motionUV = input.motionUV;
	float4 outColor;
	float InExposureScale = 1.0f;
	//float2 historyUV = input.uv + 0.5 * historyTexture_Size.zw;
	//return tex2D(historyTexture, colorUV);
	//return tex2D(inputColor, colorUV);
	//return zbuffer2view(tex2D(inputDepth, depthUV).r)/50;
	//return float4(tex2D(motionBuffer, motionUV).xy, 0, 1);
	//-Crazii

	// FIND MOTION OF PIXEL AND NEAREST IN NEIGHBORHOOD
	// ------------------------------------------------
	float3 PosN; // Position of this pixel, possibly later nearest pixel in neighborhood.
	PosN.xy = input.cpos.xy;
	PosN.z = tex2D(inputDepth, depthUV).r;
	// Screen position of minimum depth.
	float2 motionOffset = float2(0.0, 0.0);
#if AA_CROSS
	// For motion vector, use camera/dynamic motion from min depth pixel in pattern around pixel.
	// This enables better quality outline on foreground against different motion background.
	// Larger 2 pixel distance "x" works best (because AA dilates surface).
	float4 Depths;
	Depths.x = tex2D(inputDepth, depthUV + float2(-AA_CROSS, -AA_CROSS) * inputDepthSize.zw ).r;
	Depths.y = tex2D(inputDepth, depthUV + float2(AA_CROSS, -AA_CROSS) * inputDepthSize.zw).r;
	Depths.z = tex2D(inputDepth, depthUV + float2(-AA_CROSS, AA_CROSS) * inputDepthSize.zw).r;
	Depths.w = tex2D(inputDepth, depthUV + float2(AA_CROSS, AA_CROSS) * inputDepthSize.zw).r;

	float2 DepthOffset = float2(AA_CROSS, AA_CROSS);
	float DepthOffsetXx = float(AA_CROSS);

	if (Depths.x < Depths.y)
		DepthOffsetXx = -AA_CROSS;
	if (Depths.z < Depths.w)
		DepthOffset.x = -AA_CROSS;

	float DepthsXY = min(Depths.x, Depths.y);
	float DepthsZW = min(Depths.z, Depths.w);
	if (DepthsXY < DepthsZW)
	{
		DepthOffset.y = -AA_CROSS;
		DepthOffset.x = DepthOffsetXx;
	}
	float DepthsXYZW = min(DepthsXY, DepthsZW);
	if (DepthsXYZW < PosN.z)
	{
		// This is offset for reading from velocity texture.
		// This supports half or fractional resolution velocity textures.
		// With the assumption that UV position scales between velocity and color.
		motionOffset = DepthOffset * motionBufferSize.zw;
		// This is [0 to 1] flipped in Y.
		//PosN.xy = SvPositionToScreenPosition(SvPosition).xy + DepthOffset * _ViewportSize.zw * 2.0;
		PosN.z = DepthsXYZW;
	}

#endif	// AA_CROSS

#if AA_DOF
	//Crazii
	// temp solution, BrianK knows
	//float4 ThisClip = float4((UV - View.ScreenPositionScaleBias.wz) / View.ScreenPositionScaleBias.xy, PosN.z, 1);
	//float4 ThisTranslatedWorld = mul(View.ClipToTranslatedWorld, ThisClip);
	//ThisTranslatedWorld /= ThisTranslatedWorld.w;

	//float3 PrevTranslatedWorld = ThisTranslatedWorld.xyz + (View.PrevPreViewTranslation - View.PreViewTranslation);
	//float4 PrevClip = mul(View.PrevTranslatedWorldToClip, float4(PrevTranslatedWorld, 1));
	//float2 PrevScreen = PrevClip.xy / PrevClip.w;
	////float2 PrevUV = PrevScreen.xy * Frame.ScreenPositionScaleBias.xy + Frame.ScreenPositionScaleBias.wz;

	//float2 BackN = SvPositionToScreenPosition(SvPosition).xy - PrevScreen - (View.ViewToClip[2].xy - View.PrevProjection[2].xy);
	//-Crazii
#else // AA_DOF	
	// Camera motion for pixel or nearest pixel (in ScreenPos space).
	float4 ThisClip = float4(PosN.xy, Blade_ZBuffer2NDC(PosN.z), 1);
	float4 PrevClip = mul(ThisClip, historyMatrix);
	float2 PrevScreen = PrevClip.xy / PrevClip.w;
	float2 BackN = PosN.xy - PrevScreen;
#endif // AA_DOF

#if AA_DYNAMIC
	float4 VelocityN;
#if AA_CROSS
	VelocityN = tex2D(motionBuffer, motionUV + motionOffset);
#else
	VelocityN = tex2D(motionBuffer, motionUV);
#endif
	//Crazii
	//bool DynamicN = VelocityN.x > 0.0;
	bool DynamicN = VelocityN.x != 0 || VelocityN.y != 0;
	if (DynamicN)
	{
		//BackN = DecodeVelocityFromTexture(VelocityN);
		BackN = VelocityN.xy;
	}
	//-Crazii
	//return half4(BackN, 0, 1);
	//return half4(VelocityN, 0, 1);
	//return 1;
	//return 0;
#endif

	//return half4(BackN, 0, 1);
	//return 0;

#if !AA_BICUBIC
	float2 BackTemp = BackN * viewSize.xy;
	// Save the amount of pixel offset of just camera motion, used later as the amount of blur introduced by history.
	float HistoryBlurAmp = 2.0;
	float HistoryBlur = saturate(abs(BackTemp.x) * HistoryBlurAmp + abs(BackTemp.y) * HistoryBlurAmp);
	float Velocity = sqrt(dot(BackTemp, BackTemp));
#endif
	// Easier to do off screen check before conversion.
	// BackN is in units of 2pixels/viewportWidthInPixels
	// This converts back projection vector to [-1 to 1] offset in viewport.
	BackN = input.cpos.xy - BackN;
	bool OffScreen = max(abs(BackN.x), abs(BackN.y)) >= 1.0;
	// Also clamp to be on screen (fixes problem with DOF).
	// The .z and .w is the 1/width and 1/height.
	// This clamps to be a pixel inside the viewport.
	BackN.xy = clamp(BackN.xy, -1.0 + viewSize.zw, 1.0 - viewSize.zw);
	//BackN.x = clamp(BackN.x, -1.0 + viewSize.z, 1.0 - viewSize.z);
	//BackN.y = clamp(BackN.y, -1.0 + viewSize.w, 1.0 - viewSize.w);
	// Convert from [-1 to 1] to view rectangle which is somewhere in [0 to 1].
	// The extra +0.5 factor is because ScreenPosToPixel.zw is incorrectly computed
	// as the upper left of the pixel instead of the center of the pixel.

	//Crazii
	BackN = Blade_TransformUVSpace(Blade_Clip2UV(BackN), historyTexture_UVSpace);
	//-Crazii

	// FILTER PIXEL (RESAMPLE TO REMOVE JITTER OFFSET) AND GET NEIGHBORHOOD
	// --------------------------------------------------------------------
	// 012
	// 345
	// 678
#if AA_YCOCG	
	// Special case, only using 5 taps.
	float4 Neighbor1 = tex2D(inputColor, colorUV + float2(0, -1) * inputColorSize.zw);
	float4 Neighbor3 = tex2D(inputColor, colorUV + float2(-1, 0) * inputColorSize.zw);
	float4 Neighbor4 = tex2D(inputColor, colorUV);
	float4 Neighbor5 = tex2D(inputColor, colorUV + float2(1, 0) * inputColorSize.zw);
	float4 Neighbor7 = tex2D(inputColor, colorUV + float2(0, 1) * inputColorSize.zw);
	Neighbor1.rgb = RGBToYCoCg(Neighbor1.rgb);
	Neighbor3.rgb = RGBToYCoCg(Neighbor3.rgb);
	Neighbor4.rgb = RGBToYCoCg(Neighbor4.rgb);
	Neighbor5.rgb = RGBToYCoCg(Neighbor5.rgb);
	Neighbor7.rgb = RGBToYCoCg(Neighbor7.rgb);
#if AA_TONE
	Neighbor1.xyz *= HdrWeightY(Neighbor1.x, InExposureScale);
	Neighbor3.xyz *= HdrWeightY(Neighbor3.x, InExposureScale);
	Neighbor4.xyz *= HdrWeightY(Neighbor4.x, InExposureScale);
	Neighbor5.xyz *= HdrWeightY(Neighbor5.x, InExposureScale);
	Neighbor7.xyz *= HdrWeightY(Neighbor7.x, InExposureScale);
#endif
#if AA_FILTERED
	float4 Filtered =
		Neighbor1 * PlusWeights[0] +
		Neighbor3 * PlusWeights[1] +
		Neighbor4 * PlusWeights[2] +
		Neighbor5 * PlusWeights[3] +
		Neighbor7 * PlusWeights[4];
#if AA_BORDER
	// Use unfiltered for 1 pixel border.
	float2 TestPos = abs(input.cpos.xy);
	// Add 1 pixel and check if off screen.
	TestPos += viewSize.zw * 2.0;
	bool FilteredOffScreen = max(TestPos.x, TestPos.y) >= 1.0;
	if (FilteredOffScreen)
	{
		Filtered = Neighbor4;
	}
#endif
#else
	// Unfiltered.
	float4 Filtered = Neighbor4;
#endif
	float4 FilteredLow = Filtered;
	// Neighborhood seems to only need the "+" pattern.
	float4 NeighborMin = min(min(min(Neighbor1, Neighbor3), min(Neighbor4, Neighbor5)), Neighbor7);
	float4 NeighborMax = max(max(max(Neighbor1, Neighbor3), max(Neighbor4, Neighbor5)), Neighbor7);
#else
	float4 Neighbor0 = tex2D(inputColor, colorUV + float2(-1, -1) * inputColorSize.zw);
	float4 Neighbor1 = tex2D(inputColor, colorUV + float2(0, -1) * inputColorSize.zw);
	float4 Neighbor2 = tex2D(inputColor, colorUV + float2(1, -1) * inputColorSize.zw);
	float4 Neighbor3 = tex2D(inputColor, colorUV + float2(-1, 0) * inputColorSize.zw);
	float4 Neighbor4 = tex2D(inputColor, colorUV);
	float4 Neighbor5 = tex2D(inputColor, colorUV + float2(1, 0) * inputColorSize.zw);
	float4 Neighbor6 = tex2D(inputColor, colorUV + float2(-1, 1) * inputColorSize.zw);
	float4 Neighbor7 = tex2D(inputColor, colorUV + float2(0, 1) * inputColorSize.zw);
	float4 Neighbor8 = tex2D(inputColor, colorUV + float2(1, 1) * inputColorSize.zw);
#if AA_GREEN_AS_LUMA
	Neighbor0.rgb *= HdrWeightG(Neighbor0.rgb, InExposureScale);
	Neighbor1.rgb *= HdrWeightG(Neighbor1.rgb, InExposureScale);
	Neighbor2.rgb *= HdrWeightG(Neighbor2.rgb, InExposureScale);
	Neighbor3.rgb *= HdrWeightG(Neighbor3.rgb, InExposureScale);
	Neighbor4.rgb *= HdrWeightG(Neighbor4.rgb, InExposureScale);
	Neighbor5.rgb *= HdrWeightG(Neighbor5.rgb, InExposureScale);
	Neighbor6.rgb *= HdrWeightG(Neighbor6.rgb, InExposureScale);
	Neighbor7.rgb *= HdrWeightG(Neighbor7.rgb, InExposureScale);
	Neighbor8.rgb *= HdrWeightG(Neighbor8.rgb, InExposureScale);
#else
	Neighbor0.rgb *= HdrWeight4(Neighbor0.rgb, InExposureScale);
	Neighbor1.rgb *= HdrWeight4(Neighbor1.rgb, InExposureScale);
	Neighbor2.rgb *= HdrWeight4(Neighbor2.rgb, InExposureScale);
	Neighbor3.rgb *= HdrWeight4(Neighbor3.rgb, InExposureScale);
	Neighbor4.rgb *= HdrWeight4(Neighbor4.rgb, InExposureScale);
	Neighbor5.rgb *= HdrWeight4(Neighbor5.rgb, InExposureScale);
	Neighbor6.rgb *= HdrWeight4(Neighbor6.rgb, InExposureScale);
	Neighbor7.rgb *= HdrWeight4(Neighbor7.rgb, InExposureScale);
	Neighbor8.rgb *= HdrWeight4(Neighbor8.rgb, InExposureScale);
#endif
#if AA_FILTERED
	float4 Filtered =
		Neighbor0 * SampleWeights[0] +
		Neighbor1 * SampleWeights[1] +
		Neighbor2 * SampleWeights[2] +
		Neighbor3 * SampleWeights[3] +
		Neighbor4 * SampleWeights[4] +
		Neighbor5 * SampleWeights[5] +
		Neighbor6 * SampleWeights[6] +
		Neighbor7 * SampleWeights[7] +
		Neighbor8 * SampleWeights[8];
#if AA_LOWPASS
	float4 FilteredLow =
		Neighbor0 * LowpassWeights[0] +
		Neighbor1 * LowpassWeights[1] +
		Neighbor2 * LowpassWeights[2] +
		Neighbor3 * LowpassWeights[3] +
		Neighbor4 * LowpassWeights[4] +
		Neighbor5 * LowpassWeights[5] +
		Neighbor6 * LowpassWeights[6] +
		Neighbor7 * LowpassWeights[7] +
		Neighbor8 * LowpassWeights[8];
#else
	float4 FilteredLow = Filtered;
#endif
#if AA_BORDER
	// Use unfiltered for 1 pixel border.
	float2 TestPos = abs(input.cpos.xy);
	// Add 1 pixel and check if off screen.
	TestPos += viewSize.zw * 2.0;
	bool FilteredOffScreen = max(TestPos.x, TestPos.y) >= 1.0;
	if (FilteredOffScreen)
	{
		Filtered = Neighbor4;
		FilteredLow = Neighbor4;
	}
#endif
#else
	// Unfiltered.
	float4 Filtered = Neighbor4;
	float4 FilteredLow = Neighbor4;
#endif
#if AA_ROUND
	float4 NeighborMin2 = min(min(Neighbor0, Neighbor2), min(Neighbor6, Neighbor8));
	float4 NeighborMax2 = max(max(Neighbor0, Neighbor2), max(Neighbor6, Neighbor8));
	float4 NeighborMin = min(min(min(Neighbor1, Neighbor3), min(Neighbor4, Neighbor5)), Neighbor7);
	float4 NeighborMax = max(max(max(Neighbor1, Neighbor3), max(Neighbor4, Neighbor5)), Neighbor7);
	NeighborMin2 = min(NeighborMin2, NeighborMin);
	NeighborMax2 = max(NeighborMax2, NeighborMax);
	NeighborMin = NeighborMin * 0.5 + NeighborMin2 * 0.5;
	NeighborMax = NeighborMax * 0.5 + NeighborMax2 * 0.5;
#else
	float4 NeighborMin = min(min(
		min(min(Neighbor0, Neighbor1), min(Neighbor2, Neighbor3)),
		min(min(Neighbor4, Neighbor5), min(Neighbor6, Neighbor7))), Neighbor8);
	float4 NeighborMax = max(max(
		max(max(Neighbor0, Neighbor1), max(Neighbor2, Neighbor3)),
		max(max(Neighbor4, Neighbor5), max(Neighbor6, Neighbor7))), Neighbor8);
#endif
#endif

	// FETCH HISTORY
	// -------------
#if AA_BICUBIC
	float2 Weight[3];
	float2 Sample[3];
	Bicubic2DCatmullRom(BackN.xy, historyTexture_Size, Sample, Weight);

	outColor = tex2D(historyTexture, float2(Sample[0].x, Sample[0].y)) * Weight[0].x * Weight[0].y;
	outColor += tex2D(historyTexture, float2(Sample[1].x, Sample[0].y)) * Weight[1].x * Weight[0].y;
	outColor += tex2D(historyTexture, float2(Sample[2].x, Sample[0].y)) * Weight[2].x * Weight[0].y;

	outColor += tex2D(historyTexture, float2(Sample[0].x, Sample[1].y)) * Weight[0].x * Weight[1].y;
	outColor += tex2D(historyTexture, float2(Sample[1].x, Sample[1].y)) * Weight[1].x * Weight[1].y;
	outColor += tex2D(historyTexture, float2(Sample[2].x, Sample[1].y)) * Weight[2].x * Weight[1].y;

	outColor += tex2D(historyTexture, float2(Sample[0].x, Sample[2].y)) * Weight[0].x * Weight[2].y;
	outColor += tex2D(historyTexture, float2(Sample[1].x, Sample[2].y)) * Weight[1].x * Weight[2].y;
	outColor += tex2D(historyTexture, float2(Sample[2].x, Sample[2].y)) * Weight[2].x * Weight[2].y;
#else
	outColor = tex2D(historyTexture, BackN.xy);
#endif

#if AA_DEBUG
	Neighbor4.rg = float2(0.0, 0.0);
	NeighborMin.rg = float2(0.0, 0.0);
	NeighborMax.rg = float2(0.0, 0.0);
	Filtered.rg = float2(0.0, 0.0);
	FilteredLow.rg = float2(0.0, 0.0);
	float DebugDiffCurrent = Filtered.b;
#endif
#if AA_YCOCG
	outColor.rgb = RGBToYCoCg(outColor.rgb);
#if AA_TONE
	outColor.xyz *= HdrWeightY(outColor.x, InExposureScale);
#endif
#else
#if AA_GREEN_AS_LUMA
	outColor.rgb *= HdrWeightG(outColor.rgb, InExposureScale);
#else
	outColor.rgb *= HdrWeight4(outColor.rgb, InExposureScale);
#endif
#endif
#if AA_DEBUG
	outColor.rg = float2(0.0, 0.0);
	float DebugDiffPrior = outColor.b;
#endif


	// FIND LUMA OF CLAMPED HISTORY
	// ----------------------------
	// Save off luma of history before the clamp.
#if AA_YCOCG
	float LumaMin = NeighborMin.x;
	float LumaMax = NeighborMax.x;
	float LumaHistory = outColor.x;
#else
#if AA_GREEN_AS_LUMA
	float LumaMin = NeighborMin.g;
	float LumaMax = NeighborMax.g;
	float LumaHistory = outColor.g;
#else
	float LumaMin = Luma4(NeighborMin.rgb);
	float LumaMax = Luma4(NeighborMax.rgb);
	float LumaHistory = Luma4(outColor.rgb);
#endif
#endif
	float LumaContrast = LumaMax - LumaMin;
#if AA_YCOCG
	outColor.rgb = clamp(outColor.rgb, NeighborMin.rgb, NeighborMax.rgb);
#if (AA_ALPHA == 0)
	outColor.a = clamp(outColor.a, NeighborMin.a, NeighborMax.a);
#endif
#else
#if AA_AABB
	// Clamp history, this uses color AABB intersection for tighter fit.
	// Clamping works with the low pass (if available) to reduce flicker.
	float ClampBlend = HistoryClamp(outColor.rgb, FilteredLow.rgb, NeighborMin.rgb, NeighborMax.rgb);
#if AA_ALPHA
	outColor.rgb = lerp(outColor.rgb, FilteredLow.rgb, ClampBlend);
#else
	outColor.rgba = lerp(outColor.rgba, FilteredLow.rgba, ClampBlend);
#endif
#else
	outColor = clamp(outColor, NeighborMin, NeighborMax);
#endif
#endif
#if AA_DEBUG
	outColor.rg = float2(0.0, 0.0);
#endif

	// ADD BACK IN ALIASING TO SHARPEN
	// -------------------------------
#if AA_FILTERED && !AA_BICUBIC
	// Blend in non-filtered based on the amount of sub-pixel motion.
	float AddAliasing = saturate(HistoryBlur) * 0.5;
	float LumaContrastFactor = 32.0;
#if AA_GREEN_AS_LUMA || AA_YCOCG
	// GREEN_AS_LUMA is 1/4 as bright.
	LumaContrastFactor *= 4.0;
#endif
	AddAliasing = saturate(AddAliasing + rcp(1.0 + LumaContrast * LumaContrastFactor));
	Filtered.rgb = lerp(Filtered.rgb, Neighbor4.rgb, AddAliasing);
#endif
#if AA_YCOCG
	float LumaFiltered = Filtered.x;
#else
#if AA_GREEN_AS_LUMA
	float LumaFiltered = Filtered.g;
#else
	float LumaFiltered = Luma4(Filtered.rgb);
#endif
#endif

	// COMPUTE BLEND AMOUNT 
	// --------------------
	float DistToClamp = min(abs(LumaMin - LumaHistory), abs(LumaMax - LumaHistory));
#if AA_BICUBIC
	float HistoryFactor = 0.125 * DistToClamp;
#else
	float HistoryAmount = (1.0 / 8.0) + HistoryBlur * (1.0 / 8.0);
	float HistoryFactor = DistToClamp * HistoryAmount * (1.0 + HistoryBlur * HistoryAmount * 8.0);
#endif
	float BlendFinal = saturate(HistoryFactor / (DistToClamp + LumaMax - LumaMin));
#if AA_TONE
	BlendFinal = 0.04;
#endif
#if RESPONSIVE
	// Responsive forces 1/4 of new frame.
	BlendFinal = 1.0 / 4.0;
#elif AA_NAN && (COMPILER_GLSL || COMPILER_METAL)
	// The current Metal & GLSL compilers don't handle saturate(NaN) -> 0, instead they return NaN/INF.
	BlendFinal = -min(-BlendFinal, 0.0);
#endif

	// Offscreen feedback resets.
#if AA_LERP 
	float FixedLerp = 1.0 / float(AA_LERP);
#endif
	if (OffScreen)
	{
		outColor = Filtered;
#if AA_ALPHA
		outColor.a = 0.0;
#endif
#if AA_LERP
		FixedLerp = 1.0;
#endif
	}

	// DO FINAL BLEND BETWEEN HISTORY AND FILTERED COLOR
	// -------------------------------------------------
#if (AA_LERP == 0)
#if AA_ALPHA
	// Blend in linear to hit luma target.
	outColor.rgb = lerp(outColor.rgb, Filtered.rgb, BlendFinal);
#if RESPONSIVE
	outColor.a = max(outColor.a, 1.0 / 2.0);
#endif
#else
	outColor = lerp(outColor, Filtered, BlendFinal);
#if AA_FORCE_ALPHA_CLAMP
	outColor.a = clamp(outColor.a, NeighborMin.a, NeighborMax.a);
#endif
#endif
#else
	outColor = lerp(outColor, Filtered, FixedLerp);
#endif
#if AA_YCOCG
#if AA_TONE
	outColor.xyz *= HdrWeightInvY(outColor.x, InExposureScale);
#endif
	outColor.rgb = YCoCgToRGB(outColor.rgb);
#else
	// Convert back into linear.
#if AA_GREEN_AS_LUMA
	outColor.rgb *= HdrWeightInvG(outColor.rgb, InExposureScale);
#else
	outColor.rgb *= HdrWeightInv4(outColor.rgb, InExposureScale);
#endif
#endif
#if AA_NAN
	// Transform NaNs to black, transform negative colors to black.
	outColor.rgb = -min(-outColor.rgb, 0.0);
#endif
#if AA_DEBUG
	outColor.g = abs(DebugDiffPrior - DebugDiffCurrent);
	outColor.r = outColor.a;
#endif

	outColor0 = outColor1 = outColor;
}
