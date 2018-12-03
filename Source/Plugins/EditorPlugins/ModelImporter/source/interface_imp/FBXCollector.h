/********************************************************************
	created:	2015/09/23
	filename: 	FBXCollector.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_FBXCollector_h__
#define __Blade_FBXCollector_h__
#include <BladePlatform.h>
#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
#include <interface/IModelBuilder.h>
#include <interface/IAnimationBuilder.h>
#include <utility/BladeContainer.h>
#include "FBXHelper.h"

namespace Blade
{
	struct BladeFbxBoneData
	{
		size_t			dataIndex;	//index to BONE_DATA array
		float			weight;
	};

	typedef TempVector<FbxSurfaceMaterial*>				MaterialList;

	typedef TempVector<Vector3>	SubMeshVertices;
	typedef TempVector<IModelCollector::Triangle>		SubMeshFaces;
	typedef TempVector<IModelCollector::SubMaterial>	SubMeshMaterials;
	typedef TempVector<BONE_DATA>						BoneCollection;
	typedef TempVector<BladeFbxBoneData>				VertexBone;
	typedef TempVector<VertexBone>						BoneList;
	typedef TempVector<FbxNode*>						BoneNodeList;
	struct SubMeshData
	{
		FbxMesh*			mesh;
		TString				name;
		SubMeshMaterials	materialData;
		SubMeshVertices		vertexData;
		SubMeshFaces		indexData;
		BoneList			boneData;
	};
	typedef TempVector<SubMeshData>						MeshList;
	typedef TempVector<AnimationDef>					AnimationDefList;


	class FBXCollector : public IModelCollector, public IAnimationCollector, public TempAllocatable
	{
	public:
		FBXCollector();
		virtual ~FBXCollector();

		/************************************************************************/
		/* IModelCollector interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual size_t			getMaterialCount();

		/** @brief  */
		virtual bool			getMaterial(index_t index, IModelResource::MATERIAL_INFO& info);

		/** @brief  */
		virtual size_t			getSubMeshCount();

		/** @brief  */
		virtual const tchar*	getSubMeshName(index_t meshIndex);

		/** @brief note: Blade model's sub mesh only have unique material, the collector is allowed to have more. */
		virtual const SubMaterial*getSubMeshMaterials(index_t meshIndex, size_t& materialCount);

		/** @brief  */
		virtual const Vector3*	getSubMeshVertices(index_t meshIndex, size_t& vertexCount);

		/** @brief get indices data for triangle lists */
		virtual const Triangle*	getSubMeshTriangles(index_t meshIndex, size_t& triangleCount);

		/** @brief  */
		virtual size_t			getVertexBoneCount(index_t meshIndex, uint32 vertexIndex);

		/** @brief  */
		virtual float			getVertexBoneWeight(index_t meshIndex, uint32 vertexIndex, index_t boneIndex);

		/** @brief get one bone element in array of getBoneData() */
		virtual const BONE_DATA*getVertexBone(size_t meshIndex, uint32 vertexIndex , index_t boneIndex);

		/** @brief  */
		virtual TString			getBoneName(const BONE_DATA* bone);

		/** @brief get bone array */
		virtual const BONE_DATA*getBoneData(size_t& count);

		/** @brief  */
		virtual size_t			getCameraCount();

		/** @brief  */
		virtual IGraphicsCamera*getCamera(index_t index);

		/************************************************************************/
		/* IAnimationCollector interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual size_t			getAnimationCount();
		/** @brief  */
		virtual AnimationDef*	getAnimationDef(index_t index);

		/** @brief  */
		virtual size_t			getBoneCount();

		/** @brief  */
		virtual TString			getBoneName(index_t index);

		/** @brief get bone array */
		virtual const BONE_DATA*getBones();

		/** @brief  */
		virtual void			setCurrentAnimation(const TString& name);

		/** @brief  */
		virtual scalar			getFrameTime(uint32 frame);

		/** @brief  */
		virtual bool			getBoneTransform(BoneDQ& transform, index_t boneIndex, scalar time);

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		void					setTimeMode(FbxTime::EMode mode)	{mTimeMode = mode;}
		/** @brief initialize bone & animation data
		return: true if has animations
		*/
		bool					initAnimData(FbxScene* scene);
		/** @brief  */
		void					setUseExternalTexture(bool external){mExternalTexture = external;}

	public:
		MaterialList	mMaterials;
		MeshList		mMeshes;
		BoneCollection	mBoneData;
		BoneNodeList	mBoneNodes;
		FbxTime::EMode	mTimeMode;
		AnimationDefList	mAnimations;
		FbxScene*		mAnimtaionScene;
		bool			mExternalTexture;
	};
	
}//namespace Blade

#endif//BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
#endif // __Blade_FBXCollector_h__
