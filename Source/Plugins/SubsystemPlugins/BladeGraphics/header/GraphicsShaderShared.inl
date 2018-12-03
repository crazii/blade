#ifndef __Blade_GraphicsShaderShared_inl__
#define __Blade_GraphicsShaderShared_inl__

//note: those constants are shared by HLSL & C++ codes, changing them will need to rebuild tools and recompile shaders.

#define BLADE_BONE_PALETTE_SIZE 110

//deferred directional lights
#define BLADE_MAX_DEFFERD_DIRECTIONAL_LIGHT_COUNT 16	//about 80 max

//dynamic textures
#define BLADE_MAX_DYNAMIC_TEXTURE_COUNT 8

//forward shading lights
#define BLADE_MAX_LIGHT_COUNT 8		//max local light count (for SM 3.0+) (SM 20 has no local lights, SM 2a has 4 lights @see light.hlsl)
#define MAX_GLOBAL_LIGHT_COUNT 1	//global light

#define MAX_SHADOW_CASCADE 4

#define BLADE_ENABLE_SHADOW 1

//apply AO only to ambient while shading. when enabled, AO pass should be executed before shading pass in render scheme scripts
#define BLADE_AO_SHADING_INTEGRATION 1

#endif // __Blade_GraphicsShaderShared_inl__