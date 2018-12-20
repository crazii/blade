/********************************************************************
	created:	2013/04/09
	filename: 	Model.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <interface/IPlatformManager.h>
#include <TextureState.h>
#include "Model.h"
#include <interface/ITimeService.h>
#include <interface/IEntity.h>
#include <interface/public/graphics/GraphicsUtility.h>
#include <Element//GraphicsElement.h>
#include <interface/public/geometry/IGeometry.h>
#include <interface/IRenderScene.h>
#include <interface/IRenderQueue.h>
#include <Frustum.h>
#include "ModelConfigManager.h"
#include "ModelType.h"
#include "ModelBatchCombiner.h"
#include "BoneVisualizer.h"
#include "IKSolver.h"
#include "StaticModelResource.h"

namespace Blade
{
#define MULTITHREAD_ANIMATING 1

	///map of squared projected size to skeleton update frequency
	static const size_t SKELETON_LOD_COUNT = 5;
	static const scalar SKELETON_LOD_TABLE[SKELETON_LOD_COUNT][2] =
	{
		{(scalar)(0.01875*0.01875),	(scalar)(1.0/15.0),},
		{(scalar)(0.0375*0.0375),	(scalar)(1.0/20.0),},
		{(scalar)(0.0625*0.0625),	(scalar)(1.0/30.0),},
		{(scalar)(0.125*0.125),		(scalar)(1.0/40.0),},
		{(scalar)(1.0*1.0),			(scalar)(1.0/60.0),},
	};

	enum DirtyFlagEx
	{
		DF_ANIMATION = 0x10,	//initial force update for animation. needed by skinned bounding & bone palette.
	};

	//////////////////////////////////////////////////////////////////////////
	Model::Model()
	{
		static_assert((DF_ANIMATION&SpaceContent::DF_MASK) == 0, "dirty mask conflict");

		mUpdateFlags = CUF_DEFAULT_VISIBLE|CUF_VISIBLE_UPDATE;
		mSpaceFlags = CSF_ELEMENT | CSF_CONTENT | CSF_SHADOWCASTER;

		mUpdateStateMask.raiseBitAtIndex( SS_PRE_CULLING );
		//mUpdateStateMask.raiseBitAtIndex( SS_POST_CULLING );

		mTransform = Matrix44::IDENTITY;
		mResource = NULL;
		mSkeleton = NULL;
		mSharedMesh = NULL;
		mVisualizer = NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	Model::~Model()
	{
		BLADE_DELETE mSkeleton;
		BLADE_DELETE mVisualizer;
		this->clearSubMeshes();
	}

	/************************************************************************/
	/* SpaceContent interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	uint32			Model::getAppFlag() const
	{
#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable:4640)//construction of local static object is not thread-safe
#endif

		//TODO: HOWTO: submesh's alpha type?
		if (this->hasSkinnedAnimation())
		{
			static const AppFlag flag = ModelConfigManager::getSingleton().getSkinnedModelType().getAppFlag();
			return flag.getMask();
		}
		else
		{
			static const AppFlag flag = ModelConfigManager::getSingleton().getStaticModelType().getAppFlag();
			return flag.getMask();
		}

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(pop)
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	void			Model::visibleUpdate(const ICamera* current, const ICamera* main, const ICamera::VISIBILITY _visibility)
	{
		if( mResource == NULL)
			return;

		assert( _visibility != ICamera::CV_NULL );
		bool allVisible = (_visibility == ICamera::CV_ALL);

		//note: sub mesh visibility need update for all input cameras
		const POINT3& cameraPos = current->getEyePosition();
		const Frustum& frustum = *current->getFrustum();

		//update bone matrices directly here, or set update flag and update through interval updater
		if( this->hasSkinnedAnimation())
		{
			//skeleton LOD (frequency)
			scalar size = this->getWorldAABB().getHalfSize().getSquaredLength();
			const Vector4& pos = mTransform.getTranslation();
			float dist = cameraPos.getSquaredDistance( pos );
			//projectedSize / size = 1 / dist:
			scalar projectedSizeSQ = size / dist;
			size_t skeletonLOD = 0;
			for(size_t i = 0; i < SKELETON_LOD_COUNT; ++i)
			{
				if( projectedSizeSQ <= SKELETON_LOD_TABLE[i][0] )
				{
					skeletonLOD = i;
					if(current == main)
						mSkeleton->setUpdateFrequency(SKELETON_LOD_TABLE[i][1]);
					break;
				}
			}

			bool updateBounding = !allVisible && skeletonLOD >= 3 && mSubMeshes.size() > 1
				&& ModelConfigManager::getSingleton().isUpdatingSkinnedSubmeshBound();
			const TimeSource& time = ITimeService::getSingleton().getTimeSource();

#if MULTITHREAD_ANIMATING
			//Note: submeshes's bounding need update after updateSkinnedAnimation() && before updateRender(). 
			//For current implementation, submeshes's bounding updates before updateSkinnedAnimation(), and so sub mesh's visibility is 1 frame delayed after skinned animation
			//That'll not harm anyway.
			if (mSkeleton->getCurrentAnimation() != NULL && mSkeleton->needUpdateBoneTransforms(time.getLoopID()))
			{
				ParallelAnimation* pa = ModelConfigManager::getSingleton().getParallelAnimationUpdater(static_cast<IRenderScene*>(this->getElement()->getScene()));
				pa->addDelayedAnimation(this, updateBounding);
			}
#else
			//don't need worry about multiple cameras, since we has loop ID to avoid redundant updates.
			this->updateSkinnedAnimation(time.getLoopID(), time.getTime(), time.getTimeThisLoop(), updateBounding);
#endif

			//calculate bone's bounding box and transform it.
			//but this need update bone transform first, which need a model bounding to culling against camera
			//so we use a large enough bounding for the model to cull against camera
			//if visible, we'll then update sub mesh 's bounding and cull them
			//if model bounding is not visible from camera, the skeleton animation is skipped, and sub mesh bounding will not be updated.

			//note: there are some extreme cases that need calculate model bounding before culling, mostly for large movements
			//i.e. very high jump. for these cases we just calculate the position of the model, using root bone's translation, see @Model::update
			//for most circumstances we don't need to update skinned bound before culling.
			index_t index = 0;
			for(SubMeshList::iterator i = mSubMeshes.begin(); i != mSubMeshes.end(); ++i, ++index)
			{
				SubMesh* mesh = *i;
				if(mesh->isVisible())
					mSubMeshVisibility[index] = !updateBounding || frustum.isVisible(mesh->getWorldBound());
			}
		}
		else
		{
			index_t index = 0;
			for(SubMeshList::iterator i = mSubMeshes.begin(); i != mSubMeshes.end(); ++i, ++index)
			{
				SubMesh* mesh = *i;
				if (mesh->isVisible())
				{
					mSubMeshVisibility[index] = (allVisible || frustum.isVisible(mesh->getWorldBound()));
				}
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void			Model::updateRender(IRenderQueue* queue)
	{
#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable:4640)//construction of local static object is not thread-safe
#endif
		if( mResource == NULL )
			return;
		assert( mResource->getSubMeshCount() == mSubMeshes.size() );

		//merge batch for shadow pass. To be profiled
		//if (mSharedMesh != NULL && queue->getUsage() == IRenderQueue::RQU_SHADOW)
		//{
		//	queue->addRenderable(mSharedMesh);

		//	size_t index = 0;
		//	for (SubMeshList::iterator i = mSubMeshes.begin(); i != mSubMeshes.end(); ++i, ++index)
		//	{
		//		SubMesh* mesh = *i;
		//		if (mSubMeshVisibility[index] && mesh->isVisible() && mesh->hasAlpha() && !mesh->isPreTransformed())
		//		{
		//			StaticModelType& t = static_cast<StaticModelType&>(mesh->getRenderType());
		//			t.getCombiner()->updateRender(mesh, queue);
		//		}
		//	}
		//	return;
		//}

		size_t index = 0;
		for(SubMeshList::iterator i = mSubMeshes.begin(); i != mSubMeshes.end(); ++i, ++index)
		{
			SubMesh* mesh = *i;
			if (mSubMeshVisibility[index] && mesh->isVisible())
			{
				if(mesh->isAnimated() || !mesh->isPreTransformed())
					queue->addRenderable(mesh);
				else
				{
					StaticModelType& t = static_cast<StaticModelType&>(mesh->getRenderType());
					t.getCombiner()->updateRender(mesh, queue);
				}
			}
		}
		if (mVisualizer != NULL)
			mVisualizer->updateRender(queue);

		if (this->getSkeletonResource() == NULL && mSpaceFlags.checkBits(CSF_DYNAMIC))
		{
			static const IGraphicsType& type = ModelConfigManager::getSingleton().getMovingStaticType();
			index = 0;
			for (SubMeshList::iterator i = mSubMeshes.begin(); i != mSubMeshes.end(); ++i, ++index)
			{
				SubMesh* mesh = *i;
				if (mSubMeshVisibility[index] && mesh->isVisible())
					queue->addRenderable(static_cast<const RenderType&>(type), mesh);
			}
		}

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(pop)
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	bool			Model::queryNearestPoint(SpaceQuery& query, scalar& distance) const
	{
		bool result = false;
		assert(query.isPrecise());

		StaticHandle<SpaceQuery> localQuery = query.clone(mTransform.getInverse());
		scalar localDist = distance;

		index_t index = 0;
		for (SubMeshList::const_iterator iter = mSubMeshes.begin(); iter != mSubMeshes.end(); ++iter, ++index)
		{
			SubMesh* mesh = *iter;
			if(!mesh->isVisible() || !mSubMeshVisibility[index])
				continue;
			
			scalar dist = distance;
			POS_VOL pv = query.intersect(mesh->getWorldBound(), dist);
			if(pv == PV_OUTSIDE)
				continue;

			if (localQuery == NULL)
			{
				//transform input distance to local
				if (localDist < FLT_MAX)
				{
					Vector3 d = localDist * localQuery->getInsectionDirection().getNormalizedVector();//use intersection dir
					d /= mTransform.getScale();
					localDist = d.getLength();
				}
			}
#if 1
			scalar preciseDist = localDist;
			//TODO: picking skinned mesh
			if (localQuery->intersect(mesh->getGeometry(), preciseDist, localDist))
			{
				result = true;
				localDist = preciseDist;
			}
#else
			const IModelResource::MESHDATA* data = mesh->getMeshData();
			const IModelResource::POSITION* pos = (const IModelResource::POSITION*)data->mPosition->lock(IGraphicsBuffer::GBLF_READONLY);
			const IndexBufferHelper index(data->mIndices, false);
			size_t triangleCount = data->mIndices->getIndexCount();
			assert(triangleCount % 3 == 0);

			scalar preciseDist = distance;
			for (size_t i = 0; i < triangleCount; i += 3)
			{
				uint32 index0 = index[i];
				uint32 index1 = index[i + 1];
				uint32 index2 = index[i + 2];
				Vector3 p0 = reinterpret_cast<const Half3&>(pos[index0]);
				Vector3 p1 = reinterpret_cast<const Half3&>(pos[index1]);
				Vector3 p2 = reinterpret_cast<const Half3&>(pos[index2]);

				if (localQuery->intersect(p0, p1, p2, preciseDist, distance))
				{
					result = true;
					distance = preciseDist;
				}
			}

			data->mPosition->unlock();
#endif			
		}

		if (result)
		{
			assert(localQuery != NULL);
			//transform distance to world metrics using transform scale
			Vector3 d = localDist * localQuery->getInsectionDirection().getNormalizedVector();//use intersection dir
			d *= mTransform.getScale();
			distance = d.getLength();
		}
		return result;
	}

	//////////////////////////////////////////////////////////////////////////
	const tchar*	Model::getAnimationSync()
	{
		if( this->hasSkinnedAnimation() )
		{
			AnimationState* state = mSkeleton->getCurrentAnimation();
			if( state != NULL )
				return state->getName();
		}
		return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	scalar			Model::getAnimtionLengthSync() const
	{
		if( this->hasSkinnedAnimation() )
		{
			AnimationState* state = mSkeleton->getCurrentAnimation();
			if( state != NULL )
				return state->getLength();
		}
		return 0.0f;
	}

	//////////////////////////////////////////////////////////////////////////
	scalar			Model::getAnimationPosSync() const
	{
		if( this->hasSkinnedAnimation() )
		{
			AnimationState* state = mSkeleton->getCurrentAnimation();
			if( state != NULL )
				return state->getCurrentPos();
		}
		return 0.0f;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			Model::setAnimationPosSync(scalar pos)
	{
		if( this->hasSkinnedAnimation() )
		{
			//this will override animation's default timing at visibleUpdate() because loop ID is the same
			//when animation update its timing in visibleUpdate(), it will skip because the loop ID is updated already.
			const TimeSource& time = ITimeService::getSingleton().getTimeSource();
			return this->updateSkinnedAnimation(time.getLoopID(), pos, time.getTimeThisLoop());
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	scalar			Model::getFrameTimeSync() const
	{
		if( this->hasSkinnedAnimation() )
		{
			AnimationState* state = mSkeleton->getCurrentAnimation();
			if( state != NULL )
				return state->getAnimation()->getFrameTime();
		}
		return 0.0f;
	}

	//////////////////////////////////////////////////////////////////////////
	void			Model::setAniamtedBoundingVisibleSync(bool visible)
	{
		if( !this->hasSkinnedAnimation() || (mSubMeshes.size() > 1 && ModelConfigManager::getSingleton().isUpdatingSkinnedSubmeshBound()) )
		{
			IRenderScene* scene = static_cast<IRenderScene*>(this->getElement()->getScene());
			for(SubMeshList::iterator i = mSubMeshes.begin(); i != mSubMeshes.end(); ++i)
			{
				if( visible )
					scene->getAABBRenderer()->addAABB(*i);
				else
					scene->getAABBRenderer()->removeAABB(*i);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void			Model::setBoneVisibleSync(bool visible)
	{
		if( visible && mVisualizer == NULL )
			mVisualizer = BLADE_NEW BoneVisualizer(this);		

		if(mVisualizer != NULL)
		{
			mVisualizer->setEnable(visible);
			if (!visible && mVisualizer->getSelectedBone() == INVALID_INDEX)
			{
				BLADE_DELETE mVisualizer;
				mVisualizer = NULL;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void			Model::setIKChainPositionSync(const TString& type, index_t index, const Vector3& pos)
	{
		if( this->hasSkinnedAnimation() && ModelConfigManager::getSingleton().isIKEnabled() )
		{
			Vector3 modelPos = pos * mTransform.getInverse();
			mSkeleton->getIKSolver()->setTarget(type, index, modelPos);
		}
	}

	/************************************************************************/
	/* IGraphicsUpdatable                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void Model::update(SSTATE /*state*/)
	{
		//this update routine is used to update world bounding of a skinned mesh
		//because a skinned mesh's bounding is not updated (only its sub mesh updated)
		//so for some animations like jump/fall with large movement, the world bounding must be updated due to the movement
		//this is a hack to apply root motion. not working for skeletons with a "sub root" (static root with a sub node as real root).
		//another way is to pre-calculate max bounding for all animations offline.
		assert( this->hasSkinnedAnimation() );

		//get root bone init pose
		const BONE_DATA* boneData = mSkeleton->getBoneData(0);
		const AnimationState* anim = mSkeleton->getCurrentAnimation();
		if( boneData != NULL && anim != NULL )
		{
			//use root bone's translation
			const BoneDQ& dq = anim->getBoneTransform(0, anim->getCurrentPos());

			mDirtyFlag.raiseBits(DF_POSITION);
			AABB aab = this->getWorldAABB();
			Vector3 rootOffset = dq.getTranslation() - boneData->mInitialPose.getTranslation();
			aab.offset( rootOffset*this->getScale() );
			mWorldAABB = aab;
		}
		else
			assert(false);
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void			Model::setModelResource(IModelResourceImpl* modelRes)
	{
		BLADE_DELETE mSkeleton;
		mSkeleton = NULL;
		//disable notification. otherwise space partition mask will be cleared
		this->setLocalAABB(AABB::EMPTY, false);
		this->clearSubMeshes();

		mResource = modelRes;

#if MULTITHREAD_ANIMATING
		if (mResource)
			ModelConfigManager::getSingleton().createParallelAnimationUpdater(static_cast<IRenderScene*>(this->getElement()->getScene()));
		else
			ModelConfigManager::getSingleton().releaseParallelAnimationUpdater(static_cast<IRenderScene*>(this->getElement()->getScene()));
#endif

		if( mResource == NULL )
			return;

		//init skeleton runtime
		const ISkeletonResource* skeletonRes = this->getSkeletonResource();
		if( skeletonRes != NULL )
		{
			mSkeleton = BLADE_NEW Skeleton();
			const TimeSource& time = ITimeService::getSingleton().getTimeSource();
			mSkeleton->initialize(mResource->getSkeleton(), time.getLoopID()-1u);
		}

		mSubMeshes.resize( mResource->getSubMeshCount() );
		mSubMeshVisibility.resize( mResource->getSubMeshCount() );
		index_t index = 0;
		AABB modelAAB;
		for(SubMeshList::iterator i = mSubMeshes.begin(); i != mSubMeshes.end(); ++i)
		{
			const IModelResource::MESHDATA& meshData = mResource->getSubMeshData(index);
			SubMesh* mesh = BLADE_NEW SubMesh(this, meshData);
			//update bounding box
			modelAAB.merge(meshData.mAAB);
			mSubMeshVisibility[index] = true;
			mesh->setVisible(true);
			mesh->activeBones(mSkeleton, true);

			*i = mesh;
			++index;
		}

		if( this->hasSkinnedAnimation() )
		{
			mDirtyFlag.raiseBits(DF_ANIMATION);
			//make aab large enough for all animations
			//sub mesh will be culled again after animation update
			Vector3 center = modelAAB.getCenter();
			Vector3 halfSize = modelAAB.getHalfSize();
			halfSize.x = halfSize.y = halfSize.z = std::max( std::max(halfSize.x, halfSize.y), halfSize.z);
			modelAAB.set(center-halfSize, center+halfSize);

			IRenderScene* scene = static_cast<IRenderScene*>(this->getElement()->getScene());
#if BLADE_DEBUG && 0
			if( mSubMeshes.size() > 1 && ModelConfigManager::getSingleton().isUpdatingSkinnedSubmeshBound() )
			{
				for(SubMeshList::iterator i = mSubMeshes.begin(); i != mSubMeshes.end(); ++i)
					scene->getAABBRenderer()->addAABB(*i);
			}
#endif
			if( mSkeleton->getCurrentAnimation()->hasTranslation() )
				scene->getUpdater()->addForUpdate(this);
		}

		if (modelRes->getSharedOpaqueMesh() != NULL)
		{
			mSharedMesh = BLADE_NEW SubMesh(this, *modelRes->getSharedOpaqueMesh());
			mSharedMesh->setVisible(true);
		}

		this->setLocalAABB(modelAAB, false);
		this->updateSubMeshBounds();
	}

	//////////////////////////////////////////////////////////////////////////
	void			Model::clearSubMeshes()
	{
		IRenderScene* scene = static_cast<IRenderScene*>(this->getElement()->getScene());
		for(SubMeshList::iterator i = mSubMeshes.begin(); i != mSubMeshes.end(); ++i)
		{
			SubMesh* mesh = *i;
			IPlatformManager::prefetch<PM_WRITE>(mesh);
			scene->getAABBRenderer()->removeAABB(*i);
			BLADE_DELETE mesh;
		}
		BLADE_DELETE mSharedMesh;
		mSharedMesh = NULL;
		mSubMeshes.clear();
		mSubMeshVisibility.clear();
	}

	//////////////////////////////////////////////////////////////////////////
	size_t			Model::getAnimtaionCount() const
	{
		if( this->hasSkinnedAnimation() )
			return mSkeleton->getAnimtaionCount();
		else
			return 0;
	}

	//////////////////////////////////////////////////////////////////////////
	const tchar*	Model::getAnimtaionName(index_t index) const
	{
		if( this->hasSkinnedAnimation() )
			return mSkeleton->getAnimation(index);
		else
			return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			Model::setSkinnedAnimation(const TString& animationName)
	{
		if( this->hasSkinnedAnimation() )
		{
			bool translation = mSkeleton->getCurrentAnimation()->hasTranslation();
			bool ret = mSkeleton->setAnimation(animationName);

			bool newTranslation = mSkeleton->getCurrentAnimation()->hasTranslation();

			IRenderScene* scene = static_cast<IRenderScene*>(this->getElement()->getScene());
			if( translation && !newTranslation )
			{
				scene->getUpdater()->removeFromUpdate(this);
				mDirtyFlag.raiseBits(DF_POSITION);//clear root bone offset
			}
			else if( !translation && newTranslation )
				scene->getUpdater()->addForUpdate(this);
			return ret;
		}
		else
			return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			Model::updateSkinnedAnimation(uint64 frameID, scalar time, scalar frametime, bool updateBounding/* = true*/)
	{
		AnimationState* state = mSkeleton->getCurrentAnimation();
		if( state != NULL )
		{
			state->setCurrentPos(time);

			bool initialUpdate = mDirtyFlag.checkBits(DF_ANIMATION);
			bool ret = mSkeleton->updateBoneTransforms(frameID, time, frametime, initialUpdate);

			if(initialUpdate)
				mDirtyFlag.clearBits(DF_ANIMATION);

			if(ret)
			{
				index_t index = 0;
				for(SubMeshList::iterator i = mSubMeshes.begin(); i != mSubMeshes.end(); ++i, ++index)
				{
					SubMesh* mesh = *i;
					if(mesh->isVisible())
						mesh->updateBonePalette();
					if(mesh->isVisible() && (updateBounding || mesh->isTransparent()))
						mesh->updateSkinnedBound();
				}
				if( mVisualizer != NULL )
					mVisualizer->update();
			}
			return ret;
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			Model::setSubMeshVisible(SubMesh* mesh, bool visible)
	{
		if (mesh == NULL)
		{
			assert(false);
			return false;
		}

		bool activeBone = mesh->isVisible() != visible;
		if (activeBone)
		{
			mesh->setVisible(visible);
			mesh->activeBones(mSkeleton, visible);
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void			Model::setSelectedBone(const TString& bone)
	{
		if (this->getSkeleton() == NULL)
			return;

		const SkeletonResource* res = static_cast<const SkeletonResource*>(this->getSkeletonResource());
		index_t boneIndex = res->findBone(bone);
		
		if (boneIndex != INVALID_INDEX && mVisualizer == NULL)
			mVisualizer = BLADE_NEW BoneVisualizer(this);	

		if(mVisualizer != NULL)
		{
			mVisualizer->setSelectedBone(boneIndex);
			if (!mVisualizer->isEnabled() && mVisualizer->getSelectedBone() == INVALID_INDEX)
			{
				BLADE_DELETE mVisualizer;
				mVisualizer = NULL;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void			Model::updateSubMeshBounds()
	{
		if( !this->hasSkinnedAnimation() )
		{
			for(SubMeshList::iterator i = mSubMeshes.begin(); i != mSubMeshes.end(); ++i)
			{
				SubMesh* mesh = *i;
				mesh->updateStaticBound();
			}
			if (mSharedMesh != NULL)
				mSharedMesh->updateStaticBound();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void			Model::notifyPositionChange()
	{
		SpaceContent::notifyPositionChange();
		Matrix44::generateTransform(mTransform, mPosition, mScale, mRotation);
		this->updateSubMeshBounds();
	}
	//////////////////////////////////////////////////////////////////////////
	void			Model::notifyRotationChange()
	{
		SpaceContent::notifyRotationChange();
		Matrix44::generateTransform(mTransform, mPosition, mScale, mRotation);
		this->updateSubMeshBounds();
	}
	//////////////////////////////////////////////////////////////////////////
	void			Model::notifyScaleChange()
	{
		SpaceContent::notifyScaleChange();
		Matrix44::generateTransform(mTransform, mPosition, mScale, mRotation);
		this->updateSubMeshBounds();
	}

}//namespace Blade