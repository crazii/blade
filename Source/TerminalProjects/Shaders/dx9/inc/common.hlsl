#ifndef __Blade_common_hlsl__
#define __Blade_common_hlsl__

#if BLADE_D3D
#define BLADE_UNROLL	[unroll]
#define BLADE_LOOP		[loop]
#define BLADE_BRANCH	[branch]
#else
#define BLADE_UNROLL	
#define BLADE_LOOP		
#define BLADE_BRANCH	
#endif

#define MATH_PI	3.1415926536

//expand color[0,1] to vector [-1,1]
half4 Blade_ExpandVector(half4 v)
{
	return v * 255 / 128 - 1;	// v*2-1 doesn't support (0,0,1)
}

half4 Blade_PackVector(half4 v)
{
	return (v + 1) * 128 / 255;
}

half3 Blade_PackVector(half3 v)
{
	return (v + 1) * 128 / 255;
}


half3 Blade_ExpandNormal(half4 v)
{
	return (half3)normalize(Blade_ExpandVector(v).xyz);
}

//expand normal from 2 component
half3 Blade_ExpandNormal2(half4 v)
{
	half3 n;
	//note: BC5 implementation for dx9 is actually 'ATI2', which has the red & green swapped against BC5
	//so use v.gr instead of v.rg
#if BLADE_D3D9
	n.xy = v.gr * 255.0 / 128.0 - 1.0;
#else
	n.xy = v.rg * 255.0 / 128.0 - 1.0;
#endif
	//tangent space normal z always positive
	n.z = (half)sqrt( 1.0 - dot(n.xy,  n.xy) );
	return n;
}

//helper function to calc mipmap manually
float Blade_CalcMipLOD(float2 uv, float textureSize, float maxMip)
{
#if BLADE_PROFILE == profile_3_0 || BLADE_PROFILE == profile_2_a || BLADE_PROFILE == profile_2_b
    float2 dx = ddx(uv * textureSize);
    float2 dy = ddy(uv * textureSize);
    float d = max(dot(dx, dx), dot(dy, dy));
    return clamp(0.5 * log2(d), 0, maxMip);
#else
	return 0;
#endif
}

//2 component mipmap calc
float2 Blade_CalcMipLOD2(float4 uv, float textureSize, float maxMip)
{
#if BLADE_PROFILE == profile_3_0 || BLADE_PROFILE == profile_2_a || BLADE_PROFILE == profile_2_b
	float4 dx = ddx(uv * textureSize);
	float4 dy = ddy(uv * textureSize);
	float2 d = float2( max(dot(dx.xy, dx.xy), dot(dy.xy, dy.xy)), max(dot(dx.zw, dx.zw), dot(dy.zw, dy.zw)));
	return clamp(0.5 * log2(d), 0, maxMip);
#else
	return 0;
#endif
}

//helper function to calc mipmap manually
float Blade_CalcMipLOD(float2 dx, float2 dy, float maxMip)
{
#if BLADE_PROFILE == profile_3_0 || BLADE_PROFILE == profile_2_a || BLADE_PROFILE == profile_2_b
    float d = max(dot(dx, dx), dot(dy, dy));
    return clamp(0.5 * log2(d), 0, maxMip);
#else
	return 0;
#endif
}

float4 ViewDepthFactor : VIEW_DEPTH_FACTOR;

//depth to NDC [0,1] or [-1,1]
//use macros instead of shader constant to save performance
float Blade_ZBuffer2NDC(float z)
{
	//return z * ViewDepthFactor.z + ViewDepthFactor.w;
#if BLADE_GLES
	return z * 2 - 1;
#else
	return z;
#endif
}

float Blade_NDC2ZBuffer(float z)
{
	//return (z - ViewDepthFactor.w) / ViewDepthFactor.z;
#if BLADE_GLES
	return (z + 1) * 0.5f;
#else
	return z;
#endif
}

//zbuffer depth to linear view depth
float Blade_NDCZ2View(float z)
{
	return ViewDepthFactor.y / (z + ViewDepthFactor.x);	//NDC to viewZ (right handed viewZ < 0, and make it > 0 )
}

float Blade_ZBuffer2View(float z)
{
	z = Blade_ZBuffer2NDC(z);							//depth to NDC [0,1] or [-1,1]
	return Blade_NDCZ2View(z);
}


