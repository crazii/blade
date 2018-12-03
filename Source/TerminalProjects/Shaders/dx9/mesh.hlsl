//!BladeShaderHeader
//![Shader]
//!Profiles=2_0,2_a,3_0
//!VSEntry=MeshVSMain
//!FSEntry=MeshPSMain
//!
//![SubShaders]
//!static_diffuse
//!static_material_color_only=MATERIAL_COLOR_ONLY
//!static_depth=DEPTH_ONLY
//!static_depth_alphaclip=DEPTH_ONLY,ENABLE_ALPHA
//!static_normalmap=ENABLE_NORMAL_MAP
//!static_normal_specular=ENABLE_NORMAL_MAP,ENABLE_SPECULAR_MAP
//!
//!skin_diffuse=ENABLE_SKIN
//!skin_material_color_only=ENABLE_SKIN,MATERIAL_COLOR_ONLY
//!skin_depth=ENABLE_SKIN,DEPTH_ONLY
//!skin_depth_alphaclip=ENABLE_SKIN,DEPTH_ONLY,ENABLE_ALPHA
//!skin_normalmap=ENABLE_SKIN,ENABLE_NORMAL_MAP
//!skin_normal_specular=ENABLE_SKIN,ENABLE_NORMAL_MAP,ENABLE_SPECULAR_MAP
//!
//![SubShaders 2_0]
//!static_diffuse
//!static_material_color_only=MATERIAL_COLOR_ONLY
//!static_depth=DEPTH_ONLY
//!static_depth_alphaclip=DEPTH_ONLY,ENABLE_ALPHA
//!
//!skin_diffuse=ENABLE_SKIN
//!skin_material_color_only=ENABLE_SKIN,MATERIAL_COLOR_ONLY
//!skin_depth=ENABLE_SKIN,DEPTH_ONLY
//!skin_depth_alphaclip=ENABLE_SKIN,DEPTH_ONLY,ENABLE_ALPHA
//!
//![SubShaders 3_0]
//!static_diffuse=BLADE_DEFERRED_SHADING
//!static_material_color_only=MATERIAL_COLOR_ONLY
//!static_depth=DEPTH_ONLY
//!static_depth_alphaclip=DEPTH_ONLY,ENABLE_ALPHA
//!static_normalmap=BLADE_DEFERRED_SHADING,ENABLE_NORMAL_MAP
//!static_normal_specular=BLADE_DEFERRED_SHADING,ENABLE_NORMAL_MAP,ENABLE_SPECULAR_MAP
//!
//!skin_diffuse=BLADE_DEFERRED_SHADING,ENABLE_SKIN
//!skin_material_color_only=ENABLE_SKIN,MATERIAL_COLOR_ONLY
//!skin_depth=ENABLE_SKIN,DEPTH_ONLY
//!skin_depth_alphaclip=ENABLE_SKIN,DEPTH_ONLY,ENABLE_ALPHA
//!skin_normalmap=BLADE_DEFERRED_SHADING,ENABLE_SKIN,ENABLE_NORMAL_MAP
//!skin_normal_specular=BLADE_DEFERRED_SHADING,ENABLE_SKIN,ENABLE_NORMAL_MAP,ENABLE_SPECULAR_MAP
//!
//!alphablend_static_diffuse
//!alphablend_static_normalmap=ENABLE_NORMAL_MAP
//!alphablend_static_normal_specular=ENABLE_NORMAL_MAP,ENABLE_SPECULAR_MAP
//!alphablend_skin_diffuse=ENABLE_SKIN
//!alphablend_skin_normalmap=ENABLE_SKIN,ENABLE_NORMAL_MAP
//!alphablend_skin_normal_specular=ENABLE_SKIN,ENABLE_NORMAL_MAP,ENABLE_SPECULAR_MAP

#include "inc/light.hlsl"
#include "inc/common.hlsl"
#include "inc/skinned_common.hlsl"
#include "inc/material_color.hlsl"
#include "inc/depth_common.hlsl"

struct MeshVSOutput
{
	float4 pos : POSITION;
#if !defined(DEPTH_ONLY) || defined(ENABLE_ALPHA)
	half3 uv : TEXCOORD0;
#endif

#if !defined(DEPTH_ONLY)
#	if !defined(BLADE_DEFERRED_SHADING)
	float4 worldPos : TEXCOORD1;
#	endif
#	if defined(ENABLE_NORMAL_MAP)
	half3 tangent : TEXCOORD3;
	half3 bitangent : TEXCOORD4;
	half3 normal : TEXCOORD5;
#	else
	half3 worldNormal : TEXCOORD3;
#	endif
#endif
};

