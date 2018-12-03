/********************************************************************
	created:	2013/11/25
	filename: 	DataSourceWriter.h
	author:		Crazii
	
	purpose:	this file is only for final data binding usage; DO NOT include it in a public header

		note:	DataSourceWriter finishes one entry when operator<< 's parameter is a pointer to class member,
				so the pointer to member MUST be the last input. TODO: use explicit end tag?

*********************************************************************/
#ifndef __Blade_DataSourceWriter_h__
#define __Blade_DataSourceWriter_h__
#include <databinding/DataSource.h>
#include <databinding/ConfigDataHandler.h>
#include <utility/BladeContainer.h>
#include <ConfigTypes.h>
#include <interface/ILog.h>

namespace Blade
{
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	typedef struct SDataSourceSetIndex
	{
		explicit SDataSourceSetIndex(uint32 index) :mIndex(index)	{}
		uint32 mIndex;
	}DS_USE_INDEX;

	//////////////////////////////////////////////////////////////////////////
	///by default you can directly add any member that exist in global database without using DS_ADDMEMBER
	///but if you want to add a custom member that doesn't exist in global database, set the customSet to be the custom member
	typedef struct SDataSouceMember : public NonAssignable, public NonAllocatable
	{
		template<typename T, typename V>
		explicit SDataSouceMember(V T::*pmdata, const HCONFIG& customSet)
			:mCustomSet(customSet)
			,mParentToMember( (T*)NULL, pmdata, (void(T::*)(void*))NULL)
		{
			mSignature = DS_INVALID_SIGNATURE;
		}

		template<typename T, typename V>
		explicit SDataSouceMember(V T::*pmdata)
			:mCustomSet(HCONFIG::EMPTY)
			,mParentToMember( (T*)NULL, pmdata, (void(T::*)(void*))NULL)
		{
			mSignature = DS_SIGNATURE(V);
		}
		const HCONFIG&				mCustomSet;
		IDataSource::signature_t	mSignature;	//signature of member
		DataBinding					mParentToMember;
	}DS_ADDMEMBER;

	template<typename T, typename V>
	struct TDS_GETTER_SETTER : public NonAssignable, public NonAllocatable
	{
		typedef typename TBindingDesc<T, V>::GETTER GETTER;
		typedef typename TBindingDesc<T, V>::SETTER SETTER;

		explicit TDS_GETTER_SETTER(GETTER getter, SETTER setter = NULL)
			:mGetter(getter),mSetter(setter)
		{
		}
		GETTER mGetter;
		SETTER mSetter;
	};

	template<typename T, typename V>
	TDS_GETTER_SETTER<T, V>	DS_GETTER_SETTER(const V& (T::*getter)(index_t) const, bool (T::*setter)(index_t, const V&) = NULL)
	{
		return TDS_GETTER_SETTER<T, V>(getter, setter);
	}

	template<typename T>
	struct DS_GROUP : public NonAssignable, public NonAllocatable
	{
	public:
		template<typename V>
		explicit DS_GROUP(V T::*pmdata, void(T::*pmEvent)(void*) = NULL, Version version = Version::MAX)
			:mBinding((T*)NULL, pmdata, pmEvent)
			,mVersion(version)
		{

		}

		template<typename V>
		explicit DS_GROUP(TDS_GETTER_SETTER<T, V> getterSetter, Version version = Version::MAX)
			:mBinding((T*)NULL, getterSetter.mGetter, getterSetter.mSetter)
			,mVersion(version)
		{

		}
		DataBinding				mBinding;
		Version					mVersion;
	};

	//note: single default value uses variant. but multiple variant are used for options
	//this is a wrapper for multiple default values of array (rarely used)
	//other collections may be dynamically created so default value is hard to decide (only unique default value)
	//but array of class member is fixed in size and can be set to default values
	typedef struct DataSourceArrayValues
	{
		template<size_t N>
		explicit DataSourceArrayValues(const Variant(&option)[N])
		{
			mValues.reserve(N);
			mValues.insert(mValues.end(), option, option + N);
		}

		template<typename A>
		explicit DataSourceArrayValues(const std::vector<Variant, A>& option)
		{
			mValues.reserve(option.size());
			mValues.insert(mValues.end(), option.begin(), option.end());
		}

		explicit DataSourceArrayValues(const TStringParam& stringList)
		{
			mValues.resize(stringList.size());
			for (size_t i = 0; i < stringList.size(); ++i)
				mValues[i].reset(stringList[i]);
		}

		TempVector<Variant>	mValues;
	}DS_ARRAY_VALUES;

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	template<typename T>
	struct TEventWrapper
	{
		typedef typename TBindingDesc<T, int>::EVENT  TEvent;

