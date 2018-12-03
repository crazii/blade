/********************************************************************
	created:	2014/09/28
	filename: 	ExportData.h
	author:		Crazii
	
	purpose:	data structure used for exporting
*********************************************************************/
#ifndef __Blade_ExportData_h__
#define __Blade_ExportData_h__
#define NOMINMAX
//max sdk
#include "IGame/IGameMaterial.h"
#include "IGame/IGame.h"
#include "IGame/IGameModifier.h"
#undef INFINITE
#include <utility/BladeContainer.h>
#include <math/Vector2.h>
#include <math/Vector3.h>
#include <interface/IModelBuilder.h>
#include <interface/IAnimationBuilder.h>


namespace Blade
{
	typedef Vector<IGameNode*>			BoneList;
	typedef TempVector<AnimationDef>	AnimationDefList;
	typedef TempVector<BONE_DATA>		BoneCollection;

	typedef TempVector<IGameMaterial*>			MaterialList;
	typedef TempVector<IGraphicsCamera*>		CameraList;	

	typedef TempVector<Vector3>	SubMeshVertices;
	typedef TempVector<IModelCollector::Triangle>		SubMeshFaces;
	typedef TempVector<IModelCollector::SubMaterial>	SubMeshMaterials;
	struct SubMeshData
	{
		SubMeshData()	:mesh(NULL),name(TEXT(""))	{}

		IGameMesh*			mesh;
		const tchar*		name;
		SubMeshMaterials	materialData;
		SubMeshVertices		vertexData;
		SubMeshFaces		indexData;
	};
	typedef TempVector<SubMeshData>						MeshList;

	class MaxModelCollector : public IModelCollector, public IAnimationCollector
	{
	public:
		MaxModelCollector()
		{
			mBoneList.clear();
			mBoneList.reserve(512);	//reserve enough space
		}
		virtual ~MaxModelCollector()	{}

		/************************************************************************/
		/* IModelCollector interface                                                                     */
		/************************************************************************/

		/* @brief  */
		virtual size_t			getMaterialCount()
		{
			return mMaterials.size();
		}

		/* @brief  */
		virtual bool			getMaterial(index_t index, IModelResource::MATERIAL_INFO& info)
		{
			if( index >= mMaterials.size() )
			{
				assert(false);
				return false;
			}
			IGameMaterial* material = mMaterials[index];
			this->exportMaterial(material, info);
			return true;
		}

		/* @brief  */
		virtual size_t			getSubMeshCount()
		{
			return mMeshes.size();
		}

		/* @brief  */
		virtual const tchar*	getSubMeshName(index_t meshIndex)
		{
			if( meshIndex >= mMeshes.size() )
			{
				assert(false);
				return Char_Traits<tchar>::null();
			}
			return mMeshes[meshIndex].name;
		}

		/* @brief note: Blade model's sub mesh only have unique material, the collector is allowed to have more. */
		virtual const SubMaterial*getSubMeshMaterials(index_t meshIndex, size_t& materialCount)
		{
			if(meshIndex >= mMeshes.size() )
			{
				assert(false);
				return NULL;
			}
			materialCount = mMeshes[meshIndex].materialData.size();
			return materialCount == 0 ? NULL : &mMeshes[meshIndex].materialData[0];
		}

		/* @brief  */
		virtual const Vector3*	getSubMeshVertices(index_t meshIndex, size_t& vertexCount)
		{
			if(meshIndex >= mMeshes.size() )
			{
				assert(false);
				return NULL;
			}
			vertexCount = mMeshes[meshIndex].vertexData.size();
			return vertexCount == 0 ? NULL : &mMeshes[meshIndex].vertexData[0];
		}

		/* @brief get indices data for triangle lists */
		virtual const Triangle*	getSubMeshTriangles(index_t meshIndex, size_t& triangleCount)
		{
			if(meshIndex >= mMeshes.size() )
			{
				assert(false);
				return NULL;
			}
			triangleCount = mMeshes[meshIndex].indexData.size();
			return triangleCount == 0 ? NULL : &mMeshes[meshIndex].indexData[0];
		}

		/* @brief  */
		virtual size_t			getVertexBoneCount(index_t meshIndex, uint32 vertexIndex)
		{
			if(meshIndex >= mMeshes.size() || mMeshes[meshIndex].mesh == NULL || mMeshes[meshIndex].mesh->GetIGameSkin() == NULL)
			{
				assert(meshIndex < mMeshes.size() && mMeshes[meshIndex].mesh != NULL);
				return 0;
			}
			IGameSkin* skin = mMeshes[meshIndex].mesh->GetIGameSkin();
			return skin != NULL ? std::max<int>(skin->GetNumberOfBones(vertexIndex),0) : 0;
		}