MeshVSOutput MeshVSMain(
	float4 pos		: POSITION
	,half4 tbn_quat : NORMAL0		//ubyte4-n compressed quaternion
	,half2 uv		: TEXCOORD0
#if defined(ENABLE_SKIN)
	,int4 bones		: BLENDINDICES0
	,float4 weights	: BLENDWEIGHT0
#endif
	,uniform float4x4 wvp_matrix : WORLD_VIEWPROJ_MATRIX
	,uniform float4x4 world_matrix : WORLD_MATRIX
#if !defined(BLADE_DEFERRED_SHADING)
	,uniform float4x4 view_matrix : VIEW_MATRIX
#endif
	)
{
	MeshVSOutput output;

	tbn_quat = Blade_ExpandVector(tbn_quat);
	half w = (half)sign(tbn_quat.w);	//store sign before transform TBN

#if defined(ENABLE_SKIN)
	skin_vertex_tbn_weight4(pos.xyz, (float4)tbn_quat, bones, weights);
#endif
	output.pos = mul(float4(pos.xyz,1), wvp_matrix);
#if !defined(DEPTH_ONLY) || defined(ENABLE_ALPHA)
	output.uv = half3(uv, 0);
#endif

#if defined(DEPTH_ONLY)

	return output;

#else
#	if !defined(BLADE_DEFERRED_SHADING)
	output.worldPos = mul(pos, world_matrix);
	#if BLADE_ENABLE_SHADOW
	output.worldPos.w = Blade_ForwardShadowViewDistance(output.worldPos.xyz, view_matrix);
	#endif
#	endif

#	if defined(ENABLE_NORMAL_MAP)
	//because the quaternion's interpolation is not linear
	//we need to extract the normal, tangent vector before pass to pixel shader.

	//normal map: extract tbn
	//Tangent = qmul(float3(1,0,0), tbn_quat);
	//Normal = qmul(float3(0,0,1), tbn_quat);
	//a little faster than above (-3 instructions)
	Blade_Q2AxesXZ_LP(tbn_quat, output.tangent, output.normal);

	//tangent space to world space
	//note: world_matrix MUST only have uniform scale, or we have to use senmatic T(M-1)
	output.tangent = (half3)normalize( mul(output.tangent, (half3x3)world_matrix) );
	output.normal = (half3)normalize( mul(output.normal, (half3x3)world_matrix) );
	output.bitangent = (half3)cross(output.normal, output.tangent) * w;
#	else
	//vertex normal
	//tangent space normal to object space normal

	//outWorldNormal = q2axisX(tbn_quat);
	//same instruction count as above
	output.worldNormal = Blade_QMul_LP(half3(0,0,1), tbn_quat);
	//then to world space
	output.worldNormal = (half3)normalize(mul(output.worldNormal, (half3x3)world_matrix));
#	endif
#endif
	return output;
}

//forward shadow
uniform samplerShadow shadowBuffer : RENDERBUFFER_INPUT0;
uniform float4 shadowBufferSize : RENDERBUFFER_SIZE0;

BladeFSOutput MeshPSMain(
	in MeshVSOutput input
	,uniform sampler2D diffuseMap
	,uniform sampler2D normalMap
	,uniform sampler2D specularMap
	)
{
	BladeFSOutput output;
#if defined(DEPTH_ONLY)
	#if defined(ENABLE_ALPHA)
	half clipa = (half)tex2D(diffuseMap, input.uv.xy).a - (half)0.45f;
	clip(clipa);
	#endif
	output.color = half4(0,0,0,0);
#else

#if defined(MATERIAL_COLOR_ONLY)
	half4 diffuse = diffuse_color;
#else
	half4 diffuse = (half4)tex2D(diffuseMap, input.uv.xy);
#endif

#if defined(ENABLE_NORMAL_MAP)
	half3x3 WorldTBN = half3x3(input.tangent, input.bitangent, input.normal);
	half3 tangentspaceNormal = Blade_ExpandNormal2( (half4)tex2D(normalMap, input.uv.xy) ).xyz;
	//tangentspaceNormal = normalize( tangentspaceNormal );
	half3 worldNormal = mul(tangentspaceNormal, WorldTBN);
	worldNormal = (half3)normalize(worldNormal);
#else
	half3 worldNormal = input.worldNormal;
#endif

#if defined(ENABLE_SPECULAR_MAP)
	half4 specular = (half4)tex2D(specularMap, input.uv.xy);
#else
	half4 specular = half4(1,1,1,0.5);
#endif
	half specularGloss = 32;

#if defined(BLADE_DEFERRED_SHADING)
	half specularLevel = dot(specular, half4(0.25,0.25,0.25,0.25));
	output.color = half4(diffuse.rgb, specularLevel);
	output.normal = half4(Blade_PackVector(worldNormal), specularGloss/255.0);
	//outColor = float4(worldNormal,1);
#else
	half4 shadow = 1;
#if (BLADE_ENABLE_SHADOW)
	shadow = Blade_GetShadow(shadowBuffer, shadowBufferSize, input.worldPos.xyz, input.worldPos.w);
#endif

	half4 color = Blade_CalculateLight(input.worldPos.xyz, worldNormal*shadow.xyz, diffuse, specular, specularGloss);
	output.color = half4(color.rgb, diffuse.a*blend_factor);
	//outColor = float4(worldNormal,1);
	//outColor = diffuse;
#endif
#endif//DEPTH_ONLY
	return output;
}