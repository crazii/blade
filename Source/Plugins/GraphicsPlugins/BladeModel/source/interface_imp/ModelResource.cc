/********************************************************************
	created:	2013/04/03
	filename: 	ModelResource.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "ModelResource.h"
#include <ExceptionEx.h>

#if BLADE_COMPILER_GNU_EXTENSION
#pragma GCC diagnostic ignored "-Wattributes"
#endif

#include <interface/IResourceManager.h>
#include <TextureState.h>

namespace Blade
{
	const Version IModelResource::MODEL_LATEST_SERIALIZER_VERSION = Version(0,1,'A');
	const TString IModelResource::MAP_CHANNEL_NAMES[MC_COUNT] = 
	{
		BTString("diffuseMap"),			//MC_DIFFUSE
		BTString("normalMap"),			//MC_NORMAL
		BTString("specularLevelMap"),	//MC_SPECULAR_LEVEL
		BTString("glossMap"),			//MC_SPECULAR_GLOSS
		BTString("specularMap"),		//MC_SPECULAR
		BTString("glowMap"),			//MC_GLOW
	};

	//////////////////////////////////////////////////////////////////////////
	ModelResource::ModelResource()
	{
		
	}

	//////////////////////////////////////////////////////////////////////////
	ModelResource::~ModelResource()
	{
		for(SubMeshList::iterator i = mSubMesh.begin(); i != mSubMesh.end(); ++i)
		{
			MESHDATA& mesh = *i;
			BLADE_MODEL_FREE(mesh.mBones);
			BLADE_DELETE[] mesh.mBoneBoundings;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	const TString&	ModelResource::getType() const
	{
		return ModelConsts::MODEL_RESOURCE_TYPE;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t		ModelResource::getSubMeshCount() const
	{
		return mSubMesh.size();
	}

	//////////////////////////////////////////////////////////////////////////
	const IModelResource::MESHDATA&	ModelResource::getSubMeshData(index_t index) const
	{
		if( index < mSubMesh.size() )
		{
			SubMeshList::const_iterator i = mSubMesh.begin();
			std::advance(i, index);
			return *i;
		}
		else
			BLADE_EXCEPT(EXC_OUT_OF_RANGE, BTString("subscript out of range."));
	}

	//////////////////////////////////////////////////////////////////////////
	index_t		ModelResource::findSubMeshData(const TString& name) const
	{
		for(SubMeshList::const_iterator i = mSubMesh.begin(); i != mSubMesh.end(); ++i)
		{
			if( i->mName == name )
				return (index_t)std::distance(mSubMesh.begin(), i);
		}
		return INVALID_INDEX;
	}

	//////////////////////////////////////////////////////////////////////////
	index_t		ModelResource::addSubMesh(const MESHDATA& meshData)
	{
		mAAB.merge( meshData.mAAB );
		mRadius = mAAB.getHalfSize().getLength();

		mSubMesh.push_back(meshData);

		if(meshData.mBones != NULL || meshData.mBoneBoundings != NULL)
		{
			assert(meshData.mBones != NULL && meshData.mBoneBoundings != NULL);
			assert( meshData.mBoneCount != 0 );
			MESHDATA& mesh = mSubMesh.back();

			mesh.mBones = BLADE_MODEL_ALLOCT(uint32, meshData.mBoneCount);
			std::memcpy(mesh.mBones, meshData.mBones, sizeof(uint32)*meshData.mBoneCount );

			mesh.mBoneBoundings = BLADE_MODEL_ALLOCT(BONE_BOUNDING, meshData.mBoneCount);
			std::memcpy(mesh.mBoneBoundings, meshData.mBoneBoundings, sizeof(BONE_BOUNDING)*meshData.mBoneCount );
		}
		else
			assert( meshData.mBoneCount == 0 );

		return mSubMesh.size() - 1;
	}

	//////////////////////////////////////////////////////////////////////////
	index_t		ModelResource::getMaterialCount() const
	{
		return mMaterials.size();
	}

	//////////////////////////////////////////////////////////////////////////
	index_t		ModelResource::addMaterial(const MATERIAL_INFO& material)
	{
		mMaterialInfo.push_back(material);
		return mMaterialInfo.size() - 1;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		ModelResource::setSkeletonRefernce(const TString& skeletonName, bool useFullPath/* = false*/)
	{
		if( !useFullPath )
		{
			TString animDir, animFile;
			TStringHelper::getFilePathInfo(skeletonName, animDir, animFile);
			mSkeletonResource.setResourcePath(animFile);
		}
		else
			mSkeletonResource.setResourcePath(TStringHelper::standardizePath(skeletonName));
		return true;
	}

	/************************************************************************/
	/*  run-time function (NOT available on exporting/importing)                                                                    */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool		ModelResource::setSkeleton(const HRESOURCE& skeletonResource)
	{
		mSkeletonResource.setManualResource(false, skeletonResource);
		//if( skeletonResource != NULL)
		//{
		//	TString dir, file;
		//	TStringHelper::getFilePathInfo(skeletonResource->getSource(), dir, file);
		//	mSkeletonResource.setResourcePath(file);
		//}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	const ModelResource::MATERIAL&			ModelResource::getMaterial(index_t index) const
	{
		if( index < mMaterials.size() )
		{
			MaterialList::const_iterator i = mMaterials.begin();
			std::advance(i, index);
			return *i;
		}
		else
			BLADE_EXCEPT(EXC_OUT_OF_RANGE, BTString("subscript out of range."));
	}

	//////////////////////////////////////////////////////////////////////////
	const AABB&	ModelResource::getModelAABB() const
	{
		return mAAB;
	}

	//////////////////////////////////////////////////////////////////////////
	void			ModelResource::setupMaterial()
	{
		mMaterials.clear();
		mMaterials.resize(mMaterialInfo.size() );
		MaterialList::iterator index = mMaterials.begin();

		for(MaterialInfo::iterator i = mMaterialInfo.begin(); i != mMaterialInfo.end(); ++i)
		{
			const MATERIAL_INFO& info = *i;
			MATERIAL& material = *index++;

			ColorProperty* prop = BLADE_NEW ColorProperty();
			prop->setDiffuse( info.diffuse );
			prop->setSpecular(info.specular);
			prop->setEmissive(info.emissive);
			material.color.bind( prop );
			material.hasTransparency = info.hasTransparency;
			material.texture.bind( BLADE_NEW TextureState() );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool			ModelResource::checkHasBones() const
	{
		for(SubMeshList::const_iterator i = mSubMesh.begin(); i != mSubMesh.end(); ++i)
		{
			const MESHDATA& mesh = *i;
			if (mesh.mActiveBones.size() > 0)
				return true;
		}
		return false;
	}

}//namespace Blade