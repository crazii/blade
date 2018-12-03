#ifndef __Blade_terrain_common_hlsl__
#define __Blade_terrain_common_hlsl__

//LEVEL0_RADIUS
//LEVEL0_SCALE
//LEVEL0_DISTANCE
#include "../../../../Plugins/GraphicsPlugins/BladeTerrain/header/TerrainShaderShared.inl"

//height contains the current vertex height and a low LOD height
float getMorphHeight(float2 height, float2 pos, float2 camera_pos)
{
#if BLADE_PLATFORM == BLADE_PLATFORM_ANDROID || BLADE_PROFILE == profile_2_0	//disable morphing for Android.
	return height.x;
#else
	float dist = distance(pos, camera_pos);
	//int LOD = round( abs(fmod(height.y*32767,8)) );
	//x*32767 = i*8 + f
	//x*4095.875 = i + f/8 => f = (x*4095.875-i)*8
	int LOD = (int)round(frac(height.y*4095.875) * 8);

	//if( LOD >= 5 || LOD < 0)	//debug
	//	return 999;

	float LODDist = LOD_DISTANCES[LOD];
	float LODPrevDist = PREV_LOD_DISTANCES[LOD];
	float morph = saturate((dist - LODPrevDist) / (LODDist - LODPrevDist));

	return lerp(height.x,height.y,morph);
#endif
}


//binlinear sampling by shader, and blend between channels to avoid seams
//note: use mask will have better result
float3 tex2D_Bilinear( sampler2D s, float2 uv, float2 halfTexelSize, float textureSize )
{
	float scale = 0.6;

    float3 p0q0 = tex2D(s, uv - halfTexelSize ).rgb;
	float3  weight = 1 - step(p0q0, 0)*scale;

    float3 p1q0 = tex2D(s, uv + float2(halfTexelSize.x, 0) ).rgb;
	weight *= 1 - step(p1q0, 0 )*scale;

    float3 p0q1 = tex2D(s, uv + float2(0, halfTexelSize.y) ).rgb;
	weight *= 1 - step(p0q1, 0 )*scale;

    float3 p1q1 = tex2D(s, uv + halfTexelSize ).rgb;
	weight *= 1 - step(p1q1, 0 )*scale;

    float2 lrp = frac( (uv+halfTexelSize) * textureSize );
    float3 pInterp_q0 = lerp( p0q0, p1q0, lrp.x );
    float3 pInterp_q1 = lerp( p0q1, p1q1, lrp.x );
	return lerp( pInterp_q0, pInterp_q1, lrp.y )*weight;
}

#endif //__Blade_terrain_common_hlsl__