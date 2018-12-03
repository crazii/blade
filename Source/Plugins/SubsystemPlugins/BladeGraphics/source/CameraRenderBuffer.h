/********************************************************************
	created:	2010/06/27
	filename: 	CameraRenderBuffer.h
	author:		Crazii
	purpose:	internal queue impl for camera only
*********************************************************************/
#ifndef __Blade_CameraRenderBuffer_h__
#define __Blade_CameraRenderBuffer_h__
#include <utility/BladeContainer.h>
#include <interface/IRenderQueue.h>

namespace Blade
{
	class CameraRenderBuffer : public IRenderQueue, public Allocatable
	{
	public:
		CameraRenderBuffer();
		~CameraRenderBuffer();

		/************************************************************************/
		/* IRenderQueue interface                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		virtual void			initialize(const RenderType& renderType)	{BLADE_UNREFERENCED(renderType);}

		/*
		@describe
		@param
		@return
		*/
		virtual const TString&	getBoundRenderType() const	{return TString::EMPTY;}

		/*
		@describe
		@param
		@return
		*/
		virtual size_t			getGroupCount() const	{return 0;}

		/*
		@describe
		@param
		@return
		*/
		virtual IRenderGroup*	getRenderGroup(index_t /*indexx*/) const		{return NULL;}

		/*
		@describe
		@param
		@return
		*/
		virtual bool			addRenderable(IRenderable* renderable);

		/**
		@describe
		@param
		@return
		*/
		virtual bool			addRenderable(const RenderType& type, IRenderable* renderable);

		/*
		@describe
		@param
		@return
		*/
		virtual void			sort(IRenderSorter*)	{assert(false);}

		/*
		@describe
		@param
		@return
		*/
		virtual void			clear();

		/*
		@describe
		@param
		@return
		*/
		virtual void			reset()	{}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			isEmpty() const	{return true;}

		/**
		@describe make group count not less than parameter 'count'
		@param
		@return
		*/
		virtual bool			ensureGroups(size_t /*count*/) { return false; }

		/*
		@describe 
		@param
		@return
		*/
		virtual size_t			countGroups(const uint8* /*groups*/, size_t /*groupCount*/) const{return 0;}

		/**
		@describe
		@param
		@return
		*/
		virtual EUsage			getUsage() const { return (EUsage)mUsage; }

		/**
		@describe 
		@param
		@return
		*/
		void					setUsage(EUsage usage) { mUsage = (uint8)usage; }

		/**
		@describe get index of the same usage. usually is 0. for RBU_SHADOW, index may vary (for CSMs)
		@param
		@return
		*/
		virtual index_t			getIndex() const { return (index_t)mIndex; }

		/**
		@describe
		@param
		@return
		*/
		virtual void			setIndex(index_t index) { mIndex = (uint8)index; }

		/**
		@describe 
		@param
		@return
		*/
		virtual AABB			finalize(const POINT3& /*camPos*/, const Vector3& /*camDir*/) { return AABB::EMPTY; }

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		IRenderQueue*			getRenderQueue(const RenderType* rendertype);

		/*
		@describe setup/initialize render group for all render types
		@param 
		@return 
		*/
		void					initRenderBuffers();

		/*
		@describe process render buffers
		@param 
		@return 
		*/
		AABB					processRenderQueues(const POINT3& camPos, const Vector3& camDir);

	protected:
		typedef Vector<IRenderQueue*> RenderBufferMap;

		RenderBufferMap	mBufferMap;
		uint8	mUsage;
		uint8	mIndex;
	};//class CameraRenderBuffer
	
}//namespace Blade


#endif //__Blade_CameraRenderBuffer_h__