/********************************************************************
	created:	2010/09/02
	filename: 	ShaderResource.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <ShaderResource.h>
#include <interface/public/graphics/ShaderInfoTable.h>
#include <interface/IShaderVariableSource.h>
#include <interface/IRenderQueue.h>
#include <MaterialInstance.h>
#include "SubShaderImpl.h"

namespace Blade
{
	const TString ShaderResource::SHADER_RESOURCE_TYPE = BTString("Shader");
	const TString ShaderResource::BINARY_SHADER_SERIALIZER = BTString("BinaryShader");
	const TString ShaderResource::TEXT_SHADER_SERIALIZER = BTString("SourceShader");
	const TString ShaderResource::HYBRID_SHADER_SERIALIZER = BTString("HybridShader");

	namespace Impl
	{
		/** @brief  */
		static inline AutoShaderVariable* createCustomVariable(InstanceVariableMap& map, const TString& name, SHADER_CONSTANT_TYPE type, size_t count)
		{
			index_t exist = map.getStorageIndex(name);
			if (exist == INVALID_INDEX)
			{
				exist = map.size();
				map.createStorage(name, type, count);
			}
			//WARNING: unsafe cast, be careful! don't call update
			return BLADE_NEW AutoShaderVariable((AutoShaderVariable::IUpdater*)(uintptr_t)exist, type, count, SVU_INSTANCE_CUSTOM);
		}

		static inline bool createCustomVariable(AutoShaderVariable* var, InstanceVariableMap& map, const TString& name, SHADER_CONSTANT_TYPE type, size_t count)
		{
			index_t exist = map.getStorageIndex(name);
			if (exist == INVALID_INDEX)
			{
				exist = map.size();
				map.createStorage(name, type, count);
			}
			index_t index = (index_t)(uintptr_t)var->getUpdater();
			//material instances should create custom instance variables in fixed order
			//and indices in instances MUST be the same as those in shader instance
			assert(index == exist);
			if (index != exist)
				BLADE_EXCEPT(EXCE_STATE_ERROR, BTString("corrupted order of custom instance shader."));
			return index == exist;
		}

		/** @brief  */
		static inline const void* getCustomVariableData(AutoShaderVariable* var)
		{
			assert(var != NULL);
			const RenderOperation* rop = IShaderVariableSource::getSingleton().getRenderOperation();
			if (rop != NULL && rop->material != NULL)
				return rop->material->getShaderVariables().getStorage((index_t)(uintptr_t)var->getUpdater());
			assert(false);
			return NULL;
		}
	};

	 /************************************************************************/
	 /*                                                                      */
	 /************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	SubShader::SubShader()
	{
		mInstanceVariableCount = INVALID_INDEX;
	}

	//////////////////////////////////////////////////////////////////////////
	SubShader::~SubShader()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	void				SubShader::updateBultInVariable(EShaderVariableUsage usage, IRenderDevice* renderer) const
	{
		if( usage < SVU_COUNT && usage >= 0)
			this->updateShaderVariable(mBuiltInData[usage], renderer);
		else
		{
			assert(false);
			BLADE_EXCEPT(EXC_OUT_OF_RANGE,BTString("invalid usage"));
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void				SubShader::updateCustomVariable(EShaderVariableUsage usage, IRenderDevice* renderer) const
	{
		usage = EShaderVariableUsage( usage & SVU_TYPE_MASK );
		if( usage < SVU_COUNT && usage >= 0)
		{
			if( usage == SVU_INSTANCE )
				this->updateCustomInstanceShaderVariable(mCustomData[usage], renderer);
			else
				this->updateCustomShaderVariable(mCustomData[usage], renderer);
		}
		else
		{
			assert(false);
			BLADE_EXCEPT(EXC_OUT_OF_RANGE,BTString("invalid usage"));
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void				SubShader::createAndLinkCustomPassVariable(PassVariableMap& variableData)
	{
		using namespace Impl;

		for(int i = SHADER_TYPE_BEGIN; i < SHADER_TYPE_COUNT; ++i)
		{
			EShaderType type = EShaderType(i);
			const SubShaderSemanticDataImpl::CustomSemanticMap& customSemantics = mSemantics[i]->mCusomSmantics;
			const IShader::INFO_TABLE& infoTable = this->getShader()->getConstantTable();

			for( SubShaderSemanticDataImpl::CustomSemanticMap::const_iterator iter = customSemantics.begin(); iter != customSemantics.end(); ++iter)
			{
				const TString& name = iter->first;
				EShaderVariableUsage _usage = iter->second;
				if( _usage == SVU_PASS_CUSTOM )
				{
					assert( variableData.getVariable(name) == NULL );
					const IShader::CONSTANT_INFO* info = infoTable.getConstantInfoByName(type, name);
					assert( info != NULL );
					assert( !this->hasVariable(mCustomData[SVU_PASS], info) );
					HAUTOSHADERVARIABLE hAsv = variableData.createVariable(name, (EShaderConstantType)info->mType, info->mElementCount);

					assert( hAsv != NULL && hAsv->isCustom() && hAsv->getType().isCompatible( (EShaderConstantType)info->mType )
						&& info->mElementCount == hAsv->getCount() );

					mCustomData[SVU_PASS]->addVariable(info, hAsv);
				}
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void				SubShader::createAndLinkCustomInstanceVariable(InstanceVariableMap& variableData)
	{
		using namespace Impl;

		for(int i = SHADER_TYPE_BEGIN; i < SHADER_TYPE_COUNT; ++i)
		{
			EShaderType type = EShaderType(i);
			const SubShaderSemanticDataImpl::CustomSemanticMap& customSemantics = mSemantics[i]->mCusomSmantics;
			const IShader::INFO_TABLE& infoTable = this->getShader()->getConstantTable();

			for( SubShaderSemanticDataImpl::CustomSemanticMap::const_iterator iter = customSemantics.begin(); iter != customSemantics.end(); ++iter)
			{
				const TString& name = iter->first;
				EShaderVariableUsage _usage = iter->second;
				if( _usage == SVU_INSTANCE_CUSTOM )
				{
					const IShader::CONSTANT_INFO* info = infoTable.getConstantInfoByName(type, name);
					assert( info != NULL );
					//vertex/fragment shader may use the same variable
					AutoShaderVariable* var = mCustomData[SVU_INSTANCE] != NULL ? mCustomData[SVU_INSTANCE]->findVariable(info) : NULL;
					if (var == NULL)
					{
						assert(variableData.getStorage(name, info->mType, info->mElementCount) == NULL);
						var = Impl::createCustomVariable(variableData, info->mName, info->mType, info->mElementCount);
						mCustomData[SVU_INSTANCE]->addVariable(info, HAUTOSHADERVARIABLE(var));
					}
					else
						Impl::createCustomVariable(var, variableData, info->mName, info->mType, info->mElementCount);
				}
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	size_t				SubShader::getInstanceVariableCount()
	{
		if( mInstanceVariableCount != INVALID_INDEX )
			return mInstanceVariableCount;

		using namespace Impl;
		
		mInstanceVariableCount = 0;
		for(int i = SHADER_TYPE_BEGIN; i < SHADER_TYPE_COUNT; ++i)
		{
			EShaderType type = EShaderType(i);
			const SubShaderSemanticDataImpl::CustomSemanticMap& customSemantics = mSemantics[i]->mCusomSmantics;
			const IShader::INFO_TABLE& infoTable = this->getShader()->getConstantTable();

			for( SubShaderSemanticDataImpl::CustomSemanticMap::const_iterator iter = customSemantics.begin(); iter != customSemantics.end(); ++iter)
			{
				const TString& name = iter->first;
				EShaderVariableUsage _usage = iter->second;
				if( _usage == SVU_INSTANCE_CUSTOM )
				{
					const IShader::CONSTANT_INFO* info = infoTable.getConstantInfoByName(type, name);
					if( info != NULL )
						++mInstanceVariableCount;
					else
						assert(false);
				}
			}
		}

		return mInstanceVariableCount;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				SubShader::linkShaderVariable(EShaderType type, const TString& name, const TString& semantic)
	{
		if( mShader == NULL )
			return false;
		const IShader::INFO_TABLE& table = this->getShader()->getConstantTable();
		const IShader::CONSTANT_INFO* info = table.getConstantInfoByName(type, name);

		if(info == NULL && type != SHADER_ANY )
			info = table.getConstantInfoByName(SHADER_ANY, name);

		if( info == NULL || (info->mShaderType != SHADER_ANY && info->mShaderType != type) )
			return false;

		//already linked
		for(int i = 0; i < SVU_COUNT; ++i)
		{
			if( this->hasVariable(mBuiltInData[i], info) )
				return false;
		}

		AutoShaderVariable* asv = IShaderVariableSource::getSingleton().getBuiltInVariable(semantic);
		EShaderVariableUsage usage = asv->getUsage();
		if( !(usage >= 0 && usage < SVU_COUNT ) || asv->isCustom() )
		{
			assert(false);
			return false;
		}

		if( !asv->getType().isCompatible( (EShaderConstantType)info->mType) || info->mElementCount > asv->getCount() )
		{
			BLADE_LOG(Error, BTString("semantic data type mismatch - ") << name << BTString(" : ") << semantic
				<< BTString(" valid type is: ") << ShaderConstantType::getConstanName(asv->getType(), 0)
				<< (asv->getCount() > 1 ? BTString(" to ") + ShaderConstantType::getConstanName(asv->getType(), asv->getCount()) : TEXT("")) );
			return false;
		}
		return mBuiltInData[usage]->addVariable(info, asv);
	}

	//////////////////////////////////////////////////////////////////////////
	bool				SubShader::linkCustomShaderVariable(EShaderType type, const TString& name, EShaderConstantType ctype, size_t count,
		EShaderVariableUsage usage)
	{
		if( mShader == NULL )
			return false;

		const IShader::INFO_TABLE& table = this->getShader()->getConstantTable();
		const IShader::CONSTANT_INFO* info = table.getConstantInfoByName(type, name);

		if(info == NULL && type != SHADER_ANY )
			info = table.getConstantInfoByName(SHADER_ANY, name);

		if( info == NULL || (info->mShaderType != SHADER_ANY && info->mShaderType != type && !info->mType.isCompatible(ctype) ) )
		{
			BLADE_LOG(Warning, BTString("custom variable not found: ") << name);
			return false;
		}
		if( info->mType == SCTF_SAMPLER )
			return false;

		//already linked
		for(int i = 0; i < SVU_COUNT; ++i)
		{
			if( this->hasVariable(mCustomData[i], info) )
				return false;
		}

		int scope_type = usage&SVU_TYPE_MASK;

		HAUTOSHADERVARIABLE hAsv;
		if( scope_type == SVU_GLOBAL )
			hAsv = IShaderVariableSource::getSingleton().createGlobalCustomVariable(name, ctype,count);
		else if( scope_type == SVU_PASS )
			return true;	//postpone
		else if( scope_type == SVU_SHADER ) //per shader variables, get from me
		{
			if( mCustomShaderVariables.getVariable(name) == NULL )
				mCustomShaderVariables.createVariable(name, ctype, count);
			hAsv = mCustomShaderVariables.getVariable(name);
		}
		else if( scope_type == SVU_INSTANCE )
			return true;	//postpone
		else
		{
			assert(false);
			return false;
		}

		if(!hAsv->isCustom() || (hAsv->getUsage()&SVU_TYPE_MASK) != scope_type)
		{
			assert(false);
			return false;
		}

		if( info->mElementCount != hAsv->getCount() || !hAsv->getType().isCompatible( (EShaderConstantType)info->mType) )
		{
			assert(false);
			return false;
		}
		return mCustomData[scope_type]->addVariable(info, hAsv);
	}

	//////////////////////////////////////////////////////////////////////////
	void				SubShader::linkInternalShaderVariable(EShaderType type, const TString& name,const TString& semantic)
	{
		if( this->linkShaderVariable(type, name, semantic) )
			mSemantics[type]->addSemantic(semantic, name);
	}

	//////////////////////////////////////////////////////////////////////////
	void				SubShader::linkInternalCustomVariable(EShaderType type, const TString& name, EShaderConstantType ctype, size_t count,
		EShaderVariableUsage usage)
	{
		if( this->linkCustomShaderVariable(type, name, ctype, count, usage) )
			mSemantics[type]->addCustomSemantic(name, usage);
	}

	//////////////////////////////////////////////////////////////////////////
	void				SubShader::setShader(const HSHADER& shader)
	{
		mShader = shader;
	}

	//////////////////////////////////////////////////////////////////////////
	void				SubShader::setName(const TString& name)
	{
		mName = name;
	}

	//////////////////////////////////////////////////////////////////////////
	void				SubShader::updateShaderVariable(const LazyPimpl<Impl::SubShaderVaraibleDataImpl>& data, IRenderDevice* renderer) const
	{
		const HSHADER& shader = this->getShader();
		if( shader == NULL || data == NULL )
			return;

		for(size_t i = 0; i < data->mFloatVars.size(); ++i)
		{
			const Impl::ShaderVariableEntry& entry = data->mFloatVars[i];
			if( entry.variable->needCommit() )
				renderer->setShaderConstant(entry.key, entry.variable->getRealSize(), (const float*)entry.variable->getData() );
		}

		for(size_t i = 0; i < data->mIntVars.size(); ++i)
		{
			const Impl::ShaderVariableEntry& entry = data->mIntVars[i];
			if( entry.variable->needCommit() )
				renderer->setShaderConstant(entry.key, entry.variable->getRealSize(), (const int*)entry.variable->getData() );
		}

		for(size_t i = 0; i < data->mBoolVars.size(); ++i)
		{
			const Impl::ShaderVariableEntry& entry = data->mBoolVars[i];
			if( entry.variable->needCommit() )
				renderer->setShaderConstant(entry.key, entry.variable->getRealSize(), (const SC_BOOL*)entry.variable->getData() );
		}

		for(size_t i = 0; i < data->mSamplerVars.size(); ++i)
		{
			const Impl::ShaderVariableEntry& entry = data->mSamplerVars[i];
			const HTEXTURE* texture = (const HTEXTURE*)entry.variable->getData();
			const Sampler* sampler = (*texture) != NULL ? &(*texture)->getSampler() : NULL;
			renderer->setTexture(entry.key.mShaderType, entry.key.mIndex, *texture, sampler);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void				SubShader::updateCustomShaderVariable(const LazyPimpl<Impl::SubShaderCustomVaraibleDataImpl>& data, IRenderDevice* renderer) const
	{
		const HSHADER& shader = this->getShader();
		if( shader == NULL || data == NULL )
			return;

		//////////////////////////////////////////////////////////////////////////
		//update custom data
		for(size_t i = 0; i < data->mFloatVars.size(); ++i)
		{
			const Impl::CustomShaderVariableEntry& entry = data->mFloatVars[i];
			renderer->setShaderConstant(entry.key, entry.hVariable->getRealSize(), (const float*)entry.hVariable->getData() );
		}

		for(size_t i = 0; i < data->mIntVars.size(); ++i)
		{
			const Impl::CustomShaderVariableEntry& entry = data->mIntVars[i];
			renderer->setShaderConstant(entry.key, entry.hVariable->getRealSize(), (const int*)entry.hVariable->getData() );
		}

		for(size_t i = 0; i < data->mBoolVars.size(); ++i)
		{
			const Impl::CustomShaderVariableEntry& entry = data->mBoolVars[i];
			renderer->setShaderConstant(entry.key, entry.hVariable->getRealSize(), (const SC_BOOL*)entry.hVariable->getData() );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void				SubShader::updateCustomInstanceShaderVariable(const LazyPimpl<Impl::SubShaderCustomVaraibleDataImpl>& data, IRenderDevice* renderer) const
	{
		const HSHADER& shader = this->getShader();
		if( shader == NULL || data == NULL )
			return;

		//////////////////////////////////////////////////////////////////////////
		//update custom data
		for(size_t i = 0; i < data->mFloatVars.size(); ++i)
		{
			const Impl::CustomShaderVariableEntry& entry = data->mFloatVars[i];
			renderer->setShaderConstant(entry.key, entry.hVariable->getRealSize(), (const float*)Impl::getCustomVariableData(entry.hVariable));
		}

		for(size_t i = 0; i < data->mIntVars.size(); ++i)
		{
			const Impl::CustomShaderVariableEntry& entry = data->mIntVars[i];
			renderer->setShaderConstant(entry.key, entry.hVariable->getRealSize(), (const int*)Impl::getCustomVariableData(entry.hVariable));
		}

		for(size_t i = 0; i < data->mBoolVars.size(); ++i)
		{
			const Impl::CustomShaderVariableEntry& entry = data->mBoolVars[i];
			renderer->setShaderConstant(entry.key, entry.hVariable->getRealSize(), (const SC_BOOL*)Impl::getCustomVariableData(entry.hVariable));
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool				SubShader::hasVariable(const LazyPimpl<Impl::SubShaderVaraibleDataImpl>& data, const IShader::CONSTANT_INFO* info) const
	{
		assert(info != NULL);
		if(data == NULL)
			return false;
		return data->hasVariable(info);
	}

	//////////////////////////////////////////////////////////////////////////
	bool				SubShader::hasVariable(const LazyPimpl<Impl::SubShaderCustomVaraibleDataImpl>& data, const IShader::CONSTANT_INFO* info) const
	{
		assert(info != NULL);
		if(data == NULL)
			return false;
		return data->hasVariable(info);
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	ShaderResource::ShaderResource()
		:GraphicsResource(SHADER_RESOURCE_TYPE)
	{
		mSubShaders = NULL;
		mSubShaderCount = 0;
	}

	//////////////////////////////////////////////////////////////////////////
	ShaderResource::~ShaderResource()
	{
		if( mSubShaderCount != NULL )
		{
			assert(mSubShaders != NULL);
			for(size_t i = 0; i < mProfiles.size(); ++i)
			{
				BLADE_DELETE[] mSubShaders[i];
			}

			BLADE_RES_FREE(mSubShaders);
			BLADE_RES_FREE(mSubShaderCount);
		}
		else
			assert(mSubShaders == NULL);
	}

	//////////////////////////////////////////////////////////////////////////
	struct FnFindSubShader : public NonAssignable
	{
		const TString&	NameRef;
		FnFindSubShader(const TString& name) :NameRef(name)	{}

		bool	operator()(const SubShader& target) const
		{
			return NameRef == target.getName();
		}
	};
	SubShader*			ShaderResource::findShader(const TString& profile, const TString& name)
	{
		if( mSubShaders == NULL )
			return NULL;

		index_t profileIndex = mProfiles.find(profile);
		if( profileIndex == INVALID_INDEX )
		{
			assert(false);
			return NULL;
		}
		SubShader* ret = std::find_if(mSubShaders[profileIndex], mSubShaders[profileIndex]+mSubShaderCount[profileIndex], FnFindSubShader(name) );
		return ret != mSubShaders[profileIndex] + mSubShaderCount[profileIndex] ? ret : NULL;
	}

}//namespace Blade