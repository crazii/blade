/********************************************************************
	created:	2012/02/16
	filename: 	RenderSchemeManager.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_RenderSchemeManager_h__
#define __Blade_RenderSchemeManager_h__
#include <interface/IRenderSchemeManager.h>
#include <utility/BladeContainer.h>
#include <interface/public/IRenderSorter.h>
#include "RenderSchemeDesc.h"

namespace Blade
{
	class IShader;
	class ShaderInstance;

	class RenderSchemeManager : public IRenderSchemeManager, public Singleton<RenderSchemeManager>
	{
	public:
		using Singleton<RenderSchemeManager>::getSingleton;
		using Singleton<RenderSchemeManager>::getSingletonPtr;
	public:
		RenderSchemeManager();
		~RenderSchemeManager();

		/************************************************************************/
		/* IRenderSchemeManager interface                                                                     */
		/************************************************************************/

		/*
		@describe perform initialization routine , i.e. applying render scheme configs
		@param
		@return
		*/
		virtual void			initialize(bool sharedOutputBuffer);

		/*
		@describe
		@param
		@return
		*/
		virtual void			shutdown();

		/*
		@describe
		@param
		@return
		*/
		virtual IRenderQueue*	createRenderQueue();

		/*
		@describe create scheme from pre-defined schemes
		@param
		@return
		*/
		virtual IRenderScheme*	createRenderScheme(const TString& name) const;

		/*
		@describe
		@param
		@return
		*/
		virtual bool			destroyRenderScheme(IRenderScheme* scheme) const;

		/*
		@describe
		@param
		@return
		*/
		virtual size_t			getSchemeCount() const;

		/*
		@describe
		@param
		@return
		*/
		virtual const TString&	getScheme(index_t index) const;

		/*
		@describe 
		@param
		@return
		*/
		virtual const TStringList*	getSchemeProfiles(index_t index) const;

		/*
		@describe 
		@param
		@return
		*/
		virtual const TStringList*	getSchemeProfiles(const TString& schemeName) const;

		/*
		@describe 
		@param
		@return
		*/
		virtual size_t			getActiveSchemes(TPointerParam<IRenderScheme>& schemeList) const;

		/*
		@describe apply render settings for each set of primitives
		@param
		@return
		*/
		virtual void			applyRenderSettings(IRenderDevice* device, const ShaderOption* shaderOpt, const TextureStateInstance textures, const SamplerState* samplers);

		/*
		@describe
		@param
		@return
		*/
		virtual void			applyRenderProperty(IRenderDevice* device, const RenderPropertySet& prop,FILL_MODE fm/* = GFM_DEFAULT*/);

		/*
		@describe
		@param
		@return
		*/
		virtual void			applyGlobalShaderSetting(IRenderDevice* device, const ShaderOption* shaderOpt);

		/*
		@describe
		@param
		@return
		*/
		virtual void			applyInstanceShaderSetting(IRenderDevice* device, const ShaderOption* shaderOpt);

		/*
		@describe
		@param
		@return
		*/
		virtual void			applyTextureSetting(IRenderDevice* device, const ShaderOption* shaderOpt, const TextureStateInstance textures, const SamplerState* samplers);

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief helper */
		IRenderSorter*			createRenderSorter(const TStringList& sorters);

	protected:
		/** @brief  */
		IRenderScheme*	createScheme(const SCHEME_DESC& desc) const;

		typedef TStringMap<SCHEME_DESC*>	SchemeInfoMap;
		typedef Set<IRenderScheme*>			SchemeSet;

		const RenderPropertySet*	mLastProperty;
		const TextureState*			mLastTextureStates;

		const IShader*				mLastShaderProgram;
		const ShaderInstance*		mLastShaderInst;
		TextureStateInstance		mLastTexture;
		const SamplerState*			mLastSampler;

		SchemeInfoMap				mSchemeMap;
		mutable SchemeSet			mActiveSchemes;
	};//class RenderSchemeManager


}//namespace Blade


#endif //__Blade_RenderSchemeManager_h__