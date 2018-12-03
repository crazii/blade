//!BladeShaderHeader
//![Shader]
//!Profiles=2_0,2_a,3_0
//!VSEntry=TerrainVSMain
//!FSEntry=TerrainPSMain
//!
//![SubShaders]
//!depth=DEPTH_ONLY
//!vertex_normal=USE_VERTEX_NORMAL
//!normal_map
//!detail_normal_map=ENABLE_DETAIL_NORMAL
//![SubShaders 2_0]
//!depth=DEPTH_ONLY
//!vertex_normal=USE_VERTEX_NORMAL
//![SubShaders 3_0]
//!depth=DEPTH_ONLY
//!vertex_normal=USE_VERTEX_NORMAL
//!normal_map=BLADE_DEFERRED_SHADING
//!detail_normal_map=BLADE_DEFERRED_SHADING,ENABLE_DETAIL_NORMAL,ENABLE_DYNAMIC_TILING

#include "inc/light.hlsl"
#include "inc/common.hlsl"
#include "inc/terrain_common.hlsl"
//#define BLADE_SHADOW_DEBUG 1	//define before depth_common.hlsl
#include "inc/depth_common.hlsl"

//parameter for ENABLE_DYNAMIC_TILING
#define DYNAMIC_TILING_DOWNSCALE 8	//far away tiling is 1/8
#define DYNAMIC_TILING_SCALE 2	//near tiling is DYNAMIC_TILING_DOWNSCALE*DYNAMIC_TILING_SCALE
#define DYNAMIC_TILING_DIST 50	//far away tiling distance, better match material LOD distance

struct TerrainVSOutput
{
	float4 pos : POSITION;
#if !defined(DEPTH_ONLY)
	half4 uv : TEXCOORD0;
	half2 blendUV : TEXCOORD1;
	float4 worldPos : TEXCOORD2;
#	if defined(USE_VERTEX_NORMAL)
	half3 worldNormal : TEXCOORD3;
#	endif
	float dist : TEXCOORD4;
#endif
};

TerrainVSOutput TerrainVSMain(
	float4 hpos		: POSITION0
	,float4 vpos	: POSITION1
#if defined(USE_VERTEX_NORMAL)
	,half4 normal	: NORMAL0
#endif
	,uniform float4x4 wvp_matrix : WORLD_VIEWPROJ_MATRIX
	,uniform float4 world_translate : WORLD_POSITION
#if !defined(BLADE_DEFERRED_SHADING)
	,uniform float4x4 view_matrix : VIEW_MATRIX
#endif
	,uniform float4 scaleFactor
	,uniform float4 UVInfo
	,uniform float4 atlasInfo
	,uniform float4 blendInfo
	,uniform float4 cameraPos
	)
{
	float blendOffset = UVInfo[0];
	float tileSize = UVInfo[1];
	float blockSize = UVInfo[2];
	float blockUVMultiple = UVInfo[3] / DYNAMIC_TILING_DOWNSCALE;	//note: down tiling for all shaders, or tiling will be different between material LODs

	TerrainVSOutput output;

	hpos.xy = hpos.xy * blockSize + hpos.zw;
	float4 pos = float4(hpos.x, getMorphHeight(vpos.xy/32767.0f, hpos.xy+world_translate.xz, cameraPos.xz), hpos.y, 1);
	pos = pos*scaleFactor;
	output.pos = mul(pos, wvp_matrix);

#if defined(DEPTH_ONLY)
	return output;
#else
	output.dist = distance(pos.xyz + world_translate.xyz, cameraPos.xyz);

	//normalUV
	output.uv.xy = (half2)(pos.xz/tileSize);	//now the normal /blend map sampler addressing uses CLAMP, which should fix the border
	//block repeat UV
#	if BLADE_TERRAIN_ENABLE_UVSCALE	//scale tiling based on height
	output.uv.zw = (half2)((vpos.zw)*blockUVMultiple / blockSize);
#	else
#		if BLADE_PROFILE == profile_2_0
	output.uv.zw = (half2)(hpos.zw / blockSize);
	output.uv.zw = (half2)clamp(output.uv.zw, 0.5 * atlasInfo[3], 1 - 0.5 * atlasInfo[3]);
#		else
	output.uv.zw = (half2)(pos.xz*blockUVMultiple / blockSize);
#		endif
#	endif

	//blendUV
	output.blendUV.xy = (half2)(pos.xz/tileSize);
	if (hpos.z == 0)
		output.blendUV.x += (half)(0.5*blendInfo[1]);
	if(hpos.z == blockSize)
		output.blendUV.x -= (half)(0.5*blendInfo[1]);
	if (hpos.w == 0)
		output.blendUV.y += (half)(0.5*blendInfo[1]);
	if (hpos.w == blockSize)
		output.blendUV.y -= (half)(0.5*blendInfo[1]);

	//don't use full transform because our terrain has no scale/rotation
	output.worldPos.xyz = pos.xyz+world_translate.xyz;
	output.worldPos.w = 1;
#	if !defined(BLADE_DEFERRED_SHADING) && BLADE_ENABLE_SHADOW
	output.worldPos.w = Blade_ForwardShadowViewDistance(output.worldPos.xyz, view_matrix);
#	endif

#	if defined(USE_VERTEX_NORMAL)
	//ubytes4 normal ranges 0-1, need convert to [-1,1]
	output.worldNormal = Blade_ExpandVector(normal).xyz;
#	endif
#endif//DEPTH_ONLY
	return output;
}

