/********************************************************************
	created:	2013/10/13
	filename: 	BatchCombiner.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_BatchCombiner_h__
#define __Blade_BatchCombiner_h__
#include <LoopContainers.h>
#include <interface/IRenderSchemeManager.h>
#include "../terrain_interface/ITerrainBatchCombiner.h"
#include "TerrainBatchRenderable.h"

namespace Blade
{

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class CombinedList : public LoopVector<TerrainBatchRenderable*>
	{
		typedef LoopVector<TerrainBatchRenderable*> base;
	public:
		CombinedList() :mReserved(0) {}

		virtual void onLoopClear()
		{
			for (size_t i = 0; i < this->size(); ++i)
				BLADE_DELETE (*this)[i];
			LoopVector<TerrainBatchRenderable*>::onLoopClear();
		}
		virtual void onLoopInit()
		{
			if (mReserved)
				this->base::reserve(mReserved);
		}

		~CombinedList()
		{
			this->clearAll();
		}
		void clearAll()
		{
			for (size_t i = 0; i < this->size(); ++i)
				BLADE_DELETE (*this)[i];
			this->clear();
		}
		void reserve(size_t count)
		{
			mReserved = count;
			this->base::reserve(mReserved);
		}
	protected:
		size_t	mReserved;
	};

	class BatchCombinerBase : public ITerrainBatchCombiner
	{
	public:
		BatchCombinerBase()
		{
		}

		~BatchCombinerBase()
		{
			for (size_t i = 0; i < mBuffers.size(); ++i)
				BLADE_DELETE mBuffers[i];

			mBuffers.clear();
		}

		/** @brief  */
		virtual IRenderQueue*				createCustomRenderBuffer()
		{
			IRenderQueue* queue = IRenderSchemeManager::getSingleton().createRenderQueue();
			mBuffers.push_back(queue);
			queue->initialize(TerrainConfigManager::getRenderType());
			return queue;
		}

		/** @brief  */
		void								releaseCustomRenderBuffer(IRenderQueue* queue)
		{
			if (queue == NULL)
				return;

			BufferList::iterator i = std::find(mBuffers.begin(), mBuffers.end(), queue);
			if (i != mBuffers.end())
			{
				mBuffers.erase(i);
				BLADE_DELETE queue;
			}
		}

		/** @brief  */
		virtual void						stealFrom(ITerrainBatchCombiner& src)
		{
			BatchCombinerBase& srcBase = static_cast<BatchCombinerBase&>(src);
			mBuffers.resize(srcBase.mBuffers.size());
			for (size_t i = 0; i < srcBase.mBuffers.size(); ++i)
			{
				mBuffers[i] = srcBase.mBuffers[i];
				srcBase.mBuffers[i] = NULL;
			}
		}

	protected:
		typedef Vector<IRenderQueue*>	BufferList;
		BufferList			mBuffers;
		CombinedList		mCombinedGeom[IRenderQueue::RQU_COUNT];
	};


	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class EmptyBatchCombiner : public BatchCombinerBase, public Allocatable
	{
	public:
		EmptyBatchCombiner()		{}

		~EmptyBatchCombiner()		{}

		/** @brief  */
		virtual bool		needCombine(IRenderQueue* queue)
		{
			return queue->getUsage() == IRenderQueue::RQU_SCENE;
		}

		/** @brief  */
		virtual bool		processQueue(IRenderQueue* queue);

		/** @brief  */
		virtual IGraphicsResourceManager*	getResourceManager() const
		{
			return IGraphicsResourceManager::getSingletonPtr();
		}

		/** @brief  */
		virtual IGraphicsBuffer::USAGE		getIndexBufferUsage() const
		{
			return IGraphicsBuffer::GBU_STATIC;
		}

		/** @brief  */
		virtual void					releaseIndexBuffer() {}
	protected:
	};//class EmptyBatchCombiner

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class FullBatchCombiner : public BatchCombinerBase, public Allocatable
	{
	public:
		FullBatchCombiner(bool combineForAllPass = true);
		~FullBatchCombiner();

		/** @brief  */
		virtual bool		needCombine(IRenderQueue* queue)
		{
			if (mCombineForAll)
				return true;
			return queue->getUsage() == IRenderQueue::RQU_SCENE;
		}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool		processQueue(IRenderQueue* queue);

		/*
		@describe get the software index buffer( CPU buffer ) for terrain blocks' indices \n
		because those buffers are not directly used in rendering \n
		we will combine those software buffer and create hardware buffer later.
		@param 
		@return 
		*/
		virtual IGraphicsResourceManager*	getResourceManager() const
		{
			//note that singleton class 's factory can directly createInstance and
			//without deleting it.
			return mCombineForAll ? IGraphicsResourceManager::getOtherSingletonPtr( BTString("Software") ) :
				/*need HW buffer for none combined situation*/IGraphicsResourceManager::getSingletonPtr();
		}

		/** @brief  */
		virtual IGraphicsBuffer::USAGE		getIndexBufferUsage() const
		{
			return mCombineForAll ? IGraphicsBuffer::GBU_STATIC : 
				/*cpu read needed for combine indices*/ IGraphicsBuffer::GBU_CPU_READ;
		}

		/** @brief  */
		virtual void					releaseIndexBuffer()
		{
			for (int i = 0; i < IRenderQueue::RQU_COUNT; ++i)
			{
				mIndexBuffer[i].clear();
				mTotalCount[i].clear();
			}
		}

	protected:
		/** @brief  */
		const HIBUFFER&		setupIndexBuffer(size_t count, IRenderQueue::EUsage usage, index_t renderBufferIndex);

		typedef FixedVector<HIBUFFER, IRenderQueue::MAX_INDEX>	IndexBufferList;
		typedef FixedVector<size_t, IRenderQueue::MAX_INDEX>	SizeList;

		//combined index buffer
		IndexBufferList		mIndexBuffer[IRenderQueue::RQU_COUNT];
		SizeList			mTotalCount[IRenderQueue::RQU_COUNT];
		IIndexBuffer::EIndexType mIndexType;
		bool				mCombineForAll;
	};//class FullBatchCombiner

	///batch combiner only for common scene rendering. (for reflection/shadows don't combine)
	class SceneBatchombiner : public FullBatchCombiner
	{
	public:
		SceneBatchombiner() :FullBatchCombiner(false) {}
	};
	
}//namespace Blade

#endif //  __Blade_BatchCombiner_h__