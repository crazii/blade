/********************************************************************
	created:	2013/04/09
	filename: 	Model.h
	author:		Crazii
	purpose:	runtime instance of model resource. better be named as ModelInstance
*********************************************************************/
#ifndef __Blade_Model_h__
#define __Blade_Model_h__
#include <interface/ISyncAnimation.h>
#include <math/Matrix44.h>
#include <SpaceContent.h>
#include "ModelResource.h"
#include "SubMesh.h"
#include "Skeleton.h"

namespace Blade
{
	typedef List<SubMesh*>	SubMeshList;
	class BoneVisualizer;
	class StaticModelResource;

	class Model : public SpaceContent, public ISyncAnimation, public IGraphicsUpdatable, public Allocatable
	{
	public:
		Model();
		~Model();

		/************************************************************************/
		/* SpaceContent interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual	uint32			getAppFlag() const;

		/*
		@describe this function will be called if content is visible and CUF_VISIBLE_UPDATE is set
		@param
		@return
		*/
		virtual void			visibleUpdate(const ICamera* current, const ICamera* main, const ICamera::VISIBILITY _visibility);

		/*
		@describe add renderables to render buffer
		@return
		@note  this function will be called if CUF_*_VISIBLE is set
		*/
		virtual void			updateRender(IRenderQueue* queue);

		/**
		@describe
		@param
		@return
		*/
		virtual	bool			queryNearestPoint(SpaceQuery& query, scalar& distance) const;

		/************************************************************************/
		/* ISyncAnimation interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual const tchar*	getAnimationSync();

		/** @brief  */
		virtual scalar			getAnimtionLengthSync() const;

		/** @brief  */
		virtual scalar			getAnimationPosSync() const;

		/** @brief  */
		virtual bool			setAnimationPosSync(scalar pos);

		/** @brief  */
		virtual scalar			getFrameTimeSync() const;

		/** @brief  */
		virtual	void			setAniamtedBoundingVisibleSync(bool visible);

		/** @brief  */
		virtual void			setBoneVisibleSync(bool visible);

		/** @brief  */
		virtual void			setIKChainPositionSync(const TString& type, index_t index, const Vector3& pos);

		/************************************************************************/
		/* IGraphicsUpdatable                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual void			update(SSTATE state);

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		const IModelResource*	getModelResource() const	{return mResource;}

		/** @brief  */
		const ISkeletonResource*getSkeletonResource() const	{return mResource != NULL ? mResource->getSkeletonResource() : NULL;}
		inline bool		hasSkinnedAnimation() const			{return this->getSkeletonResource() != NULL;}

		/** @brief  */
		inline const Skeleton*	getSkeleton() const			{return mSkeleton;}

		/** @brief render helper */
		inline const Matrix44&	getWorldTransform() const	{return mTransform;}
		/** @brief  */
		inline const DualQuaternion* getBoneTransforms() const		{return mSkeleton->getBlendedTransform();}
		/** @brief  */
		void			setModelResource(IModelResourceImpl* modelRes);
		/** @brief  */
		void			clearSubMeshes();

		/** @brief  */
		size_t			getAnimtaionCount() const;

		/** @brief  */
		const tchar*	getAnimtaionName(index_t index) const;

		/** @brief  */
		bool			setSkinnedAnimation(const TString& animationName);

		/** @brief  */
		bool			updateSkinnedAnimation(uint64 frameID, scalar time, scalar frametime, bool updateBounding = true);

		/** @brief  */
		bool			setSubMeshVisible(SubMesh* mesh, bool visible);

		inline bool	setSubMeshVisible(index_t index, bool visible)
		{
			SubMeshList::const_iterator i = mSubMeshes.begin();
			std::advance(i, index);
			return this->setSubMeshVisible(*i, visible);
		}

		inline bool			setSubMeshVisible(const TString& name, bool visible)
		{
			for (SubMeshList::iterator i = mSubMeshes.begin(); i != mSubMeshes.end(); ++i)
			{
				if ((*i)->getName() == name)
					return this->setSubMeshVisible(*i, visible);
			}
			return false;
		}

		/** @brief  */
		void			setSelectedBone(const TString& bone);

		/** @brief  */
		const TString&	getSkeletonFile() const {return this->hasSkinnedAnimation() ? mResource->getSkeletonFile() : TString::EMPTY;}

		/** @brief  */
		size_t			getSubMeshCount() const	{return mSubMeshes.size();}

		/** @brief  */
		SubMesh*		findSubmesh(const TString& name) const
		{
			for (SubMeshList::const_iterator i = mSubMeshes.begin(); i != mSubMeshes.end(); ++i)
			{
				if ((*i)->getName() == name)
					return *i;
			}
			return NULL;
		}
		
		/** @brief  */
		const TString&	getSubMeshName(index_t index) const
		{
			if( index >= mSubMeshes.size() )
			{
				assert(false);
				return TString::EMPTY;
			}
			SubMeshList::const_iterator i = mSubMeshes.begin();
			std::advance(i, index);
			return (*i)->getName();
		}

		/** @brief  */
		inline const SubMeshList& getSubMeshes() const { return mSubMeshes; }

	protected:
		/** @brief update sub mesh 's world pos/bounds for material LOD & culling */
		void			updateSubMeshBounds();
		//SpaceContent callbacks
		/** @brief  */
		virtual	void			notifyPositionChange();
		/** @brief  */
		virtual void			notifyRotationChange();
		/** @brief  */
		virtual void			notifyScaleChange();

		typedef Vector<bool>	VisibilityList;

		//resource data: hardware buffer holder
		IModelResourceImpl*	mResource;
		//runtime data
		SubMeshList		mSubMeshes;
		VisibilityList	mSubMeshVisibility;	//culling result
		Skeleton*		mSkeleton;
		SubMesh*		mSharedMesh;
		Matrix44		mTransform;
		BoneVisualizer*	mVisualizer;
	};
}//namespace Blade

#endif //  __Blade_Model_h__