		static void valueInitialize(Variant&)
		{
			assert(false);
		}
	};

	template<typename T, typename V, bool bindableV>
	struct TEventWrapperImpl : public TEventWrapper<T>
	{
		static void valueInitialize(Variant&)
		{
			assert(false);
		}
	};

	template<typename T, typename V>
	struct TEventWrapperImpl<T, V, true> : public TEventWrapper<T>
	{
		static void valueInitialize(Variant&)
		{
			assert(false);
		}
	};

	template<typename T, typename V>
	struct TEventWrapperImpl<T, V, false> : public TEventWrapper<T>
	{
		static void valueInitialize(Variant& v)
		{
			v.reset(V());
		}
	};

	template<typename T, typename V>
	struct DataHandlerBinding : public TEventWrapperImpl<T, V, IsBindable<V>::value>
	{
		typedef typename TEventWrapper<T>::TEvent TEvent;
		static const bool collection = false;

		static HCDHANDLER createHandler(V T::*pmdata, TEvent evt)
		{
			return HCDHANDLER(BLADE_NEW ConfigDataHandler(pmdata, (T*)NULL, evt));
		}

		static HCDHANDLER createHandler(const DataBinding& binding)
		{
			return HCDHANDLER(BLADE_NEW ConfigDataHandler(binding));
		}
	};

	template<typename T>
	struct DataHandlerBinding<T, TStringList> : public TEventWrapper<T>
	{
		typedef typename TEventWrapper<T>::TEvent TEvent;

		static const bool collection = true;
		static HCDHANDLER createHandler(TStringList T::*pmdata, TEvent evt)
		{
			return HCDHANDLER(BLADE_NEW CollectionConfigDataHandler(pmdata, (T*)NULL, evt));
		}

		static HCDHANDLER createHandler(const DataBinding& binding)
		{
			return HCDHANDLER(BLADE_NEW CollectionConfigDataHandler(binding, (T*)NULL, (TStringList*)NULL));
		}
	};

	template<typename T, typename V>
	struct DataHandlerBinding<T, Vector<V> > : public TEventWrapper<T>
	{
		typedef typename TEventWrapper<T>::TEvent TEvent;

		static const bool collection = true;
		static HCDHANDLER createHandler(Vector<V> T::*pmdata, TEvent evt)
		{
			return HCDHANDLER(BLADE_NEW CollectionConfigDataHandler(pmdata, (T*)NULL, evt));
		}

		static HCDHANDLER createHandler(const DataBinding& binding)
		{
			return HCDHANDLER(BLADE_NEW CollectionConfigDataHandler(binding, (T*)NULL, (Vector<V>*)NULL));
		}
	};

	template<typename T, typename V>
	struct DataHandlerBinding<T, List<V> > : public TEventWrapper<T>
	{
		typedef typename TEventWrapper<T>::TEvent TEvent;

		static const bool collection = true;
		static HCDHANDLER createHandler(List<V> T::*pmdata, TEvent evt)
		{
			return HCDHANDLER(BLADE_NEW CollectionConfigDataHandler(pmdata, (T*)NULL, evt));
		}

		static HCDHANDLER createHandler(const DataBinding& binding)
		{
			return HCDHANDLER(BLADE_NEW CollectionConfigDataHandler(binding, (T*)NULL, (List<V>*)NULL));
		}
	};

	template<typename T, typename V, typename C>
	struct DataHandlerBinding<T, Set<V,C> > : public TEventWrapper<T>
	{
		typedef typename TEventWrapper<T>::TEvent TEvent;

		static const bool collection = true;
		static HCDHANDLER createHandler(Set<V,C> T::*pmdata, TEvent evt)
		{
			return HCDHANDLER(BLADE_NEW CollectionConfigDataHandler(pmdata, (T*)NULL, evt));
		}

