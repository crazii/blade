/********************************************************************
created:	2017/10/20
filename: 	GrassRenderable.h
author:		Crazii
purpose:	
*********************************************************************/
#ifndef __Blade_GrassRenderable_h__
#define __Blade_GrassRenderable_h__
#include <interface/public/graphics/GraphicsGeometry.h>
#include <interface/public/graphics/IIndexBuffer.h>
#include <interface/public/graphics/IVertexSource.h>
#include <interface/public/IRenderable.h>
#include "GrassConfig.h"

namespace Blade
{
	class GrassCluster;

	class GrassRenderable : public IRenderable, public Allocatable
	{
	public:
		GrassRenderable(GrassCluster* content);
		~GrassRenderable();

		/************************************************************************/
		/* IRenderable interface                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		virtual RenderType&				getRenderType() const { return GrassConfig::getSingleton().getRenderType(); }

		/*
		@describe
		@param
		@return
		*/
		virtual const GraphicsGeometry&	getGeometry() const { return mGeometry; }

		/*
		@describe
		@param
		@return
		*/
		virtual const MaterialInstance*	getMaterial() const { return mMaterial; }

		/*
		@describe
		@param
		@return
		*/
		virtual const Matrix44&			getWorldTransform() const;

		/**
		@describe get hosted content
		@param
		@return
		*/
		virtual ISpaceContent*			getSpaceContent() const;

		/**
		@describe
		@param
		@return
		*/
		virtual const AABB&				getWorldBounding() const;

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		inline bool		isGeometryValid() const { return mGeometry.mInstanceCount != 0 && mMaterial != NULL; }

		/** @brief  */
		void			setupGeometry(index_t layer, const AABB& areaMeshBounding, const HIBUFFER& indices, const HVERTEXSOURCE& vsource, size_t instanceCount);

		/** @brief  */
		void			clearGeometry();
	protected:
		GrassCluster*		mContent;
		GraphicsGeometry	mGeometry;
		HMATERIALINSTANCE	mMaterial;
	};

}//namespace Blade 

#endif // __Blade_GrassRenderable_h__