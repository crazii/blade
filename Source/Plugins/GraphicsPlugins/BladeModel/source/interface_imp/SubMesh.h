/********************************************************************
	created:	2013/04/09
	filename: 	SubMesh.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_SubMesh_h__
#define __Blade_SubMesh_h__
#include <utility/Bindable.h>
#include <interface/IAABBRenderer.h>
#include <interface/public/IRenderable.h>
#include <interface/IModelResource.h>
#include <interface/ISkeletonResource.h>
#include <interface/public/graphics/IVertexSource.h>
#include <interface/IMaterialLODUpdater.h>

namespace Blade
{
	//mutual dependency..TODO:better ways?
	class Model;
	class Skeleton;

	class SubMesh : public IRenderable, public IMaterialLODUpdatable, public IAABBTarget, public Allocatable
	{
	public:
		SubMesh(Model* parent, const IModelResource::MESHDATA& mesh);
		~SubMesh();

		/************************************************************************/
		/* IRenderable interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual RenderType&				getRenderType() const;

		/*
		@describe
		@param
		@return
		*/
		virtual const GraphicsGeometry&	getGeometry() const;

		/*
		@describe
		@param
		@return
		*/
		virtual const MaterialInstance*	getMaterial() const;

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

		/*
		@describe 
		@param
		@return
		*/
		virtual size_t					getBonePalette(const DualQuaternion*& palette) const;

		/**
		@describe
		@param
		@return
		*/
		virtual const AABB&	getWorldBounding() const { return mWorldBouding; }

		/************************************************************************/
		/* IMaterialLODUpdatable interface                                                                     */
		/************************************************************************/
		virtual index_t					getMaxLOD() const	{return mMaxLOD;}
		virtual MaterialInstance*		getMatInstance()	{return mMaterial;}
		virtual Vector3					getMatLODPosition() const;
		virtual scalar					getRadiusSQR() const;

		/************************************************************************/
		/* IAABBTarget interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual size_t			getAABBCount() const	{return 1;}
		/** @brief  */
		virtual const AABB&		getAABB(index_t /*index*/) const	{return this->getWorldBound();}
		/** @brief  */
		virtual Color			getAABBColor(index_t /*index*/) const	{return Color::WHITE;}

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief custom visibility */
		inline bool				isVisible() const { return mVisible; }
		/** @brief skeleton animation */
		inline bool				isAnimated() const { return mHasAnimation; }
		inline bool				isPreTransformed() const { return mMeshData->mPreTransformed; }
		/** @brief  */
		inline const AABB&		getWorldBound() const { return mWorldBouding; }
		/** @brief  */
		inline const TString&	getName() const
		{
			return mMeshData != NULL ? mMeshData->mName : TString::EMPTY;
		}
		/** @brief  */
		inline bool				isTransparent() const
		{
			return mHasAlpha || (mMaterial->getRenderProperty() != NULL && mMaterial->getRenderProperty()->hasProperty(RPT_ALPHABLEND));
		}

		/** @brief  */
		inline bool	isUsingGlobalBone() const { return mUseGobalBone; }
		/** @brief  */
		inline const Model* getParent() const { return mParent; }
		/** @brief  */
		inline const IModelResource::MESHDATA* getMeshData() const { return mMeshData; }
		/** @brief  */
		inline const HMATERIALINSTANCE& getMaterialInstance() const { return mMaterial; }
		inline bool hasAlpha() const { return mHasAlpha; }

	protected:
		friend class Model;
		/** @brief  */
		void					updateBonePalette();
		/** @brief update static mesh's world bounding box */
		void					updateStaticBound();
		/** @brief calculate skinned bounds & visibility */
		void					updateSkinnedBound();
		/** @brief  */
		void					activeBones(Skeleton* skeleton, bool active);
		/** @brief custom visibility set by user */
		inline void				setVisible(bool visible) { mVisible = visible; }

		typedef Vector<uint32>	UsedBoneID;
		Model*				mParent;
		const IModelResource::MESHDATA* mMeshData;
		mutable BoneDQ*		mBonePalette;
		UsedBoneID			mUsedBones;
		HMATERIALINSTANCE	mMaterial;
		AABB				mWorldBouding;

		uint8				mMaxLOD;

		//runtime data (changing data)
		bool				mHasAnimation : 1;
		bool				mVisible : 1;
		bool				mUseGobalBone : 1;	//use skeleton's bone palette directly. when this is true, mBonePalette keeps NULL
		bool				mHasAlpha : 1;
	};
	
}//namespace Blade

#endif //  __Blade_SubMesh_h__