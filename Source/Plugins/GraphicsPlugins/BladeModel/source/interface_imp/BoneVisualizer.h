/********************************************************************
	created:	2015/11/16
	filename: 	BoneVisualizer.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_BoneVisualizer_h__
#define __Blade_BoneVisualizer_h__
#include <math/Matrix44.h>
#include <interface/public/graphics/IVertexSource.h>
#include <interface/public/graphics/IVertexDeclaration.h>
#include <RenderUtility.h>
#include <interface/public/IRenderable.h>
#include <interface/IRenderTypeManager.h>
#include <MaterialInstance.h>
#include <interface/ISkeletonResource.h>

namespace Blade
{
	class Model;
	class SingleBoneVisualizer;

	class BoneVisualizer : public IRenderable, public Allocatable
	{
	public:
		BoneVisualizer(const Model* model);
		~BoneVisualizer();

		/************************************************************************/
		/* IRenderable interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual RenderType&				getRenderType() const
		{
			return *IRenderTypeManager::getSingleton().getRenderType( IRenderTypeManager::HELPER_RENDER_TYPE );
		}

		/*
		@describe
		@param
		@return
		*/
		virtual const GraphicsGeometry&	getGeometry() const
		{
			return mGeometry;
		}

		/*
		@describe
		@param
		@return
		*/
		virtual const MaterialInstance*	getMaterial() const
		{
			return mMaterial;
		}

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

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/**
		@describe 
		@param
		@return
		*/
		bool			update();

		/**
		@describe 
		@param
		@return
		*/
		void			updateRender(IRenderQueue* queue);

		/** @brief  */
		inline index_t	getSelectedBone() const { return mSelectedBone; }
		/** @brief  */
		inline void		setSelectedBone(index_t boneIndex) { mSelectedBone = boneIndex; }

		/** @brief  */
		inline bool		isEnabled() const { return mEnabled; }
		/** @brief  */
		inline void		setEnable(bool enable) { mEnabled = enable; }

	protected:
		const Model*		mModel;
		index_t				mSelectedBone;
		HVERTEXSOURCE		mVertexSource;
		GraphicsGeometry	mGeometry;
		HMATERIALINSTANCE	mMaterial;
		SingleBoneVisualizer* mSelectedBoneVisualizer;
		bool				mEnabled;
	};//class BoneVisualizer


	class SingleBoneVisualizer : public IRenderable, public Allocatable
	{
	public:
		SingleBoneVisualizer(const Model* model, BoneVisualizer* visualizer) :mModel(model), mVisualizer(visualizer)
		{
			RenderUtility::GEOMETRY geom(mGeometry, mIndices, mVertexSource);
			RenderUtility::createUnitSphere(geom, 4);
		}

		/************************************************************************/
		/* IRenderable interface                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		virtual RenderType&				getRenderType() const
		{
			return *IRenderTypeManager::getSingleton().getRenderType(IRenderTypeManager::HELPER_RENDER_TYPE);
		}

		/*
		@describe
		@param
		@return
		*/
		virtual const GraphicsGeometry&	getGeometry() const
		{
			return mGeometry;
		}

		/*
		@describe
		@param
		@return
		*/
		virtual const MaterialInstance*	getMaterial() const
		{
			return mVisualizer->getMaterial();
		}

		/*
		@describe
		@param
		@return
		*/
		virtual const Matrix44&			getWorldTransform() const { return mBoneTransform; }

		/**
		@describe get hosted content
		@param
		@return
		*/
		virtual ISpaceContent*			getSpaceContent() const { return mVisualizer->getSpaceContent(); }

		/** @brief  */
		inline void				setBoneTransform(const BoneDQ& dq, const Vector3& scale)
		{
			dq.toMatrix(mBoneTransform);
			mBoneTransform = Matrix44::generateScale(scale) * mBoneTransform;
		}

	protected:
		const Model* mModel;
		BoneVisualizer*		mVisualizer;
		GraphicsGeometry	mGeometry;
		HVERTEXSOURCE		mVertexSource;
		HIBUFFER			mIndices;
		Matrix44			mBoneTransform;
	};
	
}//namespace Blade

#endif // __Blade_BoneVisualizer_h__