		static HCDHANDLER createHandler(const DataBinding& binding)
		{
			return HCDHANDLER(BLADE_NEW CollectionConfigDataHandler(binding, (T*)NULL, (Set<V,C>*)NULL));
		}
	};

	template<typename T, typename V, size_t N>
	struct DataHandlerBinding<T, V[N]> : public TEventWrapper<T>
	{
		typedef typename TEventWrapper<T>::TEvent TEvent;

		static const bool collection = true;
		static HCDHANDLER createHandler(V(T::*pmdata)[N], TEvent evt)
		{
			return HCDHANDLER(BLADE_NEW CollectionConfigDataHandler(pmdata, (T*)NULL, evt));
		}

		static HCDHANDLER createHandler(const DataBinding& binding)
		{
			return HCDHANDLER(BLADE_NEW CollectionConfigDataHandler(binding, (T*)NULL, (V(*)[N])NULL));
		}
	};

	template<typename T, typename V>
	struct DataHandlerBinding<T, TList<V> > : public TEventWrapper<T>
	{
		typedef typename TEventWrapper<T>::TEvent TEvent;

		static const bool collection = true;
		static HCDHANDLER createHandler(TList<V> T::*pmdata, TEvent evt)
		{
			return HCDHANDLER(BLADE_NEW CollectionConfigDataHandler(pmdata, (T*)NULL, evt));
		}

		static HCDHANDLER createHandler(const DataBinding& binding)
		{
			return HCDHANDLER(BLADE_NEW CollectionConfigDataHandler(binding, (T*)NULL, (TList<V>*)NULL));
		}
	};

	template<typename T, typename K, typename V, typename C>
	struct DataHandlerBinding<T, Map<K, V, C> > : public TEventWrapper<T>
	{
		typedef typename TEventWrapper<T>::TEvent TEvent;

		static const bool collection = true;
		static HCDHANDLER createHandler(Map<K, V, C> T::*pmdata, TEvent evt)
		{
			return HCDHANDLER(BLADE_NEW MapConfigDataHandler(pmdata, (T*)NULL, evt));
		}

		static HCDHANDLER createHandler(const DataBinding& binding)
		{
			return HCDHANDLER(BLADE_NEW MapConfigDataHandler(binding, (T*)NULL, (Map<K, V, C>*)NULL));
		}
	};

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	template<typename T>
	class DataSourceWriter : public NonAssignable, public NonAllocatable
	{
	public:
		typedef typename TBindingDesc<T, int>::EVENT  TEvent;
	public:
		///create a data source and prepare to modifying it
		DataSourceWriter(const TString& name, const TString& superName)
			:mCurrentUIHint(CUIH_NONE)
		{
			mGroup = NULL;

			assert(name != TString::EMPTY && IDataSourceDataBase::getSingleton().getDataSource(name) == NULL);
			IDataSource* superData = NULL;
			if( superName != TString::EMPTY )
			{
				superData = IDataSourceDataBase::getSingleton().getDataSource(superName);
				assert(superData != NULL);
			}
			mSource = BLADE_NEW DataSource(name, DS_SIGNATURE(T), superData);
			//reset data
			mCurrentList = NULL;
			this->resetInput();
		}

		///create a data source and prepare to modifying it
		DataSourceWriter(const TString& name, const IDataSource::signature_t superSignature = DS_INVALID_SIGNATURE)
			:mCurrentUIHint(CUIH_NONE)
		{
			mGroup = NULL;

			assert(name != TString::EMPTY && IDataSourceDataBase::getSingleton().getDataSource(name) == NULL);
			IDataSource* superData = NULL;
			if( superSignature != NULL )
			{
				superData = IDataSourceDataBase::getSingleton().getDataSource(superSignature);
				assert(superData != NULL);
			}
			mSource = BLADE_NEW DataSource(name, DS_SIGNATURE(T), superData);
			//reset data
			mCurrentList = NULL;
			this->resetInput();
		}

		///attach to a set add prepare setting data
		///note: the set is not added to global database
		DataSourceWriter(const HCONFIG& config)
			:mConfig(config)
			,mCurrentUIHint(CUIH_NONE)
		{
			mSource = NULL;
			mGroup = NULL;
			mVersion = Version::MAX;
			mCurrentVersion = NULL;
			mCurrentList = NULL;
			//reset data
			this->resetInput();
		}

