//!BladeShaderHeader
//![Shader]
//!Profiles=3_0
//!VSEntry=LightVSMain
//!FSEntry=LightPSMain
//!
//![SubShaders]
//!directional_light=DIRECTIONAL
//!point_light=POINT
//!spot_light=SPOT
//!ambient_light=DIRECTIONAL,AMBIENT
#include "inc/common.hlsl"
#include "inc/depth_common.hlsl"
#include "../../../Plugins/SubsystemPlugins/BladeGraphics/header/GraphicsShaderShared.inl"

//deferred shading shader

//use unprojection to get world position
#define USE_UNPROJECTION (0)

#if defined(DIRECTIONAL)
float light_directional_count : DEFERRED_LIGHT_COUNT;
half4 light_directions[BLADE_MAX_DEFFERD_DIRECTIONAL_LIGHT_COUNT] : DEFERRED_LIGHT_DIRECTIONS;
half4 light_diffuses[BLADE_MAX_DEFFERD_DIRECTIONAL_LIGHT_COUNT] : DEFERRED_LIGHT_DIFFUSES;
half4 light_speculars[BLADE_MAX_DEFFERD_DIRECTIONAL_LIGHT_COUNT] : DEFERRED_LIGHT_SPECULARS;
#endif
half4 light_ambient : GLOBAL_AMBIENT;

struct DeferredShadingVSOutput
{
	float4 pos : POSITION;
	float3 dir : TEXCOORD0;
	float2 colorUV : TEXCOORD1;
	float2 normalUV : TEXCOORD2;
	float2 depthUV : TEXCOORD3;
	float2 aoUV :TEXCOORD4;
#if !defined(DIRECTIONAL)
	float2 depthRange : TEXCOORD5;
#endif
};

DeferredShadingVSOutput LightVSMain(
	float4 pos : POSITION
	,uniform float4 eye_position : EYE_POS	//eye pos in world space
	,uniform float4x4 inv_viewproj_matrix : INV_VIEWPROJ_MATRIX
#if !defined(DIRECTIONAL)
	,uniform float4x4 projMatrix : PROJ_MATRIX
#	if defined(POINT)
	,uniform float4 viewSpacePos : DEFERRED_LIGHT_POSITION_VIEW_SPACE
#	else
	,uniform float4 viewSpaceBounding[2] : VIEW_SPACE_BOUNDING
#	endif
#endif
	//some buffers are not fully used, use uv dynamically calculated by programming,
	//this also apply the half pixel offset
	,uniform float4 colorUVSpace	: RENDERBUFFER_UVSPACE0
	,uniform float4 normalUVSpace	: RENDERBUFFER_UVSPACE1
	,uniform float4 depthUVSpace	: RENDERBUFFER_UVSPACE2
	,uniform float4 aoUVSpace		: RENDERBUFFER_UVSPACE4
	)
{
	DeferredShadingVSOutput output;
#if !defined(DIRECTIONAL)
	//use outer sphere to make sure rays covers entire volume
#	if defined(POINT)
	float4 posVS = float4(viewSpacePos.xyz, 1);
	float3 halfSize = viewSpacePos.w;
#	else
	float4 posVS = viewSpaceBounding[0];
	float3 halfSize = viewSpaceBounding[1].xyz;
#	endif

	//view space depth range. note posVS.z < 0 in Right handed system
	output.depthRange = float2(-(posVS.z + halfSize.z), -(posVS.z - halfSize.z));	//min,max

	/* using nearer plane:
	+--------------------------+
	 \                        /
	  \                   +--/--+ farther plane
	   \                  | /   |
	    \                 |/    | Center
		 \                /     |
		  \              /+-----+ nearer plane  - clipped!
	*/


	/* using farther plane:
	light volume: use farther plane causing volume not fully covered (filled in x)
	+------------------+
	|\                /|
	|x\              /x|
	|xx\            /xx|
	|xxx\          /xxx|
	+----\        /----+
	  minRay    maxRay
	*/

	//screen pos:
	posVS.z -= halfSize.z;	//use farther plane, to avoid near clip (xy outrange[-1,1])
	float4 projCenter = mul(posVS, projMatrix);
	projCenter /= projCenter.w;
	//farther plane is too small to cover the volume, scale it to the same projection size of nearer plane
	float scale = output.depthRange.y / output.depthRange.x;
	posVS.xy += halfSize.xy*scale;
	float4 projMax = mul(posVS, projMatrix);
	projMax /= projMax.w;

	pos.xy = pos.xy*(projMax.xy - projCenter.xy) + projCenter.xy;
#endif

	pos = float4(pos.xy, 0, 1);	//any depth will be OK because we only need the ray. all depth values with the same xy are in the same ray

#if !defined(USE_UNPROJECTION)
	float4 unProjectedPos = mul(pos, inv_viewproj_matrix);
	float3 worldPos = unProjectedPos.xyz / unProjectedPos.w;
#endif

	output.pos = float4(pos.xy, 1, 1);
	
	float2 uv = Blade_Clip2UV(output.pos).xy;
	output.colorUV = Blade_TransformUVSpace(uv, colorUVSpace);
	output.normalUV = Blade_TransformUVSpace(uv, normalUVSpace);
	output.depthUV = Blade_TransformUVSpace(uv, depthUVSpace);
	output.aoUV = Blade_TransformUVSpace(uv, aoUVSpace);

#if defined(USE_UNPROJECTION)
	output.dir.xyz = output.pos.xyz;
#else
	//DO NOT normalize, or it will break direction (with length) interpolation
	output.dir.xyz = worldPos - eye_position.xyz;
#endif
	return output;
}

