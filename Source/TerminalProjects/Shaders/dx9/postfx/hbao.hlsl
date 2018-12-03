//!BladeShaderHeader
//![Shader]
//!Profiles=3_0
//![SubShaders]
//!ao_pass = [BladeVSMain BladeAOFSMain], AO_PASS, USE_UNPROJECTION
//!ao_3d = [BladeVSMain BladeAOFSMain], AO_PASS, USE_UNPROJECTION, AO_3D
//!final = [BladeVSMain BladeFSMain]

#include "../inc/common.hlsl"

//use matrix unprojectio to get position, this get actually position when TAA jitter applied
//disable it may cause more flickering when work with TAA
//#define USE_UNPROJECTION 1

#if defined(AO_PASS)
//depth
uniform sampler2D inputDepth : RENDERBUFFER_INPUT0;
uniform float4 inputDepthUVSpace : RENDERBUFFER_UVSPACE0;
uniform float4 inputDepthSize : RENDERBUFFER_SIZE0;

//normal
uniform sampler2D inputNormal : RENDERBUFFER_INPUT1;
uniform float4 inputNormalUVSpace : RENDERBUFFER_UVSPACE1;
uniform float4 inputNormalSize : RENDERBUFFER_SIZE1;

//noise
uniform sampler2D noiseTexture;
uniform float4 noiseTexture_Size;
uniform float4 noiseTexture_UVSpace;

uniform float3 furstumDirs[2] : VIEW_FRUSTUM_RAYS;	//view space rays

uniform float4 directions[16];
uniform float4 directions3d[16];
uniform float4 params[3];
#define dirCount params[0][0]
#define stepCount params[0][1]
#define aoMultiplier params[0][2]
#define aoDistLimit params[0][3]

#define angleBias params[1][0]
#define aoScale	params[1][1]
#define aoRange params[1][2]
#define aoRangeFalloff params[1][3]

#define aoPixelScale params[2].xy

#else

//color
uniform sampler2D inputColor : RENDERBUFFER_INPUT0;
uniform float4 inputColorUVSpace : RENDERBUFFER_UVSPACE0;

//ao
uniform sampler2D inputAO : RENDERBUFFER_INPUT1;
uniform float4 inputAOUVSpace : RENDERBUFFER_UVSPACE1;

#endif

struct BladeVSOutput
{
	float4	pos : POSITION;
#if defined(AO_PASS)

#	if defined(USE_UNPROJECTION)
	float2 cpos : TEXCOORD0;
#	else
	float3  dir : TEXCOORD0;
#	endif

	half2	uvDepth	: TEXCOORD1;
	half2	uvNormal : TEXCOORD2;
#else
	half2  uv : TEXCOORD0;
	half2  uvAO : TEXCOORD1;
#endif
};


BladeVSOutput BladeVSMain( 
	float4 pos		: POSITION
	,uniform float4x4 inv_proj_matrix : INV_PROJ_MATRIX)
{
	BladeVSOutput output;
	pos.zw = float2(0, 1);
	output.pos = pos;
	float2 uv = Blade_Clip2UV(pos.xy);

#if defined(AO_PASS)
	float4 unProjectedPos = mul(pos, inv_proj_matrix);
	float3 viewPos = unProjectedPos.xyz / unProjectedPos.w;

#	if defined(USE_UNPROJECTION)
	output.cpos = pos.xy;
#	else
	output.dir = viewPos / (-viewPos.z);
#	endif

	output.uvDepth = (half2)Blade_TransformUVSpace(uv, inputDepthUVSpace);
	output.uvNormal = (half2)Blade_TransformUVSpace(uv, inputNormalUVSpace);
#else
	output.uv = (half2)Blade_TransformUVSpace(uv, inputColorUVSpace);
	output.uvAO = (half2)Blade_TransformUVSpace(uv, inputAOUVSpace);
#endif
	return output;
}

#if defined(AO_PASS)

