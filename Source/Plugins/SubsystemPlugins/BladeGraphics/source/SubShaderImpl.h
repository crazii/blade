/********************************************************************
	created:	2014/12/25
	filename: 	SubShaderImpl.h
	author:		Crazii
	
	purpose:	pimpl implementations shared internally
*********************************************************************/
#ifndef __Blade_SubShaderImpl_h__
#define __Blade_SubShaderImpl_h__
#include <ShaderResource.h>
#include <utility/IOBuffer.h>
#include <utility/BladeContainer.h>
#include <interface/public/graphics/IRenderDevice.h>
#include <interface/public/graphics/ShaderInfoTable.h>
#include <interface/public/graphics/IGraphicsResourceManager.h>
#include <TextureState.h>
#include <TextureStage.h>
#include "TextureShaderHelper.h"

namespace Blade
{

	namespace Impl
	{
		/************************************************************************/
		/*                                                                      */
		/************************************************************************/
		//////////////////////////////////////////////////////////////////////////
		struct ShaderVariableEntry
		{
			AutoShaderVariable*		variable;
			IShader::CONSTANT_KEY	key;

			ShaderVariableEntry(AutoShaderVariable* var, const IShader::CONSTANT_INFO* info)
				:variable(var)
				,key(*info)
			{

			}

			bool operator<(const ShaderVariableEntry& rhs) const
			{
				return key < rhs.key;
			}
		};

		class SubShaderVaraibleDataImpl : public Allocatable
		{
		public:
			typedef Vector<ShaderVariableEntry>	VariableList;
			SubShaderVaraibleDataImpl()
			{
				mFloatVars.reserve(8);
				mIntVars.reserve(8);
				mBoolVars.reserve(8);
				mSamplerVars.reserve(8);
			}

			/** @brief  */
			bool	hasVariable(const IShader::CONSTANT_INFO* info) const
			{
				ShaderVariableEntry entry(NULL, info);

				if( info->mType.getAtomType() == SCTF_FLOAT )
					return std::binary_search(mFloatVars.begin(), mFloatVars.end(), entry);
				else if( info->mType.getAtomType() == SCTF_INT )
					return std::binary_search(mIntVars.begin(), mIntVars.end(), entry);
				else if( info->mType.getAtomType() == SCTF_BOOL )
					return std::binary_search(mBoolVars.begin(), mBoolVars.end(), entry);
				else if( info->mType.getAtomType() == SCTF_SAMPLER )
					return std::binary_search(mSamplerVars.begin(), mSamplerVars.end(), entry);
				else
				{
					assert(false);
					return false;
				}
			}

			/** @brief  */
			bool	addVariable(const IShader::CONSTANT_INFO* info, AutoShaderVariable* variable)
			{
				if( variable == NULL )
				{
					assert(false);
					return false;
				}

				ShaderVariableEntry entry(variable, info);

				if( variable->getType().getAtomType() == SCTF_FLOAT )
				{
					mFloatVars.push_back(entry);
					//some float4x4 actually used as float3x3, and takes 3 registers
					//but we update 4, that will overwrite successive constants if successive constants updated first
					//sort constants by key (register index) so that successive constant will update after previous, may avoid this problem
					//but if variables are not in the same usage (SVU_GLOBAL/SVU_INSTANCE), it will still overwrite previous constants been set.
					//solution: fix in graphics resource manager to use the right register count.
					//PROBLEM FIXED.
					std::sort(mFloatVars.begin(), mFloatVars.end());
				}
				else if( variable->getType().getAtomType() == SCTF_INT )
				{
					mIntVars.push_back(entry);
					std::sort(mIntVars.begin(), mIntVars.end());
				}
				else if( variable->getType().getAtomType() == SCTF_BOOL )
				{
					mBoolVars.push_back(entry);
					std::sort(mBoolVars.begin(), mBoolVars.end());
				}
				else if( variable->getType().getAtomType() == SCTF_SAMPLER )
				{
					mSamplerVars.push_back(entry);
					std::sort(mSamplerVars.begin(), mSamplerVars.end());
				}
				else
				{
					assert(false);
					return false;
				}
				return true;
			}
			
			VariableList mFloatVars;
			VariableList mIntVars;
			VariableList mBoolVars;
			VariableList mSamplerVars;
		};


		/************************************************************************/
		/*                                                                      */
		/************************************************************************/
		//////////////////////////////////////////////////////////////////////////
		struct CustomShaderVariableEntry
		{
			HAUTOSHADERVARIABLE hVariable;
			IShader::CONSTANT_KEY key;
			const IShader::CONSTANT_INFO* info;

