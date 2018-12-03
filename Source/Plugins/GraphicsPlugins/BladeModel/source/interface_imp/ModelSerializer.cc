/********************************************************************
	created:	2013/04/03
	filename: 	ModelSerializer.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "ModelResource.h"
#include "ModelSerializer.h"
#include "SkeletonResourceState.h"
#include "ModelConfigManager.h"

#include <interface/IPlatformManager.h>
#include <interface/public/graphics/IGraphicsResourceManager.h>

#if BLADE_COMPILER_GNU_EXTENSION
#pragma GCC diagnostic ignored "-Wattributes"
#endif

#define MODEL_VERTEXPOS_CHECK 1

#include <interface/IResourceManager.h>
#include <TextureState.h>
#include <TextureStage.h>

namespace Blade
{
	const uint32 MODEL_HEADER_MAGIC = BLADE_FCC('B', 'M', 'H', 'D');
	const uint32 MODEL_MATERIAL_MAGIC = BLADE_FCC('B', 'M', 'M', 'T');
	const uint32 MODEL_SUBMESH_MAGIC = BLADE_FCC('B', 'M', 'S', 'M');

	//streams
	const uint32 MODEL_INDEX_MAGIC = BLADE_FCC('B', 'M', 'I', 'X');
	const uint32 MODEL_POSITION_MAGIC = BLADE_FCC('B', 'M', 'P', 'S');
	const uint32 MODEL_NORMAL_MAGIC = BLADE_FCC('B', 'M', 'N', 'M');
	const uint32 MODEL_UV_MAGIC = BLADE_FCC('B', 'M', 'T', 'C');
	const uint32 MODEL_SUBBONE_MAGIC = BLADE_FCC('B', 'M', 'S', 'B');
	const uint32 MODEL_BLENDINDICE_MAGIC = BLADE_FCC('B', 'M', 'B', 'I');
	const uint32 MODEL_BLENDWEIGHTS_MAGIC = BLADE_FCC('B', 'M', 'B', 'W');

	//header
	typedef struct SModelHeader
	{
		//note: we don't need version info because it is saved before header
		uint32	mMagic;
		uint32	mSubMeshCount;
		uint32	mMaterialCount;
		uint32	mNormalType;
		//model bounds
		fp32	mMin[3];
		fp32	mMax[3];
		fp32	mRadius;
		char	mAnimtaionFile[IModelResource::MAX_ANIMATION_FILE_BASE + 1];	//animation file: UTF8 string 
		bool	mHasBones;	//true if any sub-mesh has bones

		SModelHeader()
		{
			mMagic = MODEL_HEADER_MAGIC;
			mSubMeshCount = 0;
			mMaterialCount = 0;
			mNormalType = IModelResource::NT_NONE;
		}
	}MHEADER;

	//
	typedef struct SSubMeshHeader
	{
		uint32	mMagic;
		uint32	mMaterialID;
		uint32	mIndexType;
		uint32	mIndexCount;
		uint32	mVertexCount;
		uint32	mTexMapCount;		//number of texture coordinates set
		uint32	mActiveBoneCount;
		uint32	mUsedBoneCount;
		//note: do not use masks (bit-fields) since they may have order problem on different platform.
		uint8	mBlendWeights;	//bool
		uint8	mBlendIndices;	//bool
		//sub-mesh bounds
		fp32	mBMin[3];
		fp32	mBMax[3];
		fp32	mRadius;

		SSubMeshHeader()
		{
			mMagic = MODEL_SUBMESH_MAGIC;
			mMaterialID = (uint32)-1;
			mIndexType = IIndexBuffer::IT_16BIT;
			mIndexCount = 0;
			mVertexCount = 0;
			mTexMapCount = 0;
			mActiveBoneCount = 0;
			mUsedBoneCount = 0;
			mBlendWeights = (uint8)false;
			mBlendIndices = (uint8)false;
		}
	}SMHEADER;

	//////////////////////////////////////////////////////////////////////////
	ModelSerializer_Binary::ModelSerializer_Binary()
		:mSkeletonAnimation(NULL)
		, mNormalType(0)
		, mSoftWareMesh(false)
	{
		mSharedBuffer = IModelConfigManager::getSingleton().isMergingMeshBuffer();
	}

	//////////////////////////////////////////////////////////////////////////
	ModelSerializer_Binary::~ModelSerializer_Binary()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	bool	ModelSerializer_Binary::loadResource(IResource* resource, const HSTREAM& stream, const ParamList& params)
	{
		assert(resource->getType() == ModelConsts::MODEL_RESOURCE_TYPE);
		BLADE_UNREFERENCED(resource);
		bool ret = this->loadModel(stream, params);
		if (!ret)
			this->clearSubResources();
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	void	ModelSerializer_Binary::postProcessResource(IResource* resource)
	{
		assert(resource->getType() == ModelConsts::MODEL_RESOURCE_TYPE);
		ModelResource* res = static_cast<ModelResource*>(resource);
		bool ret = this->createModel(res);
		assert(ret);
		BLADE_UNREFERENCED(ret);
	}

	//////////////////////////////////////////////////////////////////////////
	bool	ModelSerializer_Binary::saveResource(const IResource* resource, const HSTREAM& stream)
	{
		assert(resource->getType() == ModelConsts::MODEL_RESOURCE_TYPE);
		const ModelResource* res = static_cast<const ModelResource*>(resource);

		IOBuffer buffer;

		//model header
		MHEADER header;
		header.mNormalType = res->getNormalType();
		header.mSubMeshCount = (uint32)res->getSubMeshCount();
		header.mMaterialCount = (uint32)res->mMaterialInfo.size();
		std::memcpy(header.mMin, &res->mAAB.getMinPoint(), sizeof(header.mMin));
		std::memcpy(header.mMax, &res->mAAB.getMaxPoint(), sizeof(header.mMax));
		header.mRadius = res->mRadius;
		//header: animation file
		std::memset(header.mAnimtaionFile, 0, IModelResource::MAX_ANIMATION_FILE_BASE);
		const TString animFile = res->getSkeletonFile();
		if (!animFile.empty())
		{
			TString animBase, animExt;
			TStringHelper::getFileInfo(animFile, animBase, animExt);
			String s = StringConverter::TStringToUTF8String(animBase);
			size_t n = std::min(s.size(), (size_t)IModelResource::MAX_ANIMATION_FILE_BASE);
			std::memcpy(header.mAnimtaionFile, s.c_str(), n);
			header.mAnimtaionFile[IModelResource::MAX_ANIMATION_FILE_BASE] = '\0';
		}
		header.mHasBones = res->checkHasBones();
		buffer.write(&header);

		//materials
		for (ModelResource::MaterialInfo::const_iterator i = res->mMaterialInfo.begin(); i != res->mMaterialInfo.end(); ++i)
		{
			const IModelResource::MATERIAL_INFO& materialInfo = *i;
			buffer.write(&MODEL_MATERIAL_MAGIC);
			buffer.write(&materialInfo.ambient);
			buffer.write(&materialInfo.diffuse);
			buffer.write(&materialInfo.specular);
			buffer.write(&materialInfo.emissive);
			buffer.write(&materialInfo.hasTransparency);
			uint8 textCount = (uint8)materialInfo.textures.size();
			buffer.write(&textCount);
			buffer.write(materialInfo.channelMap, IModelResource::MC_COUNT);
			for (uint8 j = 0; j < textCount; ++j)
			{
				if (materialInfo.textureFullPath)
				{
					if (!TStringHelper::isAbsolutePath(materialInfo.textures[j]))
					{
						const TString& cwd = IPlatformManager::getSingleton().getCurrentWorkingPath();
						buffer.writeTString(TStringHelper::getAbsolutePath(cwd, materialInfo.textures[j]));
					}
					else
						buffer.writeTString(materialInfo.textures[j]);
				}
				else
				{
					TString dir, file, base, ext;
					TStringHelper::getFilePathInfo(materialInfo.textures[j], dir, file, base, ext);
					buffer.writeTString(base);
				}
			}
		}

		for (uint32 i = 0; i < header.mSubMeshCount; ++i)
		{
			const IModelResource::MESHDATA& subMesh = res->getSubMeshData(i);
			assert(subMesh.mPosition != NULL);
			assert(subMesh.mTexcoordCount <= ITexture::MAX_TEXCOORD_COUNT);
			assert(subMesh.mIndices != NULL);

			size_t vertexCount = subMesh.mPosition->getVertexCount();
			assert(subMesh.mNormalBuffer == NULL || subMesh.mNormalBuffer->getVertexCount() == vertexCount);
			for (int j = 0; j < subMesh.mTexcoordCount; ++j)
				assert(subMesh.mTexcoord[j] != NULL && subMesh.mTexcoord[j]->getVertexCount() == vertexCount);

			SMHEADER subMeshHeader;
			std::memcpy(subMeshHeader.mBMin, &subMesh.mAAB.getMinPoint(), sizeof(subMeshHeader.mBMin));
			std::memcpy(subMeshHeader.mBMax, &subMesh.mAAB.getMaxPoint(), sizeof(subMeshHeader.mBMax));
			subMeshHeader.mRadius = subMesh.mRadius;
			subMeshHeader.mIndexType = subMesh.mIndices->getIndexType();
			subMeshHeader.mIndexCount = (uint32)subMesh.mIndices->getIndexCount();
			subMeshHeader.mVertexCount = (uint32)vertexCount;
			subMeshHeader.mTexMapCount = (uint32)subMesh.mTexcoordCount;
			subMeshHeader.mMaterialID = (uint32)subMesh.mMaterialID;
			subMeshHeader.mActiveBoneCount = (uint32)subMesh.mActiveBones.size();
			subMeshHeader.mUsedBoneCount = subMesh.mBoneCount;
			subMeshHeader.mBlendWeights = (uint8)(subMesh.mBoneWeights != NULL);
			subMeshHeader.mBlendIndices = (uint8)(subMesh.mBoneIndices != NULL);
			buffer.write(&subMeshHeader);
			buffer.writeTString(subMesh.mName);

			//index buffer
			buffer.write(&MODEL_INDEX_MAGIC);
			const char* normalData = (const char*)subMesh.mIndices->lock(IGraphicsBuffer::GBLF_NORMAL);
			buffer.write(normalData, subMesh.mIndices->getIndexSize()*subMesh.mIndices->getIndexCount());
			subMesh.mIndices->unlock();

			//position
			buffer.write(&MODEL_POSITION_MAGIC);
			const IModelResource::POSITION* positionData = (const IModelResource::POSITION*)subMesh.mPosition->lock(IGraphicsBuffer::GBLF_NORMAL);
			subMesh.mPosition->unlock();
			buffer.write(positionData, vertexCount);

			//normal
			if (header.mNormalType != IModelResource::NT_NONE)
			{
				buffer.write(&MODEL_NORMAL_MAGIC);
				assert(subMesh.mNormalBuffer != NULL);

				size_t vertexSize = header.mNormalType == IModelResource::NT_NORMAL_VECTOR
					? sizeof(IModelResource::NORMAL) : sizeof(IModelResource::TANGENT_FRAME);
				const char* data = (const char*)subMesh.mNormalBuffer->lock(IGraphicsBuffer::GBLF_NORMAL);
				subMesh.mNormalBuffer->unlock();
				buffer.write(data, vertexCount*vertexSize);
			}

			//uv
			if (subMesh.mTexcoordCount > 0)
			{
				buffer.write(&MODEL_UV_MAGIC);
				for (int j = 0; j < subMesh.mTexcoordCount; ++j)
				{
					const IModelResource::UV* data = (const IModelResource::UV*)subMesh.mTexcoord[j]->lock(IGraphicsBuffer::GBLF_NORMAL);
					subMesh.mTexcoord[j]->unlock();
					buffer.write(data, vertexCount);
				}
			}

			//animation data:
			if (subMeshHeader.mActiveBoneCount != 0)
			{
				buffer.write(&MODEL_SUBBONE_MAGIC);

				assert(subMesh.mActiveBones.size() > 0);
				for (size_t j = 0; j < subMesh.mActiveBones.size(); ++j)
					buffer.writeTString(subMesh.mActiveBones[j]);

				if (subMeshHeader.mUsedBoneCount != 0)
				{
					assert(subMesh.mBones != NULL);
					assert(subMesh.mBoneBoundings != NULL);
					buffer.write(subMesh.mBones, subMeshHeader.mUsedBoneCount);
					buffer.write(subMesh.mBoneBoundings, subMeshHeader.mUsedBoneCount);
				}
			}
			else
				assert(subMesh.mBoneCount == 0 && subMesh.mBones == NULL && subMesh.mBoneBoundings == NULL);

			if (subMeshHeader.mBlendWeights)
			{
				buffer.write(&MODEL_BLENDWEIGHTS_MAGIC);
				const IModelResource::BONE_WEIGHT* data = (IModelResource::BONE_WEIGHT*)subMesh.mBoneWeights->lock(IGraphicsBuffer::GBLF_NORMAL);
				subMesh.mBoneWeights->unlock();
				buffer.write(data, vertexCount);
			}

			if (subMeshHeader.mBlendIndices)
			{
				buffer.write(&MODEL_BLENDINDICE_MAGIC);
				const IModelResource::BONE_INDICE* data = (IModelResource::BONE_INDICE*)subMesh.mBoneIndices->lock(IGraphicsBuffer::GBLF_NORMAL);
				subMesh.mBoneIndices->unlock();
				buffer.write(data, vertexCount);
			}
		}
		buffer.saveToStream(stream);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	ModelSerializer_Binary::createResource(IResource* resource, ParamList& /*params*/)
	{
		return (resource->getType() == ModelConsts::MODEL_RESOURCE_TYPE);
	}

	//////////////////////////////////////////////////////////////////////////
	bool	ModelSerializer_Binary::reloadResource(IResource* /*resource*/, const HSTREAM& stream, const ParamList& params)
	{
		return this->loadModel(stream, params);
	}

	//////////////////////////////////////////////////////////////////////////
	bool	ModelSerializer_Binary::reprocessResource(IResource* resource)
	{
		assert(resource->getType() == ModelConsts::MODEL_RESOURCE_TYPE);
		ModelResource* res = static_cast<ModelResource*>(resource);
		return this->createModel(res);
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool	ModelSerializer_Binary::loadModel(const HSTREAM& stream, const ParamList& params)
	{
		IOBuffer buffer(stream);

		const Variant& softMesh = params[ModelConsts::MODEL_PARAM_SOFTMODE];
		if (softMesh.isValid())
			mSoftWareMesh = softMesh;

		//header
		MHEADER header;
		if (!buffer.read(&header) || header.mMagic != MODEL_HEADER_MAGIC)
			return false;

		mModelAABB.set(Vector3(header.mMin), Vector3(header.mMax));
		mModelRadius = header.mRadius;
		mNormalType = header.mNormalType;

		uint32 magic = 0;

		//skeleton/animations sub resource
		String animFileUtf8 = String(header.mAnimtaionFile);
		TString file = StringConverter::UTF8StringToTString(animFileUtf8);
		if (!file.empty())
		{
			TStringHelper::appendFileExtension(file, ModelConsts::SKELETON_FILE_EXT);
			HSTREAM s = IResourceManager::getSingleton().loadStream(file);
			if (s != NULL && s->isValid())
			{
				mSkeletonAnimation = this->addSubGroup();
				SkeletonResourceState* skeleton = BLADE_NEW SkeletonResourceState();
				skeleton->setResourcePath(file, ModelConsts::SKELETON_RESOURCE_TYPE);
				mSkeletonAnimation->addSubState(skeleton);
			}
		}
		mSharedBuffer = mSharedBuffer && !header.mHasBones;	//merge mesh for skinned mesh may cause vertex shader constant buffer overflow

		mMaterials.resize(header.mMaterialCount);
		mMeshes.resize(header.mSubMeshCount);
		IGraphicsResourceManager& manager = IGraphicsResourceManager::getOtherSingleton(IGraphicsResourceManager::TYPE_TEMPORARY);

		size_t matCount = 0;
		size_t meshCount = 0;
		while (matCount < header.mMaterialCount || meshCount < header.mSubMeshCount)
		{
			if (!buffer.read(&magic))
				return false;
			switch (magic)
			{
			case MODEL_MATERIAL_MAGIC:
			{
				assert(matCount < header.mMaterialCount);
				ModelResource::MATERIAL_INFO& materialInfo = mMaterials[matCount++];
				buffer.read(&materialInfo.ambient);
				buffer.read(&materialInfo.diffuse);
				buffer.read(&materialInfo.specular);
				buffer.read(&materialInfo.emissive);
				buffer.read(&materialInfo.hasTransparency);
				uint8 textCount = 0;
				buffer.read(&textCount);
				buffer.read(materialInfo.channelMap, IModelResource::MC_COUNT);
				SubResourceGroup* group = this->addSubGroup();
				if (textCount > 0)
				{
					for (uint8 j = 0; j < textCount; ++j)
					{
						materialInfo.textures.push_back(TString::EMPTY);
						buffer.readTString(materialInfo.textures[j]);
						if (!TStringHelper::isAbsolutePath(materialInfo.textures[j]))
							materialInfo.textures[j] += BTString(".") + IGraphicsResourceManager::getSingleton().getGraphicsConfig().TextureExt;
						else
							materialInfo.textureFullPath = true;
					}

					//bind texture by channel order ( as required in shader)
					for (uint8 j = 0; j < IModelResource::MC_COUNT; ++j)
					{
						int8 index = materialInfo.channelMap[j];
						if (index == -1)
							continue;
						assert(index < (int8)materialInfo.textures.size());
						TextureStage* state = BLADE_NEW TextureStage(IModelResource::MAP_CHANNEL_NAMES[j]);
						state->setResourcePath(materialInfo.textures[(index_t)index]);
						//only load diffuse texture, the other textures are handled by material LOD system
						state->setEnable(j == IModelResource::MC_DIFFUSE);

						//make sure external texture has the right format
						if (j == IModelResource::MC_NORMAL)
							state->setTargetPixelFormat(IGraphicsResourceManager::getSingleton().getGraphicsConfig().CompressedNormal);
						if (j == IModelResource::MC_NORMAL || j == IModelResource::MC_SPECULAR_LEVEL || j == IModelResource::MC_SPECULAR_GLOSS)
							state->setLinearSpace(true);
						group->addSubState(state);
					}
				}
				mTextures.push_back(group);
			}
			break;
			case MODEL_SUBMESH_MAGIC:
			{
				assert(meshCount  < header.mSubMeshCount);
				//re-read header with magic
				buffer.seekBack();

				SMHEADER subMeshHeader;
				TString	name;

				if (!buffer.read(&subMeshHeader) || subMeshHeader.mMagic != MODEL_SUBMESH_MAGIC)
					return false;
				if (!buffer.readTString(name))
					return false;

				assert(subMeshHeader.mIndexType == IIndexBuffer::IT_16BIT ||
					subMeshHeader.mIndexType == IIndexBuffer::IT_32BIT);

				bool indexRead = false;
				bool positionRead = false;
				bool normalRead = false;
				bool uvRead = false;
				bool boneRead = false;
				bool blendWeightRead = false;
				bool blendIndiceRead = false;

				HIBUFFER hIB;
				HVBUFFER hVB;
				HVBUFFER hNormal;
				HVBUFFER hUV[ITexture::MAX_TEXCOORD_COUNT];
				TStringParam activeBones;
				uint32* subMeshBone = NULL;
				IModelResource::BONE_BOUNDING* boneBounding = NULL;
				HVBUFFER hBlendWeights;
				HVBUFFER hBlendIndices;

				while (!positionRead
					|| (!normalRead && header.mNormalType != IModelResource::NT_NONE)
					|| (!indexRead && subMeshHeader.mIndexCount != 0)
					|| (!uvRead && subMeshHeader.mTexMapCount != 0)
					|| (!boneRead && subMeshHeader.mActiveBoneCount != 0)
					|| (!blendIndiceRead && subMeshHeader.mBlendIndices)
					|| (!blendWeightRead && subMeshHeader.mBlendWeights)
					)
				{
					if (!buffer.read(&magic))
						return false;

					switch (magic)
					{
					case MODEL_INDEX_MAGIC:
					{
						assert(!indexRead && subMeshHeader.mIndexCount != 0);
						hIB = manager.createIndexBuffer(NULL,
							(IIndexBuffer::EIndexType)subMeshHeader.mIndexType,
							subMeshHeader.mIndexCount,
							IGraphicsBuffer::GBU_DEFAULT
						);

						size_t indexBytes = hIB->getSizeInBytes();
						char* indices = (char*)hIB->lock(IGraphicsBuffer::GBLF_NORMAL);
						bool ret = buffer.read(indices, indexBytes);
						hIB->unlock();
						if (!ret)
							return ret;
						indexRead = true;
					}
					break;
					case MODEL_POSITION_MAGIC:
					{
						assert(!positionRead && subMeshHeader.mVertexCount != 0);

						hVB = manager.createVertexBuffer(NULL, sizeof(IModelResource::POSITION), subMeshHeader.mVertexCount, IGraphicsBuffer::GBU_DEFAULT);
						IModelResource::POSITION* data = (IModelResource::POSITION*)hVB->lock(IGraphicsBuffer::GBLF_NORMAL);
						bool ret = buffer.read(data, subMeshHeader.mVertexCount);
						hVB->unlock();
						if (!ret)
							return ret;
						positionRead = true;
#if MODEL_VERTEXPOS_CHECK
						{
							AABB abb(Vector3(subMeshHeader.mBMin), Vector3(subMeshHeader.mBMax));
							for (uint32 j = 0; j < subMeshHeader.mVertexCount; ++j)
							{
								scalar x = data[j].x.getFloat();
								scalar y = data[j].y.getFloat();
								scalar z = data[j].z.getFloat();
								assert(!isnan(x) && !isnan(y) && !isnan(z));
								BLADE_UNREFERENCED(x); BLADE_UNREFERENCED(y); BLADE_UNREFERENCED(z);
								//assert( abb.contains(Vector3(x,y,z)) );
							}
						}
#endif
					}
					break;
					case MODEL_NORMAL_MAGIC:
					{
						assert(!normalRead && subMeshHeader.mVertexCount != 0 && header.mNormalType != IModelResource::NT_NONE);
						size_t vertexSize = header.mNormalType == IModelResource::NT_NORMAL_VECTOR ?
							sizeof(IModelResource::NORMAL) : sizeof(IModelResource::TANGENT_FRAME);

						hNormal = manager.createVertexBuffer(NULL, vertexSize, subMeshHeader.mVertexCount, IGraphicsBuffer::GBU_DEFAULT);
						char* normalData = (char*)hNormal->lock(IGraphicsBuffer::GBLF_NORMAL);
						size_t bytes = subMeshHeader.mVertexCount*vertexSize;
						bool ret = buffer.read(normalData, bytes);

						hNormal->unlock();
						if (!ret)
							return ret;

						normalRead = true;
					}
					break;
					case MODEL_UV_MAGIC:
					{
						assert(!uvRead && subMeshHeader.mVertexCount != 0 && subMeshHeader.mTexMapCount != 0 && subMeshHeader.mTexMapCount < ITexture::MAX_TEXCOORD_COUNT);

						for (uint32 j = 0; j < subMeshHeader.mTexMapCount; ++j)
						{
							hUV[j] = manager.createVertexBuffer(NULL, sizeof(IModelResource::UV), subMeshHeader.mVertexCount, IGraphicsBuffer::GBU_DEFAULT);
							IModelResource::UV* uv = (IModelResource::UV*)hUV[j]->lock(IGraphicsBuffer::GBLF_NORMAL);
							bool ret = buffer.read(uv, subMeshHeader.mVertexCount);

							hUV[j]->unlock();
							if (!ret)
								return ret;
						}
						uvRead = true;
					}
					break;
					case MODEL_SUBBONE_MAGIC:
					{
						assert(!boneRead && subMeshHeader.mActiveBoneCount != 0);

						for (uint32 j = 0; j < subMeshHeader.mActiveBoneCount; ++j)
						{
							TString boneName;
							buffer.readTString(boneName);
							activeBones.push_back(boneName);
						}

						if (subMeshHeader.mUsedBoneCount != 0)
						{
							subMeshBone = BLADE_MODEL_ALLOCT(uint32, subMeshHeader.mUsedBoneCount);
							buffer.read(subMeshBone, subMeshHeader.mUsedBoneCount);

							boneBounding = BLADE_MODEL_ALLOCT(IModelResource::BONE_BOUNDING, subMeshHeader.mUsedBoneCount);
							buffer.read(boneBounding, subMeshHeader.mUsedBoneCount);
						}
						boneRead = true;
					}
					break;
					case MODEL_BLENDWEIGHTS_MAGIC:
					{
						assert(!blendWeightRead && subMeshHeader.mBlendIndices);
						hBlendWeights = manager.createVertexBuffer(NULL, sizeof(IModelResource::BONE_WEIGHT), subMeshHeader.mVertexCount, IGraphicsBuffer::GBU_DEFAULT);
						IModelResource::BONE_WEIGHT* weights = (IModelResource::BONE_WEIGHT*)hBlendWeights->lock(IGraphicsBuffer::GBLF_NORMAL);
						bool ret = buffer.read(weights, subMeshHeader.mVertexCount);

						if (!ret)
							return ret;

#if DQ_GPU_SKINNING_HACK
						IModelResource::BONE_WEIGHT* tmp = BLADE_TMP_ALLOCT(IModelResource::BONE_WEIGHT, subMeshHeader.mVertexCount);
						std::memcpy(tmp, weights, hBlendWeights->getVertexBytes());
						hBlendWeights->unlock();
						hBlendWeights = manager.createVertexBuffer(sizeof(F16X4WEIGHTS), subMeshHeader.mVertexCount, IGraphicsBuffer::GBU_DYNAMIC_WRITE);
						weights = tmp;
						F16X4WEIGHTS* newWeights = (F16X4WEIGHTS*)hBlendWeights->lock(IGraphicsBuffer::GBLF_WRITEONLY);
						for (size_t j = 0; j < subMeshHeader.mVertexCount; ++j)
						{
							const IModelResource::BONE_WEIGHT& oldWeight = weights[j];
							F16X4WEIGHTS& newWeight = newWeights[j];
							newWeight.weight[0] = (scalar)oldWeight.weight[0] / scalar(0xFF);
							newWeight.weight[1] = (scalar)oldWeight.weight[1] / scalar(0xFF);
							newWeight.weight[2] = (scalar)oldWeight.weight[2] / scalar(0xFF);
							newWeight.weight[3] = (scalar)oldWeight.weight[3] / scalar(0xFF);
						}
						BLADE_TMP_FREE(tmp);
#endif
						hBlendWeights->unlock();

						blendWeightRead = true;
					}
					break;
					case MODEL_BLENDINDICE_MAGIC:
					{
						assert(!blendIndiceRead && subMeshHeader.mBlendIndices);
#if DQ_GPU_SKINNING_HACK
						hBlendIndices = manager.createVertexBuffer(NULL, sizeof(IModelResource::BONE_INDICE), subMeshHeader.mVertexCount, IGraphicsBuffer::GBU_DYNAMIC);
#else
						hBlendIndices = manager.createVertexBuffer(NULL, sizeof(IModelResource::BONE_INDICE), subMeshHeader.mVertexCount, IGraphicsBuffer::GBU_DEFAULT);
#endif
						IModelResource::BONE_INDICE* boneIndices = (IModelResource::BONE_INDICE*)hBlendIndices->lock(IGraphicsBuffer::GBLF_NORMAL);
						bool ret = buffer.read(boneIndices, subMeshHeader.mVertexCount);
#define BONE_CHECK 0
#if BONE_CHECK
						for (size_t j = 0; j < subMeshHeader.mVertexCount; ++j)
						{
							const IModelResource::BONE_INDICE& indice = indices[j];
							assert(indice.index[0] < subMeshHeader.mBoneCount);
							assert(indice.index[1] < subMeshHeader.mBoneCount);
							assert(indice.index[2] < subMeshHeader.mBoneCount);
							assert(indice.index[3] < subMeshHeader.mBoneCount);
						}
#endif
						hBlendIndices->unlock();
						if (!ret)
							return ret;

						blendIndiceRead = true;
					}
					break;
					default:
						assert(false && "error reading model file.");
						return false;
					}//switch
				}

				//fill cache
				IModelResource::MESHDATA& meshData = mMeshes[meshCount++];
				meshData.mName = name;
				meshData.mAAB = AABB(Vector3(subMeshHeader.mBMin), Vector3(subMeshHeader.mBMax));
				meshData.mRadius = subMeshHeader.mRadius;
				meshData.mIndices = hIB;
				meshData.mPosition = hVB;
				meshData.mNormalBuffer = hNormal;
				meshData.mBoneWeights = hBlendWeights;
				meshData.mBoneIndices = hBlendIndices;
				meshData.mTexcoordCount = (uint8)subMeshHeader.mTexMapCount;
				meshData.mActiveBones = activeBones;
				meshData.mBones = subMeshBone;
				meshData.mBoneBoundings = boneBounding;
				meshData.mBoneCount = subMeshHeader.mUsedBoneCount;
				for (uint32 j = 0; j < subMeshHeader.mTexMapCount; ++j)
					meshData.mTexcoord[j] = hUV[j];
				meshData.mMaterialID = (uint8)subMeshHeader.mMaterialID;
				meshData.mPreTransformed = 0;
			}
			break;
			default:
				assert(false && "error reading model file.");
				return false;
			}//switch
		}//while

		//merge all buffers.

		 //load textures & animations
		this->loadSubResources();

		//note: skip merge skinned animation since bone count may exceed shader constants
		if (mSharedBuffer && mMeshes.size() > 0)
		{

			TempVector<IModelResource::MESHDATA*> opaque, alpha;
			for (size_t i = 0; i < mMeshes.size(); ++i)
			{
				IModelResource::MATERIAL_INFO& mat = mMaterials[mMeshes[i].mMaterialID];
				if (mat.hasTransparency)
					alpha.push_back(&mMeshes[i]);
				else
					opaque.push_back(&mMeshes[i]);
			}
			if (opaque.size() > 1)
				this->mergeBuffer(&opaque[0], opaque.size(), mMergedOpaqueCache);
			if (alpha.size() > 1)
				this->mergeBuffer(&alpha[0], alpha.size(), mMergedAlphaCache);

			mSharedBuffer = opaque.size() > 1 || alpha.size() > 1;
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	ModelSerializer_Binary::createModel(ModelResource* resource)
	{
		resource->mAAB = mModelAABB;
		resource->mRadius = mModelRadius;
		resource->mNormalType = (IModelResource::ENormalType)mNormalType;

		assert(mTextures.size() == mMaterials.size());
		for (size_t i = 0; i < mMaterials.size(); ++i)
			resource->addMaterial(mMaterials[i]);

		resource->setupMaterial();

		IGraphicsResourceManager& manager = !mSoftWareMesh ? IGraphicsResourceManager::getSingleton() : IGraphicsResourceManager::getOtherSingleton(IGraphicsResourceManager::TYPE_SOFT);

		ModelResource::MaterialList::iterator iter = resource->mMaterials.begin();
		for (size_t i = 0; i < mTextures.size(); ++i)
		{
			SubResourceGroup* group = mTextures[i];
			IModelResource::MATERIAL& material = *iter++;

			//assert( group->getSubStateCount() == mMaterials[i].textures.size() );

			for (size_t j = 0; j < group->getSubStateCount(); ++j)
			{
				TextureStage* textureStage = static_cast<TextureStage*>(group->getSubState(j));
				//keep reference in material.texture (for unload)
				*material.texture->addTextureStage(textureStage->getSamplerUniform()) = *textureStage;
			}
			//group->clearSubStates();
		}

		/*size_t totalBoneCount = 0;*/

		if (mSkeletonAnimation != NULL)
		{
			assert(mSkeletonAnimation->getSubStateCount() == 1);
			SkeletonResourceState* skeleton = static_cast<SkeletonResourceState*>(mSkeletonAnimation->getSubState(0));
			//keep reference for unload
			resource->setSkeleton(skeleton->getResource());
			mSkeletonAnimation = NULL;
			//if( resource->getSkeletonResource() != NULL )
			//	totalBoneCount = resource->getSkeletonResource()->getBoneCount();
		}

		//pick any texture to decide UV flipping
		//note: only at this time (post process stage), textures are loaded ready.
		//note: uv flip should only be done ONCE, creating a model & save it won't flip uv, only at loading time will.
		//		if a model is loaded & uv probably flipped, then save it will cause problem!
		HTEXTURE hTex;
		for (size_t j = 0; j < mTextures.size(); ++j)
		{
			SubResourceGroup* group = mTextures[j];
			for (size_t k = 0; k < group->getSubStateCount(); ++k)
			{
				TextureStage* textureStage = static_cast<TextureStage*>(group->getSubState(k));
				if (textureStage != NULL)
					hTex = textureStage->getTexture();
				if (hTex != NULL)
					break;
			}
			if (hTex != NULL)
				break;
		}
		
		bool flipV = hTex != NULL && IGraphicsResourceManager::DEFAULT_TEXDIR != hTex->getLayout();

		
		return this->createBuffers(resource, manager, flipV);
	}

	//////////////////////////////////////////////////////////////////////////
	bool	ModelSerializer_Binary::createBuffers(ModelResource* resource, IGraphicsResourceManager& manager, bool flipUV)
	{
		//note: skip merge skinned animation since bone count may exceed shader constants
		if (mSharedBuffer)
		{
			IModelResource::MESHDATA* meshes[] = { &mMergedOpaqueCache, &mMergedAlphaCache, };
			Handle<IModelResource::MESHDATA>* buffers[] = { &resource->mSharedOpaque, &resource->mSharedAlpha, };
			for (size_t i = 0; i < countOf(meshes); ++i)
			{
				IModelResource::MESHDATA& mesh = *meshes[i];
				Handle<ModelResource::MESHDATA>& hBuffer = *buffers[i];
				if (mesh.mPosition != NULL)
				{
					if (hBuffer == NULL)
						hBuffer.constructInstance<IModelResource::MESHDATA>();
					IModelResource::MESHDATA* buffer = hBuffer;
					*buffer = mesh;	//copy AABB, radius, etc.
									//clear buffers to prepare copying, or lock will fail.
					buffer->mIndices = HIBUFFER::EMPTY;
					buffer->mPosition = buffer->mNormalBuffer = buffer->mBoneWeights = buffer->mBoneIndices = HVBUFFER::EMPTY;
					for (int j = 0; j < ITexture::MAX_TEXCOORD_COUNT; ++j)
						buffer->mTexcoord[j] = HVBUFFER::EMPTY;

					manager.cloneVertexBuffer(buffer->mPosition, *mesh.mPosition, IGraphicsBuffer::GBU_CPU_READ);//enable picking
					manager.cloneIndexBuffer(buffer->mIndices, *mesh.mIndices, IGraphicsBuffer::GBU_CPU_READ);//enable picking
					if (mesh.mNormalBuffer != NULL)
						manager.cloneVertexBuffer(buffer->mNormalBuffer, *mesh.mNormalBuffer, IGraphicsBuffer::GBU_CPU_READ);//for pre-transform vertices(pos & normal)

					for (int j = 0; j < mesh.mTexcoordCount; ++j)
					{
						if (flipUV)
						{
							IModelResource::UV* uv = (IModelResource::UV*)mesh.mTexcoord[j]->lock(IGraphicsBuffer::GBLF_NORMAL);
							size_t count = mesh.mTexcoord[j]->getVertexCount();
							for (size_t n = 0; n < count; ++n)
								uv[n].v = 1.0f - uv[n].v;
							mesh.mTexcoord[j]->unlock();
						}
						manager.cloneVertexBuffer(buffer->mTexcoord[j], *mesh.mTexcoord[j], IGraphicsBuffer::GBU_STATIC);
					}

					if (mesh.mBoneWeights != NULL || mesh.mBoneIndices != NULL)
					{
						assert(mesh.mBoneWeights != NULL && mesh.mBoneIndices != NULL);
						manager.cloneVertexBuffer(buffer->mBoneWeights, *mesh.mBoneWeights, IGraphicsBuffer::GBU_STATIC);
						manager.cloneVertexBuffer(buffer->mBoneIndices, *mesh.mBoneIndices, IGraphicsBuffer::GBU_DEFAULT);
					}

					buffer->mVertexSource = IVertexSource::create();
					buffer->mVertexSource->setSource(MVSI_POSITION, buffer->mPosition);
					buffer->mVertexSource->setSource(MVSI_NORMAL_TANGENT, buffer->mNormalBuffer);
					for (size_t j = 0; j < mesh.mTexcoordCount; ++j)
						buffer->mVertexSource->setSource((uint16)(MVSI_UV0 + j), buffer->mTexcoord[j]);

					if (buffer->mBoneWeights != NULL || buffer->mBoneIndices != NULL)
					{
						assert(buffer->mBoneWeights != NULL && buffer->mBoneIndices != NULL);
						buffer->mVertexSource->setSource(MVSI_BONEWEIGHTS, buffer->mBoneWeights);
						buffer->mVertexSource->setSource(MVSI_BONEINDICES, buffer->mBoneIndices);
					}

					buffer->mGeometry.mPrimitiveType = GraphicsGeometry::GPT_TRIANGLE_LIST;
					buffer->mGeometry.mVertexSource = buffer->mVertexSource;
					buffer->mGeometry.useIndexBuffer(true);
					buffer->mGeometry.mIndexBuffer = buffer->mIndices;
					buffer->mGeometry.mIndexStart = 0;
					buffer->mGeometry.mIndexCount = (uint32)buffer->mIndices->getIndexCount();

					buffer->mGeometry.mVertexDecl = ModelConfigManager::getSingleton().getVertexDeclaration();
					buffer->mGeometry.mVertexStart = 0;
					buffer->mGeometry.mVertexCount = (uint32)buffer->mPosition->getVertexCount();

				}
			}
			mSharedBuffer = mSharedBuffer && ((resource->mSharedOpaque != NULL && resource->mSharedOpaque->mPosition != NULL) || (resource->mSharedAlpha != NULL && resource->mSharedAlpha->mPosition != NULL));
		}

		//update soft buffer into video card.
		for (size_t i = 0; i < mMeshes.size(); ++i)
		{
			IModelResource::MESHDATA& softMesh = mMeshes[i];
			IModelResource::MESHDATA hdwMesh;
			bool shared = false;

			if (mSharedBuffer)
			{
				bool alpha = mMaterials[softMesh.mMaterialID].hasTransparency;
				if (alpha && resource->mSharedAlpha != NULL && resource->mSharedAlpha->mPosition != NULL)
				{
					hdwMesh.mPosition = resource->mSharedAlpha->mPosition;
					hdwMesh.mIndices = resource->mSharedAlpha->mIndices;
					hdwMesh.mNormalBuffer = resource->mSharedAlpha->mNormalBuffer;
					for (int j = 0; j < ITexture::MAX_TEXCOORD_COUNT; ++j)
						hdwMesh.mTexcoord[j] = resource->mSharedAlpha->mTexcoord[j];
					hdwMesh.mBoneWeights = resource->mSharedAlpha->mBoneWeights;
					hdwMesh.mBoneIndices = resource->mSharedAlpha->mBoneIndices;

					hdwMesh.mVertexSource = resource->mSharedAlpha->mVertexSource;
					hdwMesh.mGeometry.mVertexSource = resource->mSharedAlpha->mVertexSource;

					assert(hdwMesh.mVertexSource != NULL);
					shared = true;
				}
				else if (resource->mSharedOpaque != NULL && resource->mSharedOpaque->mPosition != NULL)
				{
					hdwMesh.mPosition = resource->mSharedOpaque->mPosition;
					hdwMesh.mIndices = resource->mSharedOpaque->mIndices;
					hdwMesh.mNormalBuffer = resource->mSharedOpaque->mNormalBuffer;
					for (int j = 0; j < ITexture::MAX_TEXCOORD_COUNT; ++j)
						hdwMesh.mTexcoord[j] = resource->mSharedOpaque->mTexcoord[j];
					hdwMesh.mBoneWeights = resource->mSharedOpaque->mBoneWeights;
					hdwMesh.mBoneIndices = resource->mSharedOpaque->mBoneIndices;

					hdwMesh.mVertexSource = resource->mSharedOpaque->mVertexSource;
					hdwMesh.mGeometry.mVertexSource = resource->mSharedOpaque->mVertexSource;

					assert(hdwMesh.mVertexSource != NULL);
					shared = true;
				}
			}

			if (shared)
			{

				//setup geometry & vertex source

				hdwMesh.mGeometry.mPrimitiveType = GraphicsGeometry::GPT_TRIANGLE_LIST;

				hdwMesh.mGeometry.useIndexBuffer(true);
				hdwMesh.mGeometry.mIndexBuffer = hdwMesh.mIndices;
				hdwMesh.mGeometry.mIndexStart = softMesh.mGeometry.mIndexStart;
				hdwMesh.mGeometry.mIndexCount = softMesh.mGeometry.mIndexCount;

				hdwMesh.mGeometry.mVertexDecl = ModelConfigManager::getSingleton().getVertexDeclaration();
				hdwMesh.mGeometry.mVertexStart = softMesh.mGeometry.mVertexStart;
				hdwMesh.mGeometry.mVertexCount = softMesh.mGeometry.mVertexCount;
			}
			else
			{
				manager.cloneVertexBuffer(hdwMesh.mPosition, *softMesh.mPosition, IGraphicsBuffer::GBU_CPU_READ);//enable picking
				if (softMesh.mNormalBuffer != NULL)
					manager.cloneVertexBuffer(hdwMesh.mNormalBuffer, *softMesh.mNormalBuffer, IGraphicsBuffer::GBU_CPU_READ);//for pre-transform vertices(pos & normal)

				for (int j = 0; j < softMesh.mTexcoordCount; ++j)
				{
					//note: flipping image content is stupid, really not a good option.
					//now Blade flips texture coordinate (V) instead of flipping image content
					if (flipUV)
					{
						IModelResource::UV* uv = (IModelResource::UV*)softMesh.mTexcoord[j]->lock(IGraphicsBuffer::GBLF_NORMAL);
						size_t count = softMesh.mTexcoord[j]->getVertexCount();
						for (size_t n = 0; n < count; ++n)
							uv[n].v = 1.0f - uv[n].v;
						softMesh.mTexcoord[j]->unlock();
					}

					manager.cloneVertexBuffer(hdwMesh.mTexcoord[j], *softMesh.mTexcoord[j], IGraphicsBuffer::GBU_STATIC);
				}
				manager.cloneIndexBuffer(hdwMesh.mIndices, *softMesh.mIndices, IGraphicsBuffer::GBU_CPU_READ);

				//setup geometry & vertex source
				hdwMesh.mGeometry.mPrimitiveType = GraphicsGeometry::GPT_TRIANGLE_LIST;
				hdwMesh.mVertexSource = IVertexSource::create();

				hdwMesh.mGeometry.useIndexBuffer(true);
				hdwMesh.mGeometry.mIndexBuffer = hdwMesh.mIndices;
				hdwMesh.mGeometry.mIndexStart = 0;
				hdwMesh.mGeometry.mIndexCount = (uint32)hdwMesh.mIndices->getIndexCount();

				hdwMesh.mGeometry.mVertexDecl = ModelConfigManager::getSingleton().getVertexDeclaration();
				hdwMesh.mGeometry.mVertexStart = 0;
				hdwMesh.mGeometry.mVertexCount = (uint32)hdwMesh.mPosition->getVertexCount();
				hdwMesh.mGeometry.mVertexSource = hdwMesh.mVertexSource;

				hdwMesh.mVertexSource->setSource(MVSI_POSITION, hdwMesh.mPosition);
				hdwMesh.mVertexSource->setSource(MVSI_NORMAL_TANGENT, hdwMesh.mNormalBuffer);
				for (size_t j = 0; j < softMesh.mTexcoordCount; ++j)
					hdwMesh.mVertexSource->setSource((uint16)(MVSI_UV0 + j), hdwMesh.mTexcoord[j]);

				if (softMesh.mBoneWeights != NULL || softMesh.mBoneIndices != NULL)
				{
					assert(softMesh.mBoneWeights != NULL && softMesh.mBoneIndices != NULL);
					manager.cloneVertexBuffer(hdwMesh.mBoneWeights, *softMesh.mBoneWeights, IGraphicsBuffer::GBU_STATIC);
					manager.cloneVertexBuffer(hdwMesh.mBoneIndices, *softMesh.mBoneIndices, IGraphicsBuffer::GBU_DEFAULT);

					assert(hdwMesh.mBoneIndices != NULL && hdwMesh.mBoneWeights != NULL);
					//assert( this->isAnimated() ); //allow no skeleton bound
					hdwMesh.mVertexSource->setSource(MVSI_BONEWEIGHTS, hdwMesh.mBoneWeights);
					hdwMesh.mVertexSource->setSource(MVSI_BONEINDICES, hdwMesh.mBoneIndices);
				}
			}

			hdwMesh.mName = softMesh.mName;
			hdwMesh.mAAB = softMesh.mAAB;
			hdwMesh.mRadius = softMesh.mRadius;
			hdwMesh.mTexcoordCount = softMesh.mTexcoordCount;
			hdwMesh.mMaterialID = softMesh.mMaterialID;
			hdwMesh.mPreTransformed = 0;
			hdwMesh.mActiveBones = softMesh.mActiveBones;
			hdwMesh.mBones = softMesh.mBones;
			hdwMesh.mBoneCount = softMesh.mBoneCount;
			hdwMesh.mBoneBoundings = softMesh.mBoneBoundings;
			
			resource->mSubMesh.push_back(hdwMesh);
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	ModelSerializer_Binary::mergeBuffer(IModelResource::MESHDATA** meshes, size_t count, IModelResource::MESHDATA& outMergedTarget)
	{
		size_t ibCount = 0;
		size_t posCount = 0;
		size_t normalCount = 0;
		size_t uvCount[ITexture::MAX_TEXCOORD_COUNT];
		size_t uvSets = 0;
		std::memset(uvCount, 0, sizeof(uvCount));
		size_t blendWeightCount = 0;
		size_t blendIndicesCount = 0;
		IIndexBuffer::EIndexType indexType = IIndexBuffer::IT_16BIT;
		for (size_t i = 0; i < count; ++i)
		{
			const IModelResource::MESHDATA& mesh = *(meshes[i]);
			size_t vertexCount = mesh.mPosition->getVertexCount();
			posCount += vertexCount;
			normalCount += mesh.mNormalBuffer != NULL ? mesh.mNormalBuffer->getVertexCount() : 0;

			if (indexType == IIndexBuffer::IT_16BIT)
			{
				if (mesh.mIndices->getIndexType() != IIndexBuffer::IT_16BIT)
					indexType = IIndexBuffer::IT_32BIT;
			}
			ibCount += mesh.mIndices->getIndexCount();

			uvSets = std::max(uvSets, (size_t)mesh.mTexcoordCount);
			for (size_t j = 0; j < mesh.mTexcoordCount; ++j)
			{
				assert(mesh.mTexcoord[j]->getVertexCount() == vertexCount);
				uvCount[j] += vertexCount;
			}

			blendWeightCount += mesh.mBoneWeights != NULL ? mesh.mBoneWeights->getVertexCount() : 0;
			blendIndicesCount += mesh.mBoneIndices != NULL ? mesh.mBoneIndices->getVertexCount() : 0;

			outMergedTarget.mAAB.merge(mesh.mAAB);
		}

		//normal exist (or not) the same for all sub meshes
		assert(normalCount == 0 || normalCount == posCount);
		if (indexType == IIndexBuffer::IT_16BIT)
			indexType = IndexBufferHelper::calcIndexType(posCount);

		IGraphicsResourceManager& manager = IGraphicsResourceManager::getOtherSingleton(IGraphicsResourceManager::TYPE_TEMPORARY);
		HVBUFFER hVB = manager.createVertexBuffer(NULL, sizeof(IModelResource::POSITION), posCount, IGraphicsBuffer::GBU_DEFAULT);
		HIBUFFER hIB = manager.createIndexBuffer(NULL, indexType, ibCount, IGraphicsBuffer::GBU_DEFAULT);
		HVBUFFER hNormal = normalCount != 0 ? manager.createVertexBuffer(NULL, sizeof(IModelResource::NORMAL), normalCount, IGraphicsBuffer::GBU_DEFAULT) : HVBUFFER::EMPTY;
		HVBUFFER hUV[ITexture::MAX_TEXCOORD_COUNT];
		for (int i = 0; i < ITexture::MAX_TEXCOORD_COUNT; ++i)
		{
			if (uvCount[i] != 0)
			{
				uvCount[i] = posCount;	//use uv for all sub-meshes, or there's no way to merge
				hUV[i] = manager.createVertexBuffer(NULL, sizeof(IModelResource::UV), uvCount[i], IGraphicsBuffer::GBU_DEFAULT);
			}
		}
		//use blend weights, blend indices for all sub meshes, or there's no way to merge
		if (blendWeightCount != 0)
			blendWeightCount = posCount;
		if (blendIndicesCount != 0)
			blendIndicesCount = posCount;

		HVBUFFER hBlendWeights = blendWeightCount != 0 ? manager.createVertexBuffer(NULL, sizeof(IModelResource::BONE_WEIGHT), blendWeightCount, IGraphicsBuffer::GBU_DEFAULT) : HVBUFFER::EMPTY;
		HVBUFFER hBlendIndices = blendIndicesCount != 0 ? manager.createVertexBuffer(NULL, sizeof(IModelResource::BONE_INDICE), blendIndicesCount, IGraphicsBuffer::GBU_DEFAULT) : HVBUFFER::EMPTY;

		IModelResource::POSITION* vb = (IModelResource::POSITION*)hVB->lock(IGraphicsBuffer::GBLF_WRITEONLY);
		char* ib = (char*)hIB->lock(IGraphicsBuffer::GBLF_WRITEONLY);
		size_t ibSize = IndexBufferHelper::calcIndexSize(indexType);
		IModelResource::NORMAL* nb = (IModelResource::NORMAL*)(hNormal != NULL ? hNormal->lock(IGraphicsBuffer::GBLF_WRITEONLY) : NULL);
		IModelResource::UV* uv[ITexture::MAX_TEXCOORD_COUNT];
		std::memset(uv, (int)NULL, sizeof(uv));
		for (int i = 0; i < ITexture::MAX_TEXCOORD_COUNT; ++i)
		{
			if (hUV[i] != NULL)
				uv[i] = (IModelResource::UV*)hUV[i]->lock(IGraphicsBuffer::GBLF_WRITEONLY);
		}
		IModelResource::BONE_WEIGHT* blendWeights = (IModelResource::BONE_WEIGHT*)(hBlendWeights != NULL ? hBlendWeights->lock(IGraphicsBuffer::GBLF_WRITEONLY) : NULL);
		IModelResource::BONE_INDICE* blendIndices = (IModelResource::BONE_INDICE*)(hBlendIndices != NULL ? hBlendIndices->lock(IGraphicsBuffer::GBLF_WRITEONLY) : NULL);

		size_t vertexOffset = 0;
		size_t indexOffset = 0;
		for (size_t i = 0; i < count; ++i)
		{
			IModelResource::MESHDATA& mesh = *(meshes[i]);
			size_t currentCount = mesh.mPosition->getVertexCount();

			void* vbSrc = mesh.mPosition->lock(IGraphicsBuffer::GBLF_READONLY);
			void* ibSrc = mesh.mIndices->lock(IGraphicsBuffer::GBLF_READONLY);
			void* nbSrc = mesh.mNormalBuffer != NULL ? mesh.mNormalBuffer->lock(IGraphicsBuffer::GBLF_READONLY) : NULL;
			void* uvSrc[ITexture::MAX_TEXCOORD_COUNT];
			std::memset(uvSrc, (int)NULL, sizeof(uvSrc));
			for (int j = 0; j < ITexture::MAX_TEXCOORD_COUNT; ++j)
			{
				if(mesh.mTexcoord[j] != NULL)
					uvSrc[j] = mesh.mTexcoord[j]->lock(IGraphicsBuffer::GBLF_READONLY);
			}
			void* blendWeightSrc = mesh.mBoneWeights != NULL ? mesh.mBoneWeights->lock(IGraphicsBuffer::GBLF_READONLY) : NULL;
			void* blendIndiceSrc = mesh.mBoneIndices != NULL ? mesh.mBoneIndices->lock(IGraphicsBuffer::GBLF_READONLY) : NULL;

			assert(vertexOffset < posCount);
			assert(indexOffset < ibCount);

			//position
			std::memcpy(vb + vertexOffset, vbSrc, mesh.mPosition->getVertexBytes());

			//indices.
			//merge indices and apply offsets so that it can be used in single draw call, and also can be used in multiple draw calls by sub-meshes (need set vertexStart,vertexCount properly)
			IndexBufferHelper::copyIndices(ib + indexOffset * ibSize, indexType, ibSrc, mesh.mIndices->getIndexType(), vertexOffset, mesh.mIndices->getIndexCount());

			if (nbSrc != NULL)
			{
				assert(nb != NULL);
				assert(mesh.mNormalBuffer->getVertexCount() == mesh.mPosition->getVertexCount());
				std::memcpy(nb + vertexOffset, nbSrc, mesh.mNormalBuffer->getVertexBytes());
			}
			else
				assert(nb == NULL);

			//UVs
			for (int j = 0; j < ITexture::MAX_TEXCOORD_COUNT; ++j)
			{
				if (uv[j] != NULL)
				{
					assert(uvSrc[j] == NULL || mesh.mTexcoord[j]->getVertexCount() == mesh.mPosition->getVertexCount());

					if (uvSrc[j] == NULL)
						std::memset(uv[j] + vertexOffset, 0, currentCount * sizeof(IModelResource::UV));
					else
						std::memcpy(uv[j] + vertexOffset, uvSrc[j], currentCount * sizeof(IModelResource::UV));
				}
				else
					assert(uvSrc[j] == NULL);
			}

			//there' should be no gap, fill 0 if current sub mesh has not bone weights
			if (blendWeightCount != 0)
			{
				if(blendWeightSrc == NULL)
					std::memset(blendWeights + vertexOffset, 0, currentCount * sizeof(IModelResource::BONE_WEIGHT));
				else
				{
					assert(mesh.mBoneWeights->getVertexCount() == mesh.mPosition->getVertexCount());
					std::memcpy(blendWeights + vertexOffset, blendWeightSrc, currentCount * sizeof(IModelResource::BONE_INDICE));
				}
			}
			else
				assert(blendWeightSrc == NULL);

			//there' should be no gap, fill 0 if current sub mesh has not bone indices
			if (blendIndicesCount != 0)
			{
				if(blendIndices == NULL)
					std::memset(blendIndices + vertexOffset, 0, currentCount * sizeof(IModelResource::BONE_INDICE));
				else
				{
					assert(mesh.mBoneIndices->getVertexCount() == mesh.mPosition->getVertexCount());
					std::memcpy(blendIndices + vertexOffset, blendIndiceSrc, currentCount * sizeof(IModelResource::BONE_INDICE));
				}
			}

			mesh.mPosition->unlock();
			mesh.mIndices->unlock();
			if (mesh.mNormalBuffer != NULL)
				mesh.mPosition->unlock();
			for (int j = 0; j < ITexture::MAX_TEXCOORD_COUNT; ++j)
			{
				if (mesh.mTexcoord[j] != NULL)
					mesh.mTexcoord[j]->unlock();
			}
			if (mesh.mBoneWeights != NULL)
				mesh.mBoneWeights->unlock();
			if (mesh.mBoneIndices != NULL)
				mesh.mBoneIndices->unlock();

			//used data
			mesh.mGeometry.mVertexStart = 0;//vertexOffset;
			mesh.mGeometry.mIndexStart = indexOffset;
			mesh.mGeometry.mVertexCount = posCount;//mesh.mPosition->getVertexCount();
			mesh.mGeometry.mIndexCount = mesh.mIndices->getIndexCount();

			indexOffset += mesh.mIndices->getIndexCount();
			vertexOffset += mesh.mPosition->getVertexCount();

			//alt mesh buffer
			//mesh.mPosition = hVB;
			//mesh.mIndices = hIB;
			//mesh.mNormalBuffer = hNormal;
			//mesh.mTexcoordCount = (uint8)uvSets;
			//for (size_t j = 0; j < uvSets; ++j)
			//	mesh.mTexcoord[j] = hUV[j];
			//mesh.mBoneWeights = hBlendWeights;
			//mesh.mBoneIndices = hBlendIndices;
			mesh.mPosition = HVBUFFER::EMPTY;
			mesh.mIndices = HIBUFFER::EMPTY;
			mesh.mNormalBuffer = HVBUFFER::EMPTY;
			mesh.mTexcoordCount = (uint8)uvSets;
			for (size_t j = 0; j < uvSets; ++j)
				mesh.mTexcoord[j] = HVBUFFER::EMPTY;
			mesh.mBoneWeights = HVBUFFER::EMPTY;
			mesh.mBoneIndices = HVBUFFER::EMPTY;
		}

		hVB->unlock();
		outMergedTarget.mPosition = hVB;
		hIB->unlock();
		outMergedTarget.mIndices = hIB;

		if (hNormal != NULL)
			hNormal->unlock();
		outMergedTarget.mNormalBuffer = hNormal;

		for (int j = 0; j < ITexture::MAX_TEXCOORD_COUNT; ++j)
		{
			if (hUV[j] != NULL)
				hUV[j]->unlock();
			else
				break;
			outMergedTarget.mTexcoord[j] = hUV[j];
		}
		outMergedTarget.mTexcoordCount = (uint8)uvSets;
		if (hBlendWeights != NULL)
			hBlendWeights->unlock();
		outMergedTarget.mBoneWeights = hBlendWeights;

		if (hBlendIndices != NULL)
			hBlendIndices->unlock();
		outMergedTarget.mBoneIndices = hBlendIndices;

		outMergedTarget.mRadius = outMergedTarget.mAAB.getHalfSize().getLength();
		outMergedTarget.mName = BTString("merged_mesh_") + TStringHelper::fromPointer(meshes[0]);
		outMergedTarget.mBones = NULL;
		outMergedTarget.mBoneBoundings = NULL;
		outMergedTarget.mBoneCount = 0;
		outMergedTarget.mMaterialID = meshes[0]->mMaterialID;
		outMergedTarget.mPreTransformed = 0;
		return true;
	}

}//namespace Blade