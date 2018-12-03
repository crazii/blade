/********************************************************************
	created:	2015/12/18
	filename: 	LightType.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <interface/IMaterialManager.h>
#include <interface/IRenderQueue.h>
#include <RenderUtility.h>
#include "RenderHelper/HelperRenderType.h"
#include "LightType.h"

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	Material*		LightType::getMaterial() const
	{
		if( mMaterial == NULL )
			mMaterial = IMaterialManager::getSingleton().getMaterial(BTString("Light"));
		return mMaterial;
	}

	//////////////////////////////////////////////////////////////////////////
	void			LightType::onRenderDeviceReady()
	{
		RenderType::onRenderDeviceReady();
		LightRenderManager::getSingleton().initialize();
	}

	//////////////////////////////////////////////////////////////////////////
	void			LightType::onRenderDeviceClose()
	{
		RenderType::onRenderDeviceClose();
		LightRenderManager::getSingleton().shutdown();
	}

	//////////////////////////////////////////////////////////////////////////
	LightRenderManager::LightRenderManager()
	{
		mQuadGeometry = NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	LightRenderManager::~LightRenderManager()
	{
		
	}

	//////////////////////////////////////////////////////////////////////////
	LightType&	LightRenderManager::getLightRenderType() const
	{
		return mLightType;
	}

	//////////////////////////////////////////////////////////////////////////
	const GraphicsGeometry&	LightRenderManager::getLightSphere() const
	{
		return mSphereGeometry;
	}

	//////////////////////////////////////////////////////////////////////////
	const GraphicsGeometry&	LightRenderManager::getLightQuad() const
	{
		//note: use projected space points directly, add full constants array for all lights. (float4 color[MAX_DIRLIGHT], float4 direction[MAX_DIRLIGHT])
		//this will uses one pass for all directional lights, with O(1) IO(buffer sampling/writing)
		//note: light volumes will cull faces if CCW, so we use quad facing inside
		return *mQuadGeometry;
	}

	//////////////////////////////////////////////////////////////////////////
	const GraphicsGeometry& LightRenderManager::getLightCone() const
	{
		return mConeGeometry;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		LightRenderManager::enqueueLightQuad(IRenderQueue* queue, IRenderable* renderable)
	{
		if( !mQuadEnqueued )
		{
			queue->addRenderable(renderable);
			mQuadEnqueued = true;
			return true;
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	const HMATERIALINSTANCE& LightRenderManager::getLightMaterial(ELightType type)
	{
		assert(type >= 0 && type <= LT_COUNT);
		return mLightMaterials[type];
	}

	//////////////////////////////////////////////////////////////////////////
	bool		LightRenderManager::initialize()
	{
		//note: create unit sphere, and set shader constants for position & radius, which uses 1 register
		RenderUtility::GEOMETRY sphere(mSphereGeometry, mSphereIndexBuffer, mSphereVertexSource);
		RenderUtility::createUnitSphere(sphere, 9);

		//note: create unit cone, and set world transform: scale for cone radius/length, position for apex, rotation for directly
		RenderUtility::GEOMETRY cone(mConeGeometry, mConeIndexBuffer, mConeVertexSource);
		RenderUtility::createCone(cone, 0.5, 1, 18);

		//pre-create
		RenderUtility::getUnitArrowHead(GA_NEG_Z);
		RenderUtility::getUnitArrowTail(GA_NEG_Z);

		mQuadGeometry = &RenderUtility::getUnitQuad(RenderUtility::FF_INSIDE);

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable:4640)//construction of local static object is not thread-safe
#endif

		static const TString LIGHT_SHADER_GROUPS[] =
		{
			BTString("directional"),
			BTString("point"),
			BTString("spot"),
			BTString("directional"), //special trick: deferred ambient
		};
		assert(countOf(LIGHT_SHADER_GROUPS) == (size_t)LT_COUNT+1 && "need update");

		static const TString LIGHT_HELPER_PASS = BTString("light bounding volume helper");
		static const TString DIRECTIONAL_HELPER_PASS = BTString("directional helper");

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(pop)
#endif

		for (int i = 0; i < LT_COUNT+1; ++i)
		{
			mLightMaterials[i].bind(BLADE_NEW MaterialInstance(LightRenderManager::getSingleton().getLightRenderType().getMaterial()));
			mLightMaterials[i]->setShaderGroup(LIGHT_SHADER_GROUPS[i]);
			if(i != LT_COUNT)
				mLightMaterials[i]->setDynamicPass(i == LT_DIRECTIONAL? DIRECTIONAL_HELPER_PASS : LIGHT_HELPER_PASS);
			mLightMaterials[i]->load();
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		LightRenderManager::shutdown()
	{
		mSphereIndexBuffer.clear();
		mSphereVertexSource.clear();

		mConeIndexBuffer.clear();
		mConeVertexSource.clear();
		for (int i = 0; i < LT_COUNT + 1; ++i)
		{
			mLightMaterials[i]->unload();
			mLightMaterials[i].clear();
		}
		return true;
	}

}//namespace Blade