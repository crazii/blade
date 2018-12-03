/********************************************************************
	created:	2013/04/09
	filename: 	ModelType.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_ModelType_h__
#define __Blade_ModelType_h__
#include <RenderType.h>

namespace Blade
{
	class ModelBatchCombiner;

	class StaticModelType : public RenderType
	{
	public:
		static const TString TYPE;
	public:
		StaticModelType(const TString& name = TYPE, bool alpha = false);
		~StaticModelType();

		/************************************************************************/
		/*  IGraphicsType interface                                                                    */
		/************************************************************************/
		/**
		@describe
		@param
		@return
		*/
		virtual Material*		getMaterial() const;

		/**
		@describe
		@param
		@return
		*/
		virtual void			processRenderQueue(IRenderQueue* queue);

		/**
		@describe 
		@param
		@return
		*/
		virtual void			onRenderDeviceReady();

		/**
		@describe 
		@param
		@return
		*/
		virtual void			onRenderDeviceClose();

		/** @brief  */
		inline ModelBatchCombiner* getCombiner() const { return mCombiner; }

		mutable Material*	mMaterial;
		ModelBatchCombiner*	mCombiner;
		bool				mAlpha;
	};


	class SkinnedModelType : public RenderType
	{
	public:
		static const TString TYPE;
	public:
		SkinnedModelType(const TString& name = TYPE);
		~SkinnedModelType();

		/************************************************************************/
		/*  IGraphicsType interface                                                                    */
		/************************************************************************/
		/**
		@describe
		@param
		@return
		*/
		virtual Material*		getMaterial() const;

		/**
		@describe
		@param
		@return
		*/
		virtual void			processRenderQueue(IRenderQueue* queue);

		/**
		@describe 
		@param
		@return
		*/
		virtual void			onRenderDeviceReady();

		/**
		@describe 
		@param
		@return
		*/
		virtual void			onRenderDeviceClose();

		mutable Material*	mMaterial;
	};

	class StaticModelAlphaType : public StaticModelType
	{
	public:
		StaticModelAlphaType();
		/** @brief  */
		virtual Material*		getMaterial() const;
	};

	class SkinnedModelAlphaType : public SkinnedModelType
	{
	public:
		SkinnedModelAlphaType();
		/** @brief  */
		virtual Material*		getMaterial() const;
	};

	class MovingStaticModelType : public StaticModelType
	{
	public:
		MovingStaticModelType();
	};
	
}//namespace Blade

#endif //  __Blade_ModelType_h__