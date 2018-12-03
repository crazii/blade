#ifndef __Blade_depth_common_hlsl__
#define __Blade_depth_common_hlsl__

//used switches: BLADE_FRAGMENT_DEPTH,BLADE_DEPTH_CLAMP,BLADE_SHADOW_PASS

#include "../../../../Plugins/SubsystemPlugins/BladeGraphics/header/GraphicsShaderShared.inl"

#if defined(BLADE_OGL) || defined(BLADE_GLES)
#define MIN_NDC_Z -1
#else
#define MIN_NDC_Z 0
#endif

#if BLADE_PROFILE == profile_2_0
#undef MAX_SHADOW_CASCADE
#define MAX_SHADOW_CASCADE 1
#elif BLADE_PROFILE < profile_3_0
#undef MAX_SHADOW_CASCADE
#define MAX_SHADOW_CASCADE 1
#endif

//#define BLADE_SHADOW_DEBUG

#if BLADE_GLES
#define samplerShadow sampler2DShadow
#else
#define samplerShadow sampler2D
#endif

//uniforms
uniform float4x4 ShadowMatrix[MAX_SHADOW_CASCADE] : SHADOW_MATRIX;
uniform float4 ShadowTextureCoordTransform[MAX_SHADOW_CASCADE] : SHADOW_TEXCOORDINATES;
uniform float4 ShadowDistances[MAX_SHADOW_CASCADE] : SHADOW_DISTANCES;

float4 Blade_ShadowProjection(float4 worldPos, int i)
{
	return mul(worldPos, ShadowMatrix[i]);
}

//TSM
//shadow depth matrix only used for TSM
uniform float4x4 ShadowDepthMatrix[MAX_SHADOW_CASCADE] : SHADOW_DEPTH_MATRIX;
uniform float4x4 CurrentShadowDepthMatrix : CURRENT_SHADOW_DEPTH_MATRIX;

float4 Blade_CurrentShadowDepth(float4 worldPos)
{
	return mul(worldPos, CurrentShadowDepthMatrix);
}

float4 Blade_ShadowDepth(float4 worldPos, int i)
{
	return mul(worldPos, ShadowDepthMatrix[i]);
}
//-TSM

//get shadow from projected shadow pos. used for forward shading if shadowPos & index is pre-calcluated in vertex shader
half4 Blade_GetShadow(samplerShadow shadowBuffer, float4 bufferSize, float4 shadowPos, int index)
{
	float4 texcoord = ShadowTextureCoordTransform[index];

	float4 shadowClip = shadowPos;
	shadowClip = Blade_Clip2UV(shadowClip);
	shadowClip.xy = shadowClip.xy * texcoord.xy + texcoord.zw;

#if defined(BLADE_FRAGMENT_DEPTH)
	float4 shadowDepth = Blade_ShadowDepth(float4(worldPos, 1), index);
	shadowClip.z = shadowDepth.z / shadowDepth.w;
#endif

#if defined(BLADE_SHADOW_DEBUG)
	static const half4 c[MAX_SHADOW_CASCADE] =
	{
		half4(1,0,0,1),
#if MAX_SHADOW_CASCADE > 1
		half4(0,1,0,1),
	#if MAX_SHADOW_CASCADE > 2
		half4(0,0,1,1),
		#if MAX_SHADOW_CASCADE > 3
		half4(1,0,1,1),
		#endif
	#endif
#endif
	};
	if (shadowClip.x < -1 || shadowClip.x > 1)
		return half4(1, 1, 0, 1);
	if (shadowClip.y < -1 || shadowClip.y > 1)
		return half4(0, 1, 1, 1);
	return c[index] * (half)tex2Dproj(shadowBuffer, shadowClip);
	//return half4(shadowClip.xy,0,1);
#endif

	half4 shadow = 0;

#if BLADE_PROFILE >= profile_3_0
#	define SHADOW_TAPS 8
	static const float4 offsets[SHADOW_TAPS] =
	{
		float4(0.000000, 0.000000, 0.0, 0.0),
		float4(0.079821, 0.165750, 0.0, 0.0),
		float4(-0.331500, 0.159642, 0.0, 0.0),
		float4(-0.239463, -0.497250, 0.0, 0.0),
		float4(0.662999, -0.319284, 0.0, 0.0),
		float4(0.399104, 0.828749, 0.0, 0.0),
		float4(-0.994499, 0.478925, 0.0, 0.0),
		float4(-0.558746, -1.160249, 0.0, 0.0),
	};

	BLADE_UNROLL for (int j = 0; j < SHADOW_TAPS; ++j)
		shadow += (half)tex2Dproj(shadowBuffer, shadowClip + (offsets[j] * 2) * bufferSize.zwzw);
#else
#	define SHADOW_TAPS 1
	shadow = (half)tex2Dproj(shadowBuffer, shadowClip);
#endif

	shadow *= (half)(1.0f / SHADOW_TAPS);
	return shadow;
}

