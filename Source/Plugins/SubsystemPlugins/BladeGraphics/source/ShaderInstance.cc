/********************************************************************
	created:	2010/09/05
	filename: 	ShaderInstance.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <ShaderInstance.h>
#include "SubShaderImpl.h"
#include <interface/IResourceManager.h>
#include <interface/public/graphics/ShaderInfoTable.h>
#include <interface/IGraphicsSystem.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	ShaderInstance::ShaderInstance(const TString& profile)
		:mOptionIndex(INVALID_INDEX)
		,mSubShader(NULL)
		,mProfile(profile)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	ShaderInstance::~ShaderInstance()
	{
	}

	ShaderInstance::ShaderInstance(const ShaderInstance& src)
		:SubResourceState(src)
		,mOptionIndex(INVALID_INDEX)
	{
		mSubShader = src.mSubShader;
		mSubShaderName = src.mSubShaderName;
	}

	ShaderInstance& ShaderInstance::operator=(const ShaderInstance& rhs)
	{
		this->ResourceState::operator=(rhs);
		mSubShader = rhs.mSubShader;
		mSubShaderName = rhs.mSubShaderName;
		mProfile = rhs.mProfile;

		return *this;
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	const HSHADER&			ShaderInstance::getShader() const
	{
		return mSubShader != NULL ? mSubShader->getShader() : HSHADER::EMPTY;
	}

	//////////////////////////////////////////////////////////////////////////
	const ShaderResource*	ShaderInstance::getShaderResource() const
	{
		return this->getTypedResource<ShaderResource>();
	}

	//////////////////////////////////////////////////////////////////////////
	void				ShaderInstance::updateBultInVariable(EShaderVariableUsage usage, IRenderDevice* renderer) const
	{
		assert(mSubShader != NULL );
		if( mSubShader != NULL && usage < SVU_COUNT)
			mSubShader->updateBultInVariable(usage, renderer);
		else
		{
			assert(false);
			BLADE_EXCEPT(EXC_OUT_OF_RANGE,BTString("invalid usage"));		
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void				ShaderInstance::updateCustomVariable(EShaderVariableUsage usage, IRenderDevice* renderer) const
	{
		usage = EShaderVariableUsage( usage & SVU_TYPE_MASK );

		if( mSubShader != NULL && usage < SVU_COUNT)
			mSubShader->updateCustomVariable(usage, renderer);
		else
		{
			assert(false);
			BLADE_EXCEPT(EXC_OUT_OF_RANGE,BTString("invalid usage"));
		}		
	}

	//////////////////////////////////////////////////////////////////////////
	void				ShaderInstance::updateTexture(IRenderDevice* renderer, const TextureStateInstance state, const SamplerState& samplers) const
	{
		if (mSamplerData != NULL)
			mSamplerData->updateTexture(renderer, state, samplers);
	}

	//////////////////////////////////////////////////////////////////////////
	bool				ShaderInstance::linkSampler(TStringList& uniforms, const SamplerState& samplers)
	{
		return mSamplerData->linkSampler(mSubShader->getShader()->getConstantTable(), uniforms, samplers, mSubShader->mSemantics);
	}

	/************************************************************************/
	/* ResourceState interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void			ShaderInstance::postLoad(const HRESOURCE& resource)
	{
		this->SubResourceState::postLoad(resource);
		ShaderResource* res = static_cast<ShaderResource*>(mResource);

		assert( res != NULL );
		mSubShader = res->findShader(mProfile, mSubShaderName);
		assert(mSubShader != NULL);

		mCustomPassVariables.clear();
		//auto-create per-pass variable data storage
		mSubShader->createAndLinkCustomPassVariable(mCustomPassVariables);
	}

	//////////////////////////////////////////////////////////////////////////
	void			ShaderInstance::onUnload()
	{
		SubResourceState::onUnload();
		mSubShader = NULL;
		mCustomPassVariables.clear();
	}
	
}//namespace Blade