/********************************************************************
	created:	2010/06/27
	filename: 	CameraRenderBuffer.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "CameraRenderBuffer.h"
#include <interface/IRenderSchemeManager.h>
#include <interface/IRenderTypeManager.h>
#include <Material.h>
#include <Technique.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	CameraRenderBuffer::CameraRenderBuffer()
	{
		mUsage = (uint8)RQU_SCENE;
		mIndex = 0;
	}

	//////////////////////////////////////////////////////////////////////////
	CameraRenderBuffer::~CameraRenderBuffer()
	{
		for( RenderBufferMap::iterator i = mBufferMap.begin(); i != mBufferMap.end(); ++i)
		{
			IRenderQueue* queue = *i;
			queue->clear();
			BLADE_DELETE queue;
		}
	}

	/************************************************************************/
	/* IRenderQueue interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool				CameraRenderBuffer::addRenderable(IRenderable* renderable)
	{
		RenderType& type = renderable->getRenderType();
		assert(type.getTypeID() < mBufferMap.size());
		return mBufferMap[type.getTypeID()]->addRenderable(renderable);
	}

	//////////////////////////////////////////////////////////////////////////
	bool				CameraRenderBuffer::addRenderable(const RenderType& type, IRenderable* renderable)
	{
		assert(type.getTypeID() < mBufferMap.size());
		return mBufferMap[type.getTypeID()]->addRenderable(renderable);
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	IRenderQueue*		CameraRenderBuffer::getRenderQueue(const RenderType* rendertype)
	{
		if (rendertype != NULL)
		{
			assert(rendertype->getTypeID() < mBufferMap.size());
			return mBufferMap[rendertype->getTypeID()];
		}
		return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	void				CameraRenderBuffer::clear()
	{
		for( RenderBufferMap::iterator i = mBufferMap.begin(); i != mBufferMap.end(); ++i)
			(*i)->clear();
	}

	//////////////////////////////////////////////////////////////////////////
	void				CameraRenderBuffer::initRenderBuffers()
	{
		IRenderSchemeManager& rsm = IRenderSchemeManager::getSingleton();
		IRenderTypeManager& rtm = IRenderTypeManager::getSingleton();
		//pre-create group
		size_t types_count = rtm.getRenderTypeCount();
		mBufferMap.resize(types_count);

		for (size_t i = 0; i < types_count; ++i)
		{
			RenderType* pType = rtm.getRenderType(i);
			IRenderQueue*& queue = mBufferMap[pType->getTypeID()];
			if (queue == NULL)
			{
				queue = rsm.createRenderQueue();
				queue->initialize(*pType);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	AABB				CameraRenderBuffer::processRenderQueues(const POINT3& camPos, const Vector3& camDir)
	{
		IRenderTypeManager& rtm = IRenderTypeManager::getSingleton();
		AABB visibleBounding;

		size_t types_count = rtm.getRenderTypeCount();
		for( size_t i = 0; i < types_count; ++i )
		{
			RenderType* pType = rtm.getRenderType(i);
			IRenderQueue* queue = mBufferMap[ pType->getTypeID() ];
			queue->setUsage((EUsage)mUsage);
			queue->setIndex((index_t)mIndex);
			pType->processRenderQueue(queue);

			AABB aab = queue->finalize(camPos, camDir);
			visibleBounding.merge(aab);
		}
		return visibleBounding;
	}
	
}//namespace Blade