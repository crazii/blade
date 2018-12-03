/********************************************************************
	created:	2015/12/18
	filename: 	LightType.h
	author:		Crazii
	
	purpose:	light render type for deferred shading
*********************************************************************/
#ifndef __Blade_LightType_h__
#define __Blade_LightType_h__
#include <Singleton.h>
#include <RenderType.h>
#include <interface/public/graphics/GraphicsGeometry.h>
#include <interface/public/graphics/IIndexBuffer.h>
#include <interface/public/graphics/IVertexSource.h>
#include <interface/public/graphics/IVertexDeclaration.h>
#include <interface/IRenderQueue.h>
#include <FrameEvents.h>
#include "Light.h"
#include <BladeGraphics_blang.h>

namespace Blade
{
	class LightRenderable;

	class LightType : public RenderType
	{
	public:
		LightType()
			:RenderType(BTString(BLANG_LIGHT))
			,mMaterial(NULL)
		{
			mFixedFillMode = true;
		}

		~LightType()					{}

		/*
		@describe
		@param
		@return
		*/
		virtual Material*		getMaterial() const;

		/**
		@describe 
		@param
		@return
		*/
		virtual void			onRenderDeviceReady();

		/**
		@describe 
		@param
		@return
		*/
		virtual void			onRenderDeviceClose();

	protected:
		mutable Material*	mMaterial;
	};

	class LightRenderManager : public Singleton<LightRenderManager>
	{
	public:
		LightRenderManager();
		~LightRenderManager();

		/** @brief  */
		LightType&	getLightRenderType() const;

		/** @brief unit sphere */
		const GraphicsGeometry&	getLightSphere() const;

		/** @brief quad in projected space */
		const GraphicsGeometry&	getLightQuad() const;

		/** @brief unit cone */
		const GraphicsGeometry& getLightCone() const;

		/** @brief  */
		bool		enqueueLightQuad(IRenderQueue* queue, IRenderable* renderable);

		/** @brief  */
		const HMATERIALINSTANCE& getLightMaterial(ELightType type);

	protected:
		/** @brief  */
		bool		initialize();
		/** @brief  */
		bool		shutdown();

		mutable LightType	mLightType;
		friend class LightType;

		GraphicsGeometry	mSphereGeometry;
		HIBUFFER			mSphereIndexBuffer;
		HVERTEXSOURCE		mSphereVertexSource;

		GraphicsGeometry	mConeGeometry;
		const GraphicsGeometry*	mQuadGeometry;
		HIBUFFER			mConeIndexBuffer;
		HVERTEXSOURCE		mConeVertexSource;
		HMATERIALINSTANCE	mLightMaterials[LT_COUNT+1];

		FrameFlag			mQuadEnqueued;
	};
	
}//namespace Blade


#endif // __Blade_LightType_h__