/********************************************************************
	created:	2013/12/08
	filename: 	DataSource.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <databinding/DataSource.h>
#include <ConfigTypes.h>
#include <databinding/ConfigDataHandler.h>

namespace Blade
{
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	namespace Impl
	{
		//////////////////////////////////////////////////////////////////////////
		struct SubDataSoruce
		{
			mutable HDATASOURCE	mSub;
			mutable DataBinding	mParent2Target;

			/** @brief  */
			SubDataSoruce(const HDATASOURCE& sub, const DataBinding& binding)
				:mSub(sub),mParent2Target(binding)
			{
				assert(mParent2Target.isDataBinable() );
			}

			/** @brief  */
			bool		bindFromParent(Bindable* parent) const
			{
				mParent2Target.rebind(parent);
				Bindable* This = parent == NULL ? NULL : mParent2Target.getBinableData();
				return mSub->bindTarget( This ) != IDataSource::INVALID_BID;
			}

			/** @brief  */
			inline bool operator<(const SubDataSoruce& rhs) const
			{
				return FnTStringFastLess().operator()(mSub->getName(),rhs.mSub->getName());
			}
		};
		//////////////////////////////////////////////////////////////////////////
		class SubDataSourceSetImpl : public Set<SubDataSoruce>, public Allocatable	{};

	}//namespace Impl
	using namespace Impl;


	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	DataSource::DataSource(signature_t signature, const HCONFIG& config)
		:mSignature(signature)
		,mTarget(NULL)
		,mSuper(NULL)
	{
		mConfig = config;
	}

	//////////////////////////////////////////////////////////////////////////
	DataSource::DataSource(const TString& name, signature_t signature, const HCONFIG& config, CONFIG_ACCESS access)
		:mSignature(signature)
		,mTarget(NULL)
		,mSuper(NULL)
	{
		mConfig = config->clone(access, name);
	}

	//////////////////////////////////////////////////////////////////////////
	DataSource::DataSource(const TString& name, signature_t signature, IDataSource* derived/* = NULL*/)
		:mSignature(signature)
		,mTarget(NULL)
		,mSuper(derived)
	{
		if (derived == NULL)
		{
			mConfig.bind(BLADE_NEW ConfigGroup(name));
			mConfig->setDataHandler(HCDHANDLER(BLADE_NEW GroupHandler()));
		}
		else
			mConfig = derived->cloneConfig(CAF_NONE, name);			
	}

	//////////////////////////////////////////////////////////////////////////
	DataSource::~DataSource()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	IDataSource::BID	DataSource::bindTarget(Bindable* bindable)
	{
		//unbind previous target
		if( mTarget != NULL && bindable != NULL )
		{
			bool ret = this->unbindTarget(DEFAULT_BID);
			assert(ret);
			BLADE_UNREFERENCED(ret);
		}
		assert(mTarget == NULL || bindable == NULL);

		//bind sub sources first so that we will skip the sub configs of member
		bool ret = true;
		if( mSubSource != NULL )
		{
			for(SubDataSourceSetImpl::iterator i = mSubSource->begin(); i != mSubSource->end(); ++i)
			{
				const SubDataSoruce& sub = *i;
				ret = ret && sub.bindFromParent(bindable);
			}
		}
		assert(ret);

		typedef TempQueue<IConfig*> ConfigQueue;
		ConfigQueue configsQueue;
		configsQueue.push( mConfig );

		while( !configsQueue.empty() )
		{
			IConfig* config = configsQueue.front();
			configsQueue.pop();

			const HCDHANDLER& handler = config->getDataHandler();
			if( handler != NULL && (bindable == NULL || handler->getTarget() == NULL) )	//target != NULL means it is a sub source's config
			{
				handler->bindTarget(bindable);
				config->updateData(false);
			}
			//bind all configs for all values
			TStringParam options = config->getOptions();
			if (options.size() == 0)
				options.push_back(TString::EMPTY);
			for (size_t j = 0; j < options.size(); ++j)
			{
				const IConfig::IList* list = config->getSubConfigs(options[j]);
				if (list == NULL)
					continue;
				size_t count = list->getCount();
				if (count == 0)
					continue;
				for (size_t i = 0; i < count; ++i)
				{
					IConfig* subConfig = list->getConfig(i);
					configsQueue.push(subConfig);
				}
			}
		}
		mTarget = bindable;
		return ret ? DEFAULT_BID : INVALID_BID;
	}

	//////////////////////////////////////////////////////////////////////////
	HDATASOURCE			DataSource::clone(const TString& name/* = TString::EMPTY*/, CONFIG_ACCESS caf/* = CAF_NONE*/) const
	{
		BLADE_TS_VERIFY(TS_MAIN_SYNC);
		DataSource* datasource = BLADE_NEW DataSource(name, mSignature, mConfig, caf);
		
		if( mSubSource != NULL  )
		{
			*datasource->mSubSource = *mSubSource;
			for(SubDataSourceSetImpl::iterator i = datasource->mSubSource->begin(); i != datasource->mSubSource->end(); ++i)
				(*i).mSub = (*i).mSub->clone();
		}

		HDATASOURCE hRet;
		hRet.bind( datasource );
		return hRet;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	DataSource::addSubSource(Version version, const HDATASOURCE& sub, const DataBinding& this2Sub)
	{
		if( !this2Sub.isDataBinable() )
		{
			assert(false);
			return false;
		}
		if(sub == NULL && sub->getName() == TString::EMPTY )
		{
			assert(false);
			return false;
		}

		std::pair<SubDataSourceSetImpl::iterator, bool> ret = mSubSource->insert( SubDataSoruce(sub, this2Sub) );
		//already exist
		if( !ret.second )
		{
			assert(false);
			return false;
		}
		else
			mConfig->getSubConfigs(version)->addConfig( ret.first->mSub->getConfig(DEFAULT_BID) );
		return ret.second;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	DataSource::addSubSource(const signature_t signature, const DataBinding& this2Sub, Version version, const HCONFIG& subConfig)
	{
		assert(subConfig->getName() != TString::EMPTY );
		HDATASOURCE member;
		member.bind( BLADE_NEW DataSource(signature, subConfig) );
		return this->addSubSource(version, member, this2Sub);
	}
	
}//namespace Blade