		/* @brief  */
		virtual float			getVertexBoneWeight(index_t meshIndex, uint32 vertexIndex, index_t boneIndex)
		{
			if(meshIndex >= mMeshes.size() )
			{
				assert(false);
				return 0;
			}
			IGameMesh* mesh = mMeshes[meshIndex].mesh;
			IGameSkin* skin = mesh->GetIGameSkin();
			return skin != NULL ? skin->GetWeight((int)vertexIndex, (int)boneIndex) : 0;
		}

		/* @brief  */
		virtual BONE_DATA*		getVertexBone(index_t meshIndex, uint32 vertexIndex , index_t boneIndex)
		{
			if(meshIndex >= mMeshes.size() || mMeshes[meshIndex].mesh == NULL || mMeshes[meshIndex].mesh->GetIGameSkin() == NULL)
			{
				assert(false);
				return 0;
			}
			IGameSkin* skin = mMeshes[meshIndex].mesh->GetIGameSkin();
			IGameNode* node = skin->GetIGameBone( (int)vertexIndex, (int)boneIndex);
			assert(node != NULL);
			BoneList::const_iterator iter = std::find(mBoneList.begin(), mBoneList.end(), node );

			index_t index = iter != mBoneList.end() ? iter - mBoneList.begin() : INVALID_INDEX;
			if( index == INVALID_INDEX)
			{
				assert(false);
				return NULL;
			}
			for(index_t i = 0; i < mBoneData.size(); ++i)
			{
				if( mBoneData[i].mIndex == index )
					return &mBoneData[i];
			}
			return NULL;
		}

		/* @brief  */
		virtual TString			getBoneName(const BONE_DATA* bone)
		{
			for(size_t i = 0; i < mBoneData.size(); ++i)
			{
				if( &mBoneData[i] == bone )
				{
					assert( i < mBoneList.size() );
					IGameNode* bone = mBoneList[i];
					return TString( bone->GetName() );
				}
			}
			assert(false);
			return TString::EMPTY;
		}

		/* @brief  */
		virtual BONE_DATA*		getBoneData(size_t& count)
		{
			count = mBoneData.size();
			return count == 0 ? NULL : &mBoneData[0];
		}

		/* @brief  */
		virtual size_t			getCameraCount()
		{
			return mCameras.size();
		}

		/* @brief  */
		virtual IGraphicsCamera*getCamera(index_t index)
		{
			if( index >= mCameras.size() )
			{
				assert(false);
				return NULL;
			}
			return mCameras[index];
		}

		/************************************************************************/
		/* IAnimationCollector interface                                                                     */
		/************************************************************************/
		/* @brief  */
		virtual size_t			getAnimationCount()
		{
			return mAnimations.size();
		}
		/* @brief  */
		virtual AnimationDef*	getAnimationDef(index_t index)
		{
			if( index < mAnimations.size() )
				return &mAnimations[index];
			else
			{
				assert(false);
				return NULL;
			}
		}

		/* @brief  */
		virtual size_t			getBoneCount()
		{
			return mBoneList.size();
		}

		/* @brief  */
		virtual TString			getBoneName(index_t index)
		{
			if( index < mBoneList.size() )
			{
				IGameNode* bone = mBoneList[index];
				return TString(bone->GetName());
			}
			assert(false);
			return TString::EMPTY;
		}

		/* @brief get bone array */
		virtual BONE_DATA*		getBones()
		{
			return mBoneData.size() > 0 ? &mBoneData[0] : NULL;
		}

		/* @brief  */
		virtual void			setCurrentAnimation(const TString& /*name*/)
		{

		}

		/* @brief  */
		virtual scalar			getFrameTime(uint32 frame)
		{
			IGameScene* game = ::GetIGameInterface();
			TimeValue t = frame*game->GetSceneTicks();
			return scalar(TicksToSec(t));
		}

		/* @brief  */
		virtual bool			getBoneTransform(BoneDQ& transform, index_t boneIndex, scalar time);

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/* @brief  */
		void		exportMaterial(IGameMaterial* material, IModelResource::MATERIAL_INFO& target);

		/* @brief  */
		bool		addCamera(IGameCamera* camera);

		/* @brief  */
		bool		getInitTransform(BoneDQ& transform, IGameNode* node);

		/* @brief  */
		bool		initBoneData();

		MeshList		mMeshes;
		MaterialList	mMaterials;
		BoneList		mBoneList;
		CameraList		mCameras;
		AnimationDefList	mAnimations;
		BoneCollection		mBoneData;
	};//class MaxModelCollector


}//namespace Blade



#endif // __Blade_ExportData_h__