		DataSourceWriter(ConfigGroup* config, Version version = Version::MAX)
			:mCurrentUIHint(CUIH_NONE)
		{
			mSource = NULL;
			mGroup = config;
			mVersion = version;
			mCurrentVersion = &mVersion;
			mCurrentList = NULL;
			//reset data
			this->resetInput();
		}

		~DataSourceWriter()
		{
			assert(mCurrentVersion == NULL && "beginVersion/endVersion mismatch.");
			if( mSource != NULL )
				IDataSourceDataBase::getSingleton().registerDataSource(HDATASOURCE(mSource));
		}

		/** @brief  */
		inline void beginVersion(Version ver)
		{
			assert(mSource == NULL || mGroup == NULL || mConfig == NULL);
			assert(mSource != NULL || mGroup != NULL || mConfig != NULL);
			assert( mCurrentList == NULL );

			const HCONFIG& config = mSource != NULL ? mSource->getConfig() : mConfig;
			assert(config != NULL);
			mCurrentList = config->addOrGetSubConfigs(ver);
			//copy super config of different version
			if (mSource != NULL && mSource->getSuper() != NULL)
			{
				IDataSource::BID bid = mSource->getSuper()->bindTarget(NULL);
				const HCONFIG& superCfg = mSource->getSuper()->getConfig(bid);

				//get exact version of super source, or latest version
				const IConfig::IList* lastVersion = superCfg->getSubConfigs(ver);
				if(lastVersion == NULL)
					lastVersion = superCfg->getSubConfigs();
				assert(lastVersion != NULL);

				for (size_t i = 0; i < lastVersion->getCount(); ++i)
				{
					const HCONFIG& cfg = lastVersion->getConfig(i);
					if(mCurrentList->getConfig(cfg->getName()) == NULL)
						mCurrentList->addConfig(cfg->clone());
				}
				mSource->getSuper()->unbindTarget(bid);
			}

			mVersion = ver;
			mCurrentVersion = &mVersion;

			//reset data
			this->resetInput();
		}

		/** @brief  */
		inline void beginLatestVersion()
		{
			const HCONFIG& config= mSource != NULL ? mSource->getConfig() : mConfig;
			assert(config != NULL);
			const Version* version = config->getLatestVersion();
			assert( version != NULL  );
			this->beginVersion(*version);
		}

		/** @brief  */
		inline void		endVersion()
		{
			assert(mCurrentList != NULL);
			this->resetInput();
			mVersion = Version::MAX;
			mCurrentVersion = NULL;
		}

		/** @brief optional: set access */
		DataSourceWriter&			operator<<(CONFIG_ACCESS caf)
		{
			mCurrentAccess = caf;
			mAccessSet = true;
			return *this;
		}

		/** @brief special range option & UI hint */
		DataSourceWriter&			operator<<(const ConfigAtom::Range& range)
		{
			assert(mCurrentOption.size() == 0);
			mCurrentRange = range;
			mHintSet = true;
			mCurrentUIHint = CUIH_INPUT_RANGE;
			TStringParam options;
			mCurrentRange.generateOptions(options);
			return this->operator<<(options);
		}

		/** @brief [optional] options */
		template<size_t N>
		DataSourceWriter&			operator<<(const Variant(&option)[N])
		{
			//only once allowed
			assert(mCurrentOption.size() == 0);
			mCurrentOption.resize(N);
			for(size_t i = 0; i < N; ++i)
				mCurrentOption[i].reset( option[i] );
			return *this;
		}

		/** @brief [optional] options */
		template<typename A>
		DataSourceWriter&			operator<<( const std::vector<Variant, A>& option)
		{
			//only once allowed
			assert(mCurrentOption.size() == 0);
			mCurrentOption.reserve(option.size());
			mCurrentOption.insert(mCurrentOption.end(), option.begin(), option.end());
			return *this;
		}

		/** @brief [optional] options: string list helper */
		DataSourceWriter&			operator<<( const TStringParam& stringList )
		{
			//only once allowed
			assert(mCurrentOption.size() == 0);
			mCurrentOption.resize( stringList.size() );
			for(size_t i = 0; i < mCurrentOption.size(); ++i)
				mCurrentOption[i].reset( stringList[i] );
			return *this;
		}

