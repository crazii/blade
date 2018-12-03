/********************************************************************
	created:	2014/05/01
	filename: 	SkeletonSerializer.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "SkeletonSerializer.h"
#include "SkeletonResource.h"
#include "IK.h"

namespace Blade
{
	static const uint32 SKELETON_MAGIC = BLADE_FCC('B','L','S','K');
	static const uint32 BONE_TABLE_MAGIC = BLADE_FCC('B','L','B','T');
	
	//////////////////////////////////////////////////////////////////////////
	SkeletonSerializer::SkeletonSerializer()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	SkeletonSerializer::~SkeletonSerializer()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	bool	SkeletonSerializer::loadResource(IResource* resource, const HSTREAM& stream, const ParamList& params)
	{
		return this->loadSkeleton(resource, stream, params);
	}

	//////////////////////////////////////////////////////////////////////////
	void	SkeletonSerializer::postProcessResource(IResource* /*resource*/)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	bool	SkeletonSerializer::saveResource(const IResource* resource, const HSTREAM& stream)
	{
		if( resource != NULL && stream != NULL && stream->isValid() && (stream->getAccesMode()&IStream::AM_WRITE) )
		{
			assert( resource->getType() == ModelConsts::SKELETON_RESOURCE_TYPE);
			const SkeletonResource* skeleton = static_cast<const SkeletonResource*>(resource);

			bool ret = stream->writeAtom(SKELETON_MAGIC) == sizeof(SKELETON_MAGIC);
			BLADE_SERIALIZE_CHECK(ret);
			uint32 framecount = (uint32)skeleton->mTotalFrameCount;
			ret = stream->writeAtom(framecount) == sizeof(framecount);
			BLADE_SERIALIZE_CHECK(ret);

			uint32 FPS = skeleton->mAnimTable.getFPS();
			ret = stream->writeAtom( FPS ) == sizeof(FPS);
			BLADE_SERIALIZE_CHECK(ret);

			//write bone table
			ret = stream->writeAtom(BONE_TABLE_MAGIC) == sizeof(BONE_TABLE_MAGIC);
			BLADE_SERIALIZE_CHECK(ret);
			ret = StringTable::writeStrings( stream, skeleton->mBoneTable.mNameTable );
			BLADE_SERIALIZE_CHECK(ret);
			uint32 boneCount = (uint32)skeleton->mBoneTable.mBoneCount;

			for(size_t i = 0; i < boneCount; ++i)
				skeleton->mBoneTable.mBoneData[i].mInitialPose.normalize();

			ret = stream->writeAtom( boneCount ) == sizeof(boneCount);
			BLADE_SERIALIZE_CHECK(ret);
			ret = (size_t)stream->writeData( skeleton->mBoneTable.mBoneData, (IStream::Size)(sizeof(BONE_DATA)*boneCount) ) == sizeof(BONE_DATA)*boneCount;
			BLADE_SERIALIZE_CHECK(ret);

			//write animation table
			ret = skeleton->mAnimTable.saveAnimationResource( stream );
			return ret;
		}
		assert(false);
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	SkeletonSerializer::createResource(IResource* /*resource*/, ParamList& /*params*/)
	{
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	SkeletonSerializer::reloadResource(IResource* resource, const HSTREAM& stream, const ParamList& params)
	{
		return this->loadSkeleton(resource, stream, params);
	}

	//////////////////////////////////////////////////////////////////////////
	bool	SkeletonSerializer::reprocessResource(IResource* /*resource*/)
	{
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	SkeletonSerializer::loadSkeleton(IResource* resource, const HSTREAM& stream, const ParamList& /*params*/)
	{
		if( resource != NULL && stream != NULL && stream->isValid() && (stream->getAccesMode()&IStream::AM_READ) )
		{
			assert( resource->getType() == ModelConsts::SKELETON_RESOURCE_TYPE);
			SkeletonResource* skeleton = static_cast<SkeletonResource*>(resource);

			uint32 magic = 0;
			bool ret = stream->readAtom(magic) == sizeof(magic);
			BLADE_SERIALIZE_CHECK(ret);
			BLADE_SERIALIZE_CHECK(magic == SKELETON_MAGIC);
			ret = stream->readAtom(skeleton->mTotalFrameCount) == sizeof(skeleton->mTotalFrameCount);
			BLADE_SERIALIZE_CHECK(ret);

			uint32 FPS = 0;
			ret = stream->readAtom( FPS ) == sizeof(FPS);
			BLADE_SERIALIZE_CHECK(ret);

			//load bone table
			magic = 0;
			ret = stream->readAtom(magic) == sizeof(magic);
			BLADE_SERIALIZE_CHECK(ret);
			BLADE_SERIALIZE_CHECK(magic == BONE_TABLE_MAGIC);
			ret = StringTable::readStrings( stream, skeleton->mBoneTable.mNameTable, IModelConfigManager::getSingleton().getModelPool() );
			BLADE_SERIALIZE_CHECK(ret);

			uint32 boneCount = 0;
			ret = stream->readAtom( boneCount ) == sizeof(boneCount);
			BLADE_SERIALIZE_CHECK(ret);
			skeleton->mBoneTable.mBoneCount = boneCount;

			BLADE_SERIALIZE_CHECK(skeleton->mBoneTable.mBoneData == NULL);
			skeleton->mBoneTable.mBoneData = BLADE_NEW BONE_DATA[boneCount];
			ret = (size_t)stream->readData(skeleton->mBoneTable.mBoneData, (IStream::Size)(sizeof(BONE_DATA)*boneCount) ) == sizeof(BONE_DATA)*boneCount;
			if( !ret )
			{
				BLADE_DELETE[] skeleton->mBoneTable.mBoneData;
				skeleton->mBoneTable.mBoneData = NULL;
			}
			BLADE_SERIALIZE_CHECK(ret);

			for(size_t i = 0; i < boneCount; ++i)
				skeleton->mBoneTable.mBoneData[i].mInitialPose.normalize();

			ret = skeleton->mAnimTable.initialize(boneCount, FPS);
			if( !ret )
			{
				BLADE_DELETE[] skeleton->mBoneTable.mBoneData;
				skeleton->mBoneTable.mBoneData = NULL;
			}
			BLADE_SERIALIZE_CHECK(ret);

			if( IModelConfigManager::getSingleton().isIKEnabled() )
			{
				IK* _IK = BLADE_NEW IK();
				IKChainBuilder builder;
				if( builder.buildIKChians(skeleton->mBoneTable, *_IK) )
					skeleton->mIK = _IK;
				else
					BLADE_DELETE _IK;
			}

			//load animation table
			size_t frameCount = 0;
			return skeleton->mAnimTable.loadAnimationResource( stream, boneCount, frameCount) && frameCount == skeleton->mTotalFrameCount;
		}
		assert(false);
		return false;
	}

}//namespace Blade