//get shadow using world position and view distance 
half4 Blade_GetShadow(samplerShadow shadowBuffer, float4 bufferSize, float3 worldPos, float distance)
{
	int index = 0;
	BLADE_UNROLL for (int i = 0; i < MAX_SHADOW_CASCADE; ++i)
	{
		if (distance < ShadowDistances[i].x)
		{
			index = i;
			break;
		}
	}

	float4 shadowClip = Blade_ShadowProjection(float4(worldPos, 1), index);
	shadowClip.z = Blade_NDC2ZBuffer(shadowClip.z);	//GL need z to [0,1] pre apply transform before w divide
	return Blade_GetShadow(shadowBuffer, bufferSize, shadowClip, index);
}


//fowrad shadow helpers

//calculate shadowIndex. could be used in vertex shader for DX10+ if disable index interpolation
float4 Blade_ForwardShadowProjection(float3 worldPos, float4x4 viewMatrix, inout float shadowIndex)
{
	int index = 0;
#if MAX_SHADOW_CASCADE > 1
	float viewDistance = -mul(float4(worldPos, 1), viewMatrix).z;
	BLADE_UNROLL for (int i = 0; i < MAX_SHADOW_CASCADE; ++i)
	{
		if (viewDistance < ShadowDistances[i].x)
		{
			index = i;
			break;
		}
	}
#endif
	shadowIndex = (float)index;
	return Blade_ShadowProjection(float4(worldPos, 1), index);
}

//calcluate view distance for CSM
float Blade_ForwardShadowViewDistance(float3 worldPos, float4x4 viewMatrix)
{
	float viewDistance = 0;
#if MAX_SHADOW_CASCADE > 1
	viewDistance = -mul(float4(worldPos,1), viewMatrix).z;
#endif
	return viewDistance;
}

///depth pass helpers

#if defined(BLADE_FRAGMENT_DEPTH)

#	define BLADE_DEPTH_VARYINGS float4 clipPos : TEXCOORD7;
#	if defined(BLADE_SHADOW_PASS)
#		define BLADE_DEPTH(vs_output, pos0, worldPos) vs_output.clipPos = Blade_CurrentShadowDepth(worldPos); pos0=Blade_ClampDepth(pos0)
#	else
#		define BLADE_DEPTH(vs_output, pos0, worldPos) vs_output.clipPos = pos0; pos0=Blade_ClampDepth(pos0)
#	endif

float4 Blade_ClampDepth(float4 clipPos)
{
#	if defined(BLADE_DEPTH_CLAMP)
	//although OGL clip space z min is -1 (here should use clamp(x,-1.1)), GL uses GL_DEPTH_CLAMP and don't do in shader.
	//d3d11 disables depth clip and don't do it in shader too.

	//change log: clamp completely in shader, use MIN_NDC_Z

	//note: clamp z in vertex will change interpolations and may cause large triangle depth change
	//especially when this vertex should be clipped by x/y clipping
	//clamp max to 1 may cause triangle depth become smaller and cause unwanted shadows in depth
	//clamp min to 0 may cause triangle dpeth become larger and missing shadows
	//perform clamping min to 0 because we want object behind light frustum still cast shadows
	float NDCZ = clipPos.z / clipPos.w;
	NDCZ = clamp(NDCZ, MIN_NDC_Z, 1);
	clipPos.z = NDCZ * clipPos.w;
#	endif
	return clipPos;
}

struct BladeDepthFSOutput
{
	float4 color : COLOR;
	float depth : DEPTH;
};

#else

#define BLADE_DEPTH_VARYINGS
#define BLADE_DEPTH(vs_output, pos0, worldPos)

struct BladeDepthFSOutput
{
	float4 color : COLOR;
};

#endif


struct BladeDepthFSInput
{
	float4 pos : POSITION;
	BLADE_DEPTH_VARYINGS
};


BladeDepthFSOutput Blade_DepthFSMain(BladeDepthFSInput input)
{
	BladeDepthFSOutput output;

	output.color = 0;
#if defined(BLADE_FRAGMENT_DEPTH)
	output.depth = input.clipPos.z / input.clipPos.w;
#	if defined(BLADE_DEPTH_CLAMP)
	output.depth = clamp(output.depth, MIN_NDC_Z, 1);
#	endif
#endif
	return output;
}

#endif//__Blade_depth_common_hlsl__