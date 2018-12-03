/********************************************************************
created:	2017/10/20
filename: 	GrassRenderable.cc
author:		Crazii
purpose:	
*********************************************************************/
#include "GrassRenderable.h"
#include "GrassContent.h"

namespace Blade 
{
	//////////////////////////////////////////////////////////////////////////
	GrassRenderable::GrassRenderable(GrassCluster* content)
	{
		mContent = content;
	}

	//////////////////////////////////////////////////////////////////////////
	GrassRenderable::~GrassRenderable()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	const Matrix44&			GrassRenderable::getWorldTransform() const
	{
		return mContent->getWorldTransform();
	}

	//////////////////////////////////////////////////////////////////////////
	ISpaceContent*			GrassRenderable::getSpaceContent() const
	{
		return mContent;
	}

	//////////////////////////////////////////////////////////////////////////
	const AABB&				GrassRenderable::getWorldBounding() const
	{
		return mContent->GrassCluster::getWorldAABB();
	}

	//////////////////////////////////////////////////////////////////////////
	void			GrassRenderable::setupGeometry(index_t layer, const AABB& areaMeshBounding, const HIBUFFER& indices, const HVERTEXSOURCE& vsource, size_t instanceCount)
	{
		this->clearGeometry();

		if (vsource == NULL || vsource->getBufferSourceCount() == 0 || instanceCount == 0)
			return;

		//group the renderable by layer (of the same mesh)
		//TODO: mesh LOD with the same layer
		mMaterial = GrassConfig::getSingleton().getLayerMaterial(layer);
		Vector4 min = Vector4(areaMeshBounding.getMinPoint(), 0);
		mMaterial->setShaderVariable(BTString("AreaBoundingMin"), SCT_FLOAT4, 1, &min);
		Vector4 size = Vector4(areaMeshBounding.getSize(), 0);
		mMaterial->setShaderVariable(BTString("AreaBoundingSize"), SCT_FLOAT4, 1, &size);

		mGeometry.useIndexBuffer(indices != NULL);
		mGeometry.mIndexBuffer = indices;
		mGeometry.mVertexSource = vsource;
		mGeometry.mVertexDecl = GrassConfig::getSingleton().getVertexDeclaration();
		mGeometry.mVertexCount = (uint32)vsource->getBuffer(GVS_POSITION)->getVertexCount();
		mGeometry.mPrimitiveType = GraphicsGeometry::GPT_TRIANGLE_LIST;
		mGeometry.setInstancing(true, (uint16)instanceCount, GVS_INSTANCING);
	}

	//////////////////////////////////////////////////////////////////////////
	void			GrassRenderable::clearGeometry()
	{
		mGeometry.reset();
		mMaterial.clear();
	}

}//namespace Blade 