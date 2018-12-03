/********************************************************************
	created:	2010/07/25
	file:		MaterialSerializer.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "MaterialSerializer.h"
#include <interface/IResourceManager.h>
#include "../Material/MaterialManager.h"
#include "../Material/DefaultMaterial.h"
#include <Technique.h>
#include <Pass.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	MaterialSerializer::TextureSerializerMap		MaterialSerializer::msTextureSerializers;
	MaterialSerializer::SamplerAddressModeDesc		MaterialSerializer::msSamplerAddressModeDesc;
	MaterialSerializer::SamplerFilterModeDesc		MaterialSerializer::msSamplerFilterDesc;
	MaterialSerializer::TextureTypeDesc				MaterialSerializer::msTextureTypeDesc;
	MaterialSerializer::ShaderConstDesc				MaterialSerializer::msShaderTypeDesc;
	MaterialSerializer::StencilOpDesc				MaterialSerializer::msStencilOpDesc;
	MaterialSerializer::CullModeDesc				MaterialSerializer::msCullModeDesc;
	MaterialSerializer::BlendModeDesc				MaterialSerializer::msBlendModeDesc;
	MaterialSerializer::BlendOpDesc					MaterialSerializer::msBlendOpDesc;
	MaterialSerializer::TestFuncDesc				MaterialSerializer::msTestFuncDesc;
	MaterialSerializer::FillModeDesc				MaterialSerializer::msFillModeDesc;
	MaterialSerializer::GroupRequireDesc			MaterialSerializer::msGroupRequireDesc;
	MaterialSerializer::SwitchDesc					MaterialSerializer::msSwitchDesc;
	MaterialSerializer::PropertyDesc				MaterialSerializer::msPropertyDesc;
	//MaterialSerializer::TextPropertyDesc			MaterialSerializer::msTexturePropertyDesc;

	bool	MaterialSerializer::msInited = false;

	namespace MATERIAL_CONST
	{
		const TString MATERIAL_SIGNATURE = BTString("BladeMaterial");
		const TString NAME = BTString("name");
		const TString DEFAULT = BTString("default");
		const TString PROFILE = BTString("profile");
		const TString SWITCH_ENABLE = BTString("enable");
		const TString SWITCH_DISABLE = BTString("disable");
		const TString MATERIAL_NODE = BTString("material");
		const TString TECHNIQUE_NODE = BTString("technique");
		const TString PASS_NODE = BTString("pass");
		const TString CLONE = BTString("clone");
		const TString PROPERTY_NODE = BTString("render_property");

		//shader uniform names
		const TString TEXTURES_NODE = BTString("textures");
		const TString TEXTURE_NODE = BTString("texture");

		//render property attributes
		const TString PROPERTY_FILLMODE = BTString("fill_mode");
		const TString PROPERTY_CULLMODE = BTString("cull_mode");

		const TString FILLMODE_SOLID = BTString("solid");
		const TString FILLMODE_POINT = BTString("point");
		const TString FILLMODE_WIREFRAME = BTString("wireframe");

		const TString CULLMODE_CCW = BTString("CCW");
		const TString CULLMODE_CW = BTString("CW");
		const TString CULLMODE_NONE = BTString("none");

		//render property sub nodes
		const TString PROPERTY_DEPTH_NODE = BTString("depth");
		const TString PROPERTY_COLOR_NODE = BTString("color");
		const TString PROPERTY_COLOR_WRITE_NODE = BTString("color_write");
		const TString PROPERTY_FOG_NODE = BTString("fog");
		const TString PROPERTY_ALPHA_BLEND_NODE = BTString("alpha_blend");
		const TString PROPERTY_STENCIL_NODE = BTString("stencil");
		const TString PROPERTY_SCISSOR_NODE = BTString("scissor");
		const TString SAMPLERS_NODE = BTString("samplers");

		//sort groups for techniques
		const TString TECHNIQUE_SORTGROUP_NODE = BTString("groups");
		const TString SORTGROUP_GROUP_NODE = BTString("group");
		const TString SORTGROUP_REQUIREMENT_NODE = BTString("requirements");
		const TString SORTGROUP_REQUIREMENT_TEXTURE = BTString("texture");
		const TString SORTGROUP_REQUIREMENT_TEXTURE_INDEX = BTString("index");
		const TString SORTGROUP_REQUIREMENT_ALPHABLEND = PROPERTY_ALPHA_BLEND_NODE;
		const TString SORTGROUP_REQUIREMENT_FOG = PROPERTY_FOG_NODE;

		//LOD for passes
		const TString PASS_LOD_NODE = BTString("LOD");
		const TString LOD_LEVEL = BTString("level");
		const TString LOD_SORTGROUP = BTString("sort_group");
		const TString LOD_START = BTString("dist_start");
		const TString LOD_END = BTString("dist_end");
		const TString LOD_MAXDIST = BTString("MAX");

		//depth test atrributes
		const TString DEPTH_TEST_SWITCH = BTString("test");
		const TString DEPTH_TEST_FUNC = BTString("test_func");
		const TString DEPTH_WRITE = BTString("write");
		const TString DEPTH_BIAS = BTString("bias");
		const TString DEPTH_SLOPE_BIAS = BTString("slope_bias");
		const TString DEPTH_CLIP = BTString("clip");
		const TString DEPTH_BIAS_CLAMP = BTString("bias_clamp");

		//(depth) test types
		const TString TEST_FUNC_NEVER = BTString("never");
		const TString TEST_FUNC_LESS = BTString("less");
		const TString TEST_FUNC_EQUAL = BTString("equal");
		const TString TEST_FUNC_LESSEQUAL = BTString("less_equal");
		const TString TEST_FUNC_GREATER = BTString("greater");
		const TString TEST_FUNC_NOTEQUAL = BTString("not_equal");
		const TString TEST_FUNC_GREATEREQUAL = BTString("greater_equal");
		const TString TEST_FUNC_ALWAYS = BTString("always");

		//color attributes
		const TString COLOR_DIFFUSE = BTString("diffuse");
		const TString COLOR_SPECULAR = BTString("specular");
		const TString COLOR_EMISSIVE = BTString("emissive");

		//color write attributes
		const TString COLOR_WRITE_RED = BTString("red");
		const TString COLOR_WRITE_GREEN = BTString("green");
		const TString COLOR_WRITE_BLUE = BTString("blue");
		const TString COLOR_WRITE_ALPHA = BTString("alpha");

		//alpha blend attributes
		const TString ALPHA_BLEND_SRC = BTString("src");
		const TString ALPHA_BLEND_DEST = BTString("dest");
		const TString ALPHA_BLEND_OP = BTString("op");
		const TString ALPHA_BLEND_INDEPENDENT = BTString("independent");

		const TString ALPHA_BLEND_ALPHASRC = BTString("alpha_src");
		const TString ALPHA_BLEND_ALPHADEST = BTString("alpha_dest");
		const TString ALPHA_BLEND_ALPHAOP = BTString("alpha_op");

		const TString ALPHA_BLEND_MODE_ZERO = BTString("zero");
		const TString ALPHA_BLEND_MODE_ONE = BTString("one");

		const TString ALPHA_BLEND_MODE_SRC_COLOR = BTString("src_color");
		const TString ALPHA_BLEND_MODE_INV_SRC_COLOR = BTString("inv_src_color");
		const TString ALPHA_BLEND_MODE_SRC_ALPHA = BTString("src_alpha");
		const TString ALPHA_BLEND_MODE_INV_SRC_ALPHA = BTString("inv_src_alpha");

		const TString ALPHA_BLEND_MODE_DEST_COLOR = BTString("dest_color");
		const TString ALPHA_BLEND_MODE_INV_DEST_COLOR = BTString("inv_dest_color");
		const TString ALPHA_BLEND_MODE_DEST_ALPHA = BTString("dest_alpha");
		const TString ALPHA_BLEND_MODE_INV_DEST_ALPHA = BTString("inv_dest_alpha");

		const TString ALPHA_BLEND_OP_ADD = BTString("add");
		const TString ALPHA_BLEND_OP_SUB = BTString("substract");
		const TString ALPHA_BLEND_OP_REVERSE_SUB = BTString("reverse_substract");
		const TString ALPHA_BLEND_OP_MIN = BTString("min");
		const TString ALPHA_BLEND_OP_MAX = BTString("max");
		//stencil attributes
		const TString STENCIL_READ_MASK = BTString("read_mask");
		const TString STENCIL_WRITE_MASK = BTString("write_mask");
		const TString STENCIL_TEST_FRONT = BTString("front_test");
		const TString STENCIL_TEST_BACK = BTString("back_test");
		const TString STENCIL_OP_FRONT_PASS = BTString("front_pass_op");
		const TString STENCIL_OP_FRONT_FAIL = BTString("front_fail_op");
		const TString STENCIL_OP_BACK_PASS = BTString("back_pass_op");
		const TString STENCIL_OP_BACK_FAIL = BTString("back_fail_op");

		const TString STENCIL_OP_KEEP = BTString("keep");
		const TString STENCIL_OP_ZERO = BTString("zero");
		const TString STENCIL_OP_REPLACE = BTString("replace");
		const TString STENCIL_OP_INC_CLAMP = BTString("inc_clamp");
		const TString STENCIL_OP_DEC_CLAMP = BTString("dec_clamp");
		const TString STENCIL_OP_INVERT = BTString("invert");
		const TString STENCIL_OP_INC_WRAP = BTString("inc_wrap");
		const TString STENCIL_OP_DEC_WRAP = BTString("dec_wrap");

		//scissor
		const TString SCISSOR_LEFT = BTString("left");
		const TString SCISSOR_RIGHT = BTString("right");
		const TString SCISSOR_TOP = BTString("top");
		const TString SCISSOR_BOTTOM = BTString("bottom");

		//texture
		const TString SAMPLER_NODE = BTString("sampler");
		const TString SAMPLER_MAX_ANISOTROPY = BTString("max_anisotropy");
		const TString SAMPLER_TEST = BTString("test");

		const TString TEXTURE_TYPE_1D = BTString("1D");
		const TString TEXTURE_TYPE_2D = BTString("2D");
		const TString TEXTURE_TYPE_3D = BTString("3D");
		const TString TEXTURE_TYPE_CUBE = BTString("cube");

		const TString SAMPLER_MIPMAP_NODE = BTString("mipmap_lod");
		const TString SAMPLER_MIPMAP_MIN = BTString("min");
		const TString SAMPLER_MIPMAP_MAX = BTString("max");
		const TString SAMPLER_MIPMAP_BIAS = BTString("bias");

		const TString SAMPLER_ADDRESS_NODE = BTString("address_mode");
		const TString SAMPLER_ADDRESS_U = BTString("u");
		const TString SAMPLER_ADDRESS_V = BTString("v");
		const TString SAMPLER_ADDRESS_W = BTString("w");

		const TString SAMPLER_ADDRESSMODE_WRAP = BTString("wrap");
		const TString SAMPLER_ADDRESSMODE_MIRROR = BTString("mirror");
		const TString SAMPLER_ADDRESSMODE_CLAMP = BTString("clamp");
		//const TString SAMPLER_ADDRESSMODE_BORDER = BTString("border");

		const TString SAMPLER_FILTER_NODE = BTString("filter");
		const TString SAMPLER_FILTER_MIN = BTString("min");
		const TString SAMPLER_FILTER_MAG = BTString("mag");
		const TString SAMPLER_FILTER_MIP = BTString("mip");

		const TString SAMPLER_FILTER_MODE_NONE = BTString("none");
		const TString SAMPLER_FILTER_MODE_POINT = BTString("point");
		const TString SAMPLER_FILTER_MODE_LINEAR = BTString("linear");
		const TString SAMPLER_FILTER_MODE_ANISOTROPY = BTString("anisotropy");

		//shader options
		const TString SHADER_OPTION_NODE = BTString("shader_option");
		const TString SHADER_OPTION_GROUP = BTString("group");

		const TString SHADER_NODE = BTString("shader");
		const TString SHADER_SOURCE = BTString("source");
		const TString SHADER_SUBSHADER = BTString("subshader");

		//shader variables
		const TString SHADER_BUILTIN_PARAM = BTString("param");
		const TString SHADER_BUILTIN_SEMATIC = BTString("semantic");

		const TString SHADER_VARIABLE_TYPE = BTString("type");
		const TString SHADER_VARIABLE_COUNT = BTString("count");
		
		const TString SHADER_VARIABLE_TYPE_INT = BTString("int");
		const TString SHADER_VARIABLE_TYPE_BOOL = BTString("bool");
		const TString SHADER_VARIABLE_TYPE_FLOAT1 = BTString("float1");
		const TString SHADER_VARIABLE_TYPE_FLOAT2 = BTString("float2");
		const TString SHADER_VARIABLE_TYPE_FLOAT3 = BTString("float3");
		const TString SHADER_VARIABLE_TYPE_FLOAT4 = BTString("float4");
		
	}//namespace MATERIAL_CONSTANT




	//////////////////////////////////////////////////////////////////////////
	MaterialSerializer::MaterialSerializer()
	{
		MaterialSerializer::initialize();
	}

	//////////////////////////////////////////////////////////////////////////
	MaterialSerializer::~MaterialSerializer()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	bool	MaterialSerializer::loadResource(IResource* res,const HSTREAM& stream, const ParamList& /*params*/)
	{
		if( res == NULL || stream == NULL)
			BLADE_EXCEPT(EXC_INVALID_PARAM,BTString("resource or stream data corrupted.") );
		else if( res->getType() != Material::MATERIAL_RESOURCE_TYPE )
			BLADE_EXCEPT(EXC_INVALID_PARAM,BTString("resource type mismatch.") );
		return this->loadData(stream);
	}

	//////////////////////////////////////////////////////////////////////////
	void	MaterialSerializer::postProcessResource(IResource* resource)
	{
		assert( resource != NULL && resource->getType() == Material::MATERIAL_RESOURCE_TYPE);

		DefaultMaterial* material = static_cast<DefaultMaterial*>(resource);
		material->removeAllTechniques();

		for(TechniqueList::iterator i = mTechniqueList.begin(); i != mTechniqueList.end(); ++i)
			material->addTechnique(*i);

		material->mName = mMaterialName;
		material->setTextureUniforms(mTextureUniforms);
	}

	//////////////////////////////////////////////////////////////////////////
	bool	MaterialSerializer::saveResource(const IResource* res,const HSTREAM& stream)
	{
		//TODO: add saving support
		BLADE_UNREFERENCED(res);
		BLADE_UNREFERENCED(stream);

		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	MaterialSerializer::createResource(IResource* res,ParamList& /*params*/)
	{
		//TODO: add create new object support
		BLADE_UNREFERENCED(res);

		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	MaterialSerializer::reloadResource(IResource* /*resource*/, const HSTREAM& stream, const ParamList& /*params*/)
	{
		return this->loadData(stream);
	}

	//////////////////////////////////////////////////////////////////////////
	bool	MaterialSerializer::reprocessResource(IResource* resource)
	{
		if( resource == NULL || resource->getType() == Material::MATERIAL_RESOURCE_TYPE)
			return false;

		DefaultMaterial* material = static_cast<DefaultMaterial*>(resource);
		material->removeAllTechniques();

		for(TechniqueList::iterator i = mTechniqueList.begin(); i != mTechniqueList.end(); ++i)
			material->addTechnique(*i);

		//DO NOT change material name when reloading
		//material->mName = mMaterialName;

		return true;
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	bool	MaterialSerializer::loadData(const HSTREAM& stream)
	{
		XmlFile xmlFile;
		if( !xmlFile.open(stream) )
			return false;


		IXmlNode* pNode = xmlFile.getRootNode();

		if( pNode == NULL || pNode->getName() != MATERIAL_CONST::MATERIAL_SIGNATURE )
		{
			BLADE_EXCEPT(EXC_INVALID_PARAM,BTString("invalid material script.") );
			//return false;
		}

		IXmlNode::IXmlNodeList* matList = pNode->getChildList( MATERIAL_CONST::MATERIAL_NODE );

		//currently support only one definition
		if( matList->safeSize() != 1)
			return false;

		IXmlNode* pMatChild = matList->getAt(0);

		const TString* pName = pMatChild->getAttributeValue(MATERIAL_CONST::NAME);
		if( pName == NULL )
		{
			BLADE_EXCEPT(EXC_INVALID_PARAM,BTString("material script \"") + stream->getName() + BTString("\" - material section without a name.") );
		}

		mMaterialName = *pName;

		//loading texture uniforms
		IXmlNode::IXmlNodeList* textureList = pMatChild->getChildList( MATERIAL_CONST::TEXTURES_NODE );
		if( textureList->safeSize() > 0 )
		{
			//use only the first ones
			IXmlNode* textureListNode = textureList->getAt(0);
			IXmlNode::IXmlNodeList* uniforms = textureListNode->getChildList( MATERIAL_CONST::TEXTURE_NODE );
			mTextureUniforms.reserve(uniforms->safeSize());
			for(size_t i = 0; i < uniforms->safeSize(); ++i)
			{
				IXmlNode* texNode = uniforms->getAt(i);
				const TString* uniform = texNode->getAttributeValue( MATERIAL_CONST::NAME );
				if( uniform != NULL && !uniform->empty() )
				mTextureUniforms.push_back(*uniform);
			}
		}

		IXmlNode::IXmlNodeList* pTechList = pMatChild->getChildList( MATERIAL_CONST::TECHNIQUE_NODE );
		for(size_t i = 0; i < pTechList->safeSize(); ++i )
		{
			IXmlNode* techNode = pTechList->getAt(i);

			const TString* pTechName = techNode->getAttributeValue(MATERIAL_CONST::NAME);
			if( pTechName == NULL )
				BLADE_EXCEPT(EXC_INVALID_PARAM,BTString("material script \"") + stream->getName() + BTString("\" - technique section without a name.") );

			const TString* pProfile = techNode->getAttributeValue(MATERIAL_CONST::PROFILE);
			if( pProfile == NULL )
				BLADE_EXCEPT(EXC_INVALID_PARAM,BTString("material script \"") + stream->getName() + BTString("\" - technique section without a profile.") );


			Technique* tech = BLADE_NEW Technique(*pTechName, *pProfile, NULL);
			this->loadSingleTechnique( techNode, tech );
			mTechniqueList.push_back( tech );
		}

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void	MaterialSerializer::loadRenderPropertyForPass(IXmlNode* propNode,Pass* pPass)
	{
		RenderPropertySet* pPropSet = pPass->createRenderProperty();

		const TString* fillmode = propNode->getAttributeValue( MATERIAL_CONST::PROPERTY_FILLMODE );
		if( fillmode != NULL )
		{
			FillModeDesc::const_iterator i = msFillModeDesc.find( *fillmode );
			if( i != msFillModeDesc.end() )
				pPropSet->setFillMode( i->second );
			else
				ILog::Error << BTString("Invalid fill mode :") << *fillmode << ILog::endLog;
		}
		const TString* cullmode = propNode->getAttributeValue( MATERIAL_CONST::PROPERTY_CULLMODE );
		if( cullmode != NULL )
		{
			CullModeDesc::const_iterator i = msCullModeDesc.find( *cullmode );
			if( i != msCullModeDesc.end() )
				pPropSet->setCullMode( i->second );
			else
				ILog::Error << BTString("Invalid cull mode :") << *cullmode << ILog::endLog;
		}

		for(size_t i = 0; i < propNode->getChildCount(); ++i)
		{
			IXmlNode* node = propNode->getChild(i);
			const TString& name = node->getName();
			PropertyDesc::const_iterator iter = msPropertyDesc.find( name );
			if( iter == msPropertyDesc.end() )
			{
				ILog::Error << BTString("invalid render property :") << name << ILog::endLog;
				continue;
			}
			RENDER_PROPERTY rpt = iter->second;
			switch(rpt)
			{
			case RPT_COLOR:
				if( node->getAttributeCount() > 0)
					this->loadColorProperty(node, pPropSet);
				break;
			case RPT_COLORWIRTE:
					this->loadColorWriteProperty(node, pPropSet);
				break;
			case RPT_FOG:
				this->loadFogProperty(node, pPropSet);
				break;
			case RPT_ALPHABLEND:
				if( node->getAttributeCount() > 0 ) //add default if no params set?
					this->loadAlphaBlendProperty(node, pPropSet);
				break;
			case RPT_DEPTH:
				if( node->getAttributeCount() > 0 )
					this->loadDepthProperty(node, pPropSet);
				break;
			case RPT_STENCIL:
				if( node->getAttributeCount() > 0 )
					this->loadStencilProperty(node, pPropSet);
				break;
			case RPT_SCISSOR:
				if( node->getChildCount() > 0 )
					this->loadScissorProperty(node, pPropSet);
				break;
			default:
				assert(false);
				break;
			}
		}

	}

	//////////////////////////////////////////////////////////////////////////
	void			MaterialSerializer::loadShaderForPass(IXmlNode* pShaderOptionNode, Pass* pPass, Technique* tech, index_t index, const Pass* cloned)
	{
		HSHADEROPTION clone;
		const TString* name = pShaderOptionNode->getAttributeValue(MATERIAL_CONST::NAME);
		TString defualtName = MATERIAL_CONST::DEFAULT + TStringHelper::fromUInt(index);
		if (name == NULL)
			name = &defualtName;

		index = cloned != NULL ? pPass->findShaderOption(*name) : INVALID_INDEX;
		clone = pPass->getShaderOption(index);

		HSHADEROPTION hShaderOption(BLADE_NEW ShaderOption(*name));
		if (clone != NULL)
			*hShaderOption = *clone;

		const ShaderGroupList& list = tech->getShaderGroupList();
		assert(list.size() > 0 );

		//read group index
		const TString* groupsString = pShaderOptionNode->getAttributeValue( MATERIAL_CONST::SHADER_OPTION_GROUP );
		if (groupsString == NULL)
		{
			if(clone == NULL)
				hShaderOption->addGroupLinks(0, (uint8)list.size());
		}
		else
		{	
			TStringTokenizer tokenizer;
			tokenizer.tokenize( *groupsString, TEXT(","));

			//although there can be unlimited count of sort groups
			//but a shader can only be bound to a limited count of groups
			assert( tokenizer.size() < MATERIALLOD::MAX_LOD );

			for(size_t i = 0; i < tokenizer.size(); ++i)
			{
				const TString& gname = tokenizer[i];
				index_t groupIndex = list.findGroup(gname);
				if(groupIndex != INVALID_INDEX)
					hShaderOption->addGroupLink( (uint8)groupIndex );
			}
		}

		ShaderInstance* inst = hShaderOption->createShader( tech->getProfile() );

		const TString* subShader = pShaderOptionNode->getAttributeValue( MATERIAL_CONST::SHADER_SUBSHADER );
		if (subShader != NULL)
			inst->setSubShaderName(*subShader);
		else if (clone != NULL)
			inst->setSubShaderName(clone->getShaderInstance()->getSubShaderName());

		//parsing shader
		const TString* source = pShaderOptionNode->getAttributeValue( MATERIAL_CONST::SHADER_SOURCE );
		if (source != NULL && *source != TString::EMPTY)
			inst->setResourceInfo(ShaderResource::SHADER_RESOURCE_TYPE, *source);
		else if (clone != NULL)
			inst->setResourceInfo(ShaderResource::SHADER_RESOURCE_TYPE, clone->getShaderInstance()->getResourcePath());
		else
			BLADE_EXCEPT(EXC_NEXIST,BTString("cannot load shader, shader file not specified.") );

		if (clone != NULL)
			pPass->setShaderOption(index, hShaderOption);
		else
			pPass->addShaderOption(hShaderOption);
	}

	//////////////////////////////////////////////////////////////////////////
	void			MaterialSerializer::loadColorProperty(IXmlNode* pColorNode, RenderPropertySet* propSet)
	{
		ColorProperty* colorProp = BLADE_NEW ColorProperty();
		//reuse clone source
		{
			ColorProperty* oldProp = static_cast<ColorProperty*>(propSet->getProperty(RPT_COLOR));
			if (oldProp != NULL)
				*colorProp = *oldProp;
		}

		Variant var(Color::WHITE);

		const TString* specular = pColorNode->getAttributeValue(MATERIAL_CONST::COLOR_SPECULAR);
		if( specular != NULL )
		{
			var.fromTString(*specular);
			colorProp->setSpecular( var );
		}

		const TString* diffuse = pColorNode->getAttributeValue(MATERIAL_CONST::COLOR_DIFFUSE);
		if( diffuse != NULL )
		{
			var.fromTString(*diffuse);
			colorProp->setDiffuse( var );
		}

		const TString* emissive = pColorNode->getAttributeValue(MATERIAL_CONST::COLOR_EMISSIVE);
		if( emissive != NULL )
		{
			var.fromTString(*emissive);
			colorProp->setEmissive( var );
		}

		propSet->setProperty(colorProp);
	}

	//////////////////////////////////////////////////////////////////////////
	void			MaterialSerializer::loadColorWriteProperty(IXmlNode* pColorWriteNode, RenderPropertySet* propSet)
	{
		ColorWriteProperty* colorWriteProp = BLADE_NEW ColorWriteProperty();
		//reuse clone source
		{
			ColorWriteProperty* oldProp = static_cast<ColorWriteProperty*>(propSet->getProperty(RPT_COLORWIRTE));
			if (oldProp != NULL)
				*colorWriteProp = *oldProp;
		}

		int mask = CWC_NONE;

		const TString* red = pColorWriteNode->getAttributeValue(MATERIAL_CONST::COLOR_WRITE_RED);
		if( red != NULL )
		{
			SwitchDesc::const_iterator i = msSwitchDesc.find( *red );
			if( i != msSwitchDesc.end() && i->second == true )
				mask |= CWC_RED;
		}

		const TString* green = pColorWriteNode->getAttributeValue(MATERIAL_CONST::COLOR_WRITE_GREEN);
		if( green != NULL )
		{
			SwitchDesc::const_iterator i = msSwitchDesc.find( *green );
			if( i != msSwitchDesc.end() && i->second == true )
				mask |= CWC_GREEN;
		}

		const TString* blue = pColorWriteNode->getAttributeValue(MATERIAL_CONST::COLOR_WRITE_BLUE);
		if( blue != NULL )
		{
			SwitchDesc::const_iterator i = msSwitchDesc.find( *blue );
			if( i != msSwitchDesc.end() && i->second == true )
				mask |= CWC_BLUE;
		}

		const TString* alpha = pColorWriteNode->getAttributeValue(MATERIAL_CONST::COLOR_WRITE_ALPHA);
		if( alpha != NULL )
		{
			SwitchDesc::const_iterator i = msSwitchDesc.find( *alpha );
			if( i != msSwitchDesc.end() && i->second == true )
				mask |= CWC_ALPHA;
		}
		colorWriteProp->setWritingColorChannel(mask);

		propSet->setProperty(colorWriteProp);
	}

	//////////////////////////////////////////////////////////////////////////
	void			MaterialSerializer::loadFogProperty(IXmlNode* fogNode, RenderPropertySet* propSet)
	{
		BLADE_UNREFERENCED(fogNode);
		FogProperty* fogProp = BLADE_NEW FogProperty();
		propSet->setProperty(fogProp);
	}

	//////////////////////////////////////////////////////////////////////////
	void			MaterialSerializer::loadAlphaBlendProperty(IXmlNode* alphaBlendNode, RenderPropertySet* propSet)
	{
		AlphaBlendProperty* blendProp = BLADE_NEW AlphaBlendProperty();
		//reuse clone source
		{
			AlphaBlendProperty* oldProp = static_cast<AlphaBlendProperty*>(propSet->getProperty(RPT_ALPHABLEND));
			if (oldProp != NULL)
				*blendProp = *oldProp;
		}

		const TString* srcBlend = alphaBlendNode->getAttributeValue( MATERIAL_CONST::ALPHA_BLEND_SRC );
		if( srcBlend != NULL )
		{
			BlendModeDesc::const_iterator i = msBlendModeDesc.find( *srcBlend );
			if( i != msBlendModeDesc.end() )
				blendProp->setSourceBlend( i->second );
			else
				ILog::Error << BTString("invalid blend mode: ") << *srcBlend << ILog::endLog;
		}

		const TString* destBlend = alphaBlendNode->getAttributeValue( MATERIAL_CONST::ALPHA_BLEND_DEST );
		if( destBlend != NULL )
		{
			BlendModeDesc::const_iterator i = msBlendModeDesc.find( *destBlend );
			if( i != msBlendModeDesc.end() )
				blendProp->setDestBlend( i->second );
			else
				ILog::Error << BTString("invalid blend mode: ") << *srcBlend << ILog::endLog;
		}

		const TString* blendOp = alphaBlendNode->getAttributeValue( MATERIAL_CONST::ALPHA_BLEND_OP );
		if( blendOp != NULL )
		{
			BlendOpDesc::const_iterator i = msBlendOpDesc.find( *blendOp );
			if( i != msBlendOpDesc.end() )
				blendProp->setBlendOperation( i->second );
			else
				ILog::Error << BTString("invalid blend operation: ") << *blendOp << ILog::endLog;
		}

		const TString* srcAlphaBlend = alphaBlendNode->getAttributeValue( MATERIAL_CONST::ALPHA_BLEND_ALPHASRC );
		if( srcAlphaBlend != NULL )
		{
			BlendModeDesc::const_iterator i = msBlendModeDesc.find( *srcAlphaBlend );
			if( i != msBlendModeDesc.end() )
				blendProp->setSourceAlphaBlend( i->second );
			else
				ILog::Error << BTString("invalid blend mode: ") << *srcAlphaBlend << ILog::endLog;
		}

		const TString* destAlphaBlend = alphaBlendNode->getAttributeValue( MATERIAL_CONST::ALPHA_BLEND_ALPHADEST );
		if( destAlphaBlend != NULL )
		{
			BlendModeDesc::const_iterator i = msBlendModeDesc.find( *destAlphaBlend );
			if( i != msBlendModeDesc.end() )
				blendProp->setDestAlphaBlend( i->second );
			else
				ILog::Error << BTString("invalid blend mode: ") << *destAlphaBlend << ILog::endLog;
		}

		const TString* AlphablendOp = alphaBlendNode->getAttributeValue( MATERIAL_CONST::ALPHA_BLEND_ALPHAOP );
		if( AlphablendOp != NULL )
		{
			BlendOpDesc::const_iterator i = msBlendOpDesc.find( *AlphablendOp );
			if( i != msBlendOpDesc.end() )
				blendProp->setAlphaBlendOperation( i->second );
			else
				ILog::Error << BTString("invalid blend operation: ") << *AlphablendOp << ILog::endLog;
		}

		const TString* independentBlend = alphaBlendNode->getAttributeValue( MATERIAL_CONST::ALPHA_BLEND_INDEPENDENT );
		if( independentBlend != NULL )
		{
			SwitchDesc::const_iterator i = msSwitchDesc.find( *independentBlend );
			if( i != msSwitchDesc.end() )
				blendProp->setIndependentBlend( i->second );
			else
				ILog::Error << BTString("invalid value -") << *independentBlend << ILog::endLog;
		}

		propSet->setProperty( blendProp );
	}

	//////////////////////////////////////////////////////////////////////////
	void			MaterialSerializer::loadDepthProperty(IXmlNode* depthNode, RenderPropertySet* propSet)
	{
		DepthProperty* depthProp = BLADE_NEW DepthProperty();
		//reuse clone source
		{
			DepthProperty* oldProp = static_cast<DepthProperty*>(propSet->getProperty(RPT_DEPTH));
			if (oldProp != NULL)
				*depthProp = *oldProp;
		}

		const TString* test = depthNode->getAttributeValue( MATERIAL_CONST::DEPTH_TEST_SWITCH );
		if( test != NULL )
		{
			SwitchDesc::const_iterator i = msSwitchDesc.find( *test );
			if( i != msSwitchDesc.end() )
				depthProp->setTestEnable( i->second );
			else
				ILog::Error << BTString("invalid depth test switch: ") << *test << ILog::endLog;
		}

		const TString* testFunc = depthNode->getAttributeValue( MATERIAL_CONST::DEPTH_TEST_FUNC );
		if( testFunc != NULL )
		{
			TestFuncDesc::const_iterator i = msTestFuncDesc.find( *testFunc );
			if( i != msTestFuncDesc.end() )
				depthProp->setDepthTestFunc( i->second );
			else
				ILog::Error << BTString("invalid test function: ") << *testFunc << ILog::endLog;
		}

		const TString* write = depthNode->getAttributeValue( MATERIAL_CONST::DEPTH_WRITE );
		if( write != NULL )
		{
			SwitchDesc::const_iterator i = msSwitchDesc.find( *write );
			if( i != msSwitchDesc.end() )
				depthProp->setWriteEnable( i->second );
			else
				ILog::Error << BTString("invalid depth write switch: ") << *write << ILog::endLog;
		}

		const TString* clip = depthNode->getAttributeValue( MATERIAL_CONST::DEPTH_CLIP );
		if( clip != NULL )
		{
			SwitchDesc::const_iterator i = msSwitchDesc.find( *clip );
			if( i != msSwitchDesc.end() )
				depthProp->setEneableDepthClip( i->second );
			else
				ILog::Error << BTString("invalid depth write switch: ") << *clip << ILog::endLog;
		}

		const TString* bias = depthNode->getAttributeValue( MATERIAL_CONST::DEPTH_BIAS );
		if( bias != NULL )
		{
			int biasVal = (int)TStringHelper::toInt( *bias );
			depthProp->setDepthBias( biasVal );
		}

		const TString* slopBias = depthNode->getAttributeValue( MATERIAL_CONST::DEPTH_SLOPE_BIAS );
		if( slopBias != NULL )
		{
			scalar slopBiasVal = TStringHelper::toScalar(*slopBias);
			depthProp->setSlopeScaledDepthBias( slopBiasVal );
		}

		const TString* biasClamp = depthNode->getAttributeValue( MATERIAL_CONST::DEPTH_BIAS_CLAMP );
		if( biasClamp != NULL )
		{
			scalar clamp = TStringHelper::toScalar( *biasClamp );
			depthProp->setDepthBiasClamp( clamp );
		}
		propSet->setProperty( depthProp );
	}

	//////////////////////////////////////////////////////////////////////////
	void			MaterialSerializer::loadStencilProperty(IXmlNode* stencilNode, RenderPropertySet* propSet)
	{
		StencilProperty* stencilProp = BLADE_NEW StencilProperty();
		//reuse clone source
		{
			StencilProperty* oldProp = static_cast<StencilProperty*>(propSet->getProperty(RPT_STENCIL));
			if (oldProp != NULL)
				*stencilProp = *oldProp;
		}

		const TString* readMask = stencilNode->getAttributeValue( MATERIAL_CONST::STENCIL_READ_MASK );
		if( readMask != NULL )
		{
			uint8 mask = (uint8)TStringHelper::toHex(*readMask);
			stencilProp->setReadMask(mask);
		}

		const TString* writeMask = stencilNode->getAttributeValue( MATERIAL_CONST::STENCIL_WRITE_MASK );
		if( writeMask != NULL )
		{
			uint8 mask = (uint8)TStringHelper::toHex(*writeMask);
			stencilProp->setWriteMask(mask);
		}

		const TString* frontTest = stencilNode->getAttributeValue( MATERIAL_CONST::STENCIL_TEST_FRONT );
		if( frontTest != NULL )
		{
			TestFuncDesc::const_iterator i = msTestFuncDesc.find( *frontTest );
			if( i != msTestFuncDesc.end() )
				stencilProp->setFrontFaceTestFunc( i->second );
			else
				ILog::Error << BTString("invalid front face stencil test: ") << *frontTest << ILog::endLog;
		}

		const TString* backTest = stencilNode->getAttributeValue( MATERIAL_CONST::STENCIL_TEST_BACK );
		if( backTest != NULL )
		{
			TestFuncDesc::const_iterator i = msTestFuncDesc.find( *backTest );
			if( i != msTestFuncDesc.end() )
				stencilProp->setBackFaceTestFunc( i->second );
			else
				ILog::Error << BTString("invalid back face stencil test: ") << *backTest << ILog::endLog;
		}

		const TString* frontFail = stencilNode->getAttributeValue( MATERIAL_CONST::STENCIL_OP_FRONT_FAIL );
		if( frontFail != NULL )
		{
			StencilOpDesc::const_iterator i = msStencilOpDesc.find( *frontFail );
			if( i != msStencilOpDesc.end() )
				stencilProp->setFrontFaceFailOperation( i->second );
			else
				ILog::Error << BTString("invalid stencil operation: ") << *frontFail << ILog::endLog;
		}

		const TString* frontPass = stencilNode->getAttributeValue( MATERIAL_CONST::STENCIL_OP_FRONT_PASS );
		if( frontPass != NULL )
		{
			StencilOpDesc::const_iterator i = msStencilOpDesc.find( *frontPass );
			if( i != msStencilOpDesc.end() )
				stencilProp->setFrontFacePassOperation( i->second );
			else
				ILog::Error << BTString("invalid stencil operation: ") << *frontPass << ILog::endLog;
		}

		const TString* backFail = stencilNode->getAttributeValue( MATERIAL_CONST::STENCIL_OP_BACK_FAIL );
		if( backFail != NULL )
		{
			StencilOpDesc::const_iterator i = msStencilOpDesc.find( *backFail );
			if( i != msStencilOpDesc.end() )
				stencilProp->setBackFaceFailOperation( i->second );
			else
				ILog::Error << BTString("invalid stencil operation: ") << *backFail << ILog::endLog;
		}

		const TString* backPass = stencilNode->getAttributeValue( MATERIAL_CONST::STENCIL_OP_BACK_PASS );
		if( backPass != NULL )
		{
			StencilOpDesc::const_iterator i = msStencilOpDesc.find( *backPass );
			if( i != msStencilOpDesc.end() )
				stencilProp->setBackFacePassOperation( i->second );
			else
				ILog::Error << BTString("invalid stencil operation: ") << *backPass << ILog::endLog;
		}
		propSet->setProperty( stencilProp );
	}

	//////////////////////////////////////////////////////////////////////////
	void			MaterialSerializer::loadScissorProperty(IXmlNode* scissorNode, RenderPropertySet* propSet)
	{
		ScissorProperty* scissorProp = BLADE_NEW ScissorProperty();
		IXmlNode::IXmlNodeList* scissorList = scissorNode->getChildList( MATERIAL_CONST::PROPERTY_SCISSOR_NODE  );
		index_t index = 0;
		for(size_t i = 0; i < scissorList->safeSize(); ++i)
		{
			IXmlNode* node = scissorList->getAt(i);
			if(node->getAttributeCount() != 4 )
				continue;

			intptr_t left = 0, top = 0, right = 0, bottom = 0;
			const TString* leftString = node->getAttributeValue( MATERIAL_CONST::SCISSOR_LEFT );
			if( leftString != NULL )
				left = TStringHelper::toInt(*leftString);
			else
			{
				ILog::Error << BTString("missing left dimesion of scissor rectangle.") << ILog::endLog;
				continue;
			}

			const TString* topString = node->getAttributeValue( MATERIAL_CONST::SCISSOR_TOP );
			if( topString != NULL )
				top = TStringHelper::toInt(*topString);
			else
			{
				ILog::Error << BTString("missing top dimesion of scissor rectangle.") << ILog::endLog;
				continue;
			}

			const TString* rightString = node->getAttributeValue( MATERIAL_CONST::SCISSOR_RIGHT );
			if( rightString != NULL )
				right = TStringHelper::toInt(*rightString);
			else
			{
				ILog::Error << BTString("missing right dimesion of scissor rectangle.") << ILog::endLog;
				continue;
			}

			const TString* bottomString = node->getAttributeValue( MATERIAL_CONST::SCISSOR_BOTTOM );
			if( bottomString != NULL )
				bottom = TStringHelper::toInt(*bottomString);
			else
			{
				ILog::Error << BTString("missing bottom dimesion of scissor rectangle.") << ILog::endLog;
				continue;
			}
			Box2i box( (int)left, (int)top, (int)right, (int)bottom);
			scissorProp->setScissorBox(index++,box);
		}
		scissorProp->setValidCount(index);
		propSet->setProperty(scissorProp);
	}

	//////////////////////////////////////////////////////////////////////////
	void			MaterialSerializer::loadSamplerStateForPass(IXmlNode* pSamplerNode, Pass* pass)
	{
		IXmlNode::IXmlNodeList* samplerNodeList = pSamplerNode->getChildList( MATERIAL_CONST::SAMPLER_NODE );

		for(size_t i = 0; i < samplerNodeList->safeSize(); ++i )
		{
			IXmlNode* subNode = samplerNodeList->getAt(i);

			const TString* max_anisotropy = subNode->getAttributeValue( MATERIAL_CONST::SAMPLER_MAX_ANISOTROPY );
			const TString* test = subNode->getAttributeValue( MATERIAL_CONST::SAMPLER_TEST );
			const TString* name = subNode->getAttributeValue(MATERIAL_CONST::NAME);
			//const TString* type = subNode->getAttributeValue( BTString("type") );

			if( name == NULL )
				BLADE_EXCEPT(EXC_INVALID_PARAM, BTString("sampler definition without a name"));

			Sampler sampler;
			index_t index = pass->getSamplerState().getSamplerIndex(*name);
			if (index != INVALID_INDEX)
				sampler = *pass->getSamplerState().getSampler(index);

			if( max_anisotropy != NULL )
			{
				sampler.setUsingGlobalAnisotropy(false);
				sampler.setMaxAnisotropy( (uint8)TStringHelper::toUInt(*max_anisotropy) );
			}
			else if(index == INVALID_INDEX)
				sampler.setUsingGlobalAnisotropy(true);

			if( test != NULL )
			{
				TestFuncDesc::const_iterator iter = msTestFuncDesc.find( *test );
				if( iter != msTestFuncDesc.end() )
					sampler.setTestFunc( iter->second );
				else
					ILog::Error << BTString("invalid test function: ") << *test << ILog::endLog;
			}

			//reading child: LOD, filter , address mode
			IXmlNode* pFilterNode = subNode->getChildList( MATERIAL_CONST::SAMPLER_FILTER_NODE )->safeAt(0);
			if( pFilterNode != NULL && pFilterNode->getAttributeCount() > 0 )
			{
				sampler.setUsingGlobalFilter(false);
				TString filter[3] = { MATERIAL_CONST::SAMPLER_FILTER_MAG,MATERIAL_CONST::SAMPLER_FILTER_MIN,MATERIAL_CONST::SAMPLER_FILTER_MIP};
				TEXFILER_MODE FilterMode[3] = { TFM_POINT,TFM_POINT,TFM_POINT};	//mag min mip
				for( int fi = 0; fi < 3; ++fi )
				{
					const TString* psval = pFilterNode->getAttributeValue( filter[fi] );
					if( psval != NULL )
					{
						SamplerFilterModeDesc::const_iterator sf_citer =  msSamplerFilterDesc.find( *psval );
						if( sf_citer == msSamplerFilterDesc.end() )
							BLADE_EXCEPT(EXC_NEXIST,BTString("filter mode \"") + *psval + BTString(" not found.") );
						else
							FilterMode[fi] = sf_citer->second;
					}
				}
				sampler.setMagFilter( FilterMode[0] );
				sampler.setMinFilter( FilterMode[1] );
				sampler.setMipFilter( FilterMode[2] );
				sampler.setUsingGlobalFilter(false);
			}
			else if(index == INVALID_INDEX)
				sampler.setUsingGlobalFilter(true);

			IXmlNode* pAddressModeNode = subNode->getChildList( MATERIAL_CONST::SAMPLER_ADDRESS_NODE )->safeAt(0);
			if( pAddressModeNode != NULL && pAddressModeNode->getAttributeCount() > 0)
			{
				TString address[3] = { MATERIAL_CONST::SAMPLER_ADDRESS_U,MATERIAL_CONST::SAMPLER_ADDRESS_V,MATERIAL_CONST::SAMPLER_ADDRESS_W};
				TEXADDR_MODE AddressMode[3] = {TAM_WRAP, TAM_WRAP, TAM_WRAP};
				for( int ai = 0; ai < 3; ++ai )
				{
					const TString* psval = pAddressModeNode->getAttributeValue( address[ai] );
					if( psval != NULL )
					{
						SamplerAddressModeDesc::const_iterator sa_citer =  msSamplerAddressModeDesc.find( *psval );
						if( sa_citer == msSamplerAddressModeDesc.end() )
						{
							BLADE_EXCEPT(EXC_NEXIST,BTString("address mode \"") + *psval + BTString("not found.") );
						}
						else
							AddressMode[ai] = sa_citer->second;
					}
				}
				sampler.setAdressModeU( AddressMode[0] );
				sampler.setAdressModeV( AddressMode[1] );
				sampler.setAdressModeW( AddressMode[2] );
				sampler.setUsingGlobalAddressMode(false);
			}
			else if(index == INVALID_INDEX)
				sampler.setUsingGlobalAddressMode(true);


			IXmlNode* pMipmapNode = subNode->getChildList( MATERIAL_CONST::SAMPLER_MIPMAP_NODE )->safeAt(0);
			if( pMipmapNode != NULL && pMipmapNode->getAttributeCount() > 0)
			{
				TString address[3] = { MATERIAL_CONST::SAMPLER_MIPMAP_MIN,MATERIAL_CONST::SAMPLER_MIPMAP_MAX,MATERIAL_CONST::SAMPLER_MIPMAP_MAX};
				scalar mipmap[3] = { 3, 0, 0 };
				for( int li = 0; li < 3; ++li )
				{
					const TString* psval = pAddressModeNode->getAttributeValue( address[li] );
					if( psval != NULL )
						mipmap[li] = TStringHelper::toScalar( *psval );
				}
				sampler.setMinLOD( mipmap[0] );
				sampler.setMaxLOD( mipmap[1] );
				sampler.setMipmapLODBias( mipmap[2] );
			}

			if (index != INVALID_INDEX)
				pass->getSamplerState().setSampler(index, sampler);
			else
				pass->getSamplerState().addSampler(*name, sampler);

		}//for
	}

	//////////////////////////////////////////////////////////////////////////
	void			MaterialSerializer::loadSingleTechnique(IXmlNode* pTechNode,Technique* tech)
	{
		//load sort groups
		bool bHasGroup = false;
		IXmlNode::IXmlNodeList* groupList = pTechNode->getChildList( MATERIAL_CONST::TECHNIQUE_SORTGROUP_NODE );
		if( groupList != NULL )
		{
			if( groupList->getSize() > 1 )
				BLADE_EXCEPT(EXC_REDUPLICATE,BTString("invalid material script - multi section \"groups\".") );

			IXmlNode* groupListNode = groupList->getAt(0);

			IXmlNode::IXmlNodeList* groups = groupListNode->getChildList( MATERIAL_CONST::SORTGROUP_GROUP_NODE );
			if( groups->safeSize() > 0 )
				bHasGroup = true;

			tech->prepareShaderGroups( std::max<size_t>(1, groups->safeSize()) );
			for(size_t i = 0; i < groups->safeSize(); ++i )
			{
				IXmlNode* groupNode = groups->getAt(i);
				const TString* pname = groupNode->getAttributeValue(MATERIAL_CONST::NAME);
				assert(pname != NULL);
				ShaderGroup group(*pname);

				//TODO: parsing name for debug
				IXmlNode::IXmlNodeList* requireList = groupNode->getChildList( MATERIAL_CONST::SORTGROUP_REQUIREMENT_NODE );

				if(requireList->safeSize() > 1 )
				{
					BLADE_EXCEPT(EXC_REDUPLICATE,BTString("invalid material script - multi section \"requiremens\".") );
				}

				IXmlNode* requreNode = requireList->safeAt(0);
				if( requreNode != NULL )
				{
					size_t requireCount = requreNode->getChildCount();
					for(size_t j = 0; j < requireCount; ++j)
					{
						IXmlNode* node = requreNode->getChild(j);
						const TString& name = node->getName();
						GroupRequireDesc::const_iterator iter = msGroupRequireDesc.find( name );
						if( iter == msGroupRequireDesc.end() )
						{
							ILog::Error << BTString(" invalid requirements for shader_option ") << name << ILog::endLog;
							continue;
						}

						ShaderGroup::ERequirement eReq = iter->second;
						switch( eReq )
						{
						case ShaderGroup::SR_TEXTURE:
							{
								//parsing index
								const TString* index = node->getAttributeValue( MATERIAL_CONST::SORTGROUP_REQUIREMENT_TEXTURE_INDEX );
								bool success = true;
								uint8 start = 0;
								uint8 end = 0;
								if( index != NULL )
								{
									TStringTokenizer token;
									token.tokenize(*index,TEXT("-"));
									if( token.size() == 1 )
									{
										const TString& id = token[0];
										start = end = (uint8)TStringHelper::toInt( id );
										success = true;
									}
									else if( token.size() == 2)
									{
										const TString& startString = token[0];
										const TString& endString = token[1];
										start = (uint8)TStringHelper::toInt( startString );
										end = (uint8)TStringHelper::toInt( endString );
										if( start > end )
											success = false;
										else
											success = true;
									}
								}

								if( !success )
								{
									ILog::Error << BTString(" invalid index attribute for texture requirement ") << name << ILog::endLog;
									continue;
								}
								else
								{
									group.setRequirement(eReq, ShaderGroup::SRequireValue(start,end) );
								}

							}
							break;
						case ShaderGroup::SR_ALPHABLEND:
						case ShaderGroup::SR_FOG:
							group.setRequirement(eReq, ShaderGroup::SRequireValue(true) );
							break;
						default:
							assert(false);
							break;
						}//switch
					}//for
				}//if

				tech->addShaderGroup(group);
			}
		}

		//default:add one group
		if( !bHasGroup )
		{
			ShaderGroup group(BTString("empty"));
			tech->addShaderGroup(group);
		}

		//load passes
		IXmlNode::IXmlNodeList* passList = pTechNode->getChildList( MATERIAL_CONST::PASS_NODE );
		for( size_t i = 0; i < passList->safeSize(); ++i)
		{
			IXmlNode* pPassNode = passList->safeAt(i);
			IXmlNode::IXmlNodeList* propertyNodeList = pPassNode->getChildList( MATERIAL_CONST::PROPERTY_NODE );
			size_t nStateCount = propertyNodeList->safeSize();

			const TString* pclone = pPassNode->getAttributeValue(MATERIAL_CONST::CLONE);

			if( nStateCount > 1 )
				BLADE_EXCEPT(EXC_REDUPLICATE,BTString("invalid material script - multi section \"render_property\".") );

			const TString* passName = pPassNode->getAttributeValue( MATERIAL_CONST::NAME );
			if( passName == NULL )
				BLADE_EXCEPT(EXC_INVALID_PARAM,BTString("material script - pass section without a name.") );

			Pass* pPass;
			Pass* pClonedPass = NULL;
			if (pclone != NULL)
				pPass = (pClonedPass=tech->getPass(*pclone))->clone(*passName);
			else
				pPass = BLADE_NEW Pass(*passName, tech);

			//load render property
			IXmlNode* propertyNode = propertyNodeList->safeAt(0);
			if( propertyNode != NULL && 
				( propertyNode->getAttributeCount() > 0 || propertyNode->getChildCount() > 0 ) )
			{
				this->loadRenderPropertyForPass(propertyNode, pPass);
			}

			//load samplers
			IXmlNode::IXmlNodeList* samplers = pPassNode->getChildList(MATERIAL_CONST::SAMPLERS_NODE);
			if(samplers != NULL && samplers->safeSize() > 0)
				this->loadSamplerStateForPass(samplers->safeAt(0), pPass);

			//load shaders
			IXmlNode::IXmlNodeList* shaderOptionList = pPassNode->getChildList( MATERIAL_CONST::SHADER_OPTION_NODE );
			for(size_t j = 0; j < shaderOptionList->safeSize(); ++j)
			{
				IXmlNode* optionNode = shaderOptionList->getAt(j);
				this->loadShaderForPass(optionNode, pPass, tech, j, pClonedPass);
			}
			
			tech->addPass( pPass );
		}

		//load LODs
		IXmlNode::IXmlNodeList* lodNodeList = pTechNode->getChildList( MATERIAL_CONST::PASS_LOD_NODE );
		typedef TempMap<index_t, MATERIALLOD> TempLODSort;
		TempLODSort LODList;
		for(size_t i = 0; i < lodNodeList->safeSize(); ++i)
		{
			IXmlNode* lodNode = lodNodeList->getAt(i);
			const TString* levelString = lodNode->getAttributeValue( MATERIAL_CONST::LOD_LEVEL );
			if( levelString == NULL )
			{
				ILog::Error << BTString("LOD level not set.") << ILog::endLog;
				continue;
			}
			const TString* groupString = lodNode->getAttributeValue( MATERIAL_CONST::LOD_SORTGROUP );
			if( groupString == NULL )
			{
				ILog::Error << BTString("corresponding shader option index not set.") << ILog::endLog;
				continue;
			}
			const TString* distanceStartString = lodNode->getAttributeValue( MATERIAL_CONST::LOD_START );
			if( distanceStartString == NULL )
			{
				ILog::Error << BTString("corresponding LOD distance not set.") << ILog::endLog;
				continue;
			}
			const TString* distanceEndString = lodNode->getAttributeValue( MATERIAL_CONST::LOD_END );
			if( distanceEndString == NULL )
			{
				ILog::Error << BTString("corresponding LOD distance not set.") << ILog::endLog;
				continue;
			}
			MATERIALLOD::LOD_INDEX level = (MATERIALLOD::LOD_INDEX)TStringHelper::toUInt( *levelString );
			index_t groupIndex = tech->getShaderGroupList().findGroup( *groupString ) ;
			assert(groupIndex != INVALID_INDEX);

			scalar distanceStart = TStringHelper::toScalar( *distanceStartString );
			scalar distanceEnd = *distanceEndString == MATERIAL_CONST::LOD_MAXDIST ? FLT_MAX : TStringHelper::toScalar( *distanceEndString );
			MATERIALLOD LOD;
			LOD.mSquaredDistanceStart = distanceStart*distanceStart;
			LOD.mSquaredDistanceEnd = distanceEnd*distanceEnd;
			LOD.mLODIndex = level;
			LOD.mTargetGroupIndex = (uint8)groupIndex;
			LODList[level] = LOD;
		}
		

		for(TempLODSort::iterator i = LODList.begin(); i != LODList.end(); ++i)
		{
			const MATERIALLOD& LOD = i->second;
			bool ret  = tech->addLOD(LOD.mLODIndex, LOD.mSquaredDistanceStart, LOD.mSquaredDistanceEnd, LOD.mTargetGroupIndex);
			assert( ret );
			BLADE_UNREFERENCED(ret);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void			MaterialSerializer::initialize()
	{
		if( msInited )
			return;

		msInited = true;

		msFillModeDesc[MATERIAL_CONST::FILLMODE_POINT] = GFM_POINT;
		msFillModeDesc[MATERIAL_CONST::FILLMODE_WIREFRAME] = GFM_WIREFRAME;
		msFillModeDesc[MATERIAL_CONST::FILLMODE_SOLID] = GFM_SOLID;


		msTestFuncDesc[MATERIAL_CONST::TEST_FUNC_NEVER] = TF_NEVER;
		msTestFuncDesc[MATERIAL_CONST::TEST_FUNC_LESS] = TF_LESS;
		msTestFuncDesc[MATERIAL_CONST::TEST_FUNC_EQUAL] = TF_EQUAL;
		msTestFuncDesc[MATERIAL_CONST::TEST_FUNC_LESSEQUAL] = TF_LESSEQUAL;
		msTestFuncDesc[MATERIAL_CONST::TEST_FUNC_GREATER] = TF_GREATER;
		msTestFuncDesc[MATERIAL_CONST::TEST_FUNC_NOTEQUAL] = TF_NOTEQUAL;
		msTestFuncDesc[MATERIAL_CONST::TEST_FUNC_GREATEREQUAL] = TF_GREATEREQUAL;
		msTestFuncDesc[MATERIAL_CONST::TEST_FUNC_ALWAYS] = TF_ALWAYS;

		msBlendModeDesc[MATERIAL_CONST::ALPHA_BLEND_MODE_ZERO] = BM_ZERO;
		msBlendModeDesc[MATERIAL_CONST::ALPHA_BLEND_MODE_ONE] = BM_ONE;
		msBlendModeDesc[MATERIAL_CONST::ALPHA_BLEND_MODE_SRC_COLOR] = BM_SRC_COLOR;
		msBlendModeDesc[MATERIAL_CONST::ALPHA_BLEND_MODE_INV_SRC_COLOR] = BM_INV_SRC_COLOR;
		msBlendModeDesc[MATERIAL_CONST::ALPHA_BLEND_MODE_SRC_ALPHA] = BM_SRC_ALPHA;
		msBlendModeDesc[MATERIAL_CONST::ALPHA_BLEND_MODE_INV_SRC_ALPHA] = BM_INV_SRC_ALPHA;
		msBlendModeDesc[MATERIAL_CONST::ALPHA_BLEND_MODE_DEST_ALPHA] = BM_DEST_ALPHA;
		msBlendModeDesc[MATERIAL_CONST::ALPHA_BLEND_MODE_INV_DEST_ALPHA] = BM_INV_DEST_ALPHA;
		msBlendModeDesc[MATERIAL_CONST::ALPHA_BLEND_MODE_DEST_COLOR] = BM_DEST_COLOR;
		msBlendModeDesc[MATERIAL_CONST::ALPHA_BLEND_MODE_INV_DEST_COLOR] = BM_INV_DEST_COLOR;


		msBlendOpDesc[MATERIAL_CONST::ALPHA_BLEND_OP_ADD] = BO_ADD;
		msBlendOpDesc[MATERIAL_CONST::ALPHA_BLEND_OP_SUB] = BO_SUBSTRACT;
		msBlendOpDesc[MATERIAL_CONST::ALPHA_BLEND_OP_REVERSE_SUB] = BO_REVSUBTRACT;
		msBlendOpDesc[MATERIAL_CONST::ALPHA_BLEND_OP_MIN] = BO_MIN;
		msBlendOpDesc[MATERIAL_CONST::ALPHA_BLEND_OP_MAX] = BO_MAX;


		msStencilOpDesc[MATERIAL_CONST::STENCIL_OP_KEEP] = SOP_KEEP;
		msStencilOpDesc[MATERIAL_CONST::STENCIL_OP_ZERO] = SOP_ZERO;
		msStencilOpDesc[MATERIAL_CONST::STENCIL_OP_REPLACE] = SOP_REPLACE;
		msStencilOpDesc[MATERIAL_CONST::STENCIL_OP_INC_CLAMP] = SOP_INCRESE_CLAMP;
		msStencilOpDesc[MATERIAL_CONST::STENCIL_OP_DEC_CLAMP] = SOP_DECRESE_CLAMP;
		msStencilOpDesc[MATERIAL_CONST::STENCIL_OP_INVERT] = SOP_INVERT;
		msStencilOpDesc[MATERIAL_CONST::STENCIL_OP_INC_WRAP] = SOP_INCRESE_WRAP;
		msStencilOpDesc[MATERIAL_CONST::STENCIL_OP_DEC_WRAP] = SOP_DECRESE_WRAP;

		msCullModeDesc[MATERIAL_CONST::CULLMODE_NONE] = HCM_NONE;
		msCullModeDesc[MATERIAL_CONST::CULLMODE_CCW] = HCM_CCW;
		msCullModeDesc[MATERIAL_CONST::CULLMODE_CW] = HCM_CW;

		msTextureSerializers[MATERIAL_CONST::TEXTURE_TYPE_1D] = &TextureResource::TEXTURE_1D_SERIALIZER;
		msTextureSerializers[MATERIAL_CONST::TEXTURE_TYPE_2D] = &TextureResource::TEXTURE_2D_SERIALIZER;
		msTextureSerializers[MATERIAL_CONST::TEXTURE_TYPE_3D] = &TextureResource::TEXTURE_3D_SERIALIZER;
		msTextureSerializers[MATERIAL_CONST::TEXTURE_TYPE_CUBE] = &TextureResource::TEXTURE_CUBE_SERIALIZER;

		msTextureTypeDesc[MATERIAL_CONST::TEXTURE_TYPE_1D] = TT_1DIM;
		msTextureTypeDesc[MATERIAL_CONST::TEXTURE_TYPE_2D] = TT_2DIM;
		msTextureTypeDesc[MATERIAL_CONST::TEXTURE_TYPE_3D] = TT_3DIM;
		msTextureTypeDesc[MATERIAL_CONST::TEXTURE_TYPE_CUBE] = TT_CUBE;

		msSamplerFilterDesc[MATERIAL_CONST::SAMPLER_FILTER_MODE_NONE] = TFM_NONE;
		msSamplerFilterDesc[MATERIAL_CONST::SAMPLER_FILTER_MODE_POINT] = TFM_POINT;
		msSamplerFilterDesc[MATERIAL_CONST::SAMPLER_FILTER_MODE_LINEAR] = TFM_LINEAR;
		msSamplerFilterDesc[MATERIAL_CONST::SAMPLER_FILTER_MODE_ANISOTROPY] = TFM_ANISOTROPIC;

		msSamplerAddressModeDesc[MATERIAL_CONST::SAMPLER_ADDRESSMODE_WRAP] = TAM_WRAP;
		msSamplerAddressModeDesc[MATERIAL_CONST::SAMPLER_ADDRESSMODE_MIRROR] = TAM_MIRROR;
		msSamplerAddressModeDesc[MATERIAL_CONST::SAMPLER_ADDRESSMODE_CLAMP] = TAM_CLAMP;
		//msSamplerAddressModeDesc[MATERIAL_CONST::SAMPLER_ADDRESSMODE_BORDER] = TAM_BORDER;


		msShaderTypeDesc[MATERIAL_CONST::SHADER_VARIABLE_TYPE_INT] = SCT_INT1;
		msShaderTypeDesc[MATERIAL_CONST::SHADER_VARIABLE_TYPE_BOOL] = SCT_BOOL1;
		msShaderTypeDesc[MATERIAL_CONST::SHADER_VARIABLE_TYPE_FLOAT1] = SCT_FLOAT1;
		msShaderTypeDesc[MATERIAL_CONST::SHADER_VARIABLE_TYPE_FLOAT2] = SCT_FLOAT2;
		msShaderTypeDesc[MATERIAL_CONST::SHADER_VARIABLE_TYPE_FLOAT3] = SCT_FLOAT3;
		msShaderTypeDesc[MATERIAL_CONST::SHADER_VARIABLE_TYPE_FLOAT4] = SCT_FLOAT4;


		msGroupRequireDesc[MATERIAL_CONST::SORTGROUP_REQUIREMENT_TEXTURE] = ShaderGroup::SR_TEXTURE;
		msGroupRequireDesc[MATERIAL_CONST::SORTGROUP_REQUIREMENT_FOG] = ShaderGroup::SR_FOG;
		msGroupRequireDesc[MATERIAL_CONST::SORTGROUP_REQUIREMENT_ALPHABLEND] = ShaderGroup::SR_ALPHABLEND;


		msSwitchDesc[MATERIAL_CONST::SWITCH_ENABLE] = true;
		msSwitchDesc[MATERIAL_CONST::SWITCH_DISABLE] = false;
		//msSwitchDesc[BTString("true")] = true;
		//msSwitchDesc[BTString("false")] = false;

		msPropertyDesc[MATERIAL_CONST::PROPERTY_COLOR_NODE] = RPT_COLOR;
		msPropertyDesc[MATERIAL_CONST::PROPERTY_COLOR_WRITE_NODE] = RPT_COLORWIRTE;
		msPropertyDesc[MATERIAL_CONST::PROPERTY_FOG_NODE] = RPT_FOG;
		msPropertyDesc[MATERIAL_CONST::PROPERTY_ALPHA_BLEND_NODE] = RPT_ALPHABLEND;
		msPropertyDesc[MATERIAL_CONST::PROPERTY_DEPTH_NODE] = RPT_DEPTH;
		msPropertyDesc[MATERIAL_CONST::PROPERTY_STENCIL_NODE] = RPT_STENCIL;
		msPropertyDesc[MATERIAL_CONST::PROPERTY_SCISSOR_NODE] = RPT_SCISSOR;
		//msPropertyDesc[MATERIAL_CONST::SAMPLERS_NODE] = RPT_SAMPLER;

		//msTexturePropertyDesc[BTString("custom")] = TPT_CUSTOM;
		//msTexturePropertyDesc[BTString("diffuse")] = TPT_DIFFUSEMAP;
		//msTexturePropertyDesc[BTString("normal")] = TPT_NORMALMAP;
		//msTexturePropertyDesc[BTString("detail")] = TPT_DETAILMAP;
		//msTexturePropertyDesc[BTString("gloss")] = TPT_GLOSSMAP;
		//msTexturePropertyDesc[BTString("glow")] = TPT_GLOWMAP;
		//msTexturePropertyDesc[BTString("dark")] = TPT_DARKMAP;
		//msTexturePropertyDesc[BTString("light")] = TPT_LIGHTMAP;
	}

	
}//namespace Blade