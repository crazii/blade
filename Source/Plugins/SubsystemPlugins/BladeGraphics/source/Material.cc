/********************************************************************
	created:	2010/05/05
	filename: 	Material.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <Material.h>
#include <Technique.h>
#include <Pass.h>

namespace Blade
{
	typedef Handle<Technique> TechHandle;
	typedef Vector<TechHandle> TechniqueList;
	typedef Set<Material::IListener*> MatListenerList;
	typedef TStringMap<uint32>	FlagMap;

	namespace Impl
	{
		class MaterialDataImpl : public Allocatable
		{
		public:
			TechniqueList mTechniqueList;
			MatListenerList mListeners;
			FlagMap			mFlagMap;
		};
	}//namespace Impl
	using namespace Impl;

	const TString Material::MATERIAL_RESOURCE_TYPE = BTString("MaterialResource");

	//////////////////////////////////////////////////////////////////////////
	Material::Material(const TString& name)
		:GraphicsResource(MATERIAL_RESOURCE_TYPE)
		,mName(name)
		,mActiveTech(NULL)
		,mData( BLADE_NEW MaterialDataImpl() )
	{
		mData->mTechniqueList.reserve(4);
		mNeedUpdate = true;
		mHasCustomInstanceShaderConstants = false;
	}

	//////////////////////////////////////////////////////////////////////////
	Material::~Material()
	{
 		this->clearSubStates();
	}


	//////////////////////////////////////////////////////////////////////////
	size_t				Material::getTechniqueCount() const
	{
		return mData->mTechniqueList.size();
	}

	//////////////////////////////////////////////////////////////////////////
	Technique*			Material::getTechnique(index_t index) const
	{
		if( index < mData->mTechniqueList.size() )
		{
			TechniqueList::const_iterator i = mData->mTechniqueList.begin();
			std::advance(i, index);
			return *i;
		}
		else
			return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	struct FnFindTechByName
	{
		FnFindTechByName(const TString& name) :mNameRef(name)	{}

		bool operator()(const Technique* pass) const
		{
			return pass->getName() == mNameRef;
		}

		const TString& mNameRef;
		FnFindTechByName& operator=(FnFindTechByName&){return *this;}
	};

	Technique*			Material::getTechnique(const TString& name) const
	{
		TechniqueList::const_iterator i = std::find_if(mData->mTechniqueList.begin(), mData->mTechniqueList.end(), FnFindTechByName(name) );
		if( i != mData->mTechniqueList.end() )
			return *i;
		else
			return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	index_t			Material::getTechniqueByProfile(const TString& name) const
	{
		TechniqueList::const_iterator highest = mData->mTechniqueList.end();
		for( TechniqueList::const_iterator i = mData->mTechniqueList.begin(); i != mData->mTechniqueList.end(); ++i)
		{
			Technique* tech = *i;
			if( tech->getProfile() <= name )
			{
				if( highest == mData->mTechniqueList.end() || (*highest)->getProfile() < (*i)->getProfile() )
					highest = i;
			}
		}
		return highest == mData->mTechniqueList.end() ? INVALID_INDEX : std::distance(mData->mTechniqueList.begin(), highest);
	}

	//////////////////////////////////////////////////////////////////////////
	bool				Material::addTechnique(Technique* tech)
	{
		if( tech == NULL )
			return false;

		if( tech->getParent() != NULL && tech->getParent() != this )
		{
			assert(false);
			return false;
		}

		tech->setParent(this);
		mData->mTechniqueList.push_back( TechHandle() );
		TechHandle& hTech = *( mData->mTechniqueList.rbegin() );
		hTech.bind( tech );

		this->addSubState(tech);

		if( mActiveTech == NULL )
			mActiveTech = tech;

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	Technique*			Material::setActiveTechnique(index_t index)
	{
		if( index < mData->mTechniqueList.size() )
		{
			Technique* tech = this->getTechnique(index);
			if( mActiveTech != tech )
			{
				std::swap(tech, mActiveTech);
				for(MatListenerList::const_iterator iter = mData->mListeners.begin(); iter != mData->mListeners.end(); ++iter)
					(*iter)->onActivateTechnique(tech, mActiveTech);
			}
			return mActiveTech;
		}
		else
			return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	struct FnTechFind
	{
		FnTechFind(const TString& name) :mNameRef(name)	{}

		bool operator()(const Technique* tech) const
		{
			return tech->getName() == mNameRef;
		}
		const TString& mNameRef;
	private:
		FnTechFind& operator=(const FnTechFind&)	{return *this;}
	};

	Technique*			Material::setActiveTechnique(const TString& techName)
	{
		if(mData->mTechniqueList.size() == 0 )
		{
			mActiveTech = NULL;
			return NULL;
		}
		TechniqueList::iterator i = std::find_if(mData->mTechniqueList.begin(), mData->mTechniqueList.end(), FnTechFind(techName) );
		if( i == mData->mTechniqueList.end() )
			return NULL;
		else
		{
			if( mActiveTech != *i )
				return this->setActiveTechnique( index_t(i- mData->mTechniqueList.begin()) );
			return mActiveTech;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool				Material::setActiveTechnique(Technique* tech)
	{
		return tech != NULL && this->setActiveTechnique(tech->getName()) == tech;
	}

	//////////////////////////////////////////////////////////////////////////
	Technique*			Material::getActiveTechnique() const
	{
		return mActiveTech;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t				Material::getMaxGroupCount() const
	{
		size_t group = 0;
		for(TechniqueList::const_iterator i = mData->mTechniqueList.begin(); i != mData->mTechniqueList.end(); ++i)
			group = std::max<size_t>(group, (*i)->getShaderGroupList().size());
		return group;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				Material::addListener(IListener* listener)
	{
		return listener != NULL && mData->mListeners.insert(listener).second;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				Material::removeListener(IListener* listener)
	{
		return listener != NULL && mData->mListeners.erase(listener) == 1;
	}

	//////////////////////////////////////////////////////////////////////////
	void				Material::setTextureUniforms(const TStringParam& uniforms)
	{
		mTextureUniforms = uniforms;
		mNeedUpdate = true;
	}

	//////////////////////////////////////////////////////////////////////////
	void				Material::setRenderProperty(const HRENDERPROPERTY& prop)
	{
		for (size_t i = 0; i < mData->mTechniqueList.size(); ++i)
		{
			Technique* tech = mData->mTechniqueList[i];
			for (size_t j = 0; j < tech->getPassCount(); ++j)
			{
				Pass* pass = tech->getPass(j);
				if (!pass->isUsingSharedRenderPropery())
					//property shared among passes. TODO: clones
					pass->getRenderProperty()->setProperty(prop);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void				Material::removeAllTechniques()
	{
		mActiveTech = NULL;
		mData->mTechniqueList.clear();
		this->recalcLoadedCount();
	}
	
	//////////////////////////////////////////////////////////////////////////
	void				Material::onAllReady()
	{
		if( mNeedUpdate )
		{
			size_t customInstanceShaderConstants = 0;
			//auto-create per-instance variable data storage
			//and link sampler index to texture stages.
			for(size_t i = 0; i < mData->mTechniqueList.size(); ++i)
			{
				const Technique* tech = mData->mTechniqueList[i];
				assert(tech->isLoaded());
				for(size_t j = 0; j < tech->getPassCount(); ++j)
				{
					const Pass* pass = tech->getPass(j);
					assert(pass->isLoaded());
					const SamplerState& samplers = pass->getSamplerState();
					for(size_t k = 0; k < pass->getShaderOptionCount(); ++k)
					{
						const ShaderOption* option = pass->getShaderOption(k);
						ShaderInstance* shaderInst = option->getShaderInstance();
						assert(shaderInst->isLoaded());
						shaderInst->linkSampler(mTextureUniforms, samplers);
						customInstanceShaderConstants += shaderInst->getInstanceVaraibleCount();
					}
				}
			}//for each technique

			mNeedUpdate = false;
			mHasCustomInstanceShaderConstants = customInstanceShaderConstants > 0;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void			Material::onUnload()
	{
		ResourceStateGroup::onUnload();
		mNeedUpdate = true;
	}

}//namespace Blade