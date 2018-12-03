/********************************************************************
	created:	2017/12/09
	filename: 	ModelBatchCombiner.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_ModelBatchCombiner_h__
#define __Blade_ModelBatchCombiner_h__
#include <interface/IRenderQueue.h>
#include <interface/IModelResource.h>
#include <interface/IRenderSchemeManager.h>
#include "SubMesh.h"

namespace Blade
{
	class SubMesh;

	class ModelBatchCombiner : public Singleton<ModelBatchCombiner>
	{
	public:
		ModelBatchCombiner();
		~ModelBatchCombiner();

		/** @brief  */
		void initialize();

		/** @brief  */
		void shutdown();

		/** @brief  */
		BLADE_ALWAYS_INLINE static index_t getRenderQueueIndex(bool alpha)
		{
			return alpha ? 1u : 0;
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE void	updateRender(SubMesh* mesh, IRenderQueue* originalBuffer)
		{
			if (!mesh->getMeshData()->mPreTransformed || mesh->isTransparent() || originalBuffer->getUsage() != IRenderQueue::RQU_SHADOW /*|| originalBuffer->getIndex() > 0*/ )
			{
				originalBuffer->addRenderable(mesh);
				return;
			}
			index_t i = ModelBatchCombiner::getRenderQueueIndex(mesh->isTransparent());
			mRenderQueue[i]->addRenderable(mesh);
		}

		/** @brief  */
		void	processRenderQueue(IRenderQueue* queue, bool alpha);

	protected:
		struct CombinedBuffer
		{
			HVBUFFER mVertices;
			HIBUFFER mIndices;
			IRenderable* mRenderable;

			inline ~CombinedBuffer()
			{
				mVertices.clear();
				mIndices.clear();
				BLADE_DELETE mRenderable;
			}
		};
		typedef TList<CombinedBuffer> CombinedBufferList;
		typedef FixedArray<CombinedBufferList, IRenderQueue::MAX_INDEX> IndexedCombinedBuffer;

		HRENDERQUEUE			mRenderQueue[2];
		IndexedCombinedBuffer	mCombinedBuffers[2][IRenderQueue::RQU_COUNT];
	};
	
}//namespace Blade

#endif//__Blade_ModelBatchCombiner_h__