/********************************************************************
	created:	2011/05/21
	filename: 	AABBRenderer.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "AABBRenderer.h"
#include <interface/public/graphics/IGraphicsResourceManager.h>
#include <interface/IGraphicsSystem.h>
#include <interface/ISpaceCoordinator.h>
#include <interface/public/graphics/IGraphicsScene.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	AABBRenderer::AABBRenderer()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	AABBRenderer::~AABBRenderer()
	{

	}

	/************************************************************************/
	/* IAABBRenderer interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool		AABBRenderer::addAABB(ISpaceContent* content, const Color& color)
	{
		//only should be called in async run state from other sub systems
		//if within graphics system thread, any state is allowed
		BLADE_TS_VERITY_GRAPHICS_PUBLIC_ACCESS();

		if(content == NULL )
			return false;

		ScopedLock lock(mSyncLock);
		return mColoredInstances.addContent(content, color);
	}

	//////////////////////////////////////////////////////////////////////////
	bool		AABBRenderer::removeAABB(ISpaceContent* content)
	{
		//only should be called in async run state from other sub systems
		//if within graphics system thread, any state is allowed
		BLADE_TS_VERITY_GRAPHICS_PUBLIC_ACCESS();
		if (content == NULL)
			return false;
		ScopedLock lock(mSyncLock);
		return mColoredInstances.removeContent(content);
	}

	//////////////////////////////////////////////////////////////////////////
	bool		AABBRenderer::changeAABBColor(ISpaceContent* content, const Color& color)
	{
		//only should be called in async run state from other sub systems
		//if within graphics system thread, any state is allowed
		BLADE_TS_VERITY_GRAPHICS_PUBLIC_ACCESS();
		if (content == NULL)
			return false;
		ScopedLock lock(mSyncLock);
		return mColoredInstances.changeColor(content, color);
	}

	//////////////////////////////////////////////////////////////////////////
	bool		AABBRenderer::addAABB(IAABBTarget* target)
	{
		//only should be called in async run state from other sub systems
		//if within graphics system thread, any state is allowed
		BLADE_TS_VERITY_GRAPHICS_PUBLIC_ACCESS();

		if (target != NULL)
		{
			ScopedLock lock(mSyncLock);
			return mColoredInstances.addTarget(target);
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		AABBRenderer::removeAABB(IAABBTarget* target)
	{
		//only should be called in async run state from other sub systems
		//if within graphics system thread, any state is allowed
		BLADE_TS_VERITY_GRAPHICS_PUBLIC_ACCESS();

		if (target != NULL)
		{
			ScopedLock lock(mSyncLock);
			return mColoredInstances.removeTarget(target);
		}
		return false;
	}

	/************************************************************************/
	/*  custom methods                                                                    */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	void		AABBRenderer::initialize(IGraphicsScene* scene, IGraphicsUpdater* updater)
	{
		ISpaceCoordinator* coord = static_cast<ISpaceCoordinator*>(scene->getSpaceCoordinator());
		assert( coord !=NULL );
		mColoredInstances.setupRenderResource();
		coord->addContent( &mColoredInstances );

		mColoredInstances.initialize(updater);
	}
	
}//namespace Blade
