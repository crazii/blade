#ifndef __Blade_light_hlsl__
#define __Blade_light_hlsl__

//shared header
#include "../../../../Plugins/SubsystemPlugins/BladeGraphics/header/GraphicsShaderShared.inl"

half4 light_ambient : GLOBAL_AMBIENT;
float4 eye_position : EYE_POS;

#if BLADE_PROFILE == profile_2_a
#undef BLADE_MAX_LIGHT_COUNT
#define BLADE_MAX_LIGHT_COUNT 3	//shadow matrix takes too many constants
#endif

#if BLADE_PROFILE == profile_2_0
#undef MAX_GLOBAL_LIGHT_COUNT
#define MAX_GLOBAL_LIGHT_COUNT 1
static const float global_light_count = 1;
#else
float global_light_count : MAIN_LIGHT_COUNT;

//per-instance affecting object local lights
#	if BLADE_MAX_LIGHT_COUNT > 0
float light_count : LIGHT_COUNT;
float4 light_position[BLADE_MAX_LIGHT_COUNT] : LIGHT_POSITION;
half4 light_direction[BLADE_MAX_LIGHT_COUNT] : LIGHT_DIRECTION;
half4 light_attenuation[BLADE_MAX_LIGHT_COUNT] : LIGHT_ATTENUATION;
half4 light_diffuse[BLADE_MAX_LIGHT_COUNT] : LIGHT_DIFFUSE;
half4 light_specular[BLADE_MAX_LIGHT_COUNT] : LIGHT_SPECULAR;
#	endif
#endif

//global
half4 global_light_vector[MAX_GLOBAL_LIGHT_COUNT] : MAIN_LIGHT_VECTOR;
half4 global_light_diffuse[MAX_GLOBAL_LIGHT_COUNT] : MAIN_LIGHT_DIFFUSE;
half4 global_light_specular[MAX_GLOBAL_LIGHT_COUNT] : MAIN_LIGHT_SPECULAR;

//calculate lighting in world space
half4 Blade_CalculateLight(float3 worldPos, half3 worldNormal, half4 albedo, half4 specular, half gloss)
{
	half3 obj2eye_dir = (half3)normalize(eye_position.xyz - worldPos);
	half4 lightingDiffuse = light_ambient;
	half4 lightingSpecular = half4(0, 0, 0, 0);

	//global lights
	#if MAX_GLOBAL_LIGHT_COUNT == 1
	const int i = 0;
	#else
	for (int i = 0; i < global_light_count; ++i)
	#endif
	{
		half3 light_dir = global_light_vector[i].xyz;
		half3 half_vec = (half3)normalize(light_dir + obj2eye_dir);

		half4 lighting = (half4)lit(dot(worldNormal, light_dir), dot(worldNormal, half_vec), gloss);
		lightingDiffuse += lighting.y*global_light_diffuse[i];
		lightingSpecular += lighting.y*lighting.z*global_light_specular[i];
	}

	//no object light for ps_2_0 due to instruction limit
#if BLADE_PROFILE != profile_2_0 && BLADE_MAX_LIGHT_COUNT > 0
	//local lights
	for( int j = 0; j < min(BLADE_MAX_LIGHT_COUNT,light_count); ++j )
	{
		//calculate diffuse
		float3 obj2light_dir = float3(light_position[j].xyz - worldPos);
		float distance = length(obj2light_dir);
		obj2light_dir /= distance;
		half3 half_vec = (half3)normalize((half3)obj2light_dir + obj2eye_dir);

		float attenuation = saturate(dot(float2(distance,1),light_attenuation[j].xy));
		//spot light falloff
		attenuation *= light_position[j].w == 1 ? 1 : saturate((dot((half3)obj2light_dir, light_direction[j].xyz) - light_attenuation[j].w) / (light_attenuation[j].z - light_attenuation[j].w));
		
		half4 lighting = (half4)(lit( dot(worldNormal, (half3)obj2light_dir), dot(worldNormal,half_vec), gloss)*attenuation);
		lightingDiffuse += lighting.y*light_diffuse[j];
		lightingSpecular += lighting.z*light_specular[j];
	}
#endif
	return (lightingDiffuse + lightingSpecular * specular * specular.a)*albedo;
}

#endif//__Blade_light_hlsl__