			CustomShaderVariableEntry(HAUTOSHADERVARIABLE var, const IShader::CONSTANT_INFO* _info)
				:hVariable(var)
				,key(*_info)
				,info(_info)
			{

			}


			bool operator<(const CustomShaderVariableEntry& rhs) const
			{
				return key < rhs.key;
			}
		};

		class SubShaderCustomVaraibleDataImpl : public Allocatable
		{
		public:
			typedef Vector<CustomShaderVariableEntry> VariableList;
			SubShaderCustomVaraibleDataImpl()
			{
				mFloatVars.reserve(8);
				mIntVars.reserve(8);
				mBoolVars.reserve(8);
			}

			/** @brief  */
			bool	hasVariable(const IShader::CONSTANT_INFO* info) const
			{
				CustomShaderVariableEntry entry(HAUTOSHADERVARIABLE::EMPTY, info);

				if( info->mType.getAtomType() == SCTF_FLOAT )
					return std::binary_search(mFloatVars.begin(), mFloatVars.end(), entry);
				if( info->mType.getAtomType() == SCTF_INT )
					return std::binary_search(mIntVars.begin(), mIntVars.end(), entry);
				if( info->mType.getAtomType() == SCTF_BOOL )
					return std::binary_search(mBoolVars.begin(), mBoolVars.end(), entry);
				assert(false);
				return false;
			}

			/** @brief  */
			AutoShaderVariable*	findVariable(const IShader::CONSTANT_INFO* info) const
			{
				CustomShaderVariableEntry entry(HAUTOSHADERVARIABLE::EMPTY, info);

				if (info->mType.getAtomType() == SCTF_FLOAT)
				{
					VariableList::const_iterator i = std::lower_bound(mFloatVars.begin(), mFloatVars.end(), entry);
					if (!(entry < *i))
						return i->hVariable;
					return NULL;
				}
				if (info->mType.getAtomType() == SCTF_INT)
				{
					VariableList::const_iterator i = std::lower_bound(mIntVars.begin(), mIntVars.end(), entry);
					if (!(entry < *i))
						return i->hVariable;
					return NULL;
				}
				if (info->mType.getAtomType() == SCTF_BOOL)
				{
					VariableList::const_iterator i = std::lower_bound(mBoolVars.begin(), mBoolVars.end(), entry);
					if (!(entry < *i))
						return i->hVariable;
					return NULL;
				}
				assert(false);
				return NULL;
			}

			/** @brief  */
			bool	addVariable(const IShader::CONSTANT_INFO* info, const HAUTOSHADERVARIABLE& variable)
			{
				if( variable == NULL )
				{
					assert(false);
					return false;
				}

				CustomShaderVariableEntry entry(variable, info);

				if( info->mType.getAtomType() == SCTF_FLOAT )
				{
					mFloatVars.push_back(entry);
					std::sort(mFloatVars.begin(), mFloatVars.end());
				}
				else if( info->mType.getAtomType() == SCTF_INT )
				{
					mIntVars.push_back(entry);
					std::sort(mIntVars.begin(), mIntVars.end());
				}
				else if( info->mType.getAtomType() == SCTF_BOOL )
				{
					mBoolVars.push_back(entry);
					std::sort(mBoolVars.begin(), mBoolVars.end());
				}
				else
				{
					assert(false);
					return false;
				}
				return true;
			}

			VariableList mFloatVars;
			VariableList mIntVars;
			VariableList mBoolVars;
		};


		/************************************************************************/
		/*                                                                      */
		/************************************************************************/
		//////////////////////////////////////////////////////////////////////////
		class SubShaderSemanticDataImpl : public Allocatable
		{
		public:
			typedef TStringMap<TString> SemanticMap;
			typedef TStringMap<EShaderVariableUsage> CustomSemanticMap;

			/** @brief write binary semantics data */
			inline void writeSemantics(IOBuffer& buffer) const
			{
				uint32 count = (uint32)mSemantics.size();
				buffer.write(&count);
				for( SemanticMap::const_iterator i = mSemantics.begin(); i != mSemantics.end(); ++i)
				{
					const TString& name = i->first;
					const TString& semantic = i->second;
					buffer.writeTString(name);
					buffer.writeTString(semantic);
				}
			}

			/** @brief  */
			static bool readCount(IOBuffer& buffer, uint32& count)
			{
				return buffer.read(&count);
			}

