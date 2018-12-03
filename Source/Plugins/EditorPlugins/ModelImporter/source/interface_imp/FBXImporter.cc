/********************************************************************
	created:	2015/09/24
	filename: 	FBXImporter.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <interface/public/ImporterParams.h>
#include <interface/IResourceManager.h>
#include "FBXImporter.h"
#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(disable: 4347 4482)
#endif

namespace Blade
{
	const TString FBXImporter::FBX_IMPORTER = BTString("FBX_Importer");
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	FBXImporter::FBXImporter()
	{
		mUnnamedIndex = 0;
	}

	//////////////////////////////////////////////////////////////////////////
	FBXImporter::~FBXImporter()
	{

	}

	/************************************************************************/
	/* IImporter interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool	FBXImporter::import(const HSTREAM& source, const HSTREAM& dest, const ParamList& params, TStringParam& extraFiles, ProgressNotifier& callback)
	{
		if( source == NULL || !source->isValid() )
		{
			assert(false);
			return true;
		}

		FbxSetMallocHandler(&BladeFbxMalloc);
		FbxSetCallocHandler(&BladeFbxCalloc);
		FbxSetReallocHandler(&BladeFbxRealloc);
		FbxSetFreeHandler(&BladeFbxFree);

		//create manager
		FbxManager* fbxManager = FbxManager::Create();

		//IO settings
		FbxIOSettings* ioSettings = FbxIOSettings::Create(fbxManager, IOSROOT);
		fbxManager->SetIOSettings(ioSettings);

		//create scene
		FbxScene* fbxScene = FbxScene::Create(fbxManager, "FbxScene");

		//import scene
		FbxImporter* fbxImporter = FbxImporter::Create(fbxManager, "FbxManager");

		BladeFbxStream stream(source);

		callback.beginStep((scalar)0.2);
		fbxImporter->SetProgressCallback(&BladeFbxProgressCallback, &callback);

		//note: int pFileFormat with -1 will cause failure, where are the valid format lists?
		if( !fbxImporter->Initialize(&stream, NULL, -1, ioSettings) 
			|| !fbxImporter->Import(fbxScene) )
		{
			BLADE_LOG(Error, BTString("failed to open fbx file: ") + source->getName());
			fbxImporter->Destroy();
			fbxManager->Destroy();
			return false;
		}
		fbxImporter->Destroy();

		//set time mode, used to calculate sampling frame rate
		mCollector.setTimeMode(fbxScene->GetGlobalSettings().GetTimeMode());

		//note: axis system only convert node transform. we need to convert vertices using node transform
		if(fbxScene->GetGlobalSettings().GetAxisSystem() != FbxAxisSystem::OpenGL)
			FbxAxisSystem::OpenGL.ConvertScene(fbxScene);

		FbxSystemUnit SceneSystemUnit = fbxScene->GetGlobalSettings().GetSystemUnit();

		//triangluate mesh: make sure all polygons are triangles
		{
			FbxGeometryConverter cvt(fbxManager);
			cvt.RemoveBadPolygonsFromMeshes(fbxScene);
			cvt.Triangulate(fbxScene, true);
		}

#if 1
		TempStack<FbxNode*> NodeStack;
		NodeStack.push(fbxScene->GetRootNode());

		while (!NodeStack.empty())
		{
			FbxNode* node = NodeStack.top();
			NodeStack.pop();

			//collect data & build model
			for (int i = 0; i < node->GetNodeAttributeCount(); ++i)
			{
				FbxNodeAttribute* attrib = node->GetNodeAttributeByIndex(i);
				if (attrib == NULL)
					continue;
				FbxNodeAttribute::EType type = attrib->GetAttributeType();
				switch (type)
				{
				case FbxNodeAttribute::eMesh:
					this->collectMesh(static_cast<FbxMesh*>(attrib));
					break;
				case FbxNodeAttribute::eSkeleton:
					//note: only need the top level(root) bone node
					//children are collected in collectSkeleton()
					if(node->GetParent() == NULL)
						this->collectSkeleton(fbxScene, static_cast<FbxSkeleton*>(attrib));
					else
					{
						FbxNode* parent = node->GetParent();
						bool parentSkeleton = false;
						for (int j = 0; j < parent->GetNodeAttributeCount(); ++j)
						{
							if (parent->GetNodeAttributeByIndex(j)->GetAttributeType() == FbxNodeAttribute::eSkeleton)
								parentSkeleton = true;
						}
						if(!parentSkeleton)
							this->collectSkeleton(fbxScene, static_cast<FbxSkeleton*>(attrib));
					}
					break;
				case FbxNodeAttribute::eLight:
					this->collectLight(static_cast<FbxLight*>(attrib));
					break;
				case FbxNodeAttribute::eCamera:
					this->collectCamera(static_cast<FbxCamera*>(attrib));
					break;
				default:
					break;
				}
			}

			for (int i = 0; i < node->GetChildCount(false); ++i)
				NodeStack.push(node->GetChild(i));
		}

#else

		FbxNode* fbxRoot = fbxScene->GetRootNode();
		if (fbxRoot != NULL)
		{
			//collect data & build model
			for (int i = 0; i < fbxRoot->GetChildCount(); ++i)
			{
				FbxNode* node = fbxRoot->GetChild(i);

				for (int j = 0; j < node->GetNodeAttributeCount(); ++j)
				{
					FbxNodeAttribute* attrib = node->GetNodeAttributeByIndex(j);
					if (attrib == NULL)
						continue;
					FbxNodeAttribute::EType type = attrib->GetAttributeType();
					switch (type)
					{
					case FbxNodeAttribute::eMesh:
						this->collectMesh(static_cast<FbxMesh*>(attrib));
						break;
					case FbxNodeAttribute::eSkeleton:
						this->collectSkeleton(static_cast<FbxSkeleton*>(attrib));
						break;
					case FbxNodeAttribute::eLight:
						this->collectLight(static_cast<FbxLight*>(attrib));
						break;
					case FbxNodeAttribute::eCamera:
						this->collectCamera(static_cast<FbxCamera*>(attrib));
						break;
					default:
						break;
					}
				}
			}
		}

		//no mesh?
		if (mCollector.mMeshes.size() == 0)
		{
			//try geometry directly
			for (int32 i = 0; i < fbxScene->GetGeometryCount(); ++i)
			{
				FbxGeometry * geometry = fbxScene->GetGeometry(i);
				if (geometry->GetAttributeType() == FbxNodeAttribute::eMesh)
				{
					FbxMesh* mesh = (FbxMesh*)geometry;
					this->collectMesh(mesh);
				}
			}
		}
#endif

		//no skeleton attributes, try to find animation node by cluster
		if( mCollector.mBoneNodes.size() == 0)
			this->collectSkeleton(fbxScene, NULL);

		//initialize animation data if any
		bool animation = mCollector.initAnimData(fbxScene);
		//TODO: popup config to toggle LOOP for all animations?


		IModelBuilder* builder = BLADE_INTERFACE_CREATE(IModelBuilder);

		//set build config using input parameter 'params'
		scalar fps = 25;
		scalar angleError = (scalar)0.4;
		scalar posError = (scalar)0.004;
		bool importAnimation = true;
		bool importMesh = true;

		if( params[IMPORT_MODEL_SAMPLING_RATE].isValid() )
			fps = params[IMPORT_MODEL_SAMPLING_RATE];
		if( params[IMPORT_MODEL_POS_ERROR].isValid() )
			posError = params[IMPORT_MODEL_POS_ERROR];
		if( params[IMPORT_MODEL_ANGLE_ERROR].isValid() )
			angleError = params[IMPORT_MODEL_ANGLE_ERROR];
		if( params[IMPORT_MODEL_ENABLE_SKELETON].isValid() )
			importAnimation = params[IMPORT_MODEL_ENABLE_SKELETON];
		if( params[IMPORT_MODEL_ENABLE_MESH].isValid() )
			importMesh = params[IMPORT_MODEL_ENABLE_MESH];

#if 0
		//by default: use external textures
		//note: if some artist are exporting meshes with textures,
		//then this option should be used.
		if (!params[IMPORT_MODEL_EXTERNAL_TEXTURE].isValid())
			mCollector.setUseExternalTexture(true);
		else
			mCollector.setUseExternalTexture(params[IMPORT_MODEL_EXTERNAL_TEXTURE]);
#else
		///note: FBX should be exported without textures
		///textures should be copied to texture source path (and compressed).
		mCollector.setUseExternalTexture( false );
#endif

		bool useSkeletonFullPath;
		if( !params[IMPORT_MODEL_SKELETON_FULLPATH].isValid() )
			useSkeletonFullPath = true;
		else
			useSkeletonFullPath = params[IMPORT_MODEL_SKELETON_FULLPATH];
		
		
		bool ret = true;

		callback.beginStep((scalar)0.5);
		TString skeletonPath;
		{
			const TString& fullPath = params[IMPORT_MODEL_PATH].isValid() ? params[IMPORT_MODEL_PATH] : TString::EMPTY;
			const TString& modelPath = importMesh ? dest->getName() : fullPath;
			TString path, dir, ext;
			TStringHelper::getFileInfo(modelPath, dir, ext);
			skeletonPath = dir + BTString(".") + ModelConsts::SKELETON_FILE_EXT;
		}

		ParamList dummy;
		if( animation && importAnimation )
		{
			HRESOURCE hSkeleton = IResourceManager::getSingleton().createResource(BTString("tmp.bls"), dummy);
			IAnimationBuilder* abuilder = BLADE_INTERFACE_CREATE(IAnimationBuilder);
			if( hSkeleton != NULL
				&& abuilder->initialize(fps, angleError, posError)
				&& abuilder->buildAnimation(&mCollector, static_cast<ISkeletonResource*>(hSkeleton), callback) )
			{
				ret = IResourceManager::getSingleton().saveResourceSync(hSkeleton, skeletonPath, false);
				assert(ret);

				TString dir, file;
				TStringHelper::getFilePathInfo(skeletonPath, dir, file);
				if( !useSkeletonFullPath )
					IResourceManager::getSingleton().addSearchPath( ModelConsts::SKELETON_RESOURCE_TYPE, dir, true);
				extraFiles.push_back(file);
			}
			IResourceManager::getSingleton().unloadResource(hSkeleton);
			BLADE_DELETE abuilder;
		}
		else
			callback.onStep(1, 1);

		//create a resource with any path, path will be overridden
		HRESOURCE hModel = IResourceManager::getSingleton().createResource(BTString("temp.blm"), dummy);
		if( hModel == NULL )
		{
			assert(false);
			return false;
		}

		//TODO: add config for tangent space handedness
		callback.beginStep((scalar)0.3);
		if( importMesh && builder->initialize(IModelResource::NT_TANGENT_FRAME, true)
			&& builder->collectMesh(&mCollector, callback)
			&& builder->buildMesh(static_cast<IModelResource*>(hModel)) )
		{
			//save model to stream
			assert(dest != NULL);
			static_cast<IModelResource*>(hModel)->setSkeletonRefernce(skeletonPath, useSkeletonFullPath);
			ret = ret && IResourceManager::getSingleton().saveResourceSync(hModel, dest);
			assert(ret);
		}
		else
			callback.onStep(1, 1);
		IResourceManager::getSingleton().unloadResource(hModel);
		BLADE_DELETE builder;

		fbxScene->Destroy();
		fbxManager->Destroy();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void			FBXImporter::collectMesh(FbxMesh* fbxMesh)
	{
		//mesh should be triangluated
		if( !fbxMesh->IsTriangleMesh() )
			return;

		mCollector.mMeshes.push_back( SubMeshData() );
		SubMeshData& subMesh = mCollector.mMeshes.back();
		subMesh.mesh = fbxMesh;

		//vertex data
		subMesh.name = StringConverter::StringToTString(fbxMesh->GetNode()->GetName());
		if( subMesh.name.empty() )
			subMesh.name = BTString("Unnamed_") + TStringHelper::fromUInt(mUnnamedIndex++);

		subMesh.vertexData.resize( (size_t)fbxMesh->GetControlPointsCount() );
		const FbxVector4* vertices = fbxMesh->GetControlPoints();

		//get node transform
		FbxAMatrix nodeMat = fbxMesh->GetNode()->EvaluateGlobalTransform();
		FbxAMatrix geomOffset;
		{
			const FbxVector4 t = fbxMesh->GetNode()->GetGeometricTranslation(FbxNode::eSourcePivot);
			const FbxVector4 r = fbxMesh->GetNode()->GetGeometricRotation(FbxNode::eSourcePivot);
			const FbxVector4 s = fbxMesh->GetNode()->GetGeometricScaling(FbxNode::eSourcePivot);
			geomOffset = FbxAMatrix(t, r, s);
		}
		//nodeMat.SetT(FbxVector4(0,0,0,1));
		nodeMat *= geomOffset;	//apply mesh offset

		for(int i = 0; i < fbxMesh->GetControlPointsCount(); ++i)
		{
			//FbxVector4 vertex = vertices[i];
			//apply node rotation (axis/coordinates conversion) to vertices
			//note: scale also need to apply to mesh, since Blade DQ animation doesn't contain scale information,
			//if we don't pre-scale mesh, the mesh cannot match skeleton animation if fbx bones contain scale
			//apply scale & rotation
			FbxVector4 vertex = nodeMat.MultT(vertices[i]);
			const double* pos = vertex;
			//subMesh.vertexData[(size_t)i] = Vector3((scalar)pos[0u], (scalar)pos[1u], (scalar)pos[2u]);
			//note: IMPORTANT: VS2010 compiler generates codes that corrupted stack frame.

			//https://msdn.microsoft.com/en-us/library/k1a8ss06.aspx
			/*
			Some SSE types require eight-byte stack alignment, forcing the compiler to emit dynamic stack-alignment code. 
			To be able to access both the local variables and the function parameters after the alignment, the compiler maintains two frame pointers. 
			If the compiler performs frame pointer omission (FPO), it will use EBP and ESP. If the compiler does not perform FPO, it will use EBX and EBP. 
			To ensure code runs correctly, do not modify EBX in asm code if the function requires dynamic stack alignment as it could modify the frame pointer. 
			Either move the eight-byte aligned types out of the function, or avoid using EBX.
			*/

			//since Vector3 probably uses SSE, then compiler will use EBX to handle stack alignment
			//but VS2010 compiler may generate assembly codes corrupting EBX register!
			//[disassembly found at subMesh.vertexData.resize( (size_t)fbxMesh->GetControlPointsCount() ); which uses EBX and never restore it!]
			//for VS2012+ compiler is OK.

			//this is a workaround that doesn't use SSE so that compiler doesn't use EBX in stack frame, and other codes uses EBX is safe.
			//or: use "Omit Frame Pointer" /Oy to avoid using EBX in stack frame, this is also tested OK.
			subMesh.vertexData[(size_t)i][0u] = (scalar)pos[0u];
			subMesh.vertexData[(size_t)i][1u] = (scalar)pos[1u];
			subMesh.vertexData[(size_t)i][2u] = (scalar)pos[2u];
		}

		//materials
		MaterialList& matList = mCollector.mMaterials;
		size_t matCount = (size_t)fbxMesh->GetNode()->GetMaterialCount();
		for(size_t i = 0; i < matCount; ++i)
		{
			FbxSurfaceMaterial* mat = fbxMesh->GetNode()->GetMaterial((int)i);
			subMesh.materialData.push_back( IModelCollector::SubMaterial() );

			MaterialList::iterator iter = std::find(matList.begin(), matList.end(), mat);
			if( iter != matList.end() )
				subMesh.materialData[i].materialID = index_t(iter - matList.begin());
			else
			{
				matList.push_back(mat);
				subMesh.materialData[i].materialID = matList.size() - 1u;
			}
			subMesh.materialData[i].name = StringConverter::StringToTString(mat->GetName());
		}

		//indices & texture coordinates
		assert( fbxMesh->GetPolygonVertexCount()%3 == 0 );
		subMesh.indexData.reserve( (size_t)fbxMesh->GetPolygonVertexCount() );

		int layerCount = fbxMesh->GetLayerCount();

		//split mesh by materials
		//note: different layers may have different material mapping to different triangle.
		//we need to split mesh for each layer

		typedef TempVector<int> TriangleList;
		typedef TempMap<index_t, TriangleList>	MatTriangles;
		typedef TempVector<const char*> UVSetNameList;
		MatTriangles matTriangles;
		UVSetNameList matUVSets;

		matUVSets.reserve( ITexture::MAX_TEXCOORD_COUNT*32 );
		for(int i = 0; i < layerCount; ++i)
		{
			FbxLayer* layer = fbxMesh->GetLayer(i);
			FbxLayerElementMaterial* layerMaterial = layer->GetMaterials();
			if(layerMaterial == NULL )
				continue;

			FbxLayerElement::EMappingMode mappingMode = layerMaterial->GetMappingMode();
			FbxLayerElement::EReferenceMode referenceMode = layerMaterial->GetReferenceMode();
			//eDirect deprecated
			if( (referenceMode != FbxLayerElementMaterial::eIndex) && (referenceMode != FbxLayerElementMaterial::eIndexToDirect) )
				continue;

			if( mappingMode == FbxLayerElement::EMappingMode::eAllSame )	//all polygon uses the same material
			{
				int index = layerMaterial->GetIndexArray()[0];
				FbxSurfaceMaterial* mat = fbxMesh->GetNode()->GetMaterial(index);

				//find material in model's global material list
				index_t matIndex = (index_t)(std::find(matList.begin(), matList.end(), mat) - matList.begin());

				//find sub material in sub mesh
				IModelCollector::SubMaterial target;
				target.materialID = matIndex;
				SubMeshMaterials::iterator iter = std::find(subMesh.materialData.begin(), subMesh.materialData.end(), target);
				assert(iter != subMesh.materialData.end());
				matIndex = (index_t)(iter - subMesh.materialData.begin());

				TriangleList& triangles = matTriangles[matIndex];
				triangles.resize( (size_t)fbxMesh->GetPolygonCount() );
				for(size_t j = 0; j < triangles.size(); ++j)
					triangles[j] = (int)j;
			}
			else if( mappingMode == FbxLayerElement::eByPolygon ) //each polygon may have different material
			{
				FbxLayerElementArrayTemplate<int>& matIndices = layerMaterial->GetIndexArray();
				int count = matIndices.GetCount();	//material count for all polygons
				//j is the index of polygon
				for(int j = 0; j < count; ++j)
				{
					int index = matIndices[j];
					FbxSurfaceMaterial* mat = fbxMesh->GetNode()->GetMaterial(index);

					//find material in model's global material list
					index_t matIndex = (index_t)(std::find(matList.begin(), matList.end(), mat) - matList.begin());

					//find sub material in sub mesh
					IModelCollector::SubMaterial target;
					target.materialID = matIndex;
					SubMeshMaterials::iterator iter = std::find(subMesh.materialData.begin(), subMesh.materialData.end(), target);
					assert(iter != subMesh.materialData.end());
					matIndex = (index_t)(iter - subMesh.materialData.begin());

					TriangleList& triangles = matTriangles[matIndex];
					if( triangles.size() == 0 )
						triangles.reserve(  (size_t)fbxMesh->GetPolygonCount() );
					triangles.push_back(j);
				}
			}

			int uvSetCount = layer->GetUVSetCount();
			FbxArray<const FbxLayerElementUV*> uvSets = layer->GetUVSets();
			for(int j = 0; j < uvSetCount; ++j)
			{
				const FbxLayerElementUV* uvSet = uvSets[j];
				const char* uvSetName = NULL;
				if(uvSet != NULL && (uvSetName=uvSet->GetName()) != NULL && std::find(matUVSets.begin(), matUVSets.end(), uvSetName) == matUVSets.end() )
					matUVSets.push_back( uvSetName );
			}
		}

		size_t startTriangle = 0;

		for(MatTriangles::iterator iter = matTriangles.begin(); iter != matTriangles.end(); ++iter)
		{
			index_t matIndex = iter->first;
			TriangleList& triangles = iter->second;

			IModelCollector::SubMaterial& subMat = subMesh.materialData[matIndex];
			//FbxSurfaceMaterial* mat = matList[subMat.materialID];
			//subMat.name = StringConverter::StringToTString(mat->GetName());	//already been set

			subMat.startTriangle = startTriangle;
			subMat.endTriangle  = startTriangle + triangles.size();
			startTriangle = subMat.endTriangle;

			for(size_t j = 0; j < triangles.size(); ++j)
			{
				int triangleIndex = triangles[j];
				subMesh.indexData.push_back(IModelCollector::Triangle());
				IModelCollector::Triangle& triangle = subMesh.indexData.back();

				assert( fbxMesh->GetPolygonSize(triangleIndex) == 3);

				for(int k = 0; k < 3; ++k)
				{
					//indices
					int vertexIndex = fbxMesh->GetPolygonVertex(triangleIndex, k);
					triangle.indices[k] = (uint32)vertexIndex;

					//texture coordinates / uv
					size_t uvCount = 0;
					FbxVector2 uv;
					bool bUnmapped = true;
					for(size_t n = 0; n < matUVSets.size(); ++n)
					{
						if( fbxMesh->GetPolygonVertexUV(triangleIndex, k, matUVSets[n], uv, bUnmapped) && !bUnmapped )
							triangle.uv[k][uvCount++] = Vector2((scalar)uv.mData[0], (scalar)uv.mData[1]);
					}

					if( subMesh.materialData[matIndex].uvCount == 0 )
						subMesh.materialData[matIndex].uvCount = uvCount;
					else
						assert( subMesh.materialData[matIndex].uvCount == uvCount);
				}// for each vertex k in triangle(polygon)

			}//for each triangle with same materials
		}//for each material
	}

	//////////////////////////////////////////////////////////////////////////
	bool			FBXImporter::collectSkeleton(FbxScene* scene, FbxSkeleton* fbxSkeleton)
	{
		mCollector.mBoneNodes.reserve(256);
		TempVector<DualQuaternion*> bindingPoses;
		TempList<DualQuaternion> bindingPoseCache;
		bindingPoses.reserve(256);

		if( fbxSkeleton != NULL )
		{
			TempStack<FbxNode*> bones;
			bones.push(fbxSkeleton->GetNode());
			while(!bones.empty())
			{
				FbxNode* node = bones.top();
				bones.pop();

				FbxSkeleton* skeleton = NULL;
				for (int i = 0; i < node->GetNodeAttributeCount(); ++i)
				{
					if (node->GetNodeAttributeByIndex(i)->GetAttributeType() == FbxNodeAttribute::eSkeleton)
					{
						skeleton = (FbxSkeleton*)node->GetNodeAttributeByIndex(i);
						break;
					}
				}
				if (skeleton != NULL)
				{
					mCollector.mBoneNodes.push_back(node);
					for (int i = 0; i < node->GetChildCount(); i++)
						bones.push(node->GetChild(i));
				}
			}
		}

		//note: check binding pose first
		//commonly some bone nodes in cluster may not in binding pose
		//but there may be weird case when bone not in cluster, but in bind pose
		if (mCollector.mBoneNodes.size() > 0)
		{
			int32 poseCount = scene->GetPoseCount();
			for (int32 j = 0; j < poseCount; j++)
			{
				const FbxPose* pose = scene->GetPose(j);
				if (pose != NULL && pose->IsBindPose())
				{
					for (size_t i = 0; i < mCollector.mBoneNodes.size(); ++i)
					{
						int k = pose->Find(mCollector.mBoneNodes[i]);
						if (k >= 0 && !pose->IsLocalMatrix(k))
						{
							FbxAMatrix bindingPoseM = reinterpret_cast<const FbxAMatrix&>(pose->GetMatrix(k));
							if (i >= bindingPoses.size())
								bindingPoses.resize(i + i / 2 + 1);
							bindingPoseM.SetS(FbxVector4(1, 1, 1, 1));
							DualQuaternion dq = FbxMatrix2DQ(bindingPoseM);
							dq.normalize();
							bindingPoseCache.push_back(dq);
							bindingPoses[i] = &bindingPoseCache.back();
						}
					}
				}
			}
		}

		//try clusters, and get binding pose from cluster
		for (size_t meshIndex = 0; meshIndex < mCollector.mMeshes.size(); ++meshIndex)
		{
			FbxMesh* mesh = mCollector.mMeshes[meshIndex].mesh;
			if (mesh == NULL)
				continue;

			int skinCount = mesh->GetDeformerCount(FbxDeformer::eSkin);
			for (int skinIndex = 0; skinIndex < skinCount; ++skinIndex)
			{
				FbxSkin* skin = (FbxSkin*)mesh->GetDeformer(skinIndex, FbxDeformer::eSkin);
				if (skin == NULL)
					continue;

				int boneCount = skin->GetClusterCount();

				//collect bone data for each mesh
				for (int i = 0; i < boneCount; i++)
				{
					FbxCluster* cluster = skin->GetCluster(i);
					FbxNode* boneNode = cluster->GetLink();
					if (boneNode == NULL)
						continue;
					BoneNodeList::iterator it = std::find(mCollector.mBoneNodes.begin(), mCollector.mBoneNodes.end(), boneNode);
					if (it == mCollector.mBoneNodes.end())
					{
						mCollector.mBoneNodes.push_back(boneNode);
						it = --mCollector.mBoneNodes.end();
					}

					index_t index = (index_t)(it - mCollector.mBoneNodes.begin());
					if (index >= bindingPoses.size())
						bindingPoses.resize(index + index / 2 + 1);

					//avoid redundant calculation
					if (bindingPoses[index] == NULL)
					{
						FbxAMatrix bindingPose;
						cluster->GetTransformLinkMatrix(bindingPose);
						DualQuaternion dq = FbxMatrix2DQ(bindingPose);
						dq.normalize();
						assert(!isnan(dq.real.x) && !isnan(dq.dual.x));
						bindingPoseCache.push_back(dq);
						bindingPoses[index] = &(bindingPoseCache.back());
					}

				}//for each bone
			}//for each skin
		}//for each mesh

		//init bond index & binding pose
		mCollector.mBoneData.resize( mCollector.mBoneNodes.size() );
		for(size_t i = 0; i < mCollector.mBoneNodes.size(); ++i)
		{
			DualQuaternion bindingPose;
			if (i < bindingPoses.size() && bindingPoses[i] != NULL)
				bindingPose = *bindingPoses[i];
			else
			{
				FbxAMatrix bindingPoseM = mCollector.mBoneNodes[i]->EvaluateGlobalTransform();

				bindingPoseM.SetS(FbxVector4(1, 1, 1, 1));
				bindingPose = FbxMatrix2DQ(bindingPoseM);
				bindingPose.normalize();
			}

			BONE_DATA boneData;
			boneData.mParent = uint32(-1);
			boneData.mIndex = (uint32)i;
			boneData.mInitialPose = bindingPose;
			mCollector.mBoneData[i] = boneData;
		}

		//update parent index
		for(size_t i = 0; i < mCollector.mBoneData.size(); ++i)
		{
			BONE_DATA& boneData = mCollector.mBoneData[i];
			FbxNode* node = mCollector.mBoneNodes[i];

			FbxNode* parent = node->GetParent();
			uint32 parentIndex = (uint32)-1;
			if( parent != NULL )
			{
				BoneNodeList::iterator iter = std::find(mCollector.mBoneNodes.begin(), mCollector.mBoneNodes.end(), parent);
				if( iter != mCollector.mBoneNodes.end() )
				{
					index_t index = index_t(iter - mCollector.mBoneNodes.begin());
					parentIndex = mCollector.mBoneData[index].mIndex;
					assert(parentIndex == (uint32)index);
				}
				else
				{
					//assert(false);
					//must be root bone (root bone may have parent, ignore it)
				}
			}
			boneData.mParent = parentIndex;
		}

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void			FBXImporter::collectLight(FbxLight* /*fbxLight*/)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void			FBXImporter::collectCamera(FbxCamera* /*fbxCamera*/)
	{
		
	}
	
}//namespace Blade

#endif//BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS