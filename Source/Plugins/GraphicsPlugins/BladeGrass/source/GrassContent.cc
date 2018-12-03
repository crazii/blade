/********************************************************************
created:	2017/10/20
filename: 	GrassContent.cc
author:		Crazii
purpose:	
*********************************************************************/
#include "GrassContent.h"
#include "GrassConfig.h"
#include "GrassResource.h"
#include <interface/IRenderQueue.h>

namespace Blade 
{
	//////////////////////////////////////////////////////////////////////////
	GrassCluster::GrassCluster()
	{
		mUpdateFlags = CUF_DEFAULT_VISIBLE;
		mSpaceFlags = CSF_CONTENT | CSF_VIRTUAL;

		for(index_t i = 0; i < GrassConfig::MAX_LAYERS; ++i)
			mRenderable[i] = BLADE_NEW GrassRenderable(this);
	}

	//////////////////////////////////////////////////////////////////////////
	GrassCluster::~GrassCluster()
	{
		for (index_t i = 0; i < GrassConfig::MAX_LAYERS; ++i)
			BLADE_DELETE mRenderable[i];
	}

	/************************************************************************/
	/* SpaceContent interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void			GrassCluster::updateRender(IRenderQueue* queue)
	{
		for (index_t i = 0; i < GrassConfig::MAX_LAYERS; ++i)
		{
			if (mRenderable[i]->isGeometryValid() && mRenderable[i]->getMaterial()->isLoaded())
				queue->addRenderable(mRenderable[i]);
		}
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void			GrassCluster::initialize(const Vector3& pos, index_t x, index_t z, const GrassResource* res)
	{
		Matrix44::generateTransform(mTransform, pos, Vector3::UNIT_ALL, Quaternion::IDENTITY);
		AABB localAABB;

		for (index_t i = 0; i < GrassConfig::MAX_LAYERS; ++i)
		{
			const AABB& bounding = res->getAreaLocalBounding(i, x, z);
			localAABB.merge(bounding);

			const HVERTEXSOURCE& meshSource = res->getVertexSource(i);

			if (mSource[i] == NULL)
				mSource[i] = meshSource->clone();
			else
			{
				for (int j = GVS_BEGIN; j < GVS_COUNT; ++j)
					mSource[i]->setSource((uint16)j, meshSource->getBuffer((uint16)j));
			}
			mSource[i]->setSource(GVS_INSTANCING, res->getAreaInstanceBuffer(i, x, z));

			mRenderable[i]->setupGeometry(i, bounding, res->getIndexBuffer(i), mSource[i], res->getAreaInstanceCount(i, x, z));
		}
		//to world
		localAABB.offset(pos);
		this->setWorldAABB(localAABB);
	}

	//////////////////////////////////////////////////////////////////////////
	void			GrassCluster::cleanup()
	{
		for (index_t i = 0; i < GrassConfig::MAX_LAYERS; ++i)
		{
			mSource[i]->clearAllSource();
			mRenderable[i]->clearGeometry();
		}
	}

}//namespace Blade 