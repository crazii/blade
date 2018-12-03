/********************************************************************
	created:	2015/09/07
	filename: 	IModelBuilder.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_IModelBuilder_h__
#define __Blade_IModelBuilder_h__
#include <math/Vector2.h>
#include <math/Vector3.h>
#include <math/Vector4.h>
#include <math/Quaternion.h>
#include <math/Matrix33.h>
#include <interface/Interface.h>
#include <interface/public/ISerializable.h>
#include <interface/public/graphics/IGraphicsCamera.h>
#include "IModelResource.h"
#include "ISkeletonResource.h"

namespace Blade
{
	
	class IModelCollector
	{
	public:
		//different triangle may have diff uv, even for same vertex positions
		class Triangle
		{
		public:
			Vector2	uv[3][ITexture::MAX_TEXCOORD_COUNT];
			uint32	indices[3];
		};
		class SubMaterial
		{
		public:
			TString	name;
			size_t	materialID;
			size_t	uvCount;
			size_t	startTriangle;
			size_t	endTriangle;
			SubMaterial() :materialID(INVALID_INDEX), uvCount(0), startTriangle(INVALID_INDEX), endTriangle(INVALID_INDEX){}
			bool operator ==(const SubMaterial& rhs) const {return materialID == rhs.materialID;}
		};

		/** @brief  */
		virtual size_t			getMaterialCount() = 0;

		/** @brief  */
		virtual bool			getMaterial(index_t index, IModelResource::MATERIAL_INFO& info) = 0;

		/** @brief  */
		virtual size_t			getSubMeshCount() = 0;

		/** @brief  */
		virtual const tchar*	getSubMeshName(index_t meshIndex) = 0;

		/** @brief return sub material array, and array size in materialCount
		note: Blade model's sub mesh only have unique material, the collector is allowed to have more. */
		virtual const SubMaterial*getSubMeshMaterials(index_t meshIndex, size_t& materialCount) = 0;

		/** @brief  */
		virtual const Vector3*	getSubMeshVertices(index_t meshIndex, size_t& vertexCount) = 0;

		/** @brief get indices data for triangle lists */
		virtual const Triangle*	getSubMeshTriangles(index_t meshIndex, size_t& triangleCount) = 0;

		/** @brief  */
		virtual size_t			getVertexBoneCount(index_t meshIndex, uint32 vertexIndex) = 0;

		/** @brief  */
		virtual float			getVertexBoneWeight(index_t meshIndex, uint32 vertexIndex, index_t boneIndex) = 0;

		/** @brief get one bone element in array of getBoneData() */
		virtual const BONE_DATA*getVertexBone(size_t meshIndex, uint32 vertexIndex , index_t boneIndex) = 0;

		/** @brief  */
		virtual TString			getBoneName(const BONE_DATA* bone) = 0;

		/** @brief get bone array */
		virtual const BONE_DATA*getBoneData(size_t& count) = 0;

		/** @brief  */
		virtual size_t			getCameraCount() = 0;

		/** @brief  */
		virtual IGraphicsCamera*getCamera(index_t index) = 0;

	};//class IModelCollector


	//HOWTO: 1.initialize, 2.collect mesh 3.build mesh
	class IModelBuilder : public Interface
	{
	public:
		BLADE_MODEL_API static const TString	MODEL_BUILDER_CLASS;
	public:
		virtual ~IModelBuilder()	{}

		/*
		@describe 
		@param [in] TSLeftHanded: tangent space handedness
		@param [in] animFrequency: animation sampling frequency
		@return
		*/
		virtual bool	initialize(IModelResource::ENormalType normalType, bool TSLeftHanded) = 0;

		/*
		@describe collect meshes & materials used by mesh
		@param
		@return
		*/
		virtual bool	collectMesh(IModelCollector* collector, ProgressNotifier& callback) = 0;

		/*
		@describe 
		@param
		@return
		*/
		virtual bool	buildMesh(IModelResource* target) = 0;
	};

	extern template class BLADE_MODEL_API Factory<IModelBuilder>;
	
}//namespace Blade


#endif // __Blade_IModelBuilder_h__
