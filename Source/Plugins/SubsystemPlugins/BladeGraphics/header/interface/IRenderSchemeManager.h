/********************************************************************
	created:	2012/02/16
	filename: 	IRenderSchemeManager.h
	author:		Crazii
	purpose:	
*********************************************************************/

#ifndef __Blade_IRenderSchemeManager_h__
#define __Blade_IRenderSchemeManager_h__
#include <BladeGraphics.h>
#include <interface/InterfaceSingleton.h>
#include <interface/IRenderScheme.h>
#include <interface/IRenderQueue.h>
#include <utility/TList.h>

namespace Blade
{
	class IRenderDevice;
	class Pass;
	class ShaderOption;
	struct TextureStateInstance;

	class IRenderSchemeManager : public InterfaceSingleton<IRenderSchemeManager>
	{
	public:
		virtual ~IRenderSchemeManager()	{}

		/*
		@describe perform initialization routine , i.e. applying render scheme configs
		@param sharedOutputBuffer share output buffers among all schemes
		@return
		*/
		virtual void			initialize(bool sharedOutputBuffer) = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual void			shutdown() = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual IRenderQueue*	createRenderQueue() = 0;

		/*
		@describe create scheme from pre-defined schemes
		@param
		@return
		*/
		virtual IRenderScheme*	createRenderScheme(const TString& name) const = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual bool			destroyRenderScheme(IRenderScheme* scheme) const = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual size_t			getSchemeCount() const = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual const TString&	getScheme(index_t index) const = 0;

		/*
		@describe get sorted profile list, in ascending order
		@param
		@return
		*/
		virtual const TStringList*	getSchemeProfiles(index_t index) const = 0;

		/*
		@describe get sorted profile list, in ascending order
		@param
		@return
		*/
		virtual const TStringList*	getSchemeProfiles(const TString& schemeName) const = 0;

		/*
		@describe 
		@param
		@return
		*/
		virtual size_t			getActiveSchemes(TPointerParam<IRenderScheme>& schemeList) const = 0;

		/*
		@describe apply render settings for each set of primitives
		@param
		@return
		*/
		virtual void			applyRenderSettings(IRenderDevice* device, const ShaderOption* shaderOpt, const TextureStateInstance textures, const SamplerState* samplers) = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual void			applyRenderProperty(IRenderDevice* device, const RenderPropertySet& prop,FILL_MODE fm/* = GFM_DEFAULT*/) = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual void			applyGlobalShaderSetting(IRenderDevice* device, const ShaderOption* shaderOpt) = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual void			applyInstanceShaderSetting(IRenderDevice* device, const ShaderOption* shaderOpt) = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual void			applyTextureSetting(IRenderDevice* device, const ShaderOption* shaderOpt, const TextureStateInstance textures, const SamplerState* samplers) = 0;

	};//class IRenderSchemeManager

	extern template class BLADE_GRAPHICS_API Factory<IRenderSchemeManager>;

}//namespace Blade


#endif //__Blade_IRenderSchemeManager_h__