//float4 Blade_ClampDepth(inout float4 clipPos)
//{
//#if BLADE_D3D9 && defined(DEPTH_CLAMP)
//	//although OGL clip space z min is -1 (here should use clamp(x,-1.1)), GL uses GL_DEPTH_CLAMP and don't do in shader.
//	//d3d11 disables depth clip and don't do it in shader too.
//
//	//note: clamp z in vertex will change interpolations and may cause large triangle depth change
//	//especially when this vertex should be clipped by x/y clipping
//	//clamp max to 1 may cause triangle depth become smaller and cause unwanted shadows in depth
//	//clamp min to 0 may cause triangle dpeth become larger and missing shadows
//	//perform clamping min to 0 because we want object behind light frustum still cast shadows
//	float NDCZ = clipPos.z / clipPos.w;
//	NDCZ = max(NDCZ, 0);
//	clipPos.z = NDCZ * clipPos.w;
//#endif
//	return clipPos;
//}


//transform clip pos to uv
uniform float4 UVFlip : UV_FLIP;
float4 Blade_Clip2UV(float4 cpos)
{
	float4 uv = cpos;
	uv /= uv.w;
	uv.y *= UVFlip.x;
	uv.xy = uv.xy * 0.5 + 0.5;
	return uv;
}
float2 Blade_Clip2UV(float2 cpos)
{
	float2 uv = cpos;
	uv.y *= UVFlip.x;
	return uv * 0.5 + 0.5;
}

float4 Blade_UV2Clip(float4 uv)
{
	uv.xy = uv.xy*2 -1;
	uv.y *= UVFlip.x;
	return uv;
}
float2 Blade_UV2Clip(float2 uv)
{
	uv.xy = uv.xy*2 -1;
	uv.y *= UVFlip.x;
	return uv;
}

//trasform uv sapce while sampling for a screen buffer
float2 Blade_TransformUVSpace(float2 uv, float4 uvspace)
{
	return uv * uvspace.zw + uvspace.xy;
}

//transform & clamp. used for sampling sub region of screen texture
float2 Blade_TransformUVSpace(float2 uv, float4 full_space[3])
{
	uv = uv * full_space[0].zw + full_space[0].xy;
	uv = clamp(uv, full_space[2].xy, full_space[2].zw);	//clamp to region
	return uv;
}

//helper function to multiply(transform) a vector by a quaternion
float3 Blade_QMul(float3 v, float4 quaternion)
{
	float3 t = cross(quaternion.xyz,v)*2;
	return v + t*quaternion.w + cross(quaternion.xyz,t);
}

half3 Blade_QMul_LP(half3 v, half4 quaternion)
{
	half3 t = (half3)cross(quaternion.xyz, v) * 2;
	return v + t * quaternion.w + (half3)cross(quaternion.xyz, t);
}

void Blade_Q2Axes(float4 quaternion, out float3 x_axis, out float3 y_axis, out float3 z_axis)
{
	float fTx  = 2*quaternion.x;
	float fTy  = 2*quaternion.y;
	float fTz  = 2*quaternion.z;
	float fTwx = fTx*quaternion.w;
	float fTwy = fTy*quaternion.w;
	float fTwz = fTz*quaternion.w;
	float fTxx = fTx*quaternion.x;
	float fTxy = fTy*quaternion.x;
	float fTxz = fTz*quaternion.x;
	float fTyy = fTy*quaternion.y;
	float fTyz = fTz*quaternion.y;
	float fTzz = fTz*quaternion.z;

	x_axis[0] = 1-(fTyy+fTzz);
	x_axis[1] = fTxy+fTwz;
	x_axis[2] = fTxz-fTwy;

	y_axis[0] = fTxy-fTwz;
	y_axis[1] = 1-(fTxx+fTzz);
	y_axis[2] = fTyz+fTwx;

	z_axis[0] = fTxz+fTwy;
	z_axis[1] = fTyz-fTwx;
	z_axis[2] = 1-(fTxx+fTyy);
}

float3 Blade_Q2AxisX(float4 quaternion)
{
	float fTx  = 2*quaternion.x;
	float fTy  = 2*quaternion.y;
	float fTz  = 2*quaternion.z;
	float fTwx = fTx*quaternion.w;
	float fTwy = fTy*quaternion.w;
	float fTwz = fTz*quaternion.w;
	float fTxx = fTx*quaternion.x;
	float fTxy = fTy*quaternion.x;
	float fTxz = fTz*quaternion.x;
	float fTyy = fTy*quaternion.y;
	float fTyz = fTz*quaternion.y;
	float fTzz = fTz*quaternion.z;

	float3 x_axis;
	x_axis[0] = 1-(fTyy+fTzz);
	x_axis[1] = fTxy+fTwz;
	x_axis[2] = fTxz-fTwy;
	return x_axis;
}

