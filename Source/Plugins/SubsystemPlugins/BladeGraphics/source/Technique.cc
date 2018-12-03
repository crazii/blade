/********************************************************************
	created:	2010/05/22
	filename: 	Technique.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <Technique.h>
#include <Pass.h>

namespace Blade
{
	typedef Handle<Pass> PassHandle;
	namespace Impl
	{
		class PassList : public List<PassHandle>, public Allocatable{};
	}//namespace Impl
	using namespace Impl;

	/************************************************************************/
	/* Technique                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	Technique::Technique(const TString& name, const TString& profile, Material* parent)
		:mParent(parent)
		,mName(name)
		,mProfile(profile)
		,mPasses( BLADE_NEW Impl::PassList() )
	{

	}

	//////////////////////////////////////////////////////////////////////////
	Technique::Technique(const Technique& src, int shareMethod)
		:mParent(src.mParent)
		,mPasses( BLADE_NEW Impl::PassList() )
	{
		mName = src.mName;
		mProfile = src.mProfile;
		*mPasses = *src.mPasses;
		mGroups = src.mGroups;
		mLODSettings = src.mLODSettings;

		for( size_t i = 0; i < src.getPassCount(); ++i)
		{
			Pass* srcPass = src.getPass(i);
			Pass* pass = srcPass->clone(srcPass->getName(), shareMethod);
			pass->setParent(NULL);
			this->addPass( pass );
		}
		this->recalcLoadedCount();
	}

	//////////////////////////////////////////////////////////////////////////
	Technique::~Technique()
	{
		this->clearSubStates();
	}

	/************************************************************************/
	/* Pass functions                                                                    */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	size_t			Technique::getPassCount() const
	{
		return mPasses->size();
	}

	//////////////////////////////////////////////////////////////////////////
	Pass*			Technique::getPass(index_t index) const
	{
		if( index < mPasses->size() )
		{
			PassList::const_iterator i = mPasses->begin();
			std::advance(i, index);
			return *i;
		}
		else
			BLADE_EXCEPT(EXC_OUT_OF_RANGE,BTString("index out of range.") );
	}

	//////////////////////////////////////////////////////////////////////////
	struct FnFindPassByName
	{
		FnFindPassByName(const TString& name) :mNameRef(name)	{}

		bool operator()(const Pass* pass) const
		{
			return pass->getName() == mNameRef;
		}

		const TString& mNameRef;
		FnFindPassByName& operator=(FnFindPassByName&){return *this;}
	};

	//////////////////////////////////////////////////////////////////////////
	Pass*			Technique::getPass(const TString& name) const
	{
		PassList::const_iterator i = std::find_if(mPasses->begin(), mPasses->end(), FnFindPassByName(name) );
		if( i != mPasses->end() )
			return *i;
		else
		{
			assert(false);
			return NULL;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool			Technique::addPass(Pass* pass)
	{
		if( pass == NULL )
			return false;

		if( pass->getParent() != NULL && pass->getParent() != this )
		{
			assert(false);
			return false;
		}

		pass->setParent(this);
		mPasses->push_back( PassHandle() );
		PassHandle& hPass = *mPasses->rbegin();
		hPass.bind( pass );
		this->addSubState(pass);
		return true;
	}

	/************************************************************************/
	/* SortGroup functions                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool		Technique::prepareShaderGroups(size_t count)
	{
		mGroups.reserve(count);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	index_t		Technique::addShaderGroup(const ShaderGroup& group)
	{
		index_t index = mGroups.size();
		mGroups.push_back(group);
		return index;
	}

	/************************************************************************/
	/* LOD functions                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool		Technique::addLOD(MATERIALLOD::LOD_INDEX level, scalar distanceStartSQ, scalar distanceEndSQ, uint8 groupIndex)
	{
		if( level != mLODSettings.size() )
			return false;

		MATERIALLOD LOD;
		LOD.mSquaredDistanceStart = distanceStartSQ;
		LOD.mSquaredDistanceEnd = distanceEndSQ;
		LOD.mTargetGroupIndex= groupIndex;
		LOD.mLODIndex = level;
		mLODSettings.push_back( LOD );
		return true;
	}


	/************************************************************************/
	/* helpers                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool	Technique::setPassVariable(index_t _pass, const TString& variableName, EShaderConstantType type, size_t count, const void* pData)
	{
		if (pData == NULL || _pass < mPasses->size())
		{
			assert(false);
			return false;
		}

		bool success = false;

		PassList::iterator iter = mPasses->begin();
		std::advance(iter, _pass);
		Pass* pass = *iter;
		size_t optionCount = pass->getShaderOptionCount();
		for(size_t j = 0; j < optionCount; ++j)
		{
			bool once_success = pass->getShaderOption(j)->getShaderInstance()->setPassShaderVariables(variableName, type, count, pData);
			success = success || once_success;
		}

		if (success && type == SCT_SAMPLER)
		{
			const HTEXTURE* phTex = (const HTEXTURE*)pData;
			scalar w = (scalar)(*phTex)->getWidth();
			scalar h = (scalar)(*phTex)->getHeight();
			Vector4 size(w, h, 1 / w, 1 / h);
			bool ret = this->setPassVariable(_pass, variableName + BTString("_Size"), SCT_FLOAT4, 1, &size);
			assert(ret); BLADE_UNREFERENCED(ret);
		}

		return success;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	Technique::setShaderVariable(const TString& variableName,EShaderConstantType type,size_t count,const void* pData)
	{
		if (pData == NULL)
		{
			assert(false);
			return false;
		}

		bool success = false;
		for(PassList::iterator i = mPasses->begin(); i != mPasses->end(); ++i)
		{
			Pass* pass = *i;
			size_t optionCount = pass->getShaderOptionCount();
			for(size_t j = 0; j < optionCount; ++j)
			{
				bool once_success = pass->getShaderOption(j)->getShaderInstance()->setShaderVariable(variableName, type, count, pData);
				success = success || once_success;
			}
		}

		if (success && type == SCT_SAMPLER)
		{
			const HTEXTURE* phTex = (const HTEXTURE*)pData;
			scalar w = (scalar)(*phTex)->getWidth();
			scalar h = (scalar)(*phTex)->getHeight();
			Vector4 size(w, h, 1/w, 1/h);
			bool ret = this->setShaderVariable(variableName + BTString("_Size"), SCT_FLOAT4, 1, &size);
			assert(ret); BLADE_UNREFERENCED(ret);
		}
		return success;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			Technique::getShaderVariableHandle(ShaderVariableMap::Handles& outHandle, const TString& variableName, EShaderConstantType type, size_t count)
	{
		bool success = false;
		outHandle.reset();
		for (PassList::iterator i = mPasses->begin(); i != mPasses->end(); ++i)
		{
			Pass* pass = *i;
			size_t optionCount = pass->getShaderOptionCount();
			for (size_t j = 0; j < optionCount; ++j)
			{
				const ShaderVariableMap& variableMap = pass->getShaderOption(j)->getShaderInstance()->getShaderVariables();
				bool once_success = variableMap.getVariableHandle(outHandle, variableName, type, count);
				success = success || once_success;
			}
		}
		return success;
	}

	//////////////////////////////////////////////////////////////////////////
	Technique*		Technique::clone(int method/* = MS_ALL*/) const
	{
		return BLADE_NEW Technique(*this, method);
	}


}//namespace Blade