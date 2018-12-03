/********************************************************************
	created:	2015/09/07
	filename: 	ModelBuilder.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "ModelBuilder.h"
#include <interface/public/graphics/IGraphicsResourceManager.h>
#include <interface/public/graphics/PixelColorFormatter.h>
#include <GraphicsShaderShared.inl>
#include "Bone.h"

namespace Blade
{
	const TString IModelBuilder::MODEL_BUILDER_CLASS = BTString("ModelBuilder");
	template class Factory<IModelBuilder>;

	//helper functions
	static Color::COLOR packFloat4(const scalar* f4)
	{
		Vector4 v(f4);
		//[-1,1] => [0,1]
		v += Vector4::UNIT_ALL;
		v /= 2.0f;

		//use Blade::Color to convert it to endian independent byte array
		const Blade::Color color = reinterpret_cast<const Blade::Color&>(v);
		return color.getRGBA();	//float(r,g,b,a) =>r8,g8,b8,a8 byte array
	}
	static Color::COLOR packFloat3(const scalar* f3)
	{
		Vector4 v(f3[0],f3[1],f3[2],1);
		return packFloat4(v.mData);
	}

	//////////////////////////////////////////////////////////////////////////
	bool	ModelBuilder::collectMesh(IModelCollector* collector, ProgressNotifier& callback)
	{
		if( collector == NULL )
		{
			assert(false);
			return false;
		}

		//collect materials
		size_t matCount = collector->getMaterialCount();
		mMaterials.resize(matCount);
		for(index_t matID = 0; matID < matCount; ++matID )
			collector->getMaterial(matID, mMaterials[matID]);
		mMeshes.resize( matCount );

		size_t meshCount = collector->getSubMeshCount();
		//calculate real sub mesh count - split mesh by materials
		{
			typedef TempMap<size_t, uint32> MaterialMeshCount;
			MaterialMeshCount matIDMeshCount;
			for(index_t meshID = 0; meshID < meshCount; ++meshID)
			{
				size_t subMatCount = 0;
				const IModelCollector::SubMaterial* meshMaterials = collector->getSubMeshMaterials(meshID, subMatCount);
				for(size_t i = 0; i < subMatCount; ++i)
					++matIDMeshCount[ meshMaterials[i].materialID ];
			}

			//set real count
			for(index_t matID = 0; matID < matCount; ++matID )
				mMeshes[matID].reserve( matIDMeshCount[matID] );
		}

		size_t boneCount = 0;
		const BONE_DATA* boneData = collector->getBoneData(boneCount);

		for(index_t meshID = 0; meshID < meshCount; ++meshID)
		{
			callback.beginStep( 1.0f/(scalar)meshCount );

			size_t subMatCount = 0;
			const IModelCollector::SubMaterial* meshMaterials = collector->getSubMeshMaterials(meshID, subMatCount);

			size_t vertexCount = 0;
			size_t triangleCount = 0;
			const Vector3* vertices = collector->getSubMeshVertices(meshID, vertexCount);
			const IModelCollector::Triangle* triangles = collector->getSubMeshTriangles(meshID, triangleCount);

			size_t triIndex = 0;
			for(size_t i = 0; i < subMatCount; ++i)
			{
				const IModelCollector::SubMaterial& subMat = meshMaterials[i];
				//this sub material is not used
				if(subMat.startTriangle == INVALID_INDEX && subMat.endTriangle == INVALID_INDEX)
					continue;

				size_t subMaterialID = subMat.materialID;
				assert( subMaterialID < matCount );
				const IModelResource::MATERIAL_INFO& matInfo = mMaterials[subMaterialID];
				
				size_t normalMapChannel = (size_t)matInfo.channelMap[IModelResource::MC_NORMAL];
				if( normalMapChannel >= subMat.uvCount )
					normalMapChannel = 0;

				MeshList& list = mMeshes[subMaterialID];
				list.push_back( SubMeshData() );
				SubMeshData& buildingMesh = list.back();
				buildingMesh.name = collector->getSubMeshName(meshID) + BTString(".") + subMat.name;

				size_t startTriangle = subMat.startTriangle;
				size_t endTriangle = subMat.endTriangle;
				assert( startTriangle <= endTriangle && endTriangle <= triangleCount );

				//reserve enough memory
				buildingMesh.triangles.reserve( triangleCount );
				buildingMesh.indices.reserve( triangleCount * 3 * 2 );
				buildingMesh.vertices.reserve( vertexCount * 2 );
				buildingMesh.uvCount = (uint8)subMat.uvCount;
				buildingMesh.boneBoundings.reserve(boneCount);

				for(size_t j = startTriangle; j < endTriangle; ++j)
				{
					callback.onStep(triIndex++, triangleCount);
					const IModelCollector::Triangle& tri = triangles[j];

					const Vector3& p0 = vertices[ tri.indices[0] ];
					const Vector3& p1 = vertices[ tri.indices[1] ];
					const Vector3& p2 = vertices[ tri.indices[2] ];

					//face normal, tangent, bitangent
					//ref: http://www.terathon.com/code/tangent.html
					//duplicate vertices for edge point of different handedness/(uv)reflection, to avoid seams
					//ideally the edge points (uv mirror center line) has different tangent frames, despite same space position

					Vector3 tangent;
					Vector3 bitangent;
					Vector3 normal;

					Vector3 P = p1 - p0;
					Vector3 Q = p2 - p0;
					normal = P.crossProduct(Q);
					//fix: SHOULD NOT normalize those!
					//P.normalize();
					//Q.normalize();
					normal.normalize();
					EUVMirror mirror = UVM_UNDECIDED;
					scalar triWeight = 1.0f;

					//check weird collapse ( triangle fall back to line/point )
					bool degenerate1 = Vector3::ZERO.equal(P, Math::HIGH_EPSILON);
					bool degenerate2 = Vector3::ZERO.equal(Q, Math::HIGH_EPSILON);
					if( degenerate1 || degenerate2 )
					{
						tangent = Vector3::UNIT_X;
						bitangent = Vector3::UNIT_Y;
						normal = Vector3::UNIT_Z;
						mirror = UVM_UNMIRRORED;
						triWeight = Math::LOW_EPSILON;
					}
					else if( mNormalType == IModelResource::NT_TANGENT_FRAME )
					{
						Vector2 UV0 = tri.uv[0][normalMapChannel];
						Vector2 UV1 = tri.uv[1][normalMapChannel];
						Vector2 UV2 = tri.uv[2][normalMapChannel];

						float v1 = UV1.y - UV0.y;
						float v2 = UV2.y - UV0.y;
						float u1 = UV1.x - UV0.x;
						float u2 = UV2.x - UV0.x;

						float r = u1*v2 - u2*v1;
						float areaMul2 = std::abs(r);
						//devide by zero
						if( areaMul2 <= Math::LOW_EPSILON || subMat.uvCount == 0)
						{
							tangent = Vector3::UNIT_X;
							bitangent = Vector3::UNIT_Y;
							if (std::abs(tangent.dotProduct(normal)) < std::abs(bitangent.dotProduct(normal)))
								bitangent = normal.crossProduct(tangent);
							else
								tangent = -normal.crossProduct(bitangent);
							//triWeight = Math::LOW_EPSILON;
							mirror = UVM_UNMIRRORED;
						}
						else
						{
							//2D triangle area = (u1*v2-u2*v1)/2
							//http://www.shaderx4.com/TangentSpaceCalculation.h
							r = 1.0f/r;
							tangent = (P*v2 - Q*v1)*r;
							bitangent = (Q*u1 - P*u2)*r;

							tangent.normalize();
							bitangent.normalize();

							tangent *= areaMul2;
							bitangent *= areaMul2;

							if( mTSLeftHanded )
								bitangent = -bitangent;

							mirror = normal.crossProduct(tangent).dotProduct(bitangent) < 0 ? UVM_MIRRORED : UVM_UNMIRRORED;
						}
					}
					else
					{
						tangent = Vector3::UNIT_X;
						bitangent = Vector3::UNIT_Y;
						mirror = UVM_UNMIRRORED;
					}

					assert(!isnan(tangent.x) && !isnan(tangent.y) && !isnan(tangent.z));
					assert(!isnan(bitangent.x) && !isnan(bitangent.y) && !isnan(bitangent.z));
					assert(!(tangent.x == 0 && tangent.y == 0 && tangent.z == 0));
					assert(!(bitangent.x == 0 && bitangent.y == 0 && bitangent.z == 0));

					//step 1: collect triangle TS data
					buildingMesh.triangles.push_back( ModelTriangle() );
					ModelTriangle& triangle = buildingMesh.triangles.back();
					triangle.normal = normal;
					triangle.tangent = tangent;
					triangle.bitangent = bitangent;
					triangle.mirror = mirror;
					triangle.weight = triWeight;

					for(int k = 0; k < 3; ++k)
					{
						uint32 vertexIndex = tri.indices[k];
						const Vector3& p = vertices[ vertexIndex ];
						ModelVertex v = p;
						v.mirror = UVM_UNDECIDED;
						v.index = vertexIndex;
						v.normalChannel = (uint32)normalMapChannel;
						std::memcpy(v.uv, tri.uv[k], buildingMesh.uvCount*sizeof(Vector2));

						//blend indices & blend weights
						std::memset( v.boneWeights, 0, sizeof(v.boneWeights) );
						std::memset( v.bones, 0xFF, sizeof(v.bones) );

						size_t vertexBoneCount = collector->getVertexBoneCount(meshID, vertexIndex);

						if( boneCount != 0 && vertexBoneCount > 0 )
						{
							buildingMesh.hasSkin = true;
							
							size_t realBoneCount = std::min<size_t>( (size_t)IModelResource::MAX_BONE_COUNT, vertexBoneCount );
							size_t maxBoneCount = std::max<size_t>( (size_t)IModelResource::MAX_BONE_COUNT, vertexBoneCount );

							//gather all weights & sort it, then
							//collect the first MAX_BONE_COUNT count of weights
							BoneInfoCache binfo;
							binfo.resize( maxBoneCount );
							for(size_t boneIndex = 0; boneIndex < vertexBoneCount; ++boneIndex)
							{
								binfo[boneIndex].weight = collector->getVertexBoneWeight(meshID, vertexIndex, (index_t)boneIndex);
								binfo[boneIndex].index = (int)boneIndex;
							}

							std::sort( binfo.begin(), binfo.end() );
							float totalRealWeight = 0;

							for(size_t n = 0; n < realBoneCount; ++n)
								totalRealWeight += binfo[n].weight;
							assert(totalRealWeight > 0);

							for(size_t n = 0; n < realBoneCount; ++n)
							{
								v.boneWeights[n] = binfo[n].weight/totalRealWeight;	//normalize

								assert( n+1 >= realBoneCount || binfo[n+1].weight <= binfo[n].weight);
								assert( binfo[n].index != -1 );
								const BONE_DATA* bone = collector->getVertexBone(meshID, vertexIndex, (index_t)binfo[n].index);

								assert(bone != NULL);

								//map to local ID
								v.bones[n] = buildingMesh.usedBoneSet.addBone( collector->getBoneName(bone) );
								if( v.bones[n] >= buildingMesh.boneBoundings.size() )
								{
									assert( v.bones[n] == buildingMesh.boneBoundings.size() );
									buildingMesh.boneBoundings.push_back(AABB::EMPTY);
								}
								if( v.boneWeights[n] > 0)
									buildingMesh.boneBoundings[ v.bones[n] ].merge( p );

								uint32 boneID = bone->mIndex;
								//record used bones for this model. note: only collect bone names, not bone indices, this makes it possible to bind model to different skeletons.
								//besides animation builder will re-sort bones, make bone index in-consistent
								while(boneID != (uint32)-1)
								{
									buildingMesh.activeBoneSet.addBone( collector->getBoneName(bone) );
									boneID = bone->mParent;
									assert(boneID == (uint32)-1 || boneID < boneCount);
									bone = boneData + boneID;
								}
							}
						}//if

						//calculate weight: angle of this point
						Vector3 s1 = vertices[ tri.indices[(k+1)%3] ] - p;
						Vector3 s2 = vertices[ tri.indices[(k+2)%3] ] - p;

						scalar weight = 0;
						if (s1.getSquaredLength() <= Math::LOW_EPSILON || s2.getSquaredLength() <= Math::LOW_EPSILON)
							weight = Math::LOW_EPSILON;
						else
						{
							s1.normalize();
							s2.normalize();
							weight = std::acos(Math::Clamp(s1.dotProduct(s2), -1.0f, 1.0f)) * triWeight;
						}

						//accumulate normals to vertices
						VertexNormalList& normalList = mNormalList[v];
						bool isNormalMerged = false;
						v.normal = normalList.addNormal(normal, weight, index_t(vertexIndex), isNormalMerged);

						//add index
						ModelIndex& index = buildingMesh.indexMap[v];
						if( !index.isValid() )
						{
							//uv may differs //assert( !isNormalMerged );
							v.tangent = Vector3::ZERO;
							v.bitangent = Vector3::ZERO;
							v.mirror = triangle.mirror;
							buildingMesh.vertices.push_back(v);
							index = buildingMesh.vertices.size() - 1;
						}
						triangle.originalIndex[k] = index;
						buildingMesh.indices.push_back( (uint32)index );

					}//for each point on triangle

				}//for each triangle

			}//for each meshID

		}//for each matID

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	ModelBuilder::buildMesh(IModelResource* target)
	{
		if(target == NULL || mMaterials.size() != mMeshes.size())
		{
			assert(false);
			return false;
		}

		size_t count = mMaterials.size();

		//exporting target data
		for(size_t i = 0; i < count; ++i)
		{
			//material
			IModelResource::MATERIAL_INFO modelMaterial = mMaterials[i];
			index_t materialID = target->addMaterial(modelMaterial);

			//sub meshes
			MeshList& list = mMeshes[i];
			for(size_t j = 0; j < list.size(); ++j )
			{
				SubMeshData& subMesh = list[j];
				this->buildSubmesh(subMesh, materialID, target);
			}
		}//for each sub mesh material 

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void	ModelBuilder::buildSubmesh(SubMeshData& subMesh, index_t materialID, IModelResource* target)
	{
		if( mNormalType == IModelResource::NT_TANGENT_FRAME )
		{
			VertexList vertices = subMesh.vertices;
			size_t originalCount = vertices.size();
			subMesh.vertices.clear();
			subMesh.vertices.reserve( originalCount * 2 );

			IndexList indices = subMesh.indices;
			subMesh.indices.clear();
			originalCount = indices.size();
			subMesh.indices.reserve( originalCount * 2 );

			VertexIndexMap indexMap = subMesh.indexMap;
			subMesh.indexMap.clear();

			subMesh.mirrorIndexMap[UVM_UNMIRRORED].clear();
			subMesh.mirrorIndexMap[UVM_MIRRORED].clear();
			subMesh.mirrorIndexMap[UVM_UNDECIDED].clear();

			for(size_t i = 0; i < subMesh.triangles.size(); ++i)
			{
				const ModelTriangle& triangle = subMesh.triangles[i];
				for(size_t j = 0; j < 3; ++j)
				{
					const ModelIndex& oldIndex = triangle.originalIndex[j];
					assert( oldIndex.isValid() );
					const ModelVertex p0 = vertices[ oldIndex ];	//DO NOT copy?
					Vector3 s1 = vertices[ triangle.originalIndex[ (j+1)%3 ] ].position - p0.position;
					Vector3 s2 = vertices[ triangle.originalIndex[ (j+2)%3 ] ].position - p0.position;
					scalar weight = 0;
					if(s1.getSquaredLength() <= Math::LOW_EPSILON || s2.getSquaredLength() <= Math::LOW_EPSILON)
						weight = Math::LOW_EPSILON;
					else
					{
						s1.normalize();
						s2.normalize();
						weight = std::acos(Math::Clamp(s1.dotProduct(s2), -1.0f, 1.0f));
						//scalar weight = triangle.weight;
					}

					assert(!isnan(weight));
					if (weight < Math::LOW_EPSILON)
						weight = Math::LOW_EPSILON;

					ModelIndex& index = subMesh.mirrorIndexMap[triangle.mirror][ p0 ];
					size_t targetIndex = INVALID_INDEX;
					//duplicate vertex
					if( !index.isValid() )
					{
						subMesh.vertices.push_back( p0 );
						targetIndex = subMesh.vertices.size() - 1;
						index = subMesh.vertices.size() - 1;
						ModelVertex& v = subMesh.vertices.back();
						v.tangent = triangle.tangent * weight;
						v.bitangent = triangle.bitangent * weight;
						v.mirror = triangle.mirror;
					}
					else
					{
						assert( index.mIndices.size() > 0 );
						for(size_t k = 0; k < index.mIndices.size(); ++k)
						{
							size_t subIndex = (size_t)index.mIndices[k];
							ModelVertex& v = subMesh.vertices[subIndex];
							if( !v.tangent.equal(Vector3::ZERO, Math::LOW_EPSILON) )
							{
								if( v.mirror != triangle.mirror )
									continue;
								Vector3 diagonal = v.tangent + v.bitangent;
								diagonal = diagonal - triangle.normal * triangle.normal.dotProduct(diagonal);
								if(  diagonal.dotProduct( triangle.tangent + triangle.bitangent ) <= 0 ) //larger than 90 degree
									continue;
							}
							targetIndex = subIndex;
							break;
						}

						if(targetIndex != INVALID_INDEX)
						{
							ModelVertex& v = subMesh.vertices[targetIndex];
							v.tangent += triangle.tangent * weight;
							v.bitangent += triangle.bitangent * weight;
							assert(v.mirror == triangle.mirror);
						}
						else
						{
							//normal split or tangent/bitangent differs too much, duplicate vertex
							const ModelVertex oldV = subMesh.vertices[ index ];
							subMesh.vertices.push_back( oldV );
							targetIndex = subMesh.vertices.size() - 1;
							index.add(targetIndex);
							ModelVertex& v = subMesh.vertices.back();
							v.tangent = triangle.tangent * weight;
							v.bitangent = triangle.bitangent * weight;
							assert(v.mirror == triangle.mirror);
						}
					}
					//add triangles
					subMesh.indices.push_back( (uint32)targetIndex );
				}
			}
		}

		size_t vertexCount = subMesh.vertices.size();
		size_t indexCount = subMesh.indices.size();

		Vector3 pmin(FLT_MAX,FLT_MAX,FLT_MAX);
		Vector3 pmax(-FLT_MAX,-FLT_MAX,-FLT_MAX);

		IGraphicsResourceManager& manager = IGraphicsResourceManager::getOtherSingleton(IGraphicsResourceManager::TYPE_TEMPORARY);

		//vertices
		HVBUFFER hVB = manager.createVertexBuffer(NULL,
			sizeof(IModelResource::POSITION),
			vertexCount,
			IGraphicsBuffer::GBU_DEFAULT);

		{
			IModelResource::POSITION* position = (IModelResource::POSITION*)hVB->lock( IGraphicsBuffer::GBLF_NORMAL);
			for(size_t i = 0; i < vertexCount; ++i)
			{
				const Vector3& pos = subMesh.vertices[i].position;
				pmin = Vector3::getMinVector(pmin, pos);
				pmax = Vector3::getMaxVector(pmax, pos);
				//Vector3 size = pmax - pmin;
				(*(position+i)).x = pos.x;
				//assert( Math::Equal((*(position+i)).x.getFloat()/size.x, pos.x/size.x, Math::LOW_EPSILON) );
				(*(position+i)).y = pos.y;
				//assert( Math::Equal((*(position+i)).y.getFloat()/size.y, pos.y/size.y, Math::LOW_EPSILON) );
				(*(position+i)).z = pos.z;
				//assert( Math::Equal((*(position+i)).z.getFloat()/size.z, pos.z/size.z, Math::LOW_EPSILON) );
				(*(position+i)).w = 1.0f;
			}
			hVB->unlock();
		}

		//export normals
		HVBUFFER hNormal;
		if( mNormalType != IModelResource::NT_NONE )
		{
			if( mNormalType == IModelResource::NT_NORMAL_VECTOR )
			{
				hNormal = manager.createVertexBuffer(NULL,
					sizeof(IModelResource::NORMAL),
					vertexCount,
					IGraphicsBuffer::GBU_DEFAULT
					);
			}
			else
			{
				hNormal = manager.createVertexBuffer(NULL,
					sizeof(IModelResource::TANGENT_FRAME),
					vertexCount,
					IGraphicsBuffer::GBU_DEFAULT
					);
			}

			void* normalData = hNormal->lock( IGraphicsBuffer::GBLF_NORMAL );
			for(size_t i = 0; i < vertexCount; ++i)
			{
				if( mNormalType == IModelResource::NT_NORMAL_VECTOR )
				{
					Vector3 normal = *subMesh.vertices[i].normal;	//DO NOT change original normal
					normal.normalize();

					( (IModelResource::NORMAL*)normalData)[ i ].normal = packFloat3(normal.mData);
				}
				else
				{
					ModelVertex& exportVertex = subMesh.vertices[i];

					Vector3 normal = *exportVertex.normal;	//DO NOT change original normal
					normal.normalize();

					Vector3 tangent = exportVertex.tangent;
					//use Gram-Schmidt process to orthogonalize TBN's basis
					tangent = tangent - normal * normal.dotProduct(tangent);
					tangent.normalize();

					Vector3 bitangent = normal.crossProduct(tangent);
					bitangent.normalize();

					bool mirror = (exportVertex.mirror == UVM_MIRRORED) || bitangent.dotProduct( exportVertex.bitangent ) < 0;

					//
					assert( Math::Equal(normal.dotProduct(tangent), 0.0f, Math::LOW_EPSILON )
						&& Math::Equal(normal.dotProduct(bitangent), 0.0f, Math::LOW_EPSILON) );

					//check the UV mirror, resulting "handedness" issue
					//if there's no UV mirror, the two vector should point to the same direction thus dot product should > 0
					//when there's UV mirror, the standard orthogonalize tangent-space need to be altered to match the normal map

					//the newly calculated orthogonalize bi-tangent replace the exportVertex's calculated right bitangent
					//but it will be restored in shader, using mirror info.

					Matrix33 TBN(tangent, bitangent, normal);
					Quaternion quat( TBN );
					quat.normalize();

					////save mirror into quaternion
					////ref: http://www.opengpu.org/forum.php?mod=viewthread&tid=10467

					if( quat.w < 0 )//because -quat == quat, we just make sure w component > 0
						quat = -quat;

					scalar bias = 1.0f / ( (1<<7) - 1 );
					if (quat.w < bias)
					{
						scalar normalizeCoeff = std::sqrt(1 - bias * bias);
						quat.x *= normalizeCoeff;
						quat.y *= normalizeCoeff;
						quat.z *= normalizeCoeff;
						quat.w = bias;
					}
					if( mirror )
						quat = -quat;

					( (IModelResource::TANGENT_FRAME*)normalData)[ i ].quat = packFloat4( quat.mData );
				}
			}//for
			hNormal->unlock();
		}//if mNormalType != IModelResource::NT_NONE

		//export bone indices & bone weights
		HVBUFFER boneIndices;
		HVBUFFER boneWeights;
		if( subMesh.hasSkin )
		{
			boneWeights = manager.createVertexBuffer(NULL, sizeof(IModelResource::BONE_WEIGHT), vertexCount, IGraphicsBuffer::GBU_DEFAULT );
			IModelResource::BONE_WEIGHT* weights = (IModelResource::BONE_WEIGHT*)boneWeights->lock(IGraphicsBuffer::GBLF_NORMAL);
			for(size_t i = 0; i < vertexCount; ++i)
			{
				IModelResource::BONE_WEIGHT weight;
				weight.weight[0] = (uint8)(subMesh.vertices[i].boneWeights[0]*255);
				weight.weight[1] = (uint8)(subMesh.vertices[i].boneWeights[1]*255);
				weight.weight[2] = (uint8)(subMesh.vertices[i].boneWeights[2]*255);
				weight.weight[3] = (uint8)(subMesh.vertices[i].boneWeights[3]*255);
				assert(weight.value != 0);
				weights[i] = weight;
			}

			boneIndices = manager.createVertexBuffer(NULL, sizeof(IModelResource::BONE_INDICE), vertexCount, IGraphicsBuffer::GBU_DEFAULT );
			IModelResource::BONE_INDICE* indices = (IModelResource::BONE_INDICE*)boneIndices->lock(IGraphicsBuffer::GBLF_NORMAL);
			for(size_t i = 0; i < vertexCount; ++i)
			{
				IModelResource::BONE_INDICE indice;
				indice.index[0] = (uint8)subMesh.vertices[i].bones[0];
				indice.index[1] = (uint8)subMesh.vertices[i].bones[1];
				indice.index[2] = (uint8)subMesh.vertices[i].bones[2];
				indice.index[3] = (uint8)subMesh.vertices[i].bones[3];
				assert( indice.value != 0xFFFFFFFF );
				assert( (indice.index[0] == 0xFF && weights[i].weight[0] == 0)  || indice.index[0] < subMesh.usedBoneSet.size() );
				assert( (indice.index[1] == 0xFF && weights[i].weight[1] == 0)  || indice.index[1] < subMesh.usedBoneSet.size() );
				assert( (indice.index[2] == 0xFF && weights[i].weight[2] == 0)  || indice.index[2] < subMesh.usedBoneSet.size() );
				assert( (indice.index[3] == 0xFF && weights[i].weight[3] == 0) || indice.index[3] < subMesh.usedBoneSet.size() );

				//fix array index overflow (255)
				if( indice.index[0] == 0xFF )
					indice.index[0] = 0;
				if( indice.index[1] == 0xFF )
					indice.index[1] = 0;
				if( indice.index[2] == 0xFF )
					indice.index[2] = 0;
				if( indice.index[3] == 0xFF )
					indice.index[3] = 0;

				indices[i] = indice;
			}
			boneWeights->unlock();
			boneIndices->unlock();
		}

		//export UVs
		HVBUFFER UV[ ITexture::MAX_TEXCOORD_COUNT ];
		for(int ch = 0; ch < subMesh.uvCount; ++ch)
		{
			HVBUFFER& UVChannel = UV[ ch ];
			UVChannel = manager.createVertexBuffer(NULL,
				sizeof(IModelResource::UV),
				vertexCount,
				IGraphicsBuffer::GBU_DEFAULT
				);

			IModelResource::UV* uv = (IModelResource::UV*)UVChannel->lock(IGraphicsBuffer::GBLF_NORMAL);
			for(size_t i = 0; i < vertexCount; ++i)
			{
				fp32 u = subMesh.vertices[i].uv[ch].x;
				//clamp to mirrored max, so that fp16 will hold the value
				//u = std::fmod(u,2.001f);
				fp32 v = subMesh.vertices[i].uv[ch].y;
				//v = std::fmod(v,2.001f);
				uv[i].u = u;
				uv[i].v = v;
				//assert( Math::Equal(uv[i].u.getFloat(), u, 1e-2f) );
				//assert( Math::Equal(uv[i].v.getFloat(), v, 1e-2f) );
			}
			UVChannel->unlock();
		}

		//export indices
		IIndexBuffer::EIndexType type = IndexBufferHelper::calcIndexType(vertexCount);
		HIBUFFER hIB = manager.createIndexBuffer(NULL, type, indexCount, IGraphicsBuffer::GBU_DEFAULT);
		IndexBufferHelper helper( hIB->lock(IGraphicsBuffer::GBLF_NORMAL), type);
		for(size_t i = 0; i < indexCount; ++i)
			helper[i] = subMesh.indices[i];
		hIB->unlock();

		//normal type
		target->setNormalType( mNormalType );

		//sub-mesh
		IModelResource::MESHDATA data;
		data.mName = subMesh.name;
		data.mAAB = AABB(pmin,pmax);
		data.mRadius = data.mAAB.getHalfSize().getLength();
		data.mIndices = hIB;
		data.mPosition = hVB;
		data.mNormalBuffer = hNormal;
		data.mBoneIndices = boneIndices;
		data.mBoneWeights = boneWeights;
		data.mTexcoordCount = subMesh.uvCount;
		data.mBoneCount = (uint32)subMesh.usedBoneSet.size();

		if( data.mBoneCount > BLADE_BONE_PALETTE_SIZE )
			BLADE_LOG(Error, BTString("sub mesh's binding bone count exceeded max limit:") << BLADE_BONE_PALETTE_SIZE << BTString(", please split mesh."));

		const TString* nameList = subMesh.activeBoneSet.getBoneNames();
		for(size_t i = 0; i < subMesh.activeBoneSet.size(); ++i)
			data.mActiveBones.push_back( nameList[i] );

		uint32* usedBones = NULL;
		TempVector<uint32> usedBoneList;

		IModelResource::BONE_BOUNDING* boneBounding = NULL;
		TempVector<IModelResource::BONE_BOUNDING> boneBoundingList( data.mBoneCount );
		if( data.mBoneCount > 0)
		{
			assert( subMesh.activeBoneSet.size() > 0 );
			usedBoneList.resize(subMesh.usedBoneSet.size());

			const TString* usedNameList = subMesh.usedBoneSet.getBoneNames();
			for(size_t i = 0; i < usedBoneList.size(); ++i)
				usedBoneList[i] = subMesh.activeBoneSet.addBone(usedNameList[i]);
			usedBones = &usedBoneList[0];

			assert( subMesh.boneBoundings.size() == data.mBoneCount );
			for(size_t i = 0; i < boneBoundingList.size(); ++i)
			{
				boneBoundingList[i].min = subMesh.boneBoundings[i].getMinPoint();
				boneBoundingList[i].max = subMesh.boneBoundings[i].getMinPoint();
			}
			boneBounding = &boneBoundingList[0];
		}
		data.mBones = usedBones;
		data.mBoneBoundings = boneBounding;

		for(int i = 0; i < subMesh.uvCount; ++i )
			data.mTexcoord[i] = UV[i];

		data.mMaterialID = (uint8)materialID;

		target->addSubMesh(data);
	}
	
}//namespace Blade