void Blade_Q2AxesXZ(float4 quaternion, out float3 x_axis, out float3 z_axis)
{
	float fTx  = 2*quaternion.x;
	float fTy  = 2*quaternion.y;
	float fTz  = 2*quaternion.z;
	float fTwx = fTx*quaternion.w;
	float fTwy = fTy*quaternion.w;
	float fTwz = fTz*quaternion.w;
	float fTxx = fTx*quaternion.x;
	float fTxy = fTy*quaternion.x;
	float fTxz = fTz*quaternion.x;
	float fTyy = fTy*quaternion.y;
	float fTyz = fTz*quaternion.y;
	float fTzz = fTz*quaternion.z;

	x_axis[0] = 1-(fTyy+fTzz);
	x_axis[1] = fTxy+fTwz;
	x_axis[2] = fTxz-fTwy;

	z_axis[0] = fTxz+fTwy;
	z_axis[1] = fTyz-fTwx;
	z_axis[2] = 1-(fTxx+fTyy);
}

void Blade_Q2AxesXZ_LP(half4 quaternion, out half3 x_axis, out half3 z_axis)
{
	half fTx = 2 * quaternion.x;
	half fTy = 2 * quaternion.y;
	half fTz = 2 * quaternion.z;
	half fTwx = fTx * quaternion.w;
	half fTwy = fTy * quaternion.w;
	half fTwz = fTz * quaternion.w;
	half fTxx = fTx * quaternion.x;
	half fTxy = fTy * quaternion.x;
	half fTxz = fTz * quaternion.x;
	half fTyy = fTy * quaternion.y;
	half fTyz = fTz * quaternion.y;
	half fTzz = fTz * quaternion.z;

	x_axis[0] = 1 - (fTyy + fTzz);
	x_axis[1] = fTxy + fTwz;
	x_axis[2] = fTxz - fTwy;

	z_axis[0] = fTxz + fTwy;
	z_axis[1] = fTyz - fTwx;
	z_axis[2] = 1 - (fTxx + fTyy);
}


//helper function to multiply quaternion q1 by quaterion q0
//note standard quaternion multiplication order is left-right (q0*q1)
//here we reversed the parameter order just for convienence.
float4 Blade_QQMul(float4 quaternion1, float4 quaternion0)
{
	//q0 = (v0, w0);
	//q1 = (v1, w1);
	//q0*q1 = ( w0*v1 + w1*v0 + cross(v0,v1),  w0*w1 - dot(v0,v1)  );
	return float4(
		quaternion0.w*quaternion1.xyz + quaternion1.w*quaternion0.xyz + cross(quaternion0.xyz, quaternion1.xyz), //float3(x,y,z)
		quaternion0.w*quaternion1.w - dot(quaternion0.xyz, quaternion1.xyz)						//w
		);
}

//dual quaternion definition : use two float4 instead of matrix2x4 to avoid transpose in GLSL 
struct DQ
{
	float4 r;	//real part
	float4 t;	//dual part
};

//helper function to normalize dual quaternion
DQ Blade_DQNormalize(DQ dq)
{
	float4 r = dq.r;
	float len = length(r);
	dq.r /= len;
	dq.t /= len;
	return dq;
}

//helper function to transform a vector with a dual-quaternion
//note: the dual-quaternion should be normalized before call
float3 Blade_DQMul(float3 v, DQ dq)
{
	float3 translation = 2 * (dq.r.w * dq.t.xyz - dq.t.w * dq.r.xyz + cross(dq.r.xyz, dq.t.xyz));
	return Blade_QMul(v, dq.r) + translation;
}

float4x4 Blade_DQ2Matrix(DQ dq)
{
    float4x4 m;
    float length = dot(dq.r, dq.r);
    float x = dq.r.x, y = dq.r.y, z = dq.r.z, w = dq.r.w;
    float t1 = dq.t.x, t2 = dq.t.y, t3 = dq.t.z, t0 = dq.t.w;
        
    m[0][0] = w*w + x*x - y*y - z*z;
    m[1][0] = 2*x*y - 2*w*z;
    m[2][0] = 2*x*z + 2*w*y;
    m[0][1] = 2*x*y + 2*w*z;
    m[1][1] = w*w + y*y - x*x - z*z;
    m[2][1] = 2*y*z - 2*w*x;
    m[0][2] = 2*x*z - 2*w*y;
    m[1][2] = 2*y*z + 2*w*x;
    m[2][2] = w*w + z*z - x*x - y*y;
    
    m[3][0] = -2*t0*x + 2*t1*w - 2*t2*z + 2*t3*y;
    m[3][1] = -2*t0*y + 2*t1*z + 2*t2*w - 2*t3*x;
    m[3][2] = -2*t0*z - 2*t1*y + 2*t2*x + 2*t3*w;
    
    m[0][3] = 0.0;
    m[1][3] = 0.0;
    m[2][3] = 0.0;
    m[3][3] = 1.0;
    
    m /= length;
    return m;
}

#if defined(BLADE_DEFERRED_SHADING)

struct BladeFSOutput
{
	half4 color : COLOR0;
	half4 normal : COLOR1;
};

#else

struct BladeFSOutput
{
	half4 color : COLOR;
};

#endif

#endif