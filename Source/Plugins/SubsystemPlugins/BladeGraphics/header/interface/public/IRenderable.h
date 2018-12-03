/********************************************************************
	created:	2010/04/30
	filename: 	IRenderable.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_IRenderable_h__
#define __Blade_IRenderable_h__
#include <math/Matrix44.h>
#include <math/DualQuaternion.h>
#include <interface/ISpaceContent.h>
#include <interface/public/graphics/GraphicsGeometry.h>
#include <RenderType.h>
#include <MaterialInstance.h>
#include <Technique.h>
#include <Pass.h>
#include <ShaderOption.h>

namespace Blade
{
	class IRenderable;

	namespace Impl
	{
		/* used by render scheme to set custom data to renderable */
		class IRenderData
		{
		public:
			virtual ~IRenderData() {}
		};
		typedef Handle<IRenderData> HRENDERDATA;

		const HRENDERDATA&  ensureRenderData(const IRenderable* renderable);
	}//namespace Impl

	class BLADE_GRAPHICS_API IRenderable
	{
	public:
		virtual ~IRenderable()		{}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual RenderType&				getRenderType() const = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual const GraphicsGeometry&	getGeometry() const = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual const MaterialInstance*	getMaterial() const = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual const Matrix44&			getWorldTransform() const = 0;

		/**
		@describe get hosted content, used for forward shading only.
		@param
		@return
		*/
		virtual ISpaceContent*			getSpaceContent() const = 0;

		/*
		@describe 
		@param
		@return
		*/
		virtual size_t					getBonePalette(const DualQuaternion*& palette) const	{palette = NULL; return 0;}

		/**
		@describe 
		@param
		@return
		*/
		virtual const AABB&	getWorldBounding() const { return this->getSpaceContent()->getWorldAABB(); }

	private:
		mutable Impl::HRENDERDATA mSchemeData;

		/** @brief  */
		inline void			setRenderData(const Impl::HRENDERDATA& data) const { mSchemeData = data; }
		/** @brief  */
		inline const Impl::HRENDERDATA& getRenderData() const { return mSchemeData; }

		friend const Impl::HRENDERDATA& Impl::ensureRenderData(const IRenderable* renderable);
	};//class IRenderable

}//namespace Blade


#endif //__Blade_IRenderable_h__