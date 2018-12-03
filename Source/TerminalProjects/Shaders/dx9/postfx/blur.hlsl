//!BladeShaderHeader
//![Shader]
//!Profiles=2_0,2_a,3_0
//![SubShaders]
//!gaussian
//!bilateral = [BladeBilateralVSMain BladeBilateralFSMain]

#include "../inc/common.hlsl"

//shared direction vector
uniform float2 sampleDir;

//shared color
uniform sampler2D inputColor : RENDERBUFFER_INPUT0;
uniform float4 inputColorUVSpace[3] : RENDERBUFFER_UVSPACE0;
uniform float4 inputColorSize : RENDERBUFFER_SIZE0;


struct BladeVSOutput
{
	float4	pos : POSITION;
	float2	uv	: TEXCOORD0;
	float4	uvs[7]	: TEXCOORD1;
};

BladeVSOutput BladeVSMain(
	float4 pos : POSITION
	,uniform float2 sampleOffsets[14]
)
{
	BladeVSOutput output;
	pos.zw = float2(0.5, 1);
	output.pos = pos;
	float2 uv = Blade_Clip2UV(pos).xy;
	uv = Blade_TransformUVSpace(uv, inputColorUVSpace[0]);
	output.uv = uv;

	//float2 min = inputColorUVSpace[2].xy;
	//float2 max = inputColorUVSpace[2].zw;

	float2 offsets[14];
	BLADE_UNROLL for (int i = 0; i < 14; i++)
	{
		offsets[i] = uv + sampleOffsets[i] * sampleDir * inputColorSize.zw;
		//rendering to sub view and sampling it back, need to clamp to sub view uv space
		//offsets[i] = clamp(offsets[i], min, max);
	}

	BLADE_UNROLL for (int j = 0; j < 7; ++j)
	{
		output.uvs[j].xy = offsets[j*2];
		output.uvs[j].zw = offsets[j*2+1];
	}
	return output;
}

float4 BladeFSMain(BladeVSOutput input
	, uniform float4 sampleWeights[15]
) : COLOR
{

	float4 accum = tex2D(inputColor, input.uv) * sampleWeights[14];
	
	BLADE_UNROLL for (int i = 0; i < 7; ++i)
	{
		accum += sampleWeights[i * 2] * tex2D(inputColor, input.uvs[i].xy);
		accum += sampleWeights[i * 2 + 1] * tex2D(inputColor, input.uvs[i].zw);
	}
	return accum;
}



/*-----------------------------------------------------------------------
Copyright (c) 2014, NVIDIA. All rights reserved.
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
* Neither the name of its contributors may be used to endorse
or promote products derived from this software without specific
prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
-----------------------------------------------------------------------*/

#define KERNEL_RADIUS 3
uniform float sharpness;

//depth
uniform sampler2D inputDepth : RENDERBUFFER_INPUT1;
uniform float4 inputDepthUVSpace : RENDERBUFFER_UVSPACE1;
uniform float4 inputDepthSize : RENDERBUFFER_SIZE1;


struct BladeBilateralVSOutput
{
	float4	pos : POSITION;
	float2	uv	: TEXCOORD0;
	float2	uvDepth : TEXCOORD1;
#if KERNEL_RADIUS <= 3
	float4	uvs[KERNEL_RADIUS*2] : TEXCOORD2;
#endif
};


BladeBilateralVSOutput BladeBilateralVSMain(float4 pos : POSITION0)
{
	BladeBilateralVSOutput output;

	pos.zw = float2(0, 1);
	output.pos = pos;
	float2 uv = Blade_Clip2UV(pos).xy;

	output.uv = (half2)Blade_TransformUVSpace(uv, inputColorUVSpace);
	output.uvDepth = (half2)Blade_TransformUVSpace(uv, inputDepthUVSpace);

#if KERNEL_RADIUS <= 3
	BLADE_UNROLL for (int i = 1; i <= KERNEL_RADIUS; ++i)
	{
		output.uvs[i-1].xy = output.uv + sampleDir * inputColorSize.zw * i;
		output.uvs[i-1].zw = output.uvDepth + sampleDir * inputDepthSize.zw * i;
	}

	BLADE_UNROLL for (int ri = 1; ri <= KERNEL_RADIUS; ++ri)
	{
		output.uvs[ri-1 + KERNEL_RADIUS].xy = output.uv - sampleDir * inputColorSize.zw * ri;
		output.uvs[ri-1 + KERNEL_RADIUS].zw = output.uvDepth - sampleDir * inputDepthSize.zw * ri;
	}
#endif
	return output;
}

float4 BilateralBlur(float2 uv, float2 uv2, float r, float4 center_c, float center_d, inout float w_total)
{
	float4  c = tex2D(inputColor, uv);
	float d = tex2D(inputDepth, uv2).r;

	const float BlurSigma = float(KERNEL_RADIUS) * 0.5;
	const float BlurFalloff = 1.0 / (2.0*BlurSigma*BlurSigma);

	float ddiff = (d - center_d) * sharpness;
	float w = exp2(-r*r*BlurFalloff - ddiff*ddiff);
	w_total += w;

	return c*w;
}

float4 BladeBilateralFSMain(BladeBilateralVSOutput input) : COLOR0
{
	float4  center_c = tex2D(inputColor, input.uv);
	float center_d = tex2D(inputDepth, input.uvDepth).r;

	float4  c_total = center_c;
	float w_total = 1.0;

#if KERNEL_RADIUS <= 3
	BLADE_UNROLL for (int i = 0; i < KERNEL_RADIUS; ++i)
	{
		float2 uv1 = input.uvs[i].xy;
		float2 uv2 = input.uvs[i].zw;
		c_total += BilateralBlur(uv1, uv2, i+1, center_c, center_d, w_total);
	}

	BLADE_UNROLL for (int j = KERNEL_RADIUS; j < KERNEL_RADIUS*2; ++j)
	{
		float2 uv1 = input.uvs[j].xy;
		float2 uv2 = input.uvs[j].zw;
		c_total += BilateralBlur(uv1, uv2, j + 1, center_c, center_d, w_total);
	}
#else
	//forth
	BLADE_UNROLL for (float r = 1; r <= KERNEL_RADIUS; ++r)
	{
		float2 uv1 = input.uv + sampleDir * inputColorSize.zw * r;
		float2 uv2 = input.uv + sampleDir * inputDepthSize.zw * r;
		c_total += BilateralBlur(uv1, uv2, r, center_c, center_d, w_total);
	}
	//back
	BLADE_UNROLL for (float rr = 1; rr <= KERNEL_RADIUS; ++rr)
	{
		float2 uv1 = input.uv - sampleDir * inputColorSize.zw * rr;
		float2 uv2 = input.uv - sampleDir * inputDepthSize.zw * rr;
		c_total += BilateralBlur(uv1, uv2, r, center_c, center_d, w_total);
	}
#endif

	return c_total / w_total;
}
