/********************************************************************
	created:	2017/12/09
	filename: 	StaticModelSerializer.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "StaticModelSerializer.h"
#include "ModelConfigManager.h"
#include <interface/public/graphics/IGraphicsResourceManager.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	const TString StaticModelResource::STATIC_MODEL_RESOURCE_TYPE = BTString("static model resource");
	const TString StaticModelResource::PARAM_ORIGINAL_MODEL = BTString("original model file");
	const TString StaticModelResource::PARAM_PRE_TRANSFORM = BTString("pre transform");

	//////////////////////////////////////////////////////////////////////////
	bool	StaticModelSerializer::loadResource(IResource* resource, const HSTREAM& /*stream*/, const ParamList& params)
	{
		if (resource == NULL || resource->getType() != StaticModelResource::STATIC_MODEL_RESOURCE_TYPE)
		{
			assert(false);
			return false;
		}
		const Variant& model = params[StaticModelResource::PARAM_ORIGINAL_MODEL];
		if (!model.isValid())
			return false;

		const TString& file = model;
		if (file.empty())
			return false;
		mModelResource = this->addSubResource(file);

		const Variant& transform = params[StaticModelResource::PARAM_PRE_TRANSFORM];
		if (!transform.isValid())
			return false;
		mTransform = static_cast<const Matrix44&>(transform);

		this->loadSubResources();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void	StaticModelSerializer::postProcessResource(IResource* resource)
	{
		StaticModelResource* res = static_cast<StaticModelResource*>(resource);
		IGraphicsResourceManager& manager = IGraphicsResourceManager::getSingleton();

		for (StaticModelResource::PreXFormedSubMeshList::iterator i = res->mSubMeshes.begin(); i != res->mSubMeshes.end(); ++i)
		{
			const HVBUFFER& pos = i->mVertexSource->getBuffer(MVSI_POSITION);
			const HVBUFFER& normal = i->mVertexSource->getBuffer(MVSI_NORMAL_TANGENT);

			HVBUFFER hwPos;
			manager.cloneVertexBuffer(hwPos, *pos, IGraphicsBuffer::GBU_CPU_READ);
			HVBUFFER hwNormal;
			manager.cloneVertexBuffer(hwNormal, *normal, IGraphicsBuffer::GBU_CPU_READ);

			i->mPosition = hwPos;
			i->mNormalBuffer = hwNormal;
			i->mVertexSource->setSource(MVSI_POSITION, hwPos);
			i->mVertexSource->setSource(MVSI_NORMAL_TANGENT, hwNormal);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void StaticModelSerializer::postLoad(IResource* resource)
	{
		IModelResourceImpl* model = static_cast<IModelResourceImpl*>(mModelResource->getResource());
		StaticModelResource* res = static_cast<StaticModelResource*>(resource);

		if (model->getSkeletonResource() != NULL)
			return;

		IGraphicsResourceManager& manager = IGraphicsResourceManager::getOtherSingleton(IGraphicsResourceManager::TYPE_TEMPORARY);

		for (size_t i = 0; i < model->getSubMeshCount(); ++i)
		{
			const IModelResource::MESHDATA& submesh = model->getSubMeshData(i);

			const HVBUFFER& posVB = submesh.mVertexSource->getBuffer(MVSI_POSITION);
			const HVBUFFER& normalVB = submesh.mVertexSource->getBuffer(MVSI_NORMAL_TANGENT);
			size_t vertexCount = posVB->getVertexCount();
			assert(posVB->getVertexSize() == sizeof(IModelResource::POSITION));
			assert(normalVB->getVertexSize() == sizeof(IModelResource::NORMAL));
			assert(normalVB->getVertexCount() == vertexCount);

			//note: world space pos cannot store in half precision
			//need change vertex declaration too
			POINT3* transformedPos = BLADE_TMP_ALLOCT(POINT3, vertexCount);
			const IModelResource::POSITION* pos = (const IModelResource::POSITION*)posVB->lock(IGraphicsBuffer::GBLF_READONLY);
			for (size_t j = 0; j < vertexCount; ++j)
			{
				POINT4 p;
				p.x = pos[j].x.getFloat();
				p.y = pos[j].y.getFloat();
				p.z = pos[j].z.getFloat();
				p.w = 1;
				p *= mTransform;
				transformedPos[j] = reinterpret_cast<POINT3&>(p);
			}
			HVBUFFER tposVB = manager.createVertexBuffer(transformedPos, sizeof(POINT3), vertexCount, IGraphicsBuffer::GBU_STATIC);
			BLADE_TMP_FREE(transformedPos);
			posVB->unlock();

			void* normaldata = NULL;
			if (model->getNormalType() == IModelResource::NT_TANGENT_FRAME)
			{
				Quaternion rotation = static_cast<Matrix33>(mTransform);

				IModelResource::TANGENT_FRAME* transformedNormal = BLADE_TMP_ALLOCT(IModelResource::TANGENT_FRAME, vertexCount);
				const IModelResource::TANGENT_FRAME* normal = (const IModelResource::TANGENT_FRAME*)normalVB->lock(IGraphicsBuffer::GBLF_READONLY);
				for (size_t j = 0; j < vertexCount; ++j)
				{
					Vector4 v = Vector4::unpack(normal[j].packed);
					Quaternion q = reinterpret_cast<Quaternion&>(v);
					q *= rotation;
					q.normalize();

					IModelResource::TANGENT_FRAME tn;
					tn.packed = Vector4::pack(reinterpret_cast<Vector4&>(q));

					transformedNormal[j] = tn;
				}
				normaldata = transformedNormal;
				normalVB->unlock();
			}

			if (model->getNormalType() == IModelResource::NT_NORMAL_VECTOR)
			{
				IModelResource::NORMAL* transformedNormal = BLADE_TMP_ALLOCT(IModelResource::NORMAL, vertexCount);
				const IModelResource::NORMAL* normal = (const IModelResource::NORMAL*)normalVB->lock(IGraphicsBuffer::GBLF_READONLY);
				for (size_t j = 0; j < vertexCount; ++j)
				{
					Vector4 v = Vector4::unpack(normal[j].packed);
					v.w = 0;
					v *= mTransform;

					IModelResource::NORMAL tn;
					tn.packed = Vector4::pack(v);

					transformedNormal[j] = tn;
				}
				normaldata = transformedNormal;
				normalVB->unlock();
			}

			HVBUFFER tnormalVB = normaldata == NULL ? HVBUFFER::EMPTY :
				manager.createVertexBuffer(normaldata, sizeof(IModelResource::NORMAL), vertexCount, IGraphicsBuffer::GBU_STATIC);
			BLADE_TMP_FREE(normaldata);

			HVERTEXSOURCE hvs = submesh.mVertexSource->clone();
			hvs->setSource(MVSI_POSITION, tposVB);
			hvs->setSource(MVSI_NORMAL_TANGENT, tnormalVB);

			IModelResource::MESHDATA XformedMesh = submesh;
			XformedMesh.mPosition = tposVB;
			XformedMesh.mNormalBuffer = tnormalVB;
			XformedMesh.mVertexSource = hvs;
			XformedMesh.mGeometry.mVertexSource = hvs;
			XformedMesh.mGeometry.mVertexDecl = ModelConfigManager::getSingleton().getCombinedBatchDeclarartion();
			XformedMesh.mPreTransformed = 1;

			res->mSubMeshes.push_back(XformedMesh);
		}

		res->mModel = mModelResource->getResource();
	}
}//namespace Blade