		/** @brief [optional] use structure to avoid conflict(uin32 & Variant), a uint32 will implicit cast to Variant by default */
		DataSourceWriter&			operator<<( DS_USE_INDEX index )
		{
			mOptionIndex = (size_t)index.mIndex;
			return *this;
		}

		/** @brief [optional] factory helper */
		template<typename O>
		DataSourceWriter&			operator<<( const Factory<O>& factory)
		{
			//only once allowed
			assert(mCurrentOption.size() == 0);
			mCurrentOption.resize( factory.getNumRegisteredClasses() );
			for(size_t i = 0; i < mCurrentOption.size(); ++i)
				mCurrentOption[i].reset( factory.getRegisteredClass(i) ) ;
			return *this;
		}

		/** @brief [optional] default value  */
		DataSourceWriter&			operator<<(const Variant& variable)
		{
			mCurrentVariable.reset(variable);
			mVariableSet = true;
			return *this;
		}

		/** @brief [mandatory] key step: name of a new data  */
		DataSourceWriter&			operator<<(const TString& name)
		{
			assert(mCurrentList != NULL);
			mCurrentElement = name;
			return *this;
		}

		/** @brief [optional] set UI hint */
		DataSourceWriter&			operator<<(const CONFIG_UIHINT& hint)
		{
			assert( mCurrentList != NULL );
			mCurrentUIHint = hint;
			mHintSet = true;
			return *this;
		}
		DataSourceWriter&			operator<<( EConfigUIHint hint)
		{
			assert( mCurrentList != NULL );
			mCurrentUIHint = hint;
			mHintSet = true;
			return *this;
		}

		/** @brief [optional] add delegate */
		DataSourceWriter&			operator<<( const Delegate& d )
		{
			assert( mCurrentList != NULL );
			assert( !d.isEmpty() );
			mCurrentDelegates.push_back(d);
			return *this;
		}

		/** @brief [optional] add self event */
		DataSourceWriter&			operator<<(const TEvent& pEvent)
		{
			assert( mCurrentList != NULL );
			mCurrentEvent = pEvent;
			return *this;
		}

		/** @brief [optional] set target */
		DataSourceWriter&			operator<<(Bindable* target)
		{
			assert( mCurrentList != NULL );
			assert(target != NULL);
			assert( mTarget == NULL);
			mTarget = target;
			return *this;
		}

		/** @brief [mandatory] key step: data binding (final step) */
		/* @note: this should be the last input
		*/
		template<typename V>
		DataSourceWriter&			operator<<( V T::*pmdata )
		{
			if ( IsBindable<V>::value )
				return (*this) << DS_ADDMEMBER(pmdata);
			else
			{
				IConfig* config = this->finish<V>();
				HCDHANDLER handler = DataHandlerBinding<T, V>::createHandler(pmdata, mCurrentEvent);
				this->finalize(config, handler);
				return *this;
			}
		}

		/** @brief [optional default multiple values for arrays */
		DataSourceWriter&			operator<<(const DS_ARRAY_VALUES& values)
		{
			assert(mCurrentArrayValues.size() == 0);
			mCurrentArrayValues.reserve(values.mValues.size());
			mCurrentArrayValues.insert(mCurrentArrayValues.end(), values.mValues.begin(), values.mValues.end());
			return *this;
		}
		
		/** @brief [optional] add custom member */
		/* @note: this should be the last input */
		DataSourceWriter&			operator<<( const DS_ADDMEMBER& dsAddMember )
		{
			assert(mCurrentList != NULL);
			assert( mCurrentVersion != NULL );
			assert( dsAddMember.mParentToMember.isDataBinable() );

			if( dsAddMember.mCustomSet == NULL )
			{
				assert( mCurrentElement != TString::EMPTY );
				assert( dsAddMember.mSignature != NULL );
				//if( mCurrentOption.size() > 0 || mOptionIndex != INVALID_INDEX)
				//	BLADE_LOG(Warning, TEXT("data input ignored."));
				IDataSource* member = IDataSourceDataBase::getSingleton().getDataSource(dsAddMember.mSignature);
				assert(member != NULL);
				CONFIG_ACCESS access = mAccessSet ? mCurrentAccess : CAF_NONE;
				HCONFIG hMemberConfig = member->cloneConfig(access, mCurrentElement);
				if( mHintSet )
					hMemberConfig->setUIHint(mCurrentUIHint);
				bool ret = mSource->addSubSource(member->getSignature(), dsAddMember.mParentToMember, *mCurrentVersion, hMemberConfig );
				assert(ret);
				BLADE_UNREFERENCED(ret);
			}
			else
			{
				assert( mCurrentElement == TString::EMPTY );
				assert( dsAddMember.mCustomSet->getName() != TString::EMPTY );
				if( mSource != NULL )
				{
					//CONFIG_ACCESS access = mAccessSet ? mCurrentAccess : CAF_NONE;
					bool ret = mSource->addSubSource(dsAddMember.mSignature, dsAddMember.mParentToMember, *mCurrentVersion, dsAddMember.mCustomSet );
					assert(ret);
					BLADE_UNREFERENCED(ret);
				}
				else
					assert(false);
			}

			this->resetInput();
			return *this;
		}

