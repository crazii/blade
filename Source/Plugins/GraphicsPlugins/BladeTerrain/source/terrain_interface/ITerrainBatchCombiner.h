/********************************************************************
	created:	2011/04/05
	filename: 	ITerrainBatchCombiner.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_ITerrainBatchCombiner_h__
#define __Blade_ITerrainBatchCombiner_h__
#include <BladeTerrain.h>
#include <RenderType.h>
#include <interface/IRenderQueue.h>
#include <interface/public/graphics/IGraphicsResourceManager.h>

namespace Blade
{
	class ITerrainBatchCombiner
	{
	public:
		virtual ~ITerrainBatchCombiner()	{}

		/**
		@describe
		@param
		@return
		*/
		virtual bool		needCombine(IRenderQueue* queue) = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual bool		processQueue(IRenderQueue* queue) = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual IGraphicsResourceManager*	getResourceManager() const = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual IGraphicsBuffer::USAGE		getIndexBufferUsage() const = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual IRenderQueue*			createCustomRenderBuffer() = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void					releaseCustomRenderBuffer(IRenderQueue* group) = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void					stealFrom(ITerrainBatchCombiner& src) = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual void					releaseIndexBuffer() = 0;
	};

	extern template class Factory<ITerrainBatchCombiner>;
	typedef Factory<ITerrainBatchCombiner> BatchCombinerFactory;

}//namespace Blade



#endif // __Blade_ITerrainBatchCombiner_h__