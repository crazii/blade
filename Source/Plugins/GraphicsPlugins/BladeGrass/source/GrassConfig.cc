/********************************************************************
	created:	2017/11/27
	filename: 	GrassConfig.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include "GrassConfig.h"
#include "GrassContent.h"
#include <utility/StringHelper.h>
#include <interface/public/graphics/IGraphicsResourceManager.h>
#include <interface/IRenderScene.h>
#include <interface/IStage.h>
#include <interface/IPage.h>
#include <interface/ISpaceCoordinator.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	GrassConfig::GrassConfig()
	{
		mInstancingSize = 16;
		mInstancingPartition = NULL;
		mLastSpaceSize = 0;
	}

	//////////////////////////////////////////////////////////////////////////
	GrassConfig::~GrassConfig()
	{
		BLADE_RES_FREE(mInstancingPartition);
	}

	//////////////////////////////////////////////////////////////////////////
	void GrassConfig::initialize()
	{
		mVertexDecl = IGraphicsResourceManager::getSingleton().createVertexDeclaration();

		mVertexDecl->addElement(GVS_POSITION, 0, GRASS_POSITION_FMT, VU_POSITION, 0);
		mVertexDecl->addElement(GVS_UV, 0, GRASS_TEXCOORD_FMT, VU_TEXTURE_COORDINATES, 0);
#if BLADE_GRASS_VERTEX_NORMAL
		mVertexDecl->addElement(GVS_NORMAL, 0, GRASS_NORMAL_FMT, VU_NORMAL, 0);
#endif
		uint16 offset = 0;
		//world position (xyz) + random y scale (w)
		const VertexElement& pos = mVertexDecl->addElement(GVS_INSTANCING, offset, VET_UBYTE4N, VU_TEXTURE_COORDINATES, 1);
		offset += pos.getSize();
#if !BLADE_GRASS_VERTEX_NORMAL
		//world normal: random scaled planting place normal (xyz) + random wind
		const VertexElement& normal = mVertexDecl->addElement(GVS_INSTANCING, offset, VET_UBYTE4N, VU_TEXTURE_COORDINATES, 2);
		offset += normal.getSize();
#endif

		for (index_t i = 0; i < MAX_LAYERS; ++i)
		{
			mLayeredMaterial[i].bind(BLADE_NEW MaterialInstance(this->getRenderType().getMaterial()));
			mLayeredMaterial[i]->setShaderGroup(BTString("layer") + TStringHelper::fromUInt(i));
			mLayeredMaterial[i]->load();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void GrassConfig::shutdown()
	{
		mVertexDecl.clear();
		mClusterPool.clear();
		mBufferPool.clear();


		for (index_t i = 0; i < MAX_LAYERS; ++i)
		{
			mLayeredMaterial[i].clear();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void GrassConfig::buildInstancingPartitions(IRenderScene* scene)
	{
		size_t pageSize = scene->getStage()->getPagingManager()->getDesc().mPageSize;
		const TString& spaceType = scene->getSpaceCoordinatorImpl()->getDesc()->defaultSpace;
		if (mInstancingPartition == NULL || mLastSpaceSize != pageSize || mLastSpaceType != spaceType)
		{
			BLADE_TS_VERIFY(TS_MAIN_SYNC);

			mLastSpaceSize = pageSize;
			mLastSpaceType = spaceType;
			BLADE_RES_FREE(mInstancingPartition);

			size_t count = mLastSpaceSize / mInstancingSize;
			count *= count;
			mInstancingPartition = BLADE_RES_ALLOCT(uint32, count);
			for (size_t i = 0; i < count; ++i)
				mInstancingPartition[i] = ISpace::INVALID_PARTITION;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	AppFlag GrassConfig::getGrassPlantingFlag() const
	{
#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(disable: 4244) //'argument': conversion from 'const Blade::uint64' to 'Blade::uint32', possible loss of data
#endif
		return static_cast<AppFlag>(mTargetFlag); 
	}

	//////////////////////////////////////////////////////////////////////////
	void				GrassConfig::onConfigChange(void*)
	{
		//note: if target app flags changed, we probably need update all visible grasses instantly.
	}
	
}//namespace Blade