		/** @brief [optional] add group (multiple configs based on value) */
		/* @note: this should be the last input */
		DataSourceWriter<T>			operator<<(const DS_GROUP<T>& dsAddGroup)
		{
			assert(mCurrentList != NULL);
			assert( mCurrentVersion != NULL );
			assert( mCurrentElement != TString::EMPTY );
			ConfigGroup* config = NULL;
			if (mCurrentOption.size() != 0)
			{
				if (mOptionIndex == INVALID_INDEX)
					config = BLADE_NEW ConfigGroup(mCurrentElement, &mCurrentOption[0], mCurrentOption.size(), mCurrentAccess);
				else
				{
					if (mOptionIndex >= mCurrentOption.size())
					{
						assert(false);
						mOptionIndex = mCurrentOption.size() - 1;
					}
					config = BLADE_NEW ConfigGroup(mCurrentElement, &mCurrentOption[0], mCurrentOption.size(), (uint32)mOptionIndex, mCurrentAccess);
				}
			}
			else
			{
				assert( mVariableSet );
				config = BLADE_NEW ConfigGroup(mCurrentElement, mCurrentVariable, mCurrentAccess);
			}
			if( mHintSet )
				config->setUIHint(mCurrentUIHint);

			bool ret = mCurrentList->addConfigPtr(config);
			if( ret )
			{
				HCDHANDLER hHander = HCDHANDLER(BLADE_NEW ConfigDataHandler(dsAddGroup.mBinding));
				config->setDataHandler(hHander);
				for(size_t i = 0; i < mCurrentDelegates.size(); ++i)
					hHander->addDelegate( mCurrentDelegates[i] );
			}
			else
				assert(false);

			this->resetInput();
			return DataSourceWriter<T>(config, dsAddGroup.mVersion);
		}

		/** @brief [optional] set related value for group's new data */
		DataSourceWriter&			operator[](const TString& val)
		{
			assert( mCurrentVersion != NULL );
			assert( mCurrentElement == TString::EMPTY );
			assert(mGroup != NULL);
			mCurrentList = mGroup->addOrGetSubConfigs(*mCurrentVersion, val);

			this->resetInput();
			return *this;
		}

		/** @brief [optional] add getter/setter */
		/* @note: this should be the last input */
		template<typename V>
		DataSourceWriter&			operator<<(TDS_GETTER_SETTER<T,V> gs)
		{
			assert(mCurrentList != NULL);
			assert( mCurrentVersion != NULL );
			assert( mCurrentElement != TString::EMPTY );

			IConfig* config = this->finish<V>();
			HCDHANDLER handler = DataHandlerBinding<T, V>::createHandler(DataBinding((T*)NULL, gs.mGetter, gs.mSetter, mCurrentEvent));
			this->finalize(config, handler);
			return *this;
		}

	protected:
		/** @brief reset data on begin/end of a complete data */
		void		resetInput()
		{
			mOptionIndex = INVALID_INDEX;
			mCurrentAccess = CAF_NORMAL;
			mCurrentElement = TString::EMPTY;
			mCurrentOption.clear();
			mCurrentArrayValues.clear();
			mCurrentRange.reset();
			mCurrentUIHint = CUIH_NONE;
			mCurrentEvent = NULL;
			mTarget = NULL;
			mCurrentDelegates.clear();
			mAccessSet = false;
			mVariableSet = false;
			mHintSet = false;
		}

