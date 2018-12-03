/********************************************************************
	created:	2011/05/21
	filename: 	AABBRenderer.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_AABBRenderer_h__
#define __Blade_AABBRenderer_h__
#include <interface/IAABBRenderer.h>
#include "HelperRenderType.h"
#include "AABBTransformSet.h"

namespace Blade
{
	class IGraphicsScene;

	class AABBRenderer : public IAABBRenderer, public Allocatable
	{
	public:
		AABBRenderer();
		~AABBRenderer();

		/************************************************************************/
		/* IAABBRenderer interface                                                                     */
		/************************************************************************/
		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool		addAABB(ISpaceContent* content, const Color& color);

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool		removeAABB(ISpaceContent* content);

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool		changeAABBColor(ISpaceContent* content,const Color& color);

		/**
		@describe 
		@param
		@return
		*/
		virtual bool		addAABB(IAABBTarget* target);

		/**
		@describe 
		@param
		@return
		*/
		virtual bool		removeAABB(IAABBTarget* target);

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/**
		@describe 
		@param 
		@return 
		*/
		void		initialize(IGraphicsScene* scene, IGraphicsUpdater* updater);

	protected:
		AABBTransformSet		mColoredInstances;
		Lock					mSyncLock;	//add spin lock to make BoundingEffect thread safe.
	};//class AABBRenderer
	

}//namespace Blade



#endif // __Blade_AABBRenderer_h__