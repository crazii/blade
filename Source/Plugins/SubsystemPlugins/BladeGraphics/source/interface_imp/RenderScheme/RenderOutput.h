/********************************************************************
	created:	2011/12/17
	filename: 	RenderOutput.h
	author:		Crazii
	purpose:	
*********************************************************************/

#ifndef __Blade_RenderOutput_h__
#define __Blade_RenderOutput_h__
#include <BladeGraphics.h>
#include <utility/BladeContainer.h>
#include <interface/public/graphics/PixelFormat.h>
#include <interface/public/graphics/IGraphicsView.h>
#include <interface/public/graphics/IGraphicsPhase.h>

#include "RenderSchemeDesc.h"
#include "RenderStep.h"

namespace Blade
{
	class ICamera;
	class Material;
	class IRenderQueue;
	class IRenderSorter;
	class RenderStepInstance;
	
	class RenderOutput : public Allocatable
	{
	public:
		RenderOutput(const OUTPUT_DESC* info);
		virtual ~RenderOutput();

		/************************************************************************/
		/* IGraphicsPhase interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		inline const TString& getName() const
		{
			return mDesc->mName;
		}

		/*
		@describe
		@param
		@return
		*/
		inline bool			isEnabled() const
		{
			return mEnabled;
		}

		/*
		@describe
		@param
		@return
		*/
		inline void			setEnable(bool enable)
		{
			mEnabled = enable;
		}

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		inline IRenderTarget*	getTarget() const
		{
			return mTarget;
		}

		/** @brief  */
		inline bool			isUsingFinalTarget() const
		{
			return mDesc->mOutputBufferRef == BR_FINAL;
		}

		/** @brief  */
		inline bool			isUsingEmptyTarget() const
		{
			return mDesc->mOutputBufferRef == BR_EMPTY;
		}

		/** @brief  */
		inline bool			hasImageEffect() const
		{
			return mDesc->mHasImageEfect;
		}

		/** @brief  */
		inline IGraphicsView*	getView() const
		{
			return mView;
		}

		/** @brief  */
		inline bool			isUsingFinalView() const
		{
			return mDesc->mViewDesc.mViewRef == VR_FINAL;
		}

		/** @brief  */
		inline const OUTPUT_DESC&	getDesc() const
		{
			return *mDesc;
		}

		/** @brief  */
		inline ICamera*		getCustomCamera() const
		{
			return mCustomCamera;
		}

		/** @brief  */
		inline bool			isUsingCustomCamera() const
		{
			return mDesc->mCameraDesc.mFlags.checkBits(CDF_CUSTOM);
		}

		/*
		@describe 
		@param 
		@return 
		*/
		size_t			getStepCount() const;

		/*
		@describe 
		@param 
		@return 
		*/
		RenderStep*		getRenderStep(index_t index) const;

		/**
		@describe 
		@param
		@return
		*/
		index_t			getRenderStepByRenderType(index_t start, const TString& type) const;


		/*
		@describe
		@param
		@return
		*/
		index_t			addRenderStep(RenderStep* step);

		/*
		@describe
		@param
		@return
		*/
		bool			setTarget(IRenderTarget* target);

		/*
		@describe 
		@param 
		@return 
		*/
		bool			setView(IGraphicsView* view);

		/*
		@describe 
		@param [view] the view helper to help setting up custom parameters
		@return 
		*/
		/** @brief  */
		void			setCustomCamera(ICamera* camera, IGraphicsView* view);

		/*
		@describe 
		@param 
		@return 
		*/
		bool			setupCustomCamera(ICamera* refernce, IGraphicsView* view);

		/**
		@describe 
		@param
		@return
		*/
		void			setShaderVarialbeSource(ICamera* cam, IGraphicsView* view);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void	renderOutput(IRenderDevice* device, ICamera* camera, IGraphicsView* view, 
			const RenderStepInstance* const* instances, size_t instanceCount);

		/**
		@describe set input buffer. the input buffers are not used here directly, it will be accessed by shader variable updating to bound to a texture sampler.
		@param
		@return
		*/
		inline void			setInputBuffer(index_t index, const HTEXTURE& input)
		{
			if (index < MAX_INPUT_COUNT)
			{
				mInputs[index] = input;
				if (input != NULL && mInputCount < index + 1)	//TODO: fix gaps
					mInputCount = index + 1;
			}
			else
				assert(false);
		}

		/**
		@describe 
		@param
		@return
		*/
		inline const HTEXTURE&	getInputBuffer(index_t index) const
		{
			if(index < MAX_INPUT_COUNT)
				return mInputs[index];
			assert(false);
			return HTEXTURE::EMPTY;
		}

		/** @brief  */
		inline void setLDR() {mLDR = true;}
		/** @brief  */
		inline bool isLDR() const  {return mLDR;}

	protected:
		/** @brief render the step with fixed Pass. */
		void			renderStepWithUniquePass(IRenderDevice* device,FILL_MODE mode,IRenderQueue* queue,Pass* pass, bool texturing);
		/** @brief render the step with variable Passes, which is obtained from each renderables.
		this won't enumerate all shaders of the pass, but only the active shader.
		*/
		void			renderStepWithDymamicPass(IRenderDevice* device,FILL_MODE mode, IRenderQueue* queue, const RenderStep* step);

		/** @brief  */
		void			renderMixedSteps(IRenderDevice* device, ICamera* camera, IGraphicsView* view, 
			const RenderStepInstance* const* instances, size_t instanceCount);

		typedef Vector<RenderStep*> RenderStepList;
		typedef FixedArray<HTEXTURE,MAX_INPUT_COUNT> InputBufferList;

		const OUTPUT_DESC*	mDesc;
		IRenderTarget*		mTarget;
		IGraphicsView*		mView;
		//created custom camera, need delete
		ICamera*			mCustomCamera;
		IRenderSorter*		mSorter;
		RenderStepList		mStepList;
		InputBufferList		mInputs;
		size_t				mInputCount;
		bool				mEnabled;
		bool				mLDR;	//true if use LDR on runtime, false doesn't care
	};//class RenderOutput
	

}//namespace Blade



#endif // __Blade_RenderOutput_h__