		/** @brief finis on final atom input */
		template<typename V>
		IConfig*	finish()
		{
			assert(mCurrentList != NULL);
			assert(mCurrentVersion != NULL);
			assert(mCurrentElement != TString::EMPTY);

			IConfig* ret = NULL;

			if (DataHandlerBinding<T,V>::collection)
			{
				//if (mCurrentOption.size() > 0 || mOptionIndex != INVALID_INDEX || mVariableSet)
				//	BLADE_LOG(Warning, TEXT("data input ignored."));

				ConfigGroup* group = BLADE_NEW ConfigGroup(mCurrentElement, mCurrentAccess, true);
				if (mHintSet)
					group->setUIHint(mCurrentUIHint);
				if (mCurrentOption.size() != 0)
					group->setOptions(&mCurrentOption[0], mCurrentOption.size(), mOptionIndex);

				if (!mCurrentList->addConfigPtr(group))
				{
					BLADE_DELETE group;
					group = NULL;
				}
				ret = group;
			}
			else
			{
				Variant typeVerify;
				DataHandlerBinding<T,V>::valueInitialize(typeVerify);
				IConfig* config = NULL;
				if (mCurrentUIHint == CUIH_INPUT_RANGE)
				{
					assert(mCurrentOption.size() == 3);	//not used. by may be used by groups
					config = BLADE_NEW ConfigAtom(mCurrentElement, mCurrentRange, mCurrentAccess);
				}
				else if (mCurrentOption.size() != 0)
				{
					if (mOptionIndex == INVALID_INDEX)
					{
						assert(mCurrentOption[0].getType() == typeVerify.getType());
						config = BLADE_NEW ConfigAtom(mCurrentElement, &mCurrentOption[0], mCurrentOption.size(), mCurrentAccess);
						if (mVariableSet)
							config->setValue(mCurrentVariable.toTString());
					}
					else
					{
						assert(sizeof(V) == sizeof(uint32));
						if (mOptionIndex >= mCurrentOption.size())
						{
							assert(false);
							mOptionIndex = mCurrentOption.size() - 1;
						}
						config = BLADE_NEW ConfigAtom(mCurrentElement, &mCurrentOption[0], mCurrentOption.size(), (uint32)mOptionIndex, mCurrentAccess);
					}
				}
				else
				{
					if (!mVariableSet)
						DataHandlerBinding<T, V>::valueInitialize(mCurrentVariable);
					else
						assert(mCurrentVariable.getType() == typeVerify.getType());

					config = BLADE_NEW ConfigAtom(mCurrentElement, mCurrentVariable, mCurrentAccess);
				}
				if (mHintSet)
					config->setUIHint(mCurrentUIHint);

				if (!mCurrentList->addConfigPtr(config))
				{
					assert(false);
					BLADE_DELETE config;
					config = NULL;
				}
				ret = config;
			}
			return ret;
		}

		/** @brief  */
		bool	finalize(IConfig* config, HCDHANDLER handler)
		{
			bool ret;
			if (config != NULL)
			{
				config->setDataHandler(handler);

				for (size_t i = 0; i < mCurrentDelegates.size(); ++i)
					handler->addDelegate(mCurrentDelegates[i]);

				if (mTarget != NULL)
					config->bindTarget(mTarget);
				ret = true;
			}
			else
			{
				assert(false);
				ret = false;
			}

			this->resetInput();

			return ret;
		}

		//only 1 of the 3 below is valid
		DataSource*			mSource;
		ConfigGroup*		mGroup;
		HCONFIG				mConfig;
		//temporary data
		IConfig::IList*		mCurrentList;

		TempVector<Variant>	mCurrentOption;
		TempVector<Variant>	mCurrentArrayValues;
		ConfigAtom::Range	mCurrentRange;
		size_t				mOptionIndex;
		Variant				mCurrentVariable;
		CONFIG_UIHINT		mCurrentUIHint;
		TEvent				mCurrentEvent;
		TempVector<Delegate>mCurrentDelegates;
		TString				mCurrentElement;
		TString				mRelatedValue;
		Version*			mCurrentVersion;
		Bindable*			mTarget;
		Version				mVersion;
		CONFIG_ACCESS		mCurrentAccess;
		bool				mVariableSet;
		bool				mHintSet;
		bool				mAccessSet;
	};//class DataSourceWriter
	
}//namespace Blade


#endif // __Blade_DataSourceWriter_h__
