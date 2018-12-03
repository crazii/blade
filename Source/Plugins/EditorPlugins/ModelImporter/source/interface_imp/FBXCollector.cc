/********************************************************************
	created:	2015/09/23
	filename: 	FBXCollector.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <interface/IResourceManager.h>
#include "FBXCollector.h"
#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS


#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(disable: 4347 4482)
#endif

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	FBXCollector::FBXCollector()
	{
		mTimeMode = FbxTime::eDefaultMode;
		mExternalTexture = false;
		mAnimtaionScene = NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	FBXCollector::~FBXCollector()
	{

	}

	/************************************************************************/
	/* IModelCollector interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	size_t			FBXCollector::getMaterialCount()
	{
		return mMaterials.size();
	}

	//////////////////////////////////////////////////////////////////////////
	bool			FBXCollector::getMaterial(index_t index, IModelResource::MATERIAL_INFO& info)
	{
		if( index < mMaterials.size() )
		{
			FbxSurfaceMaterial* mat = mMaterials[index];
			Color ambient = Color::WHITE;
			Color diffuse = Color::WHITE;
			Color emissive = Color::WHITE;
			Color specular = Color::WHITE;

			if( mat->Is<FbxSurfaceLambert>() )
			{
				FbxSurfaceLambert* matLambert = static_cast<FbxSurfaceLambert*>(mat);
				FbxDouble3 color = matLambert->Ambient.Get();
				FbxDouble factor = matLambert->AmbientFactor.Get();
				color[0] *= factor; color[1] *= factor; color[2] *= factor;
				ambient = Color((scalar)color[0], (scalar)color[1], (scalar)color[2]);

				color = matLambert->Emissive.Get();
				factor = matLambert->EmissiveFactor.Get();
				color[0] *= factor; color[1] *= factor; color[2] *= factor;
				emissive = Color((scalar)color[0], (scalar)color[1], (scalar)color[2]);

				color = matLambert->Diffuse.Get();
				factor = matLambert->DiffuseFactor.Get();
				color[0] *= factor; color[1] *= factor; color[2] *= factor;
				diffuse = Color((scalar)color[0], (scalar)color[1], (scalar)color[2]);
				if( matLambert->Is<FbxSurfacePhong>() )
				{
					FbxSurfacePhong* matPhong = static_cast<FbxSurfacePhong*>(matLambert);
					color = matPhong->Specular.Get();
					factor = matPhong->SpecularFactor.Get();
					color[0] *= factor; color[1] *= factor; color[2] *= factor;
					specular = Color((scalar)color[0], (scalar)color[1], (scalar)color[2]);
				}
			}

			info.ambient = ambient;
			info.diffuse = diffuse;
			info.emissive = emissive;
			info.specular = specular;
			info.hasTransparency = false;
			//FBX uses external texture files, keep suffix on runtime to make sure texture loading right
			info.textureFullPath = mExternalTexture;

			//note: materials: must match the order of EMapChannel
			const char* texProperties[] = 
			{
				FbxSurfaceMaterial::sDiffuse,
				FbxSurfaceMaterial::sNormalMap,
				FbxSurfaceMaterial::sSpecularFactor,
				FbxSurfaceMaterial::sShininess,
				FbxSurfaceMaterial::sSpecular,
				FbxSurfaceMaterial::sEmissive,
				//extra property to check alpha blending
				FbxSurfaceMaterial::sTransparentColor,
			};

			for(int i = IModelResource::MC_BEGIN; i < IModelResource::MC_COUNT+1; ++i)
			{
				FbxProperty prop = mat->FindProperty( texProperties[i] );
				int count = prop.IsValid() ? prop.GetSrcObjectCount<FbxTexture>() : 0;

				//hard fix on normal map
				if( count == 0 && i == IModelResource::MC_NORMAL )
				{
					prop = mat->FindProperty(FbxSurfaceMaterial::sBump);
					count = prop.IsValid() ? prop.GetSrcObjectCount<FbxTexture>() : 0;
				}

				for(int j = 0; j < count; ++j)
				{
					bool hasTexture = false;
					FbxLayeredTexture* layer = FbxCast<FbxLayeredTexture>(prop.GetSrcObject<FbxLayeredTexture>(j));
					TString fullPath;

					if( layer != NULL )
					{
						int texCount = layer->GetSrcObjectCount<FbxTexture>();
						for(int k = 0; k < texCount; ++k)
						{
							FbxTexture* tex = static_cast<FbxTexture*>( layer->GetSrcObject<FbxTexture>(k) );
							FbxFileTexture* fileTex = FbxCast<FbxFileTexture>(tex);
							if( fileTex != NULL )
							{
								if( i < IModelResource::MC_COUNT )
								{
									const char* file = fileTex->GetFileName();
									fullPath = StringConverter::StringToTString(file);
									info.textures.push_back(fullPath);

									info.channelMap[i] = (int8)info.textures.size()-1;

									hasTexture = true;
								}
								else
									info.hasTransparency = true;
								break;
							}
						}
					}
					else
					{
						for(int k = 0; k < count; ++k)
						{
							FbxTexture* tex = static_cast<FbxTexture*>( prop.GetSrcObject<FbxTexture>(k) );
							FbxFileTexture* fileTex = FbxCast<FbxFileTexture>(tex);
							if( fileTex != NULL )
							{
								if( i < IModelResource::MC_COUNT )
								{
									//const char* file = fileTex->GetRelativeFileName();
									const char* file = fileTex->GetFileName();
									fullPath = StringConverter::StringToTString(file);
									info.textures.push_back(fullPath);
									info.channelMap[i] = (int8)info.textures.size()-1;

									hasTexture = true;
								}
								else
									info.hasTransparency = true;
								break;
							}
						}
					}

					//currently 1 texture supported for each channel
					if( hasTexture )
						break;

				}//for each fbx layer

			}//for each map channel

			return true;
		}
		assert(false);
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t			FBXCollector::getSubMeshCount()
	{
		return mMeshes.size();
	}

	//////////////////////////////////////////////////////////////////////////
	const tchar*	FBXCollector::getSubMeshName(index_t meshIndex)
	{
		if( meshIndex < mMeshes.size() )
			return mMeshes[meshIndex].name.c_str();
		assert(false);
		return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	const IModelCollector::SubMaterial*FBXCollector::getSubMeshMaterials(index_t meshIndex, size_t& materialCount)
	{
		if( meshIndex < mMeshes.size() )
		{
			materialCount = mMeshes[meshIndex].materialData.size();
			if( materialCount > 0 )
				return &mMeshes[meshIndex].materialData[0];
		}
		materialCount = 0;
		return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	const Vector3*	FBXCollector::getSubMeshVertices(index_t meshIndex, size_t& vertexCount)
	{
		if( meshIndex < mMeshes.size() )
		{
			vertexCount = mMeshes[meshIndex].vertexData.size();
			if( vertexCount > 0 )
				return &mMeshes[meshIndex].vertexData[0];
		}
		vertexCount = 0;
		return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	const IModelCollector::Triangle*	FBXCollector::getSubMeshTriangles(index_t meshIndex, size_t& triangleCount)
	{
		if( meshIndex < mMeshes.size() )
		{
			triangleCount = mMeshes[meshIndex].indexData.size();
			if( triangleCount > 0 )
				return &mMeshes[meshIndex].indexData[0];
		}
		triangleCount = 0;
		return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t			FBXCollector::getVertexBoneCount(index_t meshIndex, uint32 vertexIndex)
	{
		if( meshIndex < mMeshes.size() && vertexIndex < mMeshes[meshIndex].boneData.size() )
		{
			return mMeshes[meshIndex].boneData[vertexIndex].size();
		}
		return 0;
	}

	//////////////////////////////////////////////////////////////////////////
	float			FBXCollector::getVertexBoneWeight(index_t meshIndex, uint32 vertexIndex, index_t boneIndex)
	{
		if( meshIndex < mMeshes.size() && vertexIndex < mMeshes[meshIndex].boneData.size()
			&& boneIndex < mMeshes[meshIndex].boneData[vertexIndex].size() )
		{
			return mMeshes[meshIndex].boneData[vertexIndex][boneIndex].weight;
		}
		assert(false);
		return 0;
	}

	//////////////////////////////////////////////////////////////////////////
	const BONE_DATA*FBXCollector::getVertexBone(size_t meshIndex, uint32 vertexIndex , index_t boneIndex)
	{
		if( meshIndex < mMeshes.size() && vertexIndex < mMeshes[meshIndex].boneData.size()
			&& boneIndex < mMeshes[meshIndex].boneData[vertexIndex].size() )
		{
			index_t index = mMeshes[meshIndex].boneData[vertexIndex][boneIndex].dataIndex;
			if( index < mBoneData.size() )
			{
				assert( mBoneData[index].mIndex == index );
				return &mBoneData[index];
			}
		}
		assert(false);
		return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	TString			FBXCollector::getBoneName(const BONE_DATA* bone)
	{
		assert(mBoneData.size() == mBoneNodes.size());
		if( bone != NULL && bone->mIndex < mBoneData.size() )
		{
			assert( mBoneData[bone->mIndex].mIndex == bone->mIndex );
			return StringConverter::StringToTString( mBoneNodes[bone->mIndex]->GetName() );
		}
		assert(false);
		return TString::EMPTY;
	}

	//////////////////////////////////////////////////////////////////////////
	const BONE_DATA*FBXCollector::getBoneData(size_t& count)
	{
		count = mBoneData.size();
		return count == 0 ? NULL : &mBoneData[0];
	}

	//////////////////////////////////////////////////////////////////////////
	size_t			FBXCollector::getCameraCount()
	{
		return 0;
	}

	//////////////////////////////////////////////////////////////////////////
	IGraphicsCamera*FBXCollector::getCamera(index_t /*index*/)
	{
		return NULL;
	}

	/************************************************************************/
	/* IAnimationCollector interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	size_t			FBXCollector::getAnimationCount()
	{
		return mAnimations.size();
	}

	//////////////////////////////////////////////////////////////////////////
	AnimationDef*	FBXCollector::getAnimationDef(index_t index)
	{
		return index < mAnimations.size() ? &mAnimations[index] : NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t			FBXCollector::getBoneCount()
	{
		assert( mBoneData.size() == mBoneNodes.size() );
		return mBoneData.size();
	}

	//////////////////////////////////////////////////////////////////////////
	TString			FBXCollector::getBoneName(index_t index)
	{
		assert( mBoneData.size() == mBoneNodes.size() );
		assert( index < mBoneNodes.size() );
		return index < mBoneNodes.size() ? StringConverter::StringToTString(mBoneNodes[index]->GetName()) : TString::EMPTY;
	}

	//////////////////////////////////////////////////////////////////////////
	const BONE_DATA*FBXCollector::getBones()
	{
		return mBoneData.size() > 0 ? &mBoneData[0] : NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	void			FBXCollector::setCurrentAnimation(const TString& name)
	{
		if( mAnimtaionScene != NULL )
		{
			FbxAnimStack* animStack = mAnimtaionScene->FindSrcObject<FbxAnimStack>( StringConverter::TStringToString(name).c_str() );
			if(animStack != NULL )
				mAnimtaionScene->SetCurrentAnimationStack(animStack);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	scalar			FBXCollector::getFrameTime(uint32 frame)
	{
		return scalar(frame/FbxTime::GetFrameRate(mTimeMode));
	}

	//////////////////////////////////////////////////////////////////////////
	bool			FBXCollector::getBoneTransform(BoneDQ& transform, index_t boneIndex, scalar time)
	{
		if(boneIndex < mBoneNodes.size())
		{
			FbxNode* bone = mBoneNodes[boneIndex];

			double fps = FbxTime::GetFrameRate(mTimeMode);
			double frame = fps*(double)time;
			FbxTime t;
			t.SetFramePrecise(frame, mTimeMode);

			FbxAMatrix fbxMat;
			fbxMat = bone->EvaluateGlobalTransform(t);
			fbxMat.SetS(FbxVector4(1,1,1,1));

#if 0
			if( bone->GetParent() != NULL )
			{
				assert( mBoneData[boneIndex].mParent != uint32(-1) );
#else
			if( mBoneData[boneIndex].mParent != uint32(-1) )
			{
#endif
				assert( mBoneNodes[mBoneData[boneIndex].mParent] == bone->GetParent() );

				FbxAMatrix parentMat = bone->GetParent()->EvaluateGlobalTransform(t);
				parentMat.SetS(FbxVector4(1,1,1,1));

				fbxMat = parentMat.Inverse() * fbxMat;
			}

			//hack: some nodes only contains translation and scale is 0 (corrupted data). make it 1
			//but it is the artist's duty to make things right
			double* data = (double*)fbxMat;
			for(int i = 0; i < 15; ++i)
			{
				if( isnan(data[i]) )
					data[i] = 0;
			}
			if( fbxMat.Get(0,0) == 0 && fbxMat.Get(1,1) == 0 && fbxMat.Get(2,2) == 0 )
			{
				//BLADE_LOG(Error, BTString("bone node has zero scale: ") << bone->GetName());
				data[0] = data[5] = data[10] = 1;
			}

			transform = FbxMatrix2DQ(fbxMat);
			transform.normalize();
			assert( !isnan(transform.real.x) && !isnan(transform.dual.x) );
			return true;
		}
		assert(false);
		return false;
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool			FBXCollector::initAnimData(FbxScene* scene)
	{
		assert(scene != NULL);
		if( mMeshes.size() == 0 || scene == NULL)
		{
			assert(false && "empty mesh?");
			return false;
		}

		if(mBoneNodes.size() == 0)
			return false;

		int totalClusters = 0;
		for(size_t meshIndex = 0; meshIndex < mMeshes.size(); ++meshIndex)
		{
			FbxMesh* mesh = mMeshes[meshIndex].mesh;
			if( mesh == NULL )
				continue;

			size_t vertexCount = mMeshes[meshIndex].vertexData.size();
			BoneList& boneList = mMeshes[meshIndex].boneData;
			boneList.clear();
			boneList.resize(vertexCount);

			int skinCount = mesh->GetDeformerCount(FbxDeformer::eSkin);
			for(int skinIndex = 0; skinIndex < skinCount; ++skinIndex)
			{
				FbxSkin* skin = (FbxSkin*)mesh->GetDeformer(skinIndex, FbxDeformer::eSkin);
				if( skin == NULL )
					continue;

				int boneCount = skin->GetClusterCount();
				totalClusters += boneCount;

				//collect bone data for each mesh
				for(int i = 0; i < boneCount; i++)
				{
					FbxCluster* cluster = skin->GetCluster(i);
					FbxNode* boneNode = cluster->GetLink();
					if(boneNode == NULL)
						continue;

					BoneNodeList::iterator iter = std::find(mBoneNodes.begin(), mBoneNodes.end(), boneNode);
					assert(iter != mBoneNodes.end());
					index_t boneIndex = (index_t)(iter - mBoneNodes.begin());

					int* boneVertexIndices = cluster->GetControlPointIndices();
					double* boneVertexWeights = cluster->GetControlPointWeights();

					//all vertices affected by this bone
					int numBoneVertexIndices = cluster->GetControlPointIndicesCount();
					for (int j = 0; j < numBoneVertexIndices; j++) 
					{
						index_t vertexIndex = (index_t)boneVertexIndices[j];
						VertexBone& bonesVertex = boneList[vertexIndex];
						if( bonesVertex.size() == 0 )
							bonesVertex.reserve(4);

						BladeFbxBoneData bone;
						bone.weight = (float)boneVertexWeights[j];
						bone.dataIndex = boneIndex;
						bonesVertex.push_back(bone);
					}

#if 0
					FbxAMatrix globalBindingPose;
					cluster->GetTransformLinkMatrix(globalBindingPose);
				
					//on run time animation:
					//animated V = boneMat * globalBindingPose-1 * nodeL2W-1 * (V') , where V' = nodeL2W * V, V is original vertex
					//           = boneMat * globalBindingPose-1 * V

					DualQuaternion dq = FbxMatrix2DQ(globalBindingPose);
					dq.normalize();

					assert( !isnan(dq.real.x) && !isnan(dq.dual.x) );
					mBoneData[boneIndex].mInitialPose = dq;
#endif
					assert(mBoneData[boneIndex].mIndex == boneIndex);
				}//for each bone
			}//for each skin

		}//for each mesh
		//note: multiple clusters may have the same bone. check if bones collected in line 463, that enough.
		//assert( (size_t)totalClusters <= mBoneNodes.size() );

		//collect animation info
		mAnimations.clear();
		int animCount = scene->GetSrcObjectCount<FbxAnimStack>();
		
		TempMap<TString, uint32> animNameMap;

		for(int i = 0; i < animCount; ++i)
		{
			FbxAnimStack* animStack = scene->GetSrcObject<FbxAnimStack>(i);
			const char* name = animStack->GetName();
			//FbxTakeInfo* takeInfo = scene->GetTakeInfo( name );
			//FbxTime start = takeInfo->mLocalTimeSpan.GetStart();
			//FbxTime end = takeInfo->mLocalTimeSpan.GetStop();
			FbxTime start = animStack->LocalStart.Get();
			FbxTime end = animStack->LocalStop.Get();

			AnimationDef def;
			def.Name = StringConverter::StringToTString(name);
			def.StartFrame = (uint32)start.GetFrameCount(mTimeMode);
			def.EndFrame = (uint32)end.GetFrameCount(mTimeMode);
			def.Loop = true;	//TODO: loop info

			//note: FBX SDK may have redundant animation, why?
			AnimationDefList::iterator iter = std::find(mAnimations.begin(), mAnimations.end(), def);
			if( iter != mAnimations.end() )
			{
				//note: it should be a bug while other modeling tool exporting FBX
				//on this situation, FbxTakeInfo with the same name will be lost
				//assert( iter->StartFrame == def.StartFrame && iter->EndFrame == def.EndFrame );
			}
			else
				mAnimations.push_back(def);
		}

		mAnimtaionScene = scene;

		return mAnimations.size() > 0 && mBoneData.size() > 0 && mBoneNodes.size() > 0;
	}
	
}//namespace Blade

#endif//BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS