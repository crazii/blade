/********************************************************************
	created:	2013/04/09
	filename: 	SubMesh.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "SubMesh.h"
#include "ModelConfigManager.h"
#include "Model.h"
#include <TextureState.h>
#include <Element/GraphicsElement.h>
#include <interface/IPlatformManager.h>
#include <interface/ITimeService.h>
#include <interface/IRenderTypeManager.h>
#include <GraphicsShaderShared.inl>
#include <BladeGraphics_blang.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	SubMesh::SubMesh(Model* parent, const IModelResource::MESHDATA& mesh)
		:mParent(parent)
		,mBonePalette(NULL)
		,mMaxLOD(0)
		,mHasAnimation(false)
		,mVisible(false)
		,mUseGobalBone(false)
		,mHasAlpha(false)
	{
		mMeshData = &mesh;

		mHasAnimation = mMeshData != NULL && mMeshData->mBoneCount > 0 && mMeshData->mBoneIndices != NULL && mMeshData->mBoneWeights != NULL
			&& mParent != NULL && mParent->getSkeletonResource() != NULL && mParent->getSkeleton() != NULL;

		//setup geometry & material
		const IModelResource* resource = mParent->getModelResource();
		const IModelResource::MATERIAL& material = resource->getMaterial((index_t)mesh.mMaterialID);
		mHasAlpha = material.hasTransparency;	//TODO: add alpha test option. alpha blend need sort in CPU end, and cannot sort on render states to avoid state changes, too slow.
		mMaterial.bind(BLADE_NEW MaterialInstance(this->getRenderType().getMaterial(), material.texture));
		//note: by default the material instance does has color instance to save memory
		//the color instance will be set by element, at the per entity level instead of per sub-mesh
		//that means all sub meshes of one entity can have the same color, differing from other entities
		//mMaterial->enableColor(material.color);	//TODO: share whole render property set from mesh resource

		mMaxLOD = (uint8)mMaterial->calculateMaxLOD();

		if (material.texture == NULL || material.texture->getTextureStageCount() == 0)
			mMaterial->setShaderGroup(BTString("material_color"));

		mMaterial->load();
		GraphicsElement* element = static_cast<GraphicsElement*>(mParent->getElement());
		element->getRenderScene()->getMaterialLODUpdater()->addForLODUpdate(this);

		if( this->isAnimated() )
		{
			const SkeletonResource* skeletonRes = static_cast<const SkeletonResource*>(mParent->getSkeletonResource());
			assert( mMeshData->mBoneCount <= skeletonRes->getBoneCount() );
			mUsedBones.resize( mMeshData->mBoneCount );

			TempVector<uint32> activeBoneIDs;
			activeBoneIDs.resize(mMeshData->mActiveBones.size());

			for(size_t i = 0; i < mMeshData->mActiveBones.size(); ++i)
			{
				index_t boneID = skeletonRes->findBone( mMeshData->mActiveBones[i] );
				if( boneID != INVALID_INDEX )
					activeBoneIDs[i] = (uint32)boneID;
			}

			for(size_t i = 0; i < mMeshData->mBoneCount; ++i)
			{
				index_t boneID = activeBoneIDs[ mMeshData->mBones[i] ];
				assert( boneID != INVALID_INDEX );
				mUsedBones[i] = (uint32)boneID;
			}

			//if total bone count can be passed in one draw call
			//we use the skeleton's bone palette directly
			//note: this cannot be done offline in resource building or resource loading, because meshes can be dynamically attached to different skeletons at runtime
			mUseGobalBone = (skeletonRes->getBoneCount() < BLADE_BONE_PALETTE_SIZE);
			if( mUseGobalBone )
			{
				//fix mesh vertices's blend indices, convert local bone indices to skeleton global indices
				IModelResource::BONE_INDICE* indices = (IModelResource::BONE_INDICE*)mMeshData->mBoneIndices->lock(IGraphicsBuffer::GBLF_WRITEONLY);

				for(size_t i = 0; i < mMeshData->mBoneIndices->getVertexCount(); ++i)
				{
					for(int j = 0; j < 4; ++j)
					{
						assert(indices[i].index[j] < mUsedBones.size() ); 
						assert(mUsedBones[ indices[i].index[j] ] < skeletonRes->getBoneCount() );
						indices[i].index[j] = (uint8)mUsedBones[ indices[i].index[j] ];
					}
				}
				mMeshData->mBoneIndices->unlock();
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	SubMesh::~SubMesh()
	{
		GraphicsElement* element = static_cast<GraphicsElement*>( mParent->getElement() );
		element->getRenderScene()->getMaterialLODUpdater()->removeFromLODUpdate(this);
		BLADE_DELETE[] mBonePalette;
	}

	/************************************************************************/
	/* IRenderable interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	RenderType&				SubMesh::getRenderType() const
	{
#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable:4640)//construction of local static object is not thread-safe
#endif

		if (this->isAnimated())
		{
			if (mHasAlpha)
			{
				static RenderType& type = static_cast<RenderType&>(ModelConfigManager::getSingleton().getSkinnedModelType(mHasAlpha));
				return type;
			}
			else
			{
				static RenderType& type = static_cast<RenderType&>(ModelConfigManager::getSingleton().getSkinnedModelType(mHasAlpha));
				return type;
			}
		}
		else
		{
			if (mHasAlpha)
			{
				static RenderType& type = static_cast<RenderType&>(ModelConfigManager::getSingleton().getStaticModelType(mHasAlpha));
				return type;
			}
			else
			{
				static RenderType& type = static_cast<RenderType&>(ModelConfigManager::getSingleton().getStaticModelType(mHasAlpha));
				return type;
			}
		}
#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(pop)
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	const GraphicsGeometry&	SubMesh::getGeometry() const
	{
		return mMeshData->mGeometry;
	}

	//////////////////////////////////////////////////////////////////////////
	const MaterialInstance*	SubMesh::getMaterial() const
	{
		return mMaterial;
	}

	//////////////////////////////////////////////////////////////////////////
	const Matrix44&			SubMesh::getWorldTransform() const
	{
		return mMeshData->mPreTransformed ? Matrix44::IDENTITY : mParent->getWorldTransform();
	}

	//////////////////////////////////////////////////////////////////////////
	ISpaceContent*			SubMesh::getSpaceContent() const
	{
		return mParent;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t					SubMesh::getBonePalette(const DualQuaternion*& palette) const
	{
		if( this->isAnimated() )
		{
			if( mUseGobalBone )
			{
				palette = mParent->getBoneTransforms();
				return mParent->getSkeletonResource()->getBoneCount();
			}
			else
			{
				assert( mBonePalette != NULL );
				palette = mBonePalette;
				return mMeshData->mBoneCount;
			}
		}
		else
		{
			palette = NULL;
			return 0;
		}
	}

	/************************************************************************/
	/* IMaterialLODUpdatable interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	Vector3				SubMesh::getMatLODPosition() const
	{
		return mWorldBouding.getCenter();
	}

	//////////////////////////////////////////////////////////////////////////
	scalar					SubMesh::getRadiusSQR() const
	{
		return mWorldBouding.getHalfSize().getSquaredLength();
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void					SubMesh::updateBonePalette()
	{
		if( mUseGobalBone )
			return;

		size_t boneCount = mUsedBones.size();
		assert( boneCount != 0 && boneCount <= mParent->getSkeletonResource()->getBoneCount() );

		//TODO: split transform if there are too many bones
		if( mBonePalette == NULL )
			mBonePalette = BLADE_NEW BoneDQ[ boneCount ];

		const DualQuaternion* skeleton = mParent->getBoneTransforms();
		//copy bones needed by sub mesh
		for(size_t i = 0; i < boneCount; ++i)
		{
			assert( mUsedBones[i] < mParent->getSkeletonResource()->getBoneCount() );
			IPlatformManager::prefetch<PM_WRITE>( &mBonePalette[ i+1 ], sizeof(BoneDQ));
			if( i+1 < boneCount )
				IPlatformManager::prefetch<PM_READ>( &skeleton[ mUsedBones[i+1] ], sizeof(BoneDQ) );
			mBonePalette[ i ] = skeleton[ mUsedBones[i] ];
		}

#if DQ_GPU_SKINNING_HACK
		const IModelResource::BONE_INDICE* indices = (const IModelResource::BONE_INDICE*)mMeshData->mBoneIndices->lock(IGraphicsBuffer::GBLF_READONLY);
		F16X4WEIGHTS* weights = (F16X4WEIGHTS*)mMeshData->mBoneWeights->lock(IGraphicsBuffer::GBLF_WRITEONLY);	//write_discard won't work for some devices
		for(size_t i = 0; i < mGeometry.mVertexCount; ++i)
		{
			const IModelResource::BONE_INDICE& indice = indices[i];
			F16X4WEIGHTS& weight = weights[i];
			assert( indice.index[0] < mMeshData->mBoneCount );
			const BoneDQ& dq0 = mBonePalette[ indice.index[0] ];
			for(int i = 1; i < 4; ++i)
			{
				if( !weight.weight[i].isZero() )
				{
					Half fWeight = weight.weight[i].getAbs();
					uint8 index = indice.index[i];

					assert( index < BLADE_BONE_PALETTE_SIZE && index < mMeshData->mBoneCount );
					BoneDQ& dq = mBonePalette[ index ];
					if( dq0.real.dotProduct(dq.real) < 0 )
						weight.weight[i] = -fWeight;
					else
						weight.weight[i] = fWeight;
				}
			}
		}
		mMeshData->mBoneIndices->unlock();
		mMeshData->mBoneWeights->unlock();
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	void					SubMesh::updateStaticBound()
	{
		assert( !this->isAnimated() );
		//static mesh: update world bounding for material LOD & culling
		mWorldBouding = mMeshData->mAAB * mParent->getWorldTransform();
	}

	//////////////////////////////////////////////////////////////////////////
	void					SubMesh::updateSkinnedBound()
	{
		assert( this->isAnimated() );
		assert( mUseGobalBone || mBonePalette != NULL );
		//skinned mesh: update world bounding for material LOD & culling
		if( (!mUseGobalBone && mBonePalette == NULL) || mMeshData->mBoneBoundings == NULL )
			return;

		AABB localBound;
		for(size_t i = 0; i < mMeshData->mBoneCount; ++i)
		{
			const IModelResource::BONE_BOUNDING& bound = mMeshData->mBoneBoundings[i];
			AABB aab(bound.min, bound.max);
			Matrix44 m;
			if( mUseGobalBone )
				mParent->getBoneTransforms()[ mUsedBones[i] ].toMatrix(m);
			else
				mBonePalette[i].toMatrix(m);
			aab *= m;
			localBound.merge(aab);
		}
		//local to world
		mWorldBouding = localBound*mParent->getWorldTransform();
	}

	//////////////////////////////////////////////////////////////////////////
	void					SubMesh::activeBones(Skeleton* skeleton, bool active)
	{
		const SkeletonResource* skeletonRes = static_cast<const SkeletonResource*>(mParent->getSkeletonResource());
		if( skeletonRes == NULL )
		{
			assert(skeleton == NULL);
			return;
		}
		else
			assert(skeleton != NULL);

		TempVector<uint32> activeBoneIDs;
		activeBoneIDs.resize(mMeshData->mActiveBones.size());

		for(size_t i = 0; i < mMeshData->mActiveBones.size(); ++i)
		{
			index_t boneID = skeletonRes->findBone( mMeshData->mActiveBones[i] );
			if( boneID != INVALID_INDEX )
				activeBoneIDs[i] = (uint32)boneID;
		}

		if( activeBoneIDs.size() > 0 )
			skeleton->activateBones(&activeBoneIDs[0], activeBoneIDs.size(), active);
	}

}//namespace Blade