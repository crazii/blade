/********************************************************************
	created:	2017/1/25
	filename: 	TAA.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <interface/public/graphics/IRenderTarget.h>
#include <interface/public/graphics/IGraphicsResourceManager.h>
#include <interface/ICamera.h>
#include <Material.h>
#include <MaterialInstance.h>
#include <Technique.h>
#include <interface/IMaterialManager.h>
#include "TAA.h"

namespace Blade
{
	namespace Impl
	{
		/** @brief 1 based index */
		static scalar haltonSequence(int prime, size_t index)
		{
			scalar r = 0.0f;
			scalar f = 1.0f;
			int i = (int)index;
			while (i > 0)
			{
				f /= prime;
				r += f * (i % prime);
				i = (int)std::floor(i / (scalar)prime);
			}
			return r;
		}

		static void initHalton_2_3(scalar* seq, size_t count)
		{
			for (size_t i = 0, n = count / 2; i != n; i++)
			{
				scalar u = haltonSequence(2, i + 1) - 0.5f;
				scalar v = haltonSequence(3, i + 1) - 0.5f;
				seq[2 * i + 0] = u;
				seq[2 * i + 1] = v;
			}
		}

		static scalar CatmullRom(scalar x)
		{
			float ax = std::fabs(x);
			if (ax > 1.0f)
				return ((-0.5f * ax + 2.5f) * ax - 4.0f) *ax + 2.0f;
			else
				return (1.5f * ax - 2.5f) * ax*ax + 1.0f;
		}

	}//namespace Impl

	const TString TemporalAA::NAME = BTString("TAA");
	static const TString HISTORY_TEXTURE = BTString("historyTexture");

	//////////////////////////////////////////////////////////////////////////
	TemporalAA::TemporalAA()
		:ImageEffectBase(NAME)
		,mIndex(0)
		,mHistoryIndex(0)
		,mSharpness(0.5f)
		,mMaterial(NULL)
	{
		Impl::initHalton_2_3(mHalton2x3_16, countOf(mHalton2x3_16));
	}

	//////////////////////////////////////////////////////////////////////////
	TemporalAA::~TemporalAA()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void TemporalAA::initialize()
	{
		//FIXME
		if (this->getSchemeProfile() < BTString("3_0") )	//need 3_0 profile or higher. TODO: detection by framework using material
		{
			this->setEnable(false);
			return;
		}

		mIndex = 0;
		Material* mat = IMaterialManager::getSingleton().getMaterial(BTString("TemporalAA"));
		mMaterial = BLADE_NEW MaterialInstance(mat);
		mMaterial->loadSync();
		mMaterial->getTextureState()->addTextureStage(HISTORY_TEXTURE, HTEXTURE::EMPTY);

		mMaterial->getActiveTechnique()->getShaderVariableHandle(mHandleHistoryMatrix, BTString("historyMatrix"), SCT_MATRIX44, 1);
		mMaterial->getActiveTechnique()->getShaderVariableHandle(mHandleSampleWeights, BTString("SampleWeights"), SCT_FLOAT1, 9);
		mMaterial->getActiveTechnique()->getShaderVariableHandle(mHandleLowpassWeights, BTString("LowpassWeights"), SCT_FLOAT1, 9);
		mMaterial->getActiveTechnique()->getShaderVariableHandle(mHandlePlusWeights, BTString("PlusWeights"), SCT_FLOAT1, 5);
	}

	//////////////////////////////////////////////////////////////////////////
	void TemporalAA::shutdown()
	{
		mHistory[0].clear();
		mHistory[1].clear();
		BLADE_DELETE mMaterial;
	}

	//////////////////////////////////////////////////////////////////////////
	void TemporalAA::prepareImpl()
	{
		IRenderTarget* target = this->getTarget();
		ICamera* cam = this->getSceneCamera();

		mProjectionMatrix = cam->getProjectionMatrix();
		//setup jitter matrix
		Matrix44 jitterMatrix = mProjectionMatrix;

		mIndex = (mIndex + 1) % 8;
		mJitter = *reinterpret_cast<Vector2*>(&mHalton2x3_16[mIndex * 2]);

		float jitterX = mJitter.x * 2.0f / target->getViewWidth();
		float jitterY = mJitter.y * -2.0f / target->getViewHeight();
		assert(!isnan(jitterX) && !isnan(jitterY));
		jitterMatrix[2][0] = jitterX;
		jitterMatrix[2][1] = jitterY;
		cam->setProjectionMatrix(jitterMatrix, true);
		this->applyCameraChange();
	}

	//////////////////////////////////////////////////////////////////////////
	bool TemporalAA::process(const HTEXTURE& input, const HTEXTURE& output)
	{
		if (mMaterial == NULL)	//force enabled if not supported?
			return false;

		//un-jitter projection

		if (this->ensureBuffer(mHistory[mHistoryIndex], input->getPixelFormat()))
			this->blit(input, mHistory[mHistoryIndex]);
		mMaterial->getTextureState()->getTextureStage(HISTORY_TEXTURE)->setTexture(mHistory[mHistoryIndex]);
		mHistoryIndex = (mHistoryIndex + 1) % 2;
		this->ensureBuffer(mHistory[mHistoryIndex], input->getPixelFormat());
		
		const Matrix44& reprojection = this->getReprojectionMatrix();
		mHandleHistoryMatrix.setData(&reprojection, sizeof(reprojection));

		float JitterX = mJitter.x;
		float JitterY = mJitter.y;

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable:4640)//construction of local static object is not thread-safe
#endif


		//samples
		static const Vector2 sampleOffsets[] = {
			Vector2(-1.0f, -1.0f),
			Vector2(0.0f, -1.0f),
			Vector2(1.0f, -1.0f),
			Vector2(-1.0f,  0.0f),
			Vector2(0.0f,  0.0f),
			Vector2(1.0f,  0.0f),
			Vector2(-1.0f,  1.0f),
			Vector2(0.0f,  1.0f),
			Vector2(1.0f,  1.0f),
		};

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(pop)
#endif

		float Weights[9];
		float WeightsLow[9];
		float WeightsPlus[5];
		float TotalWeight = 0.0f;
		float TotalWeightLow = 0.0f;
		float TotalWeightPlus = 0.0f;
		for (int i = 0; i < 9; i++)
		{
			float PixelOffsetX = sampleOffsets[i][0] - JitterX;
			float PixelOffsetY = sampleOffsets[i][1] - JitterY;

			if (mSharpness > 1.0f)
			{
				Weights[i] = Impl::CatmullRom(PixelOffsetX) * Impl::CatmullRom(PixelOffsetY);
				TotalWeight += Weights[i];
			}
			else
			{
				// Exponential fit to Blackman-Harris 3.3
				PixelOffsetX *= 1.0f + mSharpness* 0.5f;
				PixelOffsetY *= 1.0f + mSharpness* 0.5f;
				Weights[i] = std::exp(-2.29f * (PixelOffsetX* PixelOffsetX + PixelOffsetY* PixelOffsetY));
				TotalWeight += Weights[i];
			}

			// Low pass.
			PixelOffsetX = sampleOffsets[i][0] - JitterX;
			PixelOffsetY = sampleOffsets[i][1] - JitterY;
			PixelOffsetX *= 0.25f;
			PixelOffsetY *= 0.25f;
			PixelOffsetX *= 1.0f + mSharpness* 0.5f;
			PixelOffsetY *= 1.0f + mSharpness* 0.5f;
			WeightsLow[i] = std::exp(-2.29f * (PixelOffsetX* PixelOffsetX + PixelOffsetY* PixelOffsetY));
			TotalWeightLow += WeightsLow[i];
		}

		WeightsPlus[0] = Weights[1];
		WeightsPlus[1] = Weights[3];
		WeightsPlus[2] = Weights[4];
		WeightsPlus[3] = Weights[5];
		WeightsPlus[4] = Weights[7];
		TotalWeightPlus = WeightsPlus[0] + WeightsPlus[1] + WeightsPlus[2] + WeightsPlus[3] + WeightsPlus[4];

		for (size_t i = 0; i < countOf(Weights); ++i)
			Weights[i] /= TotalWeight;
		mHandleSampleWeights.setData(Weights, sizeof(Weights));

		for (size_t i = 0; i < countOf(WeightsLow); ++i)
			WeightsLow[i] /= TotalWeightLow;
		mHandleLowpassWeights.setData(WeightsLow, sizeof(WeightsLow));

		for (size_t i = 0; i < countOf(WeightsPlus); ++i)
			WeightsPlus[i] /= TotalWeightPlus;
		mHandlePlusWeights.setData(WeightsPlus, sizeof(WeightsPlus));

		//don't use history as input, test custom sampler constant
		//if use input, need to modify shader to add semantic RENDERBUFFER_INPUT0 to history
		//this->blit(mHistory, output);
		HTEXTURE outputs[] = { output, mHistory[mHistoryIndex] };
		this->setOutputBuffers(outputs, 2, HTEXTURE::EMPTY);
		this->drawQuad(mMaterial);

		//restore projection matrix : later draw calls use un-jittered projection matrix
		this->getSceneCamera()->setProjectionMatrix(mProjectionMatrix, false);
		this->applyCameraChange();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void TemporalAA::finalizeImpl()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void TemporalAA::onDisable()
	{
		mHistory[0].clear();
		mHistory[1].clear();
	}

	
}//namespace Blade