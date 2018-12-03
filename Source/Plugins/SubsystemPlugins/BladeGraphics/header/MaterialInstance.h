/********************************************************************
	created:	2012/03/22
	filename: 	MaterialInstance.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_MaterialInstance_h__
#define __Blade_MaterialInstance_h__
#include <BladeGraphics.h>
#include <ResourceState.h>
#include <ShaderVariableMap.h>
#include <TextureState.h>
#include <MaterialLOD.h>
#include <interface/ICamera.h>
#include <MaterialCloneShare.h>
#include <Material.h>

namespace Blade
{
	class Technique;
	class ShaderOption;

	namespace Impl
	{
		static const int TEX_PROJECTION_LIMIT = 2;
	}

	class BLADE_GRAPHICS_API MaterialInstance : public ResourceStateGroup, public Material::IListener, public Allocatable
	{
	public:
		static const int TEX_PROJECTION_LIMIT = Impl::TEX_PROJECTION_LIMIT;
	public:
		MaterialInstance(Material* pTemplate);
		MaterialInstance(Material* pTemplate, const HTEXTURESTATE& sharedTexture);
		~MaterialInstance();

		/************************************************************************/
		/*                                                                      */
		/************************************************************************/
		/** @brief  */
		virtual void			onActivateTechnique(Technique* old, Technique* now);

		/************************************************************************/
		/*                                                                      */
		/************************************************************************/
		/** @brief  */
		inline Material*		getTemplate()
		{
			return mTemplate;
		}

		/** @brief  */
		inline const Material*	getTemplate() const
		{
			return mTemplate;
		}

		/** @brief  */
		inline index_t			getActiveShaderGroup() const
		{
			return mForceGroup == 0xFF ? (index_t)mShaderGroup : (index_t)mForceGroup;
		}

		/** @brief  */
		const TString&			getName() const;

		/** @brief  */
		Technique*				getActiveTechnique() const;

		/** @brief  */
		void					setTemplate(Material* pTemplate);

		/** @brief set target shader group manually */
		index_t					setShaderGroup(const TString& groupName);

		/** @brief  */
		ShaderOption*			getDynamicShader() const;

		/** @brief  */
		inline const Pass*		getDynamicPass() const					{return mDynamicPass;}

		/** @brief  */
		inline void				setDynamicPass(const TString& passName)	{mDynamicPassName = passName;this->updateDynamicPass();}

		/************************************************************************/
		/* texture state                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		inline TextureState*	getTextureState() const					{ return mTextureState; }

		inline TextureStateInstance	getTextureStateInstance() const		{return TextureStateInstance(mTextureState, mTextureMask);}

		/*
		@describe
		@param
		@return
		*/
		const HTEXTURESTATE&	shareTextureState() const;

		/*
		@describe
		@param
		@return
		*/
		TextureState*			createTextureState(const TextureState* pToClone = NULL,bool reset = true);

		/*
		@describe
		@param
		@return
		*/
		void					setSharedTextureState(const HTEXTURESTATE& hSharedTextureState);

		/*
		@describe
		@param
		@return
		*/
		bool					isUsingSharedTexture() const;

		/************************************************************************/
		/* LODs                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		const LODSetting*		getLODSetting() const;

		/*
		@describe
		@param
		@return
		*/
		bool					setLOD(MATERIALLOD::LOD_INDEX LODIndex);

		/*
		@describe calculate max LOD level available from texture count
		@param 
		@return 
		*/
		MATERIALLOD::LOD_INDEX	calculateMaxLOD() const;

		/*
		@describe
		@param
		@return
		*/
		MATERIALLOD::LOD_INDEX	getLOD() const
		{
			return mLODIndex;
		}

		/************************************************************************/
		/* effects                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		void					setTextureProjectionCamera(index_t index,ICamera* cam);

		/*
		@describe 
		@param 
		@return 
		*/
		ICamera*				createTexutreProjectionCamera(index_t index);

		/*
		@describe 
		@param 
		@return 
		*/
		ICamera*				getTextureProjectionCamera(index_t index, bool create = false) const;

		/**
		@describe 
		@param
		@return
		*/
		const HRENDERPROPTYSET&getRenderProperty() const;

		/*
		@describe 
		@param 
		@return 
		*/
		void					enableColor(bool bEnable, const Color& diffuse, const Color& specular = Color::WHITE, const Color& emissive = Color::WHITE);

		/** @brief  */
		void					enableColor(const HRENDERPROPERTY& colorProperty)
		{
			if(colorProperty == NULL || colorProperty->getType() == RPT_COLOR)
			{
				if( mInstanceProperty == NULL )
					mInstanceProperty.constructInstance<RenderPropertySet>();
				mInstanceProperty->setProperty(colorProperty);
			}
		}

		/**
		@describe per-instance alpha blending. if an alpha blend property not attached, use srcBlend: src_alpha, destBlend: inv_src_alpha, blendOp: add
		otherwise blend mode/op is not changed.
		opacity will be set into property's srcBlendFactor
		@param
		@return
		*/
		void					enableAlphaBlend(bool enable, scalar opacity);

		/** @brief  */
		inline void				enableAlphaBlend(HRENDERPROPERTY& alphaProp)
		{
			if(alphaProp == NULL || alphaProp->getType() == RPT_ALPHABLEND)
			{
				if( mInstanceProperty == NULL )
					mInstanceProperty.constructInstance<RenderPropertySet>();
				mInstanceProperty->setProperty(alphaProp);
			}
		}

		/** @brief set per-instance shader variable */
		inline bool				setShaderVariable(const TString& variableName, EShaderConstantType ctype, size_t count, const void* pData)
		{
			return mShaderVariables.writeStorage(variableName, ctype,count, pData);
		}

		/** @brief  */
		inline const InstanceVariableMap&	getShaderVariables() const	{return mShaderVariables;}

		/************************************************************************/
		/* clone & copy                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		Handle<MaterialInstance> clone(int shareMask = MIS_ALL) const;

	protected:

		/************************************************************************/
		/* ResourceStateGroup overrides                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		virtual void	onAllReady();

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		void			switchLOD();

		/**
		@describe 
		@param
		@return
		*/
		void			updateDynamicPass();

	protected:
		InstanceVariableMap		mShaderVariables;
		Material*		mTemplate;
		const Pass*		mDynamicPass;
		TString			mDynamicPassName;
		mutable FixedArray<HCAMERA, TEX_PROJECTION_LIMIT> mTextureProjection;
		HTEXTURESTATE	mTextureState;
		mutable HRENDERPROPTYSET mInstanceProperty;
		mutable Mask	mMask;		//share mask & misc mask	
		Mask			mTextureMask;
		uint8						mShaderGroup;
		uint8						mForceGroup;
		uint8						mDynamicShaderIndex;
		MATERIALLOD::LOD_INDEX		mLODIndex;
	};//MaterialInstance
	
	typedef Handle<MaterialInstance> HMATERIALINSTANCE;

}//namespace Blade


#endif //__Blade_MaterialInstance_h__