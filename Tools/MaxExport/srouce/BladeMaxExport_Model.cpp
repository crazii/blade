/********************************************************************
	created:	2014/05/02
	filename: 	BladeMaxExport_Model.cpp
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include "BladeMaxExport.h"

using namespace Blade;

//////////////////////////////////////////////////////////////////////////
void	BladeMaxExport::CollectCamera(IGameCamera* /*camera*/, IModelResource* /*target*/)
{

}

//////////////////////////////////////////////////////////////////////////
void	BladeMaxExport::CollectMesh(const TCHAR* name, IGameMesh* mesh, IGameMaterial* material)
{
	const bool USE_OBJECT_SPACE = false;

	mCollector.mMeshes.push_back( SubMeshData() );
	SubMeshData& subMeshData = mCollector.mMeshes.back();
	subMeshData.mesh = mesh;
	subMeshData.name = name;

	//vertex data
	subMeshData.vertexData.resize( (size_t)mesh->GetNumberOfVerts() );
	for(size_t i = 0; i < subMeshData.vertexData.size(); ++i)
	{
		Point3 v;
		mesh->GetVertex((int)i, v, USE_OBJECT_SPACE);
		subMeshData.vertexData[i].x = v.x;
		subMeshData.vertexData[i].y = v.y;
		subMeshData.vertexData[i].z = v.z;
	}

	//materials
	Tab<int> matIds = mesh->GetActiveMatIDs();
	subMeshData.materialData.reserve((size_t)matIds.Count());
	for(size_t i = 0; i < matIds.Count(); ++i)
	{
		Tab<FaceEx*> faces = mesh->GetFacesFromMatID(matIds[(INT_PTR)i]);
		if( faces.Count() <= 0 )
			continue;

		IGameMaterial* faceMat = mesh->GetMaterialFromFace( faces[0] );
		if( faceMat == NULL )
			faceMat = material;

		subMeshData.materialData.push_back(IModelCollector::SubMaterial());

		MaterialList::iterator iter = std::find(mCollector.mMaterials.begin(), mCollector.mMaterials.end(), faceMat);
		if( iter != mCollector.mMaterials.end() )
			subMeshData.materialData[i].materialID = (index_t)(iter - mCollector.mMaterials.begin());
		else
		{
			mCollector.mMaterials.push_back(faceMat);
			subMeshData.materialData[i].materialID = mCollector.mMaterials.size() - 1;
		}
	}


	//index data & uv
	subMeshData.indexData.reserve( (size_t)mesh->GetNumberOfFaces() );

	Tab<int> texMaps = mesh->GetActiveMapChannelNum();
	int maxChannel = std::min<int>(texMaps.Count(), ITexture::MAX_TEXTURE_COUNT);

	//do we need to export materials without textures?
	//if( maxChannel == 0 )	//oops, no maps skip
	//{
	//	callback.onNotify(1.0f);
	//	return;
	//}

	int realChannel = 0;	//real channel count
	for(int ch = 0; ch < maxChannel; ++ch)
	{
		if( mesh->GetMaxMesh()->mapSupport(texMaps[ch]) && mesh->GetNumberOfMapVerts(texMaps[ch]) > 0 )
			++realChannel;
	}

	//IGameSkin* skin = mesh->GetIGameSkin();

	size_t startTriangle = 0;

	for(int midx = 0; midx < matIds.Count(); ++midx)
	{
		size_t maxMatID = (size_t)matIds[midx];
		Tab<FaceEx*> faces = mesh->GetFacesFromMatID((int)maxMatID);

		if( faces.Count() <= 0 )
			continue;

		IModelCollector::SubMaterial& subMat = subMeshData.materialData[(size_t)midx];
		size_t matID = subMat.materialID;
		IGameMaterial* faceMat = mCollector.mMaterials[matID];

		subMat.name = faceMat->GetMaterialName();
		subMat.startTriangle = startTriangle;
		subMat.endTriangle  = startTriangle + faces.Count();
		startTriangle = subMat.endTriangle;

		for(int i = 0; i < faces.Count(); ++i)
		{
			FaceEx* face = faces[i];

			//index data
			subMeshData.indexData.push_back(IModelCollector::Triangle());
			IModelCollector::Triangle& triangle = subMeshData.indexData.back();

			for(int k = 0; k < 3; ++k)
			{
				DWORD vertexIndex = face->vert[k];
				triangle.indices[k] = vertexIndex;

				//texture coordinates
				if(realChannel == 0)
				{
					triangle.uv[k][0].x = 0;
					triangle.uv[k][0].y = 0;
					subMeshData.materialData[maxMatID].uvCount = 1;
					continue;
				}
				else if( realChannel == 1 )
				{
					Point2 UV = mesh->GetTexVertex( (int)face->texCoord[k] );
					triangle.uv[k][0].x = UV.x;
					triangle.uv[k][0].y = UV.y;
					subMeshData.materialData[maxMatID].uvCount = 1;
					continue;
				}

				int mapCount = faceMat->GetNumberOfTextureMaps();
				int channelCount = 0;
				for(int n = 0; n < mapCount; ++n)
				{
					IGameTextureMap* map = faceMat->GetIGameTextureMap(n);
					int ch = map->GetMapChannel();
					DWORD mapIndex[3];
					Point3 uvw;
					IGameUVGen* uvGen;
					if( (uvGen=map->GetIGameUVGen()) != NULL 
						&& mesh->GetMapFaceIndex(ch, face->meshFaceIndex, mapIndex) 
						&& mesh->GetMapVertex(ch, (int)mapIndex[k], uvw)
						)
					{
						GMatrix tm = uvGen->GetUVTransform();
						uvw = uvw * tm;
						triangle.uv[k][channelCount++] = Vector2(uvw.x, uvw.y);

						if( channelCount >= ITexture::MAX_TEXCOORD_COUNT )
							break;
					}
				}

				if( subMeshData.materialData[maxMatID].uvCount == 0 )
					subMeshData.materialData[maxMatID].uvCount = (size_t)channelCount;
				else
					assert( subMeshData.materialData[maxMatID].uvCount == (size_t)channelCount);

			}// for each k in face

		}// for  each face

	}//for each material
}

//////////////////////////////////////////////////////////////////////////
void BladeMaxExport::ExportModel(Blade::IModelResource* target, ProgressNotifier& callback)
{
	IModelBuilder* builder = BLADE_INTERFACE_CREATE(IModelBuilder);

	bool ret = builder->initialize(mConfig.getNormalType(), mConfig.isTangentSpaceLeftHanded());

	ret = ret && builder->collectMesh(&mCollector, callback);

	ret = ret && builder->buildMesh(target);
	assert(ret);

	BLADE_DELETE builder;
}

//////////////////////////////////////////////////////////////////////////
void	BladeMaxExport::ExportSkeleton(Blade::ISkeletonResource* target, Blade::ProgressNotifier& callback)
{
	IAnimationBuilder* builder = BLADE_INTERFACE_CREATE(IAnimationBuilder);

	bool ret = builder->initialize(mConfig.mFPS, mConfig.mAngleError, mConfig.mPositionError);

	ret = ret && builder->buildAnimation(&mCollector, target, callback);

	assert(ret);

	BLADE_DELETE builder;
}