//atlasInfo[0]: atlas sub texture count per side
//atlasInfo[1]: sub texture size
//atlasInfo[2]: max mip level
//atlasInfo[3]: total texture size

//uvs: xy are blendmap uv, zw are atlas sub texture uv (inside the sub texture)
//atlas: four components atlas index,each component for each layer.
//the index is packed as rowIndex*atlasInfo[0]+colIndex, in which (rowIndex,colIndex) is the sub texture index of the whole atlas

//forward shadow
uniform samplerShadow shadowBuffer : RENDERBUFFER_INPUT0;
uniform float4 shadowBufferSize : RENDERBUFFER_SIZE0;

BladeFSOutput TerrainPSMain(
	in TerrainVSOutput input
	,uniform float4 atlasInfo
	,uniform sampler2D blendMap
	,uniform sampler2D normalMap
	,uniform sampler2D layerMap
	,uniform sampler2D diffAtlas
#if defined(ENABLE_DETAIL_NORMAL)
	,uniform sampler2D normalAtlas
#endif
	)
{
	BladeFSOutput output;
#if defined(DEPTH_ONLY)
	output.color = half4(0,0,0,0);
#else
	//sampling blend map
	half4 blend;
	//Note: need use point filter to work
	blend.rgb = (half3)tex2D(blendMap, input.blendUV.xy).rgb;

	//now the weight is pre calculated (averaged) in the CPU end, and uses 3 channel
	//calculate the 4th channel
	blend.a = 1 - dot(blend.rgb,1);
	
#if !defined(USE_VERTEX_NORMAL)
	//sampling object normal map (x,z stored and reconstruct y)
	half3 worldNormal = Blade_ExpandNormal2( (half4)tex2D(normalMap, input.uv.xy) ).xzy;
#else
	half3 worldNormal = input.worldNormal;
#endif
	worldNormal = (half3)normalize(worldNormal);
	
	//calculate atlas
	const float atlasCount = 4;	//layermap only support 4x4 atlas

	//calc 4 uv offset for 4 layers, within the atlas
	float4 atlas = tex2D(layerMap, input.blendUV.xy)*15;
	float4 v;
	float4 u = modf(atlas/atlasCount, v);
	v /= atlasCount;

#if BLADE_PROFILE == profile_2_0
	//shader model 2.0 disable tiling for block to avoid seam, so no wrapping
	float4 uv = input.uv.zwzw;
	float4 Coords = input.uv.zwzw;
#else
	float4 uv = float4(input.uv.zw, input.uv.zw * DYNAMIC_TILING_DOWNSCALE * DYNAMIC_TILING_SCALE);
	//wrap 
	float4 Coords = frac(uv);
#endif
	//get the uv from sub texture space to whole atlas space
	Coords = Coords / atlasCount;

	half4 color = half4(0,0,0,0);
#if BLADE_PROFILE == profile_3_0 || BLADE_PROFILE == profile_2_a || BLADE_PROFILE == profile_2_b
	//note: for ps_2_x and above we can set LINEAR filter for atlas sampler in .bmt
	float2 textureSize = atlasInfo[1];
	float maxMip = atlasInfo[2];

	//calc miplevel manually
#if defined(ENABLE_DYNAMIC_TILING)
	float2 mipLevel = Blade_CalcMipLOD2(uv, textureSize.x, maxMip);
#else
	float2 mipLevel = Blade_CalcMipLOD(uv.xy, textureSize.x, maxMip);
#endif

	//get texture size of target mip
	textureSize /= pow(2, mipLevel);

	//clamp off half texel to prevent altas bleeding
	float4 textureSize4 = textureSize.xxyy;
	Coords = clamp(Coords, 0.5 / (atlasCount*textureSize4), (textureSize4 - 0.5) / (atlasCount*textureSize4));
	//Coords = (Coords * (textureSize - 1) + 0.5) / (atlasCount*textureSize);

#	if defined(ENABLE_DETAIL_NORMAL)
	float3 tangentspaceNormal = float3(0,0,0);
#	endif
	for(int i = 0; i < 4; ++i)
	{
		float2 atlasUV;
		atlasUV.x = u[i] + Coords.x;
		atlasUV.y = v[i] + Coords.y;

		float2 atlasUV2;
		atlasUV2.x = u[i] + Coords.z;
		atlasUV2.y = v[i] + Coords.w;
#	if BLADE_PROFILE == profile_3_0

#	if defined(ENABLE_DYNAMIC_TILING)
		//half tilingMix = (half)saturate(pow(abs(input.dist) / DYNAMIC_TILING_DIST, 1.5));
		half tilingMix = (half)saturate(input.dist / DYNAMIC_TILING_DIST);
		//tilingMix *= tilingMix;
		half4 splat = (half4)lerp((half4)tex2Dlod(diffAtlas, float4(atlasUV2, 0, mipLevel[1])), (half4)tex2Dlod(diffAtlas, float4(atlasUV, 0, mipLevel[0])), tilingMix);
#else
		half4 splat = (half4)tex2Dlod(diffAtlas, float4(atlasUV, 0, mipLevel[0]));
#endif

		color += (half4)splat*blend[i];

#		if defined(ENABLE_DETAIL_NORMAL)
#			if defined(ENABLE_DYNAMIC_TILING)
			tangentspaceNormal += Blade_ExpandNormal2((half4)tex2Dlod(normalAtlas, half4(atlasUV2, 0, mipLevel[1]) ) ) * blend[i];
#			else
			tangentspaceNormal += Blade_ExpandNormal2((half4)tex2Dlod(normalAtlas, half4(atlasUV, 0, mipLevel[0]))) * blend[i];
#			endif
#		endif
#	else
		float2 dx = ddx(input.uv.zw / atlasCount);
		float2 dy = ddy(input.uv.zw / atlasCount);
		color += (half4)tex2D(diffAtlas, atlasUV, dx, dy )*blend[i];
#		if defined(ENABLE_DETAIL_NORMAL)
		tangentspaceNormal += Blade_ExpandNormal2( (half4)tex2D(normalAtlas, atlasUV, dx, dy) ) * blend[i];
#		endif
#	endif
	}

#	if defined(ENABLE_DETAIL_NORMAL)
	half3 tangent = half3(1,0,0);
	half3 bitangent = (half3)normalize( cross(tangent, worldNormal) );	//right handed
	tangent = (half3)normalize((half3)cross(bitangent, worldNormal) );
	//float3 bitangent = normalize( cross(worldNormal, tangent) );	//left handed
	//tangent = (half3)normalize((half3)cross(worldNormal, bitangent));
	half3x3 TBN = half3x3(tangent, bitangent, worldNormal);
	worldNormal = (half3)normalize(mul(tangentspaceNormal, TBN));
#	endif//ENABLE_DETAIL_NORMAL

#else //shader profile
	//note: for ps_2_0 we need set POINT filter for atlas sampler in .bmt

	for(int i = 0; i < 4; ++i)
	{
		float2 atlasUV;
		atlasUV.x = u[i] + Coords.x;
		atlasUV.y = v[i] + Coords.y;
		color += (half4)tex2D(diffAtlas, atlasUV )*blend[i];
	}
#endif
	half specularLevel = 0.8;
	half specularGloss = 32;

#if defined(BLADE_DEFERRED_SHADING)
	output.color = half4(color.rgb, specularLevel);
	output.normal = half4(Blade_PackVector(worldNormal), specularGloss/255);
#else

	half4 shadow = 1;
#if (BLADE_ENABLE_SHADOW)
	shadow = Blade_GetShadow(shadowBuffer, shadowBufferSize, input.worldPos.xyz, input.worldPos.w);
#	if defined(BLADE_SHADOW_DEBUG)
	output.color = shadow;
	return output;
#	endif
#endif

	output.color = Blade_CalculateLight(input.worldPos.xyz, worldNormal*shadow.xyz, color, half4(1, 1, 1, specularLevel), specularGloss);
#endif

#endif//DEPTH_ONLY
	return output;
}