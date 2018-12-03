/********************************************************************
	created:	2017/12/09
	filename: 	ModelBatchCombiner.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <interface/public/graphics/IGraphicsResourceManager.h>
#include "ModelBatchCombiner.h"
#include "ModelConfigManager.h"


namespace Blade
{
	class CombinedMeshRenderable : public IRenderable, public Allocatable
	{
	public:
		CombinedMeshRenderable()
			:mType(NULL)
		{
			mGeometry.useIndexBuffer(true);
			mVertexSource = IVertexSource::create();
			mGeometry.mVertexSource = mVertexSource;
			mGeometry.mVertexDecl = ModelConfigManager::getSingleton().getCombinedBatchDeclarartion();
		}

		/** @brief  */
		virtual RenderType&				getRenderType() const { return *mType; }

		/*
		@describe
		@param
		@return
		*/
		virtual const GraphicsGeometry&	getGeometry() const { return mGeometry; }

		/*
		@describe
		@param
		@return
		*/
		virtual const MaterialInstance*	getMaterial() const { return mMaterial; }

		/*
		@describe
		@param
		@return
		*/
		virtual const Matrix44&			getWorldTransform() const { return Matrix44::IDENTITY; }

		/**
		@describe get hosted content
		@param
		@return
		*/
		virtual ISpaceContent*			getSpaceContent() const { return NULL; }

		/**
		@describe
		@param
		@return
		*/
		virtual const AABB&	getWorldBounding() const { return mWorldAABB; }

		/** @brief  */
		inline void setup(const AABB& aab, const HMATERIALINSTANCE& material, const HIBUFFER& ib, const HVBUFFER& vb, size_t ic, size_t vc,
			bool hasAlpha)
		{
			mWorldAABB = aab;
			IGraphicsType& t = ModelConfigManager::getSingleton().getStaticModelType(hasAlpha);
			mType = &static_cast<RenderType&>(t);

			mMaterial = material;
			//mGeometry = geom;
			mVertexSource->setSource(MVSI_POSITION, vb);
			mIndexBuffer = ib;

			mGeometry.mIndexBuffer = mIndexBuffer;
			mGeometry.mVertexStart = 0;
			mGeometry.mVertexCount = (uint32)vc;
			mGeometry.mIndexStart = 0;
			mGeometry.mIndexCount = (uint32)ic;
			mGeometry.mInstanceCount = 0;
			mGeometry.mPrimitiveType = GraphicsGeometry::GPT_TRIANGLE_LIST;
			mGeometry.mInstanceSourceIndex = 0;
		}

	protected:
		AABB	mWorldAABB;
		HIBUFFER mIndexBuffer;
		HVERTEXSOURCE mVertexSource;
		HMATERIALINSTANCE mMaterial;
		GraphicsGeometry mGeometry;
		RenderType* mType;
	};

	//////////////////////////////////////////////////////////////////////////
	ModelBatchCombiner::ModelBatchCombiner()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	ModelBatchCombiner::~ModelBatchCombiner()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void ModelBatchCombiner::initialize()
	{
		mRenderQueue[0].bind(IRenderSchemeManager::getSingleton().createRenderQueue());
		mRenderQueue[0]->initialize(static_cast<RenderType&>(ModelConfigManager::getSingleton().getStaticModelType(false)));
		mRenderQueue[1].bind(IRenderSchemeManager::getSingleton().createRenderQueue());
		mRenderQueue[1]->initialize(static_cast<RenderType&>(ModelConfigManager::getSingleton().getStaticModelType(true)));
	}

	//////////////////////////////////////////////////////////////////////////
	void ModelBatchCombiner::shutdown()
	{
		for (size_t i = 0; i < countOf(mRenderQueue); ++i)
		{
			mRenderQueue[i].clear();
			for (int j = IRenderQueue::RQU_START; j < IRenderQueue::RQU_COUNT; ++j)
			{
				for (size_t k = 0; k < IRenderQueue::MAX_INDEX; ++k)
					mCombinedBuffers[i][j][k].clear();
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void	ModelBatchCombiner::processRenderQueue(IRenderQueue* queue, bool alpha)
	{
		IRenderQueue::EUsage usage = queue->getUsage();
		index_t index = queue->getIndex();

		if (usage != IRenderQueue::RQU_SHADOW)
		{
			return;
		}

		IGraphicsResourceManager& manager = IGraphicsResourceManager::getSingleton();

		index_t alphaIndex = ModelBatchCombiner::getRenderQueueIndex(alpha);

		CombinedBufferList& list = mCombinedBuffers[alphaIndex][usage][index];

		const size_t groupCount = mRenderQueue[alphaIndex]->getGroupCount();
		assert(groupCount == queue->getGroupCount());
		if (list.size() < groupCount)
			list.resize(groupCount);

		for (size_t j = 0; j < groupCount; ++j)
		{
			IRenderGroup* group = mRenderQueue[alphaIndex]->getRenderGroup(j);

			RenderOperation* meshList = NULL;
			size_t count = group->getROPArray(meshList);
			if (count == 0)
				continue;

			CombinedBuffer& cb = list[j];

			index_t indexCount = 0;
			index_t vertexCount = 0;

			for (size_t n = 0; n < count; ++n)
			{
				const SubMesh* mesh = static_cast<const SubMesh*>(meshList[n].renderable);
				assert(!mesh->isAnimated() && mesh->getMeshData()->mPreTransformed);
				const GraphicsGeometry& geom = mesh->getGeometry();
				
				indexCount += geom.mIndexCount;
				vertexCount += geom.mVertexCount;
			}

			if (cb.mIndices == NULL || cb.mIndices->getIndexCount() < indexCount)
				cb.mIndices = manager.createIndexBuffer(NULL, IIndexBuffer::IT_32BIT, indexCount, IGraphicsBuffer::GBU_DYNAMIC_WRITE);
			if (cb.mVertices == NULL || cb.mVertices->getVertexCount() < vertexCount)
				cb.mVertices = manager.createVertexBuffer(NULL, sizeof(POINT3), vertexCount, IGraphicsBuffer::GBU_DYNAMIC_WRITE);

			uint32* indices = (uint32*)cb.mIndices->lock(IGraphicsBuffer::GBLF_DISCARDWRITE);
			POINT3* vertices = (POINT3*)cb.mVertices->lock(IGraphicsBuffer::GBLF_DISCARDWRITE);

			size_t voffset = 0;
			size_t ioffset = 0;
			HMATERIALINSTANCE material;
			AABB aab = AABB::EMPTY;
			bool hasAlpha = false;
			bool* pAlpha = NULL;
			for (size_t n = 0; n < count; ++n)
			{
				const SubMesh* mesh = static_cast<const SubMesh*>(meshList[n].renderable);
				aab.merge(mesh->getWorldBounding());
				if (pAlpha == NULL)
				{
					pAlpha = &hasAlpha;
					hasAlpha = mesh->isTransparent();
				}
				else
					assert(hasAlpha == mesh->isTransparent());
				if (material == NULL)
					material = mesh->getMaterialInstance();

				const GraphicsGeometry& geom = mesh->getGeometry();

				//TODO: lock buffer conflicts with scene query
				{
					IIndexBuffer* srcIB = geom.mIndexBuffer;
					const void* src = srcIB->lock(IGraphicsBuffer::GBLF_READONLY);

					((const char*&)src) += geom.mIndexStart * srcIB->getIndexSize();
					IndexBufferHelper::copyIndices(indices + ioffset, IIndexBuffer::IT_32BIT, src, srcIB->getIndexType(), voffset, geom.mIndexCount);

					srcIB->unlock();
				}

				{
					const HVBUFFER& srcVB = geom.mVertexSource->getBuffer(MVSI_POSITION);
					const POINT3* src = (const POINT3*)srcVB->lock(IGraphicsBuffer::GBLF_READONLY);

					std::memcpy(vertices + voffset, src + geom.mVertexStart, geom.mVertexCount*sizeof(POINT3));
					voffset += geom.mVertexCount;
					srcVB->unlock();
				}
			}
			cb.mIndices->unlock();
			cb.mVertices->unlock();
			if (cb.mRenderable == NULL)
				cb.mRenderable = BLADE_NEW CombinedMeshRenderable();
			static_cast<CombinedMeshRenderable*>(cb.mRenderable)->setup(aab, material, cb.mIndices, cb.mVertices, indexCount, vertexCount,
				hasAlpha);
			queue->addRenderable(cb.mRenderable);
			group->clear();
		}
	}
	
}//namespace Blade