float4 BladeAOFSMain(BladeVSOutput input
	,uniform float4x4 viewMatrix : VIEW_MATRIX
	,uniform float4x4 projMatrix : PROJ_MATRIX
#if defined(USE_UNPROJECTION)
	,uniform float4x4 inv_vp_matrix: INV_VIEWPROJ_MATRIX
	,uniform float4 eye_position : EYE_POS	//eye pos in world space
	,uniform float3 viewDir : VIEW_DIR		//look at direction in world space
#endif
#if BLADE_PROFILE > profile_3_0
	,float2 screenPos : SV_Position
#elif BLADE_PROFILE == profile_3_0
	,float2 screenPos : VPOS
#else
	,float4 viewSize : VIEW_SIZE
#endif
) : COLOR0
{
#if !defined(USE_UNPROJECTION)
	//view space
	float viewDepth = Blade_ZBuffer2View(tex2Dlod(inputDepth, half4(input.uvDepth, 0, 0)).r);	//linearize
	float3 centerPos = input.dir * viewDepth;
#else
	//world space
	float depth = Blade_ZBuffer2NDC(tex2Dlod(inputDepth, half4(input.uvDepth, 0, 0)).r);
	float4 worldPos4 = mul(float4(input.cpos.xy, depth, 1), inv_vp_matrix);
	float3 centerPos = worldPos4.xyz / worldPos4.w;
	float viewDepth = dot(centerPos - eye_position.xyz, viewDir);
#endif

	//better set limit for large open world
	if (viewDepth >= aoDistLimit)
		return 1;
	float distFade = saturate((aoDistLimit - viewDepth) / max(aoDistLimit*0.05, 10));

	//note: use per-pixel normal instead of face normal, to save performance. TODO: render face normal buffer from depth, could probably be done when downscaling for half resolution
	//note: use view space by transform normal
	//in order to use world space, we need camera right/up vector in world space, and calc ray = worldRay + uv.x*right + uv.y*up; and sampling depth to reconsruct world pos for each sample
	//that is too slow even compared to this.
	half3 normal = Blade_ExpandNormal((half4)tex2Dlod(inputNormal, float4(input.uvNormal, 0, 0)) );
#if !defined(USE_UNPROJECTION)
	normal = (half3)mul(normal, (float3x3)viewMatrix);
#endif

#if defined(AO_3D)
#	if defined(USE_UNPROJECTION)
	float3 viewNormal = (half3)mul(normal, (float3x3)viewMatrix);
	float3 viewPos = mul( float4(centerPos,1), viewMatrix).xyz;
#	else
	float3 viewNormal = normal;
	float3 viewPos = centerPos;
#	endif
	float3 tangent = (float3)cross(viewNormal, float3(1, 0, 0));
	float3 bitangent = (float3)cross(viewNormal, tangent);
	float3x3 TBN = float3x3(tangent, bitangent, viewNormal);
#endif

#if BLADE_PROFILE < profile_3_0
	float2 screenPos = input.uvNormal.xy * viewSize.xy;
#endif

	float AO = 0;
	float4 random = tex2D(noiseTexture, screenPos * noiseTexture_Size.zw)*2 - 1;
	float2 pixelRange = float2(aoRange, aoRange) * aoPixelScale * inputDepthSize.xy / viewDepth;
	float2 pixelStep = pixelRange / (stepCount + 1);

	//dir of rays
	for (int i = 0; i < dirCount; ++i)
	{
#if defined(AO_3D)
		{
			//uniform dir on hemishpere
			float4 dir4 = directions3d[i];
			//float2 dir2 = float2(dir4.x*random.x - dir4.y*random.y, dir4.x*random.y + dir4.y*random.x) * dir4.w;
			float2 dir2 = dir4.xy * dir4.w;
			float3 dir3 = mul(float3(dir2, dir4.z), TBN);

			float3 pos = viewPos + dir3*(aoRange / stepCount);
			float4 pos4 = mul(float4(pos, 1), projMatrix);
			pos4 /= pos4.w;

			float4 uvDepth = float4(Blade_Clip2UV(pos4.xy), 0, 0);
			float depth = -Blade_ZBuffer2View(tex2Dlod(inputDepth, uvDepth).r);

			bool occluded = depth > pos.z - pos.z * 5e-3f;	//add depth based error to avoid flcikering on far mountains
			if (occluded)
			{
				pos.z = depth;
				float falloff = dot(pos - viewPos, pos - viewPos);
				AO += saturate(dot(viewNormal, dir3) - angleBias) * saturate(1 - falloff*aoRangeFalloff);// *log(aoScale);
			}
		}
#endif

		float2 dir = directions[i].xy;
		//rotate
		float2 uvDir = float2(dir.x*random.x - dir.y*random.y, dir.x*random.y + dir.y*random.x);
		float2 step = pixelStep * uvDir;
		//jittered start
		float2 offset = random.z * step + 1;

		//ray steps
		for (int j = 0; j < stepCount; ++j)
		{
			float2 snappedOffset = round(offset + step*j);

			float4 uvDepth = float4(input.uvDepth + snappedOffset * inputDepthSize.zw, 0, 0);
			float4 uvScreen = float4(input.uvNormal + snappedOffset * inputDepthSize.zw, 0, 0);

#if !defined(USE_UNPROJECTION)
			//view space
			float depth = Blade_ZBuffer2View(tex2Dlod(inputDepth, uvDepth).r);
			float3 pos = (furstumDirs[0].xyz + furstumDirs[1].xyz * uvScreen.xyz) * depth;
#else
			//world space
			float depth = Blade_ZBuffer2NDC(tex2Dlod(inputDepth, uvDepth).r);
			float4 pos4 = mul(float4(Blade_UV2Clip(uvScreen.xy), depth, 1), inv_vp_matrix);
			float3 pos = pos4.xyz / pos4.w;
#endif

			float3 dir = pos - centerPos;
			float distSQ = dot(dir, dir);
			//normalize
			dir *= rsqrt(distSQ);

			AO += saturate(dot(normal, dir) - angleBias) * saturate(1 - distSQ*aoRangeFalloff);
		}
	}

	AO *= distFade * aoMultiplier / (dirCount*stepCount);

	return 1 - saturate(AO*aoScale);
	//debug:
	//return float4(input.uvDepth, 0, 1);
	//return float4(furstumDirs[0].xyz + furstumDirs[1].xyz * float3(input.uvDepth, 0), 1);
	//return float4(input.dir, 1);
	//return float4(normalize(directions[i]), 0, 1);
}

#else

//final pass
float4 BladeFSMain(BladeVSOutput input) : COLOR0
{
	half4 color = (half4)tex2Dlod(inputColor, float4(input.uv, 0, 0));
	half4 ao = (half4)tex2Dlod(inputAO, float4(input.uvAO, 0, 0));
	return color * ao;
	//return ao;
}

#endif