			/** @brief  */
			static bool	readSemantic(IOBuffer& buffer, TString& name, TString& semantic)
			{
				return buffer.readTString(name) && buffer.readTString(semantic);
			}


			/** @brief write binary custom semantics data */
			template<typename USAGE_TYPE>
			inline void writeCustomSemantics(IOBuffer& buffer) const
			{
				uint32 count = (uint32)mCusomSmantics.size();
				buffer.write(&count);
				for( CustomSemanticMap::const_iterator i = mCusomSmantics.begin(); i != mCusomSmantics.end(); ++i)
				{
					const TString& name = i->first;
					USAGE_TYPE usage = (USAGE_TYPE)i->second;
					buffer.writeTString(name);
					buffer.write(&usage);
				}
			}

			/** @brief  */
			template<typename USAGE_TYPE>
			static inline bool	readCustomSemantic(IOBuffer& buffer, TString& name, USAGE_TYPE& type)
			{
				bool ret = buffer.readTString(name);
				if( ret )
					ret = buffer.read(&type);
				return ret;
			}

			/** @brief  */
			inline bool addSemantic(const TString& senamtic, const TString& variable)
			{
				std::pair<SemanticMap::iterator, bool> ret = mSemantics.insert( std::make_pair(variable, senamtic) );
				assert( ret.second );
				return ret.second;
			}

			/** @brief  */
			inline bool addCustomSemantic(const TString& customSemantic, EShaderVariableUsage usage)
			{
				std::pair<CustomSemanticMap::iterator, bool> ret = mCusomSmantics.insert( std::make_pair(customSemantic, usage) );
				assert( ret.second );
				return ret.second;
			}
		protected:
			SemanticMap			mSemantics;	//built-ins
			CustomSemanticMap	mCusomSmantics;

			friend class Blade::SubShader;
			friend class SubShaderSamplerDataImpl;
		};


		/************************************************************************/
		/*                                                                      */
		/************************************************************************/
		static const TString SAMPLER_SIZE_SUFFIX = BTString("_Size");
		static const TString SAMPLER_UVSPACE_SUFFIX = BTString("_UVSpace");

		class SubShaderSamplerDataImpl : public Allocatable
		{
		public:
			typedef struct SSamplerData
			{
				uint16	samplerIndex;	//real sampler index in shader
				uint8	stageIndex;
				uint8	samplerPropIndex;
				IShader::CONSTANT_KEY sizeKey;
				IShader::CONSTANT_KEY uvSpaceKey;
			}SAMPLER_DATA;

			SubShaderSamplerDataImpl()	{}
			~SubShaderSamplerDataImpl()	{}

			/** @brief  */
			bool		linkSampler(const IShader::INFO_TABLE& table, TStringList& uniforms, const SamplerState& samplers, const LazyPimpl<SubShaderSemanticDataImpl>* semantics)
			{
				typedef TempVector<SAMPLER_DATA> TempSamplerList;
				TempSamplerList samplerList[SHADER_TYPE_COUNT];

				for(size_t i = 0; i < table.getCount(); ++i)
				{
					const IShader::CONSTANT_INFO* info = table[i];
					if( info->mType != SCT_SAMPLER )
						continue;

					for (size_t j = 0; j < info->mRegisterCount; ++j)
					{
						TString suffix = info->mRegisterCount == 1 ? TStringConcat(TString::EMPTY) : BTString("[") + TStringHelper::fromUInt(j) + BTString("]");
						TString samplerUniform = info->mName + suffix;
						index_t index = uniforms.find(samplerUniform);

						//skip built-in shader variables for render texture (RENDERBUFFER_INPUTn, IMAGEFX_INPUT)
						//since they're updated in shader variable update/commit routine and not bound to any texture stages
						bool builtin = false;
						for (int k = SHADER_TYPE_BEGIN; k < SHADER_TYPE_COUNT; ++k)
						{
							if (semantics[k]->mSemantics.find(info->mName) != semantics[k]->mSemantics.end()
								|| semantics[k]->mSemantics.find(samplerUniform) != semantics[k]->mSemantics.end())
								builtin = true;
						}

						//texture uniforms in material config is deprecated and ought to be empty
						//add (missing/no-existing) uniforms here
						//gather full uniform to list
						if (index == INVALID_INDEX)
						{
							if (!builtin)
							{
								uniforms.push_back(samplerUniform);
								index = uniforms.size() - 1;
							}
						}
						else if (builtin)
							uniforms.erase(index);

						if(builtin)
							continue;

						index_t samplerPropIndex = INVALID_INDEX;
						samplerPropIndex = samplers.getSamplerIndex(samplerUniform);

						IShader::CONSTANT_KEY sizeKey;
						const IShader::CONSTANT_INFO* sizeInfo = table.getConstantInfoByName(info->mShaderType, info->mName + SAMPLER_SIZE_SUFFIX + suffix);
						if (sizeInfo != NULL && sizeInfo->mType.getAtomType() == SCT_FLOAT1 && sizeInfo->mBytes >= sizeof(fp32) * 2 && sizeInfo->mBytes <= sizeof(fp32) * 4)
							sizeKey = *sizeInfo;
						IShader::CONSTANT_KEY uvSpaceKey;
						const IShader::CONSTANT_INFO* uvSpaceInfo = table.getConstantInfoByName(info->mShaderType, info->mName + SAMPLER_UVSPACE_SUFFIX + suffix);
						if (uvSpaceInfo != NULL && uvSpaceInfo->mType.getAtomType() == SCT_FLOAT1 && uvSpaceInfo->mBytes == sizeof(fp32) * 4)
							uvSpaceKey = *uvSpaceInfo;

						SAMPLER_DATA data = { info->mIndex, (uint8)index, (uint8)samplerPropIndex, sizeKey, uvSpaceKey };
						samplerList[info->mShaderType].push_back(data);
					}
				}

				for(int i = SHADER_TYPE_BEGIN; i < SHADER_TYPE_COUNT; ++i)
				{
					mSamplers[i].clear();
					mSamplers[i].reserve( samplerList[i].size() );
					mSamplers[i].insert(mSamplers[i].end(), samplerList[i].begin(), samplerList[i].end() );
				}
				return true;
			}

