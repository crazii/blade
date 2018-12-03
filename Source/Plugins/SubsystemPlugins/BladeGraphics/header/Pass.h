/********************************************************************
	created:	2010/05/22
	filename: 	Pass.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_Pass_h__
#define __Blade_Pass_h__
#include <BladeGraphics.h>
#include <Pimpl.h>
#include <utility/Mask.h>
#include <ResourceState.h>
#include <TextureState.h>
#include <ShaderOption.h>
#include <MaterialCloneShare.h>

namespace Blade
{

	namespace Impl
	{
		class ShaderOptionList;	
	}//namespace Impl

	class Technique;

	class BLADE_GRAPHICS_API Pass : public ResourceStateGroup, public Allocatable
	{
	public:
		Pass(const TString& name, Technique* parent);
		virtual ~Pass();

		/** @brief  */
		inline const TString&	getName() const
		{
			return mName;
		}
		
		/** @brief  */
		inline void				setParent(Technique* parent)
		{
			mParent = parent;
		}

		/*
		@describe
		@param
		@return
		*/
		Technique*				getParent() const	{return mParent;}

		/************************************************************************/
		/* render property                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		RenderPropertySet*		getRenderProperty() const;

		/*
		@describe
		@param
		@return
		*/
		const HRENDERPROPTYSET&	shareRenderProperty() const;

		/*
		@describe
		@param
		@return
		*/
		RenderPropertySet*		createRenderProperty(const RenderPropertySet* pToClone = NULL);

		/*
		@describe
		@param
		@return
		*/
		void					setSharedRenderProperty(const HRENDERPROPTYSET& hRenderPropty);

		/*
		@describe
		@param
		@return
		*/
		bool					isUsingSharedRenderPropery() const;

		/************************************************************************/
		/* Shaders                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		const index_t			addShaderOption(const HSHADEROPTION& shaderOption);

		/**
		@describe 
		@param
		@return
		*/
		bool					setShaderOption(index_t index, const HSHADEROPTION& shaderOption);

		/*
		@describe get active shader option. commonly, active shader options is not used, unless the render step has no Pass defined.
		then the render output will use variable passes mode, which try to use the pass' active shader
		@see RenderOutput::renderStepWithVarPass
		@param
		@return
		*/
		const HSHADEROPTION&	getActiveShaderOption() const;

		/*
		@describe
		@param
		@return
		*/
		void					setActiveShaderOption(index_t optionIndex);

		/*
		@describe
		@param
		@return
		*/
		size_t					getShaderOptionCount() const;

		/*
		@describe
		@param
		@return
		*/
		const HSHADEROPTION&	getShaderOption(index_t index) const;

		/**
		@describe find the first shader option containing specified groupIndex
		@param
		@return
		*/
		index_t					findShaderOption(index_t startShaderOption, index_t groupIndex) const;

		/**
		@describe 
		@param
		@return
		*/
		index_t					findShaderOption(const TString& name) const;

		/** @brief  */
		const SamplerState&		getSamplerState() const {return mSamplerState;}
		SamplerState&		getSamplerState() {return mSamplerState;}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual Pass*			clone(const TString& name, int method = MS_ALL) const;

	protected:
		Pass(const Pass& src, const TString& name, int shareMethod);

		/*
		@describe
		@param
		@return
		*/
		void					resetResourceState();

	protected:
		Technique*					mParent;
		TString						mName;
		HRENDERPROPTYSET			mRenderProperty;
		mutable Mask				mMask;
		index_t						mActiveShadersIndex;

	private:
		Pimpl<Impl::ShaderOptionList>	mShaderOptions;
	protected:	
		SamplerState				mSamplerState;
	};//class Pass
	
}//namespace Blade


#endif //__Blade_Pass_h__

