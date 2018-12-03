/********************************************************************
	created:	2011/05/21
	filename: 	AABBTransformSet.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_AABBTransformSet_h__
#define __Blade_AABBTransformSet_h__
#include <utility/BladeContainer.h>
#include <interface/public/graphics/IVertexBuffer.h>
#include <interface/public/graphics/IIndexBuffer.h>
#include <interface/public/graphics/IVertexSource.h>
#include <interface/public/graphics/IVertexDeclaration.h>
#include <interface/public/IRenderable.h>
#include <interface/IGraphicsUpdater.h>
#include <interface/IRenderQueue.h>
#include <SpaceContent.h>
#include <MaterialInstance.h>
#include "HelperRenderType.h"

namespace Blade
{
	class IAABBTarget;

	class AABBTransformSet : public SpaceContent, public IRenderable, public IGraphicsUpdatable, public Allocatable
	{
	public:
		AABBTransformSet();
		~AABBTransformSet();

		/************************************************************************/
		/* SpaceContent                                                                     */
		/************************************************************************/
		/************************************************************************/
		/* SpaceContent override                                                                     */
		/************************************************************************/
		/*
		@describe add renderables to render buffer
		@return
		@note  this function will be called if CUF_*_VISIBLE is set
		*/
		virtual void			updateRender(IRenderQueue* queue)
		{
			if( mAABBCount > 0 )
				queue->addRenderable(this);
		}

		/************************************************************************/
		/* IRenderable interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual const GraphicsGeometry&		getGeometry() const		{return mGeometry;}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual RenderType&					getRenderType() const		{return HelperRenderType::getSingleton();}

		/*
		@describe
		@param
		@return
		*/
		virtual const MaterialInstance*		getMaterial() const			{return mMaterial;}

		/*
		@describe
		@param
		@return
		*/
		virtual const Matrix44&				getWorldTransform() const	{return Matrix44::IDENTITY;}

		/**
		@describe get hosted content
		@param
		@return
		*/
		virtual ISpaceContent*				getSpaceContent() const { return const_cast<ISpaceContent*>( static_cast< const ISpaceContent*>(this) ); }

		/************************************************************************/
		/* IGraphicsUpdatable interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual void	update(SSTATE state);

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		void			initialize(IGraphicsUpdater* queue);

		/** @brief  */
		bool			addContent(ISpaceContent* content, const Color& color);

		/** @brief  */
		bool			removeContent(ISpaceContent* content);

		/** @brief  */
		bool			changeColor(ISpaceContent* content, const Color& newColor);

		/** @brief  */
		bool			addTarget(IAABBTarget* target);

		/** @brief  */
		bool			removeTarget(IAABBTarget* target);

		/** @brief  */
		size_t			getSize() const;

		/** @brief  */
		IVertexBuffer*	getInstanceBuffer() const;

		/** @brief  */
		size_t			getValidInstanceBufferSize() const;

		/** @brief  */
		void			setupRenderResource();

		/** @brief  */
		void			setMaterial(const HMATERIALINSTANCE& hMatInstance)	{mMaterial = hMatInstance;}

	protected:
		typedef Map<ISpaceContent*, Color>	TargetContentSet;
		typedef Set<IAABBTarget*>	TargetSet;

		TargetContentSet	mContentRegistry;
		TargetSet			mTargets;
		HVBUFFER			mVertexBuffer;
		HVERTEXSOURCE		mVertexSource;
		HIBUFFER			mIndexBuffer;
		size_t				mBufferedCount;
		HVBUFFER			mInstanceBuffer;
		GraphicsGeometry	mGeometry;
		
		IGraphicsUpdater*	mUpdater;
		HVDECL				mVertexDecl;

		HMATERIALINSTANCE	mMaterial;
		size_t				mAABBCount;
	};
	

}//namespace Blade




#endif // __Blade_AABBTransformSet_h__