			/** @brief  */
			void	updateTexture(IRenderDevice* renderer, const TextureStateInstance stateInstance, const SamplerState& samplers) const
			{
				const TextureState* state = stateInstance.state;
				for(int i = SHADER_TYPE_BEGIN; i < SHADER_TYPE_COUNT; ++i)
				{
					EShaderType type = EShaderType(i);
					for(size_t j = 0; j < mSamplers[i].size(); ++j)
					{
						const SAMPLER_DATA& data = mSamplers[i][j];
						if( data.stageIndex == uint8(-1) )
							renderer->setTexture(type, (index_t)data.samplerIndex, HTEXTURE::EMPTY);
						else if (data.stageIndex < state->getTextureStageCount())
						{
							//TODO: some mesh may have invalid texture(i.e. diffuse texture not set in modeling tool)
							//need to bind a blank texture. i.e. purple
							//add some built in textures for graphics system.
							//assert( state->getTextureStageCount() != 0 );
							TextureStage* stage = state->getTextureStage(data.stageIndex);
							if (!stage->isLoaded() || !stage->isEnabled() || !stateInstance.stateMask.checkBitAtIndex(data.stageIndex) )
							{
#if BLADE_DEBUG
								renderer->setTexture(type, (index_t)data.samplerIndex, HTEXTURE::EMPTY);
#endif
								continue;
							}

							const HSAMPLER& sampler = samplers.getSampler(data.samplerPropIndex);
							const HTEXTURE& hTex = stage->getTexture();
							renderer->setTexture(type, (index_t)data.samplerIndex, hTex, sampler);

							bool updateSize = (data.sizeKey.mShaderType == i && data.sizeKey.mType != SCT_UNDEFINED);
							bool updateUVSpace = (data.uvSpaceKey.mShaderType == i && data.uvSpaceKey.mType != SCT_UNDEFINED);
							if (hTex != NULL && (updateSize || updateUVSpace) )
							{
								const scalar* size = stage->getTextureSize();
								if(updateSize)
									renderer->setShaderConstant(data.sizeKey, sizeof(scalar)*4, size);
								if (updateUVSpace)
								{
									Vector4 uvSpace[Impl::UVSPACE_DATA_COUNT] = {Vector4::ZERO, Vector4::ZERO, Vector4::ZERO, Vector4::ZERO };
									Impl::updateUVSpace(uvSpace, hTex);
									renderer->setShaderConstant(data.uvSpaceKey, sizeof(uvSpace), (const float*)&uvSpace);
								}
							}
						}
					}//for each sampler
				}//for each shader type
			}

		protected:
			typedef Vector<SAMPLER_DATA> SamplerList;

			SamplerList mSamplers[SHADER_TYPE_COUNT];
			friend class Blade::SubShader;
		};//SubShaderSamplerDataImpl

	}//namespace Impl
	
}//namespace Blade


#endif // __Blade_SubShaderImpl_h__