half4 LightPSMain(
	in DeferredShadingVSOutput input
	,uniform float4 eye_position : EYE_POS	//eye pos in world space 
	,uniform float3 viewDir : VIEW_DIR	//look at direction in world space
	,uniform float4 cameraNearFar : CAMERA_DEPTH
	,uniform float4x4 inv_viewporj_matrix : INV_VIEWPROJ_MATRIX
#if !defined(DIRECTIONAL)
	,uniform float4 depthFactor : VIEW_DEPTH_FACTOR
	,uniform float3 lightPos : DEFERRED_LIGHT_POSITION
	,uniform float4 lightAttenuation : DEFERRED_LIGHT_ATTENUATION
	,uniform half4 lightDiffuse : DEFERRED_LIGHT_DIFFUSE
	,uniform half4 lightSpecular : DEFERRED_LIGHT_SPECULAR
#	if defined(SPOT)
	,uniform half4 lightDir : DEFERRED_LIGHT_DIRECTION
#	endif
#endif
	,uniform sampler2D colorBuffer : RENDERBUFFER_INPUT0
	,uniform sampler2D normalBuffer : RENDERBUFFER_INPUT1
	,uniform sampler2D depthBuffer : RENDERBUFFER_INPUT2
	,uniform samplerShadow shadowBuffer : RENDERBUFFER_INPUT3
	,uniform sampler2D aoBuffer : RENDERBUFFER_INPUT4
	,uniform float4 shadowBufferSize : RENDERBUFFER_SIZE3
	) : COLOR0
{
	half3 worldRay = (half3)normalize(input.dir.xyz);

	//GBuffer depth
	float2 depthUV = input.depthUV;
	float depth = tex2D(depthBuffer, depthUV).r;	//INTZ
	float NDCz = Blade_ZBuffer2NDC(depth);
	float viewDepth = Blade_NDCZ2View(NDCz);		//linearize

#if !defined(DIRECTIONAL)
	if (viewDepth - input.depthRange.x < 0 || viewDepth - input.depthRange.y > 0)
	{
		clip(-1);
		return half4(0, 0, 0, 0);
	}
#endif

#if defined(USE_UNPROJECTION)
	float4 unprojectedPos = mul(float4(input.dir.xy, NDCz, 1), inv_viewporj_matrix);
	float3 worldPos = unprojectedPos.xyz / unprojectedPos.w;
	worldRay = (half3)normalize(worldPos - eye_position.xyz);
#else
	//4 instrcutions
	//matrix multiplication and homogeneous divide takes 5
	float3 worldPos = eye_position.xyz + worldRay * viewDepth / dot(worldRay, viewDir);
#endif

#if !defined(DIRECTIONAL)
	//early out to save performance
	float3 obj2Light = lightPos.xyz - worldPos;
	float distSQ = dot(obj2Light, obj2Light);
	float lightRange = -lightAttenuation.y / lightAttenuation.x;
	float rangeSQ = lightRange*lightRange;
	if (rangeSQ - distSQ < 0)
	{
		clip(-1);
		return half4(0, 0, 0, 0);
	}

	float dist = sqrt(distSQ);
	obj2Light /= dist;

#	if defined(SPOT)
	float3 dir = lightDir.xyz;
	float cosAngle = dot(-dir, obj2Light);
	float cosInner = lightAttenuation[2];
	float cosOuter = lightAttenuation[3];
	if (cosAngle - cosOuter < 0)
	{
		clip(-1);
		return half4(0, 0, 0, 0);
	}
#	endif

#endif

#if BLADE_AO_SHADING_INTEGRATION
	light_ambient *= (half4)tex2Dlod(aoBuffer, float4(input.aoUV,0,0)).rrrr;
#endif

	//GBuffer color
	float2 colorUV = input.colorUV;
	half4 albedo = (half4)tex2D(colorBuffer, colorUV);
#if defined(AMBIENT)
	return light_ambient*albedo;
#endif
	//GBuffer normal
	float2 normalUV = input.normalUV;
	half4 normal = (half4)tex2D(normalBuffer, normalUV);
	half3 worldNormal = Blade_ExpandNormal(normal);
#if defined(DIRECTIONAL)

	half4 shadow = 1;
#if BLADE_ENABLE_SHADOW
	if (viewDepth < cameraNearFar.y)
		shadow = Blade_GetShadow(shadowBuffer, shadowBufferSize, worldPos, viewDepth);
#	if defined(BLADE_SHADOW_DEBUG)
	return shadow;
#	endif
#endif

	half4 diffuse = light_ambient;
	half4 specular = half4(0,0,0,0);
	for(int i = 0; i < light_directional_count; ++i)
	{
		half3 light_dir = light_directions[i].xyz;
		//direcitonal lights doesn't need world position, thus doesn't need depth sampling
		half3 half_vec = (half3)normalize(light_dir + (-worldRay));
		
		half4 lighting = (half4)lit( dot(worldNormal, light_dir), dot(worldNormal, half_vec), normal.w*255) * shadow;
		diffuse += lighting.y*light_diffuses[i];
		specular+= lighting.y*lighting.z*light_speculars[i];
	}
	return (diffuse + specular*albedo.a)*albedo;
	//return albedo;
	//return half4(worldNormal.xyz, 1);
#else
	//linear attenuation
	float attenuation = saturate(dot(float2(dist, 1), lightAttenuation.xy));

#	if defined(SPOT)
	float falloff = saturate((cosAngle - cosOuter) / (cosInner - cosOuter));
	attenuation *= falloff;
#	endif

	half3 half_vec = (half3)normalize((half3)obj2Light + (-worldRay));
	half4 lighting = (half4)(lit(dot(worldNormal, (half3)obj2Light), dot(worldNormal, half_vec), normal.w * 255) * attenuation);

	return (lighting.y*lightDiffuse + lighting.z*lightSpecular*albedo.a)*albedo;
	//return float4(depth, depth, depth, 1);
#endif
}