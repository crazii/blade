/********************************************************************
	created:	2010/07/25
	file:		MaterialSerializer.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_MaterialSerializer_h__
#define __Blade_MaterialSerializer_h__
#include <utility/BladeContainer.h>
#include <interface/public/graphics/RenderProperty.h>
#include <interface/public/graphics/ITexture.h>
#include <interface/public/graphics/IShader.h>
#include <interface/public/ISerializer.h>
#include <ShaderGroup.h>
#include <ShaderOption.h>

namespace Blade
{
	class IXmlNode;
	class Pass;
	class Technique;

	class MaterialSerializer : public ISerializer ,public TempAllocatable
	{
	public:
		MaterialSerializer();
		~MaterialSerializer();

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool	loadResource(IResource* res,const HSTREAM& stream, const ParamList& params);

		/*
		@describe load resource in main thread,if success,return true \n
		and then the resource manager will not load it again in loading thread.
		i.e. when load a texture that already exist,the serializer just check ,and return true \n
		if the texture not exist(not loaded yet) ,then return false
		@param
		@return
		*/
		//virtual bool	preLoadResource(const TString& path,IResource* res);

		/*
		@describe process resource.like preLoadResource, this will be called in main thread.\n
		i.e.TextureResource need to be loaded into graphics card.
		@param
		@return
		*/
		virtual void	postProcessResource(IResource* resource);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool	saveResource(const IResource* res,const HSTREAM& stream);

		/*
		@describe
		@param
		@return
		*/
		virtual bool	createResource(IResource* res,ParamList& params);

		/*
		@describe this method is called when resource is reloaded,
		the serializer hold responsibility to cache the loaded data for resource,
		then in main thread ISerializer::reprocessResource() is called to fill the existing resource with new data.\n
		this mechanism is used for reloading existing resource for multi-thread,
		the existing resource is updated in main thread(synchronizing state),
		to ensure that the data is changed only when it is not used in another thread.
		@param
		@return
		*/
		virtual bool	reloadResource(IResource* resource, const HSTREAM& stream, const ParamList& params);

		/*
		@describe this method will be called in main thread (synchronous thread),
		after the ISerializer::reloadResource called in asynchronous state.
		@param 
		@return 
		*/
		virtual bool	reprocessResource(IResource* resource);

		/************************************************************************/
		/*                                                                      */
		/************************************************************************/


	protected:

		/*
		@describe
		@param
		@return
		*/
		bool			loadData(const HSTREAM& stream);

		/*
		@describe
		@param
		@return
		*/
		void			loadRenderPropertyForPass(IXmlNode* propNode, Pass* pPass);

		/*
		@describe
		@param
		@return
		*/
		void			loadShaderForPass(IXmlNode* pShaderOptionNode, Pass*pPass, Technique* tech, index_t index, const Pass* cloned);

		/*
		@describe
		@param
		@return
		*/
		void			loadColorProperty(IXmlNode* pColorNode, RenderPropertySet* propSet);

		/*
		@describe
		@param
		@return
		*/
		void			loadColorWriteProperty(IXmlNode* pColorWriteNode, RenderPropertySet* propSet);

		/*
		@describe
		@param
		@return
		*/
		void			loadFogProperty(IXmlNode* fogNode, RenderPropertySet* propSet);

		/*
		@describe
		@param
		@return
		*/
		void			loadAlphaBlendProperty(IXmlNode* alphaBlendNode, RenderPropertySet* propSet);

		/*
		@describe
		@param
		@return
		*/
		void			loadDepthProperty(IXmlNode* depthNode, RenderPropertySet* propSet);

		/*
		@describe
		@param
		@return
		*/
		void			loadStencilProperty(IXmlNode* stencilNode, RenderPropertySet* propSet);

		/*
		@describe
		@param
		@return
		*/
		void			loadScissorProperty(IXmlNode* scissorNode, RenderPropertySet* propSet);

		/*
		@describe
		@param
		@return
		*/
		void			loadSamplerStateForPass(IXmlNode* pSamplerNode, Pass* pass);

		/*
		@describe
		@param
		@return
		*/
		void			loadSingleTechnique(IXmlNode* pTechNode,Technique* tech);

		/*
		@describe
		@param
		@return
		*/
		static void		initialize();

		typedef StaticTStringMap<const TString*>			TextureSerializerMap;
		typedef StaticTStringMap<TEXADDR_MODE>				SamplerAddressModeDesc;
		typedef StaticTStringMap<TEXFILER_MODE>				SamplerFilterModeDesc;
		typedef StaticTStringMap<ETextureType>				TextureTypeDesc;
		typedef StaticTStringMap<SHADER_CONSTANT_TYPE>		ShaderConstDesc;
		typedef StaticTStringMap<STENCIL_OP>				StencilOpDesc;
		typedef StaticTStringMap<CULL_MODE>					CullModeDesc;
		typedef StaticTStringMap<BLEND_MODE>				BlendModeDesc;
		typedef StaticTStringMap<BLEND_OP>					BlendOpDesc;
		typedef StaticTStringMap<TEST_FUNC>					TestFuncDesc;
		typedef StaticTStringMap<FILL_MODE>					FillModeDesc;
		typedef StaticTStringMap<ShaderGroup::ERequirement> GroupRequireDesc;
		typedef StaticTStringMap<bool>						SwitchDesc;
		typedef StaticTStringMap<RENDER_PROPERTY>			PropertyDesc;
		//typedef StaticTStringMap<TEX_PROPERTY>				TextPropertyDesc;

		static TextureSerializerMap			msTextureSerializers;
		static SamplerAddressModeDesc		msSamplerAddressModeDesc;
		static SamplerFilterModeDesc		msSamplerFilterDesc;
		static TextureTypeDesc				msTextureTypeDesc;
		static ShaderConstDesc				msShaderTypeDesc;
		static StencilOpDesc				msStencilOpDesc;
		static CullModeDesc					msCullModeDesc;
		static BlendModeDesc				msBlendModeDesc;
		static BlendOpDesc					msBlendOpDesc;
		static TestFuncDesc					msTestFuncDesc;
		static FillModeDesc					msFillModeDesc;
		static GroupRequireDesc				msGroupRequireDesc;
		static SwitchDesc					msSwitchDesc;
		static PropertyDesc					msPropertyDesc;
		//static TextPropertyDesc				msTexturePropertyDesc;

		static bool							msInited;

		typedef TempList<Technique*>	TechniqueList;
		TechniqueList	mTechniqueList;
		TString			mMaterialName;
		TStringParam	mTextureUniforms;
	};//class MaterialSerializer
	
}//namespace Blade

#endif //__Blade_MaterialSerializer_h__