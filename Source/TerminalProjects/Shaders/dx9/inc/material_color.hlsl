#ifndef __Blade_material_color_hlsl__
#define __Blade_material_color_hlsl__

//instance color of each objects or pass

half4 diffuse_color : OBJECT_DIFFUSE;

half4 specular_color : OBJECT_SPECULAR;

half4 emissive_color : OBJECT_EMISSIVE;

half  blend_factor : BLEND_FACTOR;

#endif //__Blade_material_color_hlsl__
