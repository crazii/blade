/********************************************************************
	created:	2017/11/27
	filename: 	GrassSerializer.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include "GrassSerializer.h"
#include "GrassResource.h"
#include "GrassConfig.h"
#include <utility/StringHelper.h>
#include <interface/ILog.h>
#include <interface/IResourceManager.h>

namespace Blade
{
	const TString GrassSharedResource::GRASS_SHARED_RESOURCE_TYPE = BTString("SharedGrassResource");

	static const uint32 GRASS_HEADER_MAGIC = BLADE_FCC('B', 'L', 'G', 'S');

	struct GrassHeader
	{
		uint32 magic;
		uint32 size;
		uint32 x;
		uint32 z;
	};

	class GrassSubModel : public CascadeSerializer::SubResource
	{
	public:
		virtual void				generateParams(ParamList& params) const
		{
			params[ModelConsts::MODEL_PARAM_SOFTMODE] = true;
		}
	};

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool	GrassSharedSerializer::loadResource(IResource* resource, const HSTREAM& stream, const ParamList& /*params*/)
	{
		if (!stream->isValid() || resource == NULL)
			return false;
		if (resource->getType() != GrassSharedResource::GRASS_SHARED_RESOURCE_TYPE)
		{
			assert(false);
			return false;
		}
		GrassSharedResource* res = static_cast<GrassSharedResource*>(resource);
		GrassHeader header;
		if (stream->read(&header) != sizeof(header) || header.magic != GRASS_HEADER_MAGIC)
		{
			assert(false);
			return false;
		}
		size_t count = header.size*header.size*GrassConfig::DENSITY_BITS / (sizeof(uint32) * 8);
		res->mSize = header.size;
		res->mX = header.x;
		res->mZ = header.z;
		for (index_t i = 0; i < GrassConfig::MAX_LAYERS; ++i)
		{
			res->mDensity[i] = BLADE_RES_ALLOCT(uint32, count);
			if (res->mDensity[i] == NULL || (size_t)stream->read(res->mDensity[i], count) != count * sizeof(uint32))
			{
				assert(false);
				return false;
			}
		}

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void	GrassSharedSerializer::postProcessResource(IResource* /*resource*/)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	bool	GrassSharedSerializer::saveResource(const IResource* resource, const HSTREAM& stream)
	{
		if (resource->getType() != GrassSharedResource::GRASS_SHARED_RESOURCE_TYPE)
		{
			assert(false);
		}
		const GrassSharedResource* res = static_cast<const GrassSharedResource*>(resource);

		GrassHeader header;
		header.magic = GRASS_HEADER_MAGIC;
		header.size = res->mSize;
		header.x = res->mX;
		header.z = res->mZ;
		if (stream->write(&header) != sizeof(header))
		{
			assert(false);
			return false;
		}

		size_t count = header.size*header.size*GrassConfig::DENSITY_BITS / (sizeof(uint32) * 8);
		for (index_t i = 0; i < GrassConfig::MAX_LAYERS; ++i)
		{
			if ((size_t)stream->write(res->mDensity[i], count) != count * sizeof(uint32))
			{
				assert(false);
				return false;
			}
		}

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	GrassSharedSerializer::createResource(IResource* resource, ParamList& params)
	{
		uint32 size = params[GrassConsts::GRASS_PARAM_SIZE];
		//note: GRASS_PARAM_X GRASS_PARAM_Z used for shared resource has similar meaning as grass resource
		uint32 x = params[GrassConsts::GRASS_PARAM_X];
		uint32 z = params[GrassConsts::GRASS_PARAM_Z];

		GrassSharedResource* res = static_cast<GrassSharedResource*>(resource);
		res->mSize = size;
		res->mX = x;
		res->mZ = z;

		size_t count = size*size*GrassConfig::DENSITY_BITS / (sizeof(uint32) * 8);
		for (index_t i = 0; i < GrassConfig::MAX_LAYERS; ++i)
		{
			res->mDensity[i] = BLADE_RES_ALLOCT(uint32, count);
			std::memset(res->mDensity[i], 0, sizeof(uint32) * count);
		}
		return true;
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	GrassSerializer::GrassSerializer()
	{
		mGrassMeshes = NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	GrassSerializer::~GrassSerializer()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	bool	GrassSerializer::loadResource(IResource* resource, const HSTREAM& /*stream*/, const ParamList& params)
	{
		if (resource == NULL || resource->getType() != GrassConsts::GRASS_RESOURCE_TYPE)
		{
			assert(false);
			return false;
		}
		GrassResource* res = static_cast<GrassResource*>(resource);
		res->clear();	//in case of reloading

		const TString& sharedResPath = params[GrassConsts::GRASS_PARAM_SHARED_DATA];
		HRESOURCE sharedRes = IResourceManager::getSingleton().loadResourceSync(sharedResPath);
		res->mSharedResource = sharedRes;

		uint32 x = params[GrassConsts::GRASS_PARAM_X];
		uint32 z = params[GrassConsts::GRASS_PARAM_Z];
		uint32 range = params[GrassConsts::GRASS_PARAM_RANGE];
		this->buildInstanceBuffer(res, x, z, range);

		uint32 meshCount = params[GrassConsts::GRASS_PARAM_MESH_COUNT];
		meshCount = std::min<uint32>(meshCount, (uint32)GrassConfig::MAX_LAYERS);
		mGrassMeshes = this->addSubGroup();
		for (uint32 i = 0; i < meshCount; ++i)
		{
			const TString& path = params[GrassConsts::GRASS_PARAM_MESH_PREFIX + TStringHelper::fromUInt32(i)];
			GrassSubModel* subRes = BLADE_NEW GrassSubModel();
			subRes->setResourceInfo(ModelConsts::MODEL_RESOURCE_TYPE, path);
			mGrassMeshes->addSubState(subRes);
		}
		this->loadSubResources();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void	GrassSerializer::postProcessResource(IResource* resource)
	{
		if (resource->getType() != GrassConsts::GRASS_RESOURCE_TYPE)
		{
			assert(false);
			return;
		}

		for (size_t i = 0; i < mGrassMeshes->getSubStateCount(); ++i)
		{
			HRESOURCE meshRes = static_cast<SubResourceState*>(mGrassMeshes->getSubState(i))->getResource();
			this->adjustGrassMesh(i, meshRes);	//TODO: adjust mesh in loading process instead of post process? this need to directly load meshes instead of add it as sub resource
		}

		//update height

		GrassResource* res = static_cast<GrassResource*>(resource);
		res->mBounding = mBounding;
		res->setInstanceData(mSoftInstanceData);
	}

	//////////////////////////////////////////////////////////////////////////
	bool	GrassSerializer::saveResource(const IResource* /*resource*/, const HSTREAM& /*stream*/)
	{
		//grass resource is a PHONY compound resource which doesn't need save
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	GrassSerializer::createResource(IResource* resource, ParamList& params)
	{
		const TString& sharedResPath = params[GrassConsts::GRASS_PARAM_SHARED_DATA];
		uint32 meshCount = params[GrassConsts::GRASS_PARAM_MESH_COUNT];
		uint32 x = params[GrassConsts::GRASS_PARAM_X];
		uint32 z = params[GrassConsts::GRASS_PARAM_Z];
		uint32 range = params[GrassConsts::GRASS_PARAM_RANGE];

		GrassResource* res = static_cast<GrassResource*>(resource);
		
		meshCount = std::min<uint32>(meshCount, (uint32)GrassConfig::MAX_LAYERS);
		for (uint32 i = 0; i < meshCount; ++i)
		{
			const TString& mesh = params[GrassConsts::GRASS_PARAM_MESH_PREFIX + TStringHelper::fromUInt32(i)];
			ParamList meshParams;
			meshParams[ModelConsts::MODEL_PARAM_SOFTMODE] = true;
			HRESOURCE meshRes = IResourceManager::getSingleton().loadResourceSync(mesh, ModelConsts::MODEL_RESOURCE_TYPE, &meshParams);
			this->adjustGrassMesh(i, meshRes);
		}

		HRESOURCE sharedRes = IResourceManager::getSingleton().loadResourceSync(sharedResPath, TString::EMPTY, &params);
		if (sharedRes == NULL)
			sharedRes = IResourceManager::getSingleton().createResource(GrassSharedResource::GRASS_SHARED_RESOURCE_TYPE, sharedResPath, params);

		res->mSharedResource = sharedRes;
		this->buildInstanceBuffer(res, x, z, range);

		res->mBounding = mBounding;
		res->setInstanceData(mSoftInstanceData);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void	GrassSerializer::adjustGrassMesh(index_t targetLayer, const HRESOURCE& mesh)
	{
		assert(mesh != NULL && mesh->getType() == ModelConsts::MODEL_RESOURCE_TYPE);

		IGraphicsResourceManager& manager = IGraphicsResourceManager::getSingleton();
		IModelResource* model = static_cast<IModelResource*>(mesh);
		if (model->getSubMeshCount() == 0)
			return;
		if (model->getSubMeshCount() > 1)
			BLADE_LOG(Warning, BTString("only one sub mesh supported for grass"));

		const IModelResource::MESHDATA& meshData = model->getSubMeshData(0);

		//already processed
		if (meshData.mGeometry.mVertexCount == 0)
			return;

		size_t vertexCount = meshData.mPosition->getVertexCount();

		static_assert(
			IModelResource::POSITION_FMT == GrassConfig::GRASS_POSITION_FMT && sizeof(GrassConfig::POSITION) == sizeof(IModelResource::POSITION),
			"format mismatch.");

		IModelResource::POSITION* pos = (IModelResource::POSITION*)meshData.mPosition->lock(IGraphicsBuffer::GBLF_READONLY);
		HVBUFFER posVB = manager.createVertexBuffer(pos, sizeof(GrassConfig::POSITION), vertexCount, IGraphicsBuffer::GBU_STATIC);

		static_assert(
			IModelResource::UV_FMT != GrassConfig::GRASS_TEXCOORD_FMT || sizeof(GrassConfig::TEXCOORD) == sizeof(IModelResource::UV),
			"format mismatch.");

		HVBUFFER uvVB;
		if (meshData.mTexcoordCount > 0)
		{
			IModelResource::UV* uv = (IModelResource::UV*)meshData.mTexcoord[0]->lock(IGraphicsBuffer::GBLF_READONLY);

			if (IModelResource::UV_FMT != GrassConfig::GRASS_TEXCOORD_FMT)
			{
				GrassConfig::TEXCOORD* packUV = BLADE_TMP_ALLOCT(GrassConfig::TEXCOORD, vertexCount);
				for (size_t i = 0; i < vertexCount; ++i)
				{
					packUV[i].u = uv[i].getU();
					packUV[i].v = uv[i].getV();
				}
				uvVB = manager.createVertexBuffer(packUV, sizeof(GrassConfig::TEXCOORD), vertexCount, IGraphicsBuffer::GBU_STATIC);
				BLADE_TMP_FREE(packUV);
			}
			else
				uvVB = manager.createVertexBuffer(uv, sizeof(GrassConfig::TEXCOORD), vertexCount, IGraphicsBuffer::GBU_STATIC);
		}

		static_assert(
			IModelResource::NORMAL_FMT != GrassConfig::GRASS_NORMAL_FMT || sizeof(GrassConfig::NORMAL) == sizeof(IModelResource::NORMAL),
			"format mismatch.");

		HVBUFFER normalVB;
		IModelResource::NORMAL* normal = (IModelResource::NORMAL*)meshData.mNormalBuffer->lock(IGraphicsBuffer::GBLF_READONLY);
		if (IModelResource::NORMAL_FMT != GrassConfig::GRASS_NORMAL_FMT || model->getNormalType() != IModelResource::NT_NORMAL_VECTOR)
		{
			GrassConfig::NORMAL* packNormal = BLADE_TMP_ALLOCT(GrassConfig::NORMAL, vertexCount);
			for (size_t i = 0; i < vertexCount; ++i)
			{
				IModelResource::NORMAL n = normal[i];
				Vector3 normalv3;
				if (model->getNormalType() != IModelResource::NT_NORMAL_VECTOR)
					normalv3 = reinterpret_cast<IModelResource::TANGENT_FRAME&>(n).getNormal();
				else
					normalv3 = n.getNormal();
				packNormal[i] = Vector4::pack(Vector4(normalv3, 1));
			}
			normalVB = manager.createVertexBuffer(packNormal, sizeof(GrassConfig::TEXCOORD), vertexCount, IGraphicsBuffer::GBU_STATIC);
			BLADE_TMP_FREE(packNormal);
		}
		else
			normalVB = manager.createVertexBuffer(normal, sizeof(GrassConfig::TEXCOORD), vertexCount, IGraphicsBuffer::GBU_STATIC);

		const_cast<IModelResource::MESHDATA&>(meshData).mGeometry.reset();	//FIXME:
		meshData.mVertexSource->clearAllSource();
		meshData.mVertexSource->setSource(GVS_POSITION, posVB);
		meshData.mVertexSource->setSource(GVS_UV, uvVB);
		meshData.mVertexSource->setSource(GVS_NORMAL, normalVB);

		mSoftInstanceData[targetLayer].mesh = mesh;
		mSoftInstanceData[targetLayer].meshBounding = model->getModelAABB();
		mSoftInstanceData[targetLayer].indices = meshData.mIndices;
		mSoftInstanceData[targetLayer].vertices = meshData.mVertexSource;
	}

	//////////////////////////////////////////////////////////////////////////
	void GrassSerializer::buildInstanceBuffer(const GrassResource* res, uint32 x, uint32 z, uint32 range)
	{
		IGraphicsResourceManager& manager = IGraphicsResourceManager::getOtherSingleton(IGraphicsResourceManager::TYPE_TEMPORARY);
		const size_t size = res->getTotalSize();
		const size_t rowBitCount = size * GrassConfig::DENSITY_BITS;
		const size_t areaSize = GrassConfig::getSingleton().getInstancingSize();
		const size_t areaCount = range / areaSize;

		uint32 lx = x - res->getSharedOffsetX();	//to local data offset
		uint32 lz = z - res->getSharedOffsetZ();

		//local cache
		TempVector<Vector2> posXZ;
		TempVector<GrassConfig::INSTANCING_POSITION> instancingPos;
		posXZ.resize(GrassConfig::MAX_DENSITY);
		instancingPos.resize(GrassConfig::MAX_DENSITY);

		for (index_t i = 0; i < GrassConfig::MAX_LAYERS; ++i)
		{
			GrassResource::InstancingLayer& layer = mSoftInstanceData[i];
			layer.boundig = AABB::EMPTY;

			layer.areas.resize(areaCount);
			for (size_t j = 0; j < layer.areas.size(); ++j)
				layer.areas[j].resize(areaCount);

			const uint32* densities = res->getDensity(i);
			if (densities == NULL /*|| layer.mesh == NULL*/)
				continue;

			//TODO: get height & normal data. skip normal & position height here and fill it in postProcess by ray casting, and then calculate AAB
			//so all we need here is horizontal XZ only
			//note: post process resource now can be in specific task, so
			//ray cast can be called in post process.

			index_t startBit = (lz * size + lx) * GrassConfig::DENSITY_BITS;
			POINT3 startPos = POINT3((scalar)x, 0, (scalar)z);

			for (size_t j = 0; j < areaCount; ++j)
			{
				index_t areaRowStartBit = startBit + (areaCount * j * areaSize) * rowBitCount;

				for (size_t k = 0; k < areaCount; ++k)
				{
					index_t areaStartBit = areaRowStartBit + k * areaSize * GrassConfig::DENSITY_BITS;

					GrassResource::InstancingArea& area = layer.areas[j][k];

					POINT3 areaPos = startPos + POINT3((scalar)k*areaSize, 0, (scalar)j*areaSize);

					for (uint32 oz = 0; oz < areaSize; ++oz)
					{
						index_t rowBit = areaStartBit + oz * rowBitCount;

						for (uint32 ox = 0; ox < areaSize; ++ox)
						{
							index_t bit = rowBit + ox * GrassConfig::DENSITY_BITS;

							POINT3 densityStartPos = areaPos + POINT3((scalar)ox, 0, (scalar)oz);
							POINT3 densityEndPos = densityStartPos + Vector3(1, 0, 1);

							index_t index = bit / (sizeof(uint32) * 8);
							index_t offset = bit % (sizeof(uint32) * 8);
							uint32 density = (densities[index] >> offset) & GrassConfig::DENSITY_MASK;
							//create instancing data
							assert(density <= GrassConfig::MAX_DENSITY);
							area.count = density;
							area.bounding = AABB(densityStartPos, densityEndPos);
							layer.boundig.merge(area.bounding);
							
							Math::PoissonDistribution(reinterpret_cast<scalar*>(&posXZ[0]), density, 1.0f, 0.0f, 0.0f, false);
							for (uint32 l = 0; l < density; ++l)
							{
								POINT3 p = POINT3(posXZ[l].x, 0, posXZ[l].y);	//in AAB space. should do in shader: pos = pos*AABSize + AABmin
								instancingPos[l] = Vector4::pack01( Vector4(p,1) );
							}
							area.buffer = manager.createVertexBuffer(&instancingPos[0], sizeof(GrassConfig::INSTANCING_POSITION), density, IGraphicsBuffer::GBU_STATIC);
						}//for each (1mx1m)density in area
					}

				}//for each area
			}

		}//for each layer
	}
	
}//namespace Blade