/********************************************************************
	created:	2010/05/06
	filename: 	DefaultRenderQueue.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_DefaultRenderQueue_h__
#define __Blade_DefaultRenderQueue_h__
#include <interface/IRenderQueue.h>
#include <utility/BladeContainer.h>
#include <LoopContainers.h>
#include <interface/IRenderSchemeManager.h>
#include <interface/public/IRenderSorter.h>
#include "../../internal/GraphicsConfig.h"

namespace Blade
{
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//initial reserved data
	static const size_t INIT_RENDERABLE_COUNT = 4000u / sizeof(RenderOperation);

	class  DefaultRenderGroup : public IRenderGroup, public Allocatable
	{
	public:
		DefaultRenderGroup();
		~DefaultRenderGroup();

		/**
		@describe get the current renderable count
		@param
		@return
		*/
		virtual size_t					size() const;

		/**
		@describe
		@param
		@return
		*/
		virtual size_t					reserve(size_t capacity);

		/**
		@describe add one renderable and sort the group
		@param
		@return
		*/
		virtual void					addRenderable(IRenderable* pRenderable);

		/**
		@describe
		@param
		@return
		*/
		virtual void					addRenderables(const RenderOperation* renderables, size_t count);

		/**
		@describe
		@param
		@return
		*/
		virtual void					clear();

		/**
		@describe get the content of  this group
		@remark modifying the IRenderable array is directly modifying this group, \n
		and if you need to remove elements in this array,\n
		just set the element pointer to NULL
		@param [out] outArray the raw array data of the grouped renderables
		@return the element count of the array
		*/
		virtual size_t					getROPArray(RenderOperation*& outArray);

		/**
		@describe direct copy,there must be only one type of subclass exist at each running
		@param
		@return self reference
		*/
		virtual IRenderGroup&			copyFrom(const IRenderGroup& src);

		/************************************************************************/
		/*                                                                      */
		/************************************************************************/
		/** @brief  */
		void			finalize(IRenderQueue::EUsage usage, const POINT3& camPos, const Vector3& camDir);
		/** @brief  */
		inline const AABB&		getVisibleBounding() const { return mVisibleBounding; }

	protected:
		class RenderableList : public LoopVector<RenderOperation>
		{
			virtual void onLoopInit()
			{
				assert(this->size() == 0);
				this->reserve(INIT_RENDERABLE_COUNT);
			}
		};

		TString			mTargetRenderType;
		RenderableList	mRenderOperations;
		AABB			mVisibleBounding;
#if MULTITHREAD_CULLING
		Lock			mLock;
#endif
	};//class DefaultRenderGroup

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class DefaultRenderQueue : public IRenderQueue, public Allocatable
	{
	public:
		DefaultRenderQueue();
		~DefaultRenderQueue();

		/************************************************************************/
		/* IRenderQueue interface                                                                     */
		/************************************************************************/
		/**
		@describe
		@param
		@return
		*/
		virtual void			initialize(const RenderType& renderType);

		/**
		@describe
		@param
		@return
		*/
		virtual const TString&	getBoundRenderType() const;

		/**
		@describe
		@param
		@return
		*/
		virtual size_t			getGroupCount() const;

		/**
		@describe
		@param
		@return
		*/
		virtual IRenderGroup*	getRenderGroup(index_t index) const;

		/**
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

		/**
		@describe
		@param
		@return
		*/
		virtual void			sort(IRenderSorter* sorter);

		/**
		@describe
		@param
		@return
		*/
		virtual void			clear();

		/**
		@describe
		@param
		@return
		*/
		virtual void			reset();

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool			isEmpty() const;

		/**
		@describe make group count not less than parameter 'count'
		@param
		@return
		*/
		virtual bool			ensureGroups(size_t count);

		/**
		@describe count total renderables for specified group
		@param
		@return
		*/
		virtual size_t			countGroups(const uint8* groups, size_t groupCount) const;

		/**
		@describe
		@param
		@return
		*/
		virtual EUsage			getUsage() const { return EUsage(mUsage); }

		/**
		@describe
		@param
		@return
		*/
		virtual void			setUsage(EUsage usage) { mUsage = (uint8)usage; }

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
		virtual AABB			finalize(const POINT3& camPos, const Vector3& camDir);


		/************************************************************************/
		/*                                                                      */
		/************************************************************************/
		/** @brief  */
		bool					addRenderOP(const RenderOperation& rop);

	protected:
		typedef Vector<DefaultRenderGroup*> GroupList;
		mutable GroupList		mGroupList;
		TString					mRenderTypeBinding;
		size_t					mGroupCount;
		uint8					mDynamic;
		uint8					mUsage;
		uint8					mIndex;
#if MULTITHREAD_CULLING
		Lock					mLock;
#endif
	};//class DefaultRenderQueue;
	
}//namespace Blade


#endif //__Blade_DefaultRenderQueue_h__