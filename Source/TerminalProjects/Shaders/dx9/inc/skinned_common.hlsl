#ifndef __Blade_skinned_common_hlsl__
#define __Blade_skinned_common_hlsl__

#include "common.hlsl"
//BLADE_BONE_PALETTE_SIZE
#include "../../../../Plugins/SubsystemPlugins/BladeGraphics/header/GraphicsShaderShared.inl"

float4 bone_dq[BLADE_BONE_PALETTE_SIZE*2] : BONE_PALETTE;

DQ fetchDQ(int index)
{
	DQ dq;
	dq.r = bone_dq[ index*2 ];
	dq.t = bone_dq[ index*2+1 ];
	return dq;
}

//skin vertex for 4 weights
float3 skin_vertex_weight4(float3 v, int4 bones, float4 weights)
{
    DQ dq = fetchDQ(bones[0]);
	float4 r0 = dq.r;

	DQ finalDQ;
    finalDQ.r = weights[0] * dq.r;
	finalDQ.t = weights[0] * dq.t;
	BLADE_UNROLL for(int i = 1; i < 4; ++i)
	{
		dq = fetchDQ(bones[i]);
		if( weights[i] != 0 )
		{
#if BLADE_PLATFORM == BLADE_PLATFORM_ANDROID
			//avoid bug on Qualcomm
			//note: it's not working because GLSL Optimizer will still optimize it to dot()
			//https://developer.qualcomm.com/forum/qdn-forums/maximize-hardware/mobile-gaming-graphics-adreno/28610
			float _dot = r0.x * dq.r.x + r0.y * dq.r.y + r0.z * dq.r.z + r0.w + dq.r.w;
#else
			float _dot = dot(r0, dq.r);
#endif
			float shortPath = (float)step(0, _dot) * 2 - 1;
			//float shortPath = _dot >= 0 ? 1 : -1;
			finalDQ.r += shortPath*weights[i] * dq.r;
			finalDQ.t += shortPath*weights[i] * dq.t;
		}
	}
	finalDQ = Blade_DQNormalize(finalDQ);

	return Blade_DQMul(v, finalDQ);
}


//skin vertex & tangent frame(quaternion) for 4 weights
void skin_vertex_tbn_weight4(inout float3 v, inout float4 tbn_quat, int4 bones, float4 weights)
{
    DQ dq = fetchDQ(bones[0]);
	float4 r0 = dq.r;

	DQ finalDQ;
    finalDQ.r = weights[0] * dq.r;
	finalDQ.t = weights[0] * dq.t;
	BLADE_UNROLL for(int i = 1; i < 4; ++i)
	{	
		DQ dqi = fetchDQ(bones[i]);
		if( weights[i] != 0.0 )
		{
#if BLADE_PLATFORM == BLADE_PLATFORM_ANDROID
			//avoid bug on Qualcomm, original qeustion posted by me. (Crazii)
			//note: it's not working because GLSL Optimizer will still optimize it to dot()
			//https://developer.qualcomm.com/forum/qdn-forums/maximize-hardware/mobile-gaming-graphics-adreno/28610
			float _dot = r0.x * dqi.r.x + r0.y * dqi.r.y + r0.z * dqi.r.z + r0.w + dqi.r.w;	
#else
			float _dot = dot(r0, dqi.r);
#endif
			float shortPath = (float)step(0, _dot) * 2 - 1;
			//float shortPath = _dot >= 0 ? 1 : -1;
			finalDQ.r += shortPath*weights[i] * dqi.r;
			finalDQ.t += shortPath*weights[i] * dqi.t;
		}

	}
	finalDQ = Blade_DQNormalize(finalDQ);

	v = Blade_DQMul(v, finalDQ);
	tbn_quat = Blade_QQMul(tbn_quat, finalDQ.r);
	//tbn_quat = normalize(tbn_quat);
}

#endif //__Blade_skinned_common_hlsl__