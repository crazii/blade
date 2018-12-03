/********************************************************************
	created:	2010/06/27
	filename: 	IRenderQueue.h
	author:		Crazii
	purpose:	a render cache/queue holding renderable for rendering. contains multiple groups
	logs:		
				2018/05/20: add render operation POD types.
				2018/05/21: add multi-threading culling support, now POD is heavy weight for multi-threading, a single pointer of renderable is better.
*********************************************************************/
#ifndef __Blade_IRenderQueue_h__
#define __Blade_IRenderQueue_h__
#include <Handle.h>
#include <interface/public/graphics/IGraphicsChannel.h>
#include <interface/public/IRenderable.h>
#include <RenderType.h>

namespace Blade
{
	class IRenderSorter;

	class GraphicsGeometry;
	class MaterialInstance;
	class ShaderOption;
	class RenderPropertySet;
	class SamplerState;
	class ITexture;

	struct RenderOperation
	{
		IRenderable*			renderable;
		const GraphicsGeometry*	geometry;
		const MaterialInstance*	material;
		ITexture*				texture;
		scalar					viewDistance;		//distance from current rendering camera
		scalar					radius;
	};

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class IRenderGroup
	{
	public:

		virtual ~IRenderGroup() {}

		/**
		@describe get the current renderable count
		@param
		@return
		*/
		virtual size_t					size() const = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual size_t					reserve(size_t capacity) = 0;

		/**
		@describe add one renderable and sort the group
		@param
		@return
		*/
		virtual void					addRenderable(IRenderable* pRenderable) = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual void					addRenderables(const RenderOperation* renderables, size_t count) = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual void					clear() = 0;

		/**
		@describe get the content of  this group
		@remark modifying the IRenderable array is directly modifying this group, \n
		and if you need to remove elements in this array,\n
		just set the element pointer to NULL
		@param [out] outArray the raw array data of the grouped renderables
		@return the element count of the array
		*/
		virtual size_t					getROPArray(RenderOperation*& outArray) = 0;

		/**
		@describe direct copy,there must be only one type of subclass exist at each running
		@param
		@return self reference
		*/
		virtual IRenderGroup&			copyFrom(const IRenderGroup& src) = 0;
	};

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class IRenderQueue
	{
	public:
		enum EUsage
		{
			RQU_UNDEFINED	= 0,
			RQU_SCENE		= 1,	//common scene rendering
			RQU_SHADOW,
			RBU_REFLECTION,

			RQU_COUNT,
			RQU_START = 0,
		};
		static const index_t MAX_INDEX = 4;
	public:
		virtual ~IRenderQueue()		{}

		/**
		@describe
		@param
		@return
		*/
		virtual void			initialize(const RenderType& renderType) = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual const TString&	getBoundRenderType() const = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual size_t			getGroupCount() const = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual IRenderGroup*	getRenderGroup(index_t index) const = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual bool			addRenderable(IRenderable* renderable) = 0;

		/**
		@describe add renderable with type override
		@param
		@return
		*/
		virtual bool			addRenderable(const RenderType& type, IRenderable* renderable) = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual void			sort(IRenderSorter* sorter) = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual void			clear() = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual void			reset() = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool			isEmpty() const = 0;

		/**
		@describe make group count not less than parameter 'count'
		@param 
		@return 
		*/
		virtual bool			ensureGroups(size_t count) = 0;

		/**
		@describe count total renderables for specified group
		@param
		@return
		*/
		virtual size_t			countGroups(const uint8* groups, size_t groupCount) const = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual EUsage			getUsage() const = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual void			setUsage(EUsage usage) = 0;

		/**
		@describe get index of the same usage. usually is 0. for RBU_SHADOW, index may vary from [0, MAX_INDEX] (for CSMs)
		@param
		@return
		*/
		virtual index_t			getIndex() const = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual void			setIndex(index_t index) = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual AABB			finalize(const POINT3& camPos, const Vector3& camDir) = 0;
	};

	typedef Handle<IRenderQueue> HRENDERQUEUE;
	
}//namespace Blade


#endif //__Blade_IRenderQueue_h__