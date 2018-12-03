/********************************************************************
	created:	2017/8/1
	filename: 	HBAO.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <interface/public/graphics/PixelColorFormatter.h>
#include <RenderUtility.h>
#include <GraphicsShaderShared.inl>
#include <interface/IRenderSchemeManager.h>
#include <interface/IMaterialManager.h>
#include "HBAO.h"
#include "3rdparty/mtrand.h"

//ref: http://www.twistedsanity.org/rdimitrov/HBAO_SIGGRAPH08.pdf
//https://github.com/nvpro-samples/gl_ssao
//http://www.derschmale.com/2013/12/20/an-alternative-implementation-for-hbao-2/

namespace Blade
{
	const TString HBAO::NAME = BTString("HBAO");

	enum EAOPass
	{
		AOP_AO,
		AOP_3D,
		AOP_FINAL,
	};
#define MAX_DIRECTION_COUNT 16

	//note: render targets are always in linear color space
	//static const PixelFormat AO_FORMAT = PF_R16;
	static const PixelFormat AO_FORMAT = PF_R8;

	//////////////////////////////////////////////////////////////////////////
	HBAO::HBAO()
		:ImageEffectBase(NAME)
	{
		mMaterial = NULL;
		mSharpness = AO_DEF_BLURSHARPNESS;
		mAngleBias = AO_DEF_ANGLE_BIAS;
		mAOScale = AO_DEF_AO_SCALE;
		mAORange = AO_DEF_RANGE;
		mAOMaxDistance = AO_DEF_MAX_DISTANCE;
		mResolutionScale = AO_DEF_RESOLUTION;	//lower resolution (i.e. 0.5) will cause flickering, but be eliminated by TAA
		mDirectionCount = AO_DEF_DIR_COUNT;
		mStepCount = AO_DEF_STEP_COUNT;
		mSSAO3DRay = false;	//experimental
	}

	//////////////////////////////////////////////////////////////////////////
	HBAO::~HBAO()
	{
		BLADE_DELETE mMaterial;
	}

	//////////////////////////////////////////////////////////////////////////
	void HBAO::initialize()
	{
		mMaterial = BLADE_NEW MaterialInstance(IMaterialManager::getSingleton().getMaterial(BTString("ImageFX_HBAO")));
		mMaterial->loadSync();

		mMaterial->getActiveTechnique()->getShaderVariableHandle(mDirectionHandle, BTString("directions"), SCT_FLOAT4, MAX_DIRECTION_COUNT);
		mMaterial->getActiveTechnique()->getShaderVariableHandle(mDirection3DHandle, BTString("directions3d"), SCT_FLOAT4, MAX_DIRECTION_COUNT);
		mMaterial->getActiveTechnique()->getShaderVariableHandle(mParameterHandle, BTString("params"), SCT_FLOAT4, 3);

		this->initRandomTexture();
		this->setupDirections();
	}

	//////////////////////////////////////////////////////////////////////////
	void HBAO::shutdown()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void HBAO::prepareImpl()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	bool HBAO::process_inputoutput(const HTEXTURE& chainIn, const HTEXTURE& chainOut, const HTEXTURE& /*depth*/,
		const HTEXTURE* extraInputs, size_t inputCount, const HTEXTURE* extraOuputs, size_t /*outputCount*/)
	{
		if (extraInputs == NULL || inputCount != 2)	//depth & normal inputs
		{
			BLADE_UNREFERENCED(inputCount);
			assert(false);
			return false;
		}
#if BLADE_AO_SHADING_INTEGRATION
		BLADE_UNREFERENCED(chainOut);
		if (extraOuputs == NULL || extraOuputs[0] == NULL)
			return false;
#endif

		//find depth buffer
		size_t depthIndex = 0;
		for (; depthIndex < inputCount; ++depthIndex)
		{
			if (extraInputs[depthIndex]->getPixelFormat().isDepth())
				break;
		}
		if (depthIndex >= inputCount)
		{
			assert(false);
			return false;
		}
		size_t normalIndex = (depthIndex == 0) ? 1u : 0;

		HTEXTURE normal = extraInputs[normalIndex];
		HTEXTURE depth = extraInputs[depthIndex];
		HTEXTURE ao = extraOuputs[0];

		bool scaled = (mResolutionScale < 1 - Math::LOW_EPSILON);
		TEXCACHE scaledDepth;
		//TEXCACHE scaledNormal;

		TEXCACHE AOCache;
		{
			size_t w = chainIn->getWidth();
			size_t h = chainIn->getHeight();
			if(scaled)
			{
				w = (size_t)(w*mResolutionScale);
				h = (size_t)(h*mResolutionScale);

				//downscale depth & normal

				//note: normal is sampled only ONCE, so downscale can be skipped
				//scaledNormal.allocCache(this, normal->getPixelFormat(), w, h);
				//this->blit(normal, scaledNormal);
				//normal = scaledNormal.mCache;

				//blit depth not working, use R32F
				//cannot copy a depth stencil  to another depth stencil
				//but copy from a color to depth stencil is OK. or copy backward.
				//https://blogs.msdn.microsoft.com/wsdevsol/2013/10/24/why-cant-i-copy-the-depth-stencil-information-when-using-feature-level-of-9_x-on-windows-7-8/

				//update: it is OK to use shader to copy depth stencil by sampling another depth stencil,
				//the article above just applies to the API call "ID3D11DeviceContext::CopyResource"
				//the problem is that this BP_DEPTH pass disables ZTest, and for d3d9, Blade disables D3DRS_ZENABLE if ZTest is disabled,
				//but D3DRS_ZENABLE is state for "z buffering", which also disables ZWrite!
				//the problem now is fixed in D3DStaeBlock, if ZTest is disabled but ZWrite enabled, just enable D3DRS_ZENABLE and set ZFunc to always.
				scaledDepth.allocCache(this, depth->getPixelFormat(), w, h);
				this->blit(depth, scaledDepth, (Material*)NULL, BP_DEPTH);

				//scaledDepth.allocCache(this, PF_R32F, w, h);
				//this->blit(depth, scaledDepth /*, (Material*)NULL, BP_LINEARIZE_DEPTH2COLOR*/);	//TODO: linearize depth on blit?
				depth = scaledDepth.mCache;

#if BLADE_AO_SHADING_INTEGRATION
				AOCache.allocCache(this, AO_FORMAT, w, h);
				ao = AOCache.mCache;
#endif
			}
#if !BLADE_AO_SHADING_INTEGRATION
			AOCache.allocCache(this, AO_FORMAT, w, h);
			ao = AOCache.mCache;
#endif
		}

		ICamera* cam = this->getEffectCamera();
		float projectScaleX = (float)(std::tan(cam->getFieldOfView() / 2) * 2);
		float projectScaleY = projectScaleX / (float)ao->getWidth() * (float)chainIn->getHeight();

		//TODO: handle ortho projection
		float  params[12] = {
			(float)mDirectionCount,		(float)mStepCount,		1.0f / (1.0f - mAngleBias),		mAOMaxDistance,
			mAngleBias,					mAOScale,				mAORange,						1.0f / (mAORange*mAORange),
			projectScaleX,				projectScaleY,			0,								0,

		};
		mParameterHandle.setData(params, sizeof(params));

		HTEXTURE AO_inputs[2];
		AO_inputs[normalIndex] = normal;
		AO_inputs[depthIndex] = depth;

		this->blit(AO_inputs, inputCount, &ao, 1, mMaterial, 0, mSSAO3DRay ? AOP_3D : AOP_AO);
		this->blur(ao, (mResolutionScale < 0.75f) ? 1u : 1u, BT_BILATERAL, AO_inputs[depthIndex], mSharpness);

#if !BLADE_AO_SHADING_INTEGRATION
		//final pass with probable AO upscale
		HTEXTURE finalInputs[] = { chainIn, ao };
		this->blit(finalInputs, 2, &chainOut, 1, mMaterial, 0, AOP_FINAL);
#else
		if (scaled)
		{
			assert(ao != extraOuputs[0]);
			this->blit(ao, extraOuputs[0]);
		}
#endif

		depth.clear();
		normal.clear();
		ao.clear();
#if BLADE_AO_SHADING_INTEGRATION
		return false;
#else
		return true;
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	void HBAO::finalizeImpl()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void HBAO::initRandomTexture()
	{
		if (mMaterial == NULL)
			return;

		const size_t ditherNoiseSize = 4u;
		PixelFormat NOISE_FORMAT = IGraphicsResourceManager::getSingleton().getGraphicsConfig().ColorRGBA;
		NOISE_FORMAT.setLinearColor(true);

		IMG_ORIENT orient = IGraphicsResourceManager::getSingleton().getGraphicsConfig().TextureDir;

		HIMAGE img = IImageManager::getSingleton().createImage(ditherNoiseSize, ditherNoiseSize, IP_TEMPORARY, NOISE_FORMAT, orient, 1);
		uint8* data = img->getBits();
		//MersenneTwister noise
		MTRand mtRand(0);
		for (size_t i = 0; i < ditherNoiseSize; ++i)
		{
			uint8* line = data;
			for (size_t j = 0; j < ditherNoiseSize; ++j)
			{
				fp64 r1 = mtRand();
				fp64 r2 = mtRand();
				fp64 angle = (fp64)Math::TWICE_PI / (fp64)mDirectionCount * r1;
				Color c((scalar)std::cos(angle), (scalar)std::sin(angle), (scalar)r2);
				c += 1;
				c *= 0.5;
				ColorFormatter::writeColorLDR(line, c, NOISE_FORMAT);
				line += NOISE_FORMAT.getSizeBytes();
			}
			data += img->getPitch();
		}
		mNoiseTexture = IGraphicsResourceManager::getSingleton().createTexture(*img, 1);

		/*TextureStage* ts = */mMaterial->getTextureState()->addTextureStage(BTString("noiseTexture"), mNoiseTexture);
		//TODO: fix potential sampler mismatch for multi pass sub shaders
		mNoiseTexture->setSampler(Sampler(TAM_WRAP, TAM_WRAP, TAM_WRAP, TFM_POINT, TFM_POINT, TFM_POINT));
	}
	
	//////////////////////////////////////////////////////////////////////////
	void HBAO::setupDirections()
	{
		if (mMaterial == NULL)
			return;

		//const scalar angle = Math::TWICE_PI / (mDirectionCount-1);
		const scalar angle = Math::TWICE_PI / mDirectionCount;	// 0 & 2*PI are the same, don't use 2*PI

		Vector4 dirs[MAX_DIRECTION_COUNT];
		for (uint32 i = 0; i < mDirectionCount; ++i)
		{
			dirs[i] = Vector4::ZERO;
			dirs[i].x = std::cos(angle*i);
			dirs[i].y = std::sin(angle*i);
		}
		mDirectionHandle.setData(dirs, sizeof(dirs));

		if (mSSAO3DRay)
		{
			for (uint32 i = 0; i < mDirectionCount; ++i)
			{
				scalar z = (scalar)std::rand() / (scalar)RAND_MAX;
				z *= Math::HALF_PI;
				scalar xy = angle*i;
				scalar cosz = std::cos(z);
				dirs[i] = Vector3::ZERO;
				dirs[i].x = std::cos(xy);
				dirs[i].y = std::sin(xy);
				dirs[i].z = std::sin(z);
				dirs[i].w = cosz;
			}
			mDirection3DHandle.setData(dirs, sizeof(dirs));
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void HBAO::onConfigChange(void* data)
	{
		if (data == &mDirectionCount)
		{
			this->setupDirections();
			this->initRandomTexture();
		}
	}
	
}//namespace Blade