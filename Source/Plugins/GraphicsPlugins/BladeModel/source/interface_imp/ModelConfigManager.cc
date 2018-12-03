/********************************************************************
	created:	2013/04/09
	filename: 	ModelConfigManager.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "ModelConfigManager.h"
#include <interface/IPoolManager.h>
#include <interface/public/graphics/IGraphicsResourceManager.h>
#include <Pass.h>
#include <RenderUtility.h>
#include "ModelBatchCombiner.h"

namespace Blade
{

	//////////////////////////////////////////////////////////////////////////
	ModelConfigManager::ModelConfigManager()
	{
		mIKConfig = ModelConsts::IK_FULLBODY;
		mModelPool = BLADE_FACTORY_CREATE( IPool, BTString("Misc") );
		mModelPool->initialize(1, 16);

		mEditMode = false;
		mUpdateSkinnedSubMeshBounding = true;
		mMergeMeshBuffer = true;
	}

	//////////////////////////////////////////////////////////////////////////
	ModelConfigManager::~ModelConfigManager()
	{
		BLADE_DELETE mModelPool;
	}

	//////////////////////////////////////////////////////////////////////////
	void				ModelConfigManager::initialize()
	{
		if(mVertexDecl == NULL)
		{
			mVertexDecl = IGraphicsResourceManager::getSingleton().createVertexDeclaration();
			mVertexDecl->addElement(MVSI_POSITION,		0,	VET_HALF4,		VU_POSITION, 0);
			mVertexDecl->addElement(MVSI_NORMAL_TANGENT,0,	VET_UBYTE4N,	VU_NORMAL, 0);
#if DQ_GPU_SKINNING_HACK
			mVertexDecl->addElement(MVSI_BONEWEIGHTS,	0,	VET_HALF4,		VU_BLEND_WEIGHTS, 0);
#else
			mVertexDecl->addElement(MVSI_BONEWEIGHTS,	0,	VET_UBYTE4N,	VU_BLEND_WEIGHTS, 0);
#endif
			mVertexDecl->addElement(MVSI_BONEINDICES,	0,	VET_UBYTE4,		VU_BLEND_INDICES, 0);
			mVertexDecl->addElement(MVSI_UV0, 0, VET_HALF2, VU_TEXTURE_COORDINATES, 0);
			mVertexDecl->addElement(MVSI_UV1, 0, VET_FLOAT4, VU_TEXTURE_COORDINATES, 1);
			mVertexDecl->addElement(MVSI_UV2, 0, VET_FLOAT4, VU_TEXTURE_COORDINATES, 2);
			mVertexDecl->addElement(MVSI_UV3, 0, VET_FLOAT4, VU_TEXTURE_COORDINATES, 3);
		}

		mCombinedDecl = mVertexDecl->clone();
		mCombinedDecl->modifyElement(0, MVSI_POSITION, 0, VET_FLOAT3, VU_POSITION, 0);

		if( mBoneVisualizerDecl == NULL )
			mBoneVisualizerDecl = RenderUtility::getVertexDeclaration(RenderUtility::BE_POSITION);

		ModelBatchCombiner::getSingleton().initialize();
	}

	//////////////////////////////////////////////////////////////////////////
	void				ModelConfigManager::shutdown()
	{
		mVertexDecl.clear();
		mCombinedDecl.clear();
		mBoneVisualizerDecl.clear();
		ModelBatchCombiner::getSingleton().shutdown();
	}

}//namespace Blade