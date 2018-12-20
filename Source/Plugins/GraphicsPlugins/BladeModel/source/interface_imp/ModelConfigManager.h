/********************************************************************
	created:	2013/04/09
	filename: 	ModelConfigManager.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_ModelConfigManager_h__
#define __Blade_ModelConfigManager_h__
#include <Singleton.h>
#include <interface/IModelConfigManager.h>
#include <interface/public/graphics/IVertexDeclaration.h>

#include "ModelType.h"
#include "ParallelAnimation.h"

//note: Android GLES shader doesn't work well for shortest path handling, dunno why, maybe related to dynamic branching
//HACK it at CPU end
//this bug is known on Qualcomm Adreno series by now (3.24.2015)
//tested on ARM Mali: no such problem
//note: bugs found in shader: dot(vec4,vec4) problem, no workaround in cpu (c++code) anymore.
//https://developer.qualcomm.com/forum/qdn-forums/maximize-hardware/mobile-gaming-graphics-adreno/28610
#if BLADE_PLATFORM == BLADE_PLATFORM_ANDROID
	#define DQ_GPU_SKINNING_HACK 0
	#include <GraphicsShaderShared.inl>
	#include <math/Half.h>

namespace Blade
{
	typedef struct SHalf4Weight
	{
		fp16	weight[4];
	}F16X4WEIGHTS;

};//namespace Blade

#else
	#define DQ_GPU_SKINNING_HACK 0
#endif


namespace Blade
{
	class IVertexSource;
	class IRenderScene;

	enum EModelVertexSourceIndex
	{
		MVSI_POSITION,			//position
		MVSI_NORMAL_TANGENT,	//tangent frame
		MVSI_UV0,
		MVSI_UV1,
		MVSI_UV2,
		MVSI_UV3,
		MVSI_BONEWEIGHTS,
		MVSI_BONEINDICES,
	};

	class ModelConfigManager : public IModelConfigManager, public Singleton<ModelConfigManager>
	{
	public:
		using Singleton<ModelConfigManager>::getSingleton;
		using Singleton<ModelConfigManager>::getSingletonPtr;
	public:
		ModelConfigManager();
		~ModelConfigManager();

		/************************************************************************/
		/* IModelConfigManager interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual IGraphicsType&	getStaticModelType(bool alpha = false) {return !alpha ? mModelType: mModelAlphaType;}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual IGraphicsType&	getSkinnedModelType(bool alpha = false) {return !alpha ? mSkinnedType : mSkinnedAlphaType;}

		/**
		@describe 
		@param
		@return
		*/
		virtual IGraphicsType&	getMovingStaticType() { return mMovingType; }

		/*
		@describe 
		@param
		@return
		*/
		virtual IPool*		getModelPool() const		{return mModelPool;}

		/*
		@describe 
		@param
		@return
		*/
		virtual void		setEditingMode(bool enable)	{mEditMode = enable;}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool		isEditingModeEnabled() const{return mEditMode;}

		/** @copydoc IModelManager::isUpdatingSkinnedSubmeshBound */
		virtual bool		isUpdatingSkinnedSubmeshBound() const	{return mUpdateSkinnedSubMeshBounding;}

		/**
		@describe 
		@param
		@return
		*/
		virtual const TString&	getIKConfiguration() const	{return mIKConfig;}

		/**
		  @describe 
		  @param
		  @return
		*/
		virtual bool		isMergingMeshBuffer() const { return mMergeMeshBuffer; }

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/**
		@describe 
		@param
		@return
		*/
		void				initialize();

		/**
		@describe 
		@param
		@return
		*/
		void				shutdown();

		/** @brief  */
		size_t				createParallelAnimationUpdater(IRenderScene* scene);
		/** @brief  */
		bool				releaseParallelAnimationUpdater(IRenderScene* scene);
		/** @brief  */
		ParallelAnimation*	getParallelAnimationUpdater(IRenderScene* scene);

		/** @brief  */
		const HVDECL&		getVertexDeclaration()	{return mVertexDecl;}

		/** @brief  */
		const HVDECL&		getVisualizerDeclaration()	{return mBoneVisualizerDecl;}

		/** @brief  */
		const HVDECL&		getCombinedBatchDeclarartion() { return mCombinedDecl; }

	protected:
		struct SceneParallelAnimation
		{
			IRenderScene* scene;
			ParallelAnimation* updater;
			RefCount count;
		};
		typedef Vector<SceneParallelAnimation> ParallelAnimationMap;
		ParallelAnimationMap mParallelAnimationMap;

		StaticModelType		mModelType;
		SkinnedModelType	mSkinnedType;
		StaticModelAlphaType	mModelAlphaType;
		SkinnedModelAlphaType	mSkinnedAlphaType;
		MovingStaticModelType	mMovingType;
		TString				mIKConfig;
		HVDECL				mVertexDecl;
		HVDECL				mCombinedDecl;
		HVDECL				mBoneVisualizerDecl;
		IPool*				mModelPool;
		bool				mEditMode;
		bool				mUpdateSkinnedSubMeshBounding;
		bool				mMergeMeshBuffer;
	};//class ModelConfigManager
	
}//namespace Blade

#endif //  __Blade_ModelConfigManager_h__