/********************************************************************
	created:	2010/03/25
	filename: 	ConfigTypes.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <ConfigTypes.h>

namespace Blade
{
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable:4640) //construction of local static object is not thread safe
#endif
	class ConfigFindHelper : public IConfig, public HCONFIG, public NonAssignable
	{
	public:
		ConfigFindHelper(const TString& name)
			:mNameRef(name){ mPtr = this; }
		~ConfigFindHelper(){ mPtr = NULL;}

		virtual const TString&	getName() const {return mNameRef;}
		virtual const CONFIG_ATTR&getAttribute() const {static CONFIG_ATTR attr; return attr;}
		virtual void			setUIHint(const CONFIG_UIHINT& /*hint*/)	{}
		virtual void			setReadOnly(bool)			{}
		virtual const Option&	getOptions() const {return TStringList::EMPTY;}
		virtual bool			setOptions(const TString*, size_t, index_t){return false;}
		virtual const TString&	getValue() const	{return TString::EMPTY;}
		virtual const TString&	getDefaultValue() const	{return TString::EMPTY;}
		virtual const Variant&	getBinaryValue() const		{return Variant::EMPTY;}
		virtual bool			setValue(const TString&,CONFIG_ACCESS) {return false;}
		virtual bool			setBinaryValue(const Variant& /*Val*/, CONFIG_ACCESS /*verify = CAF_READWRITE*/)	{return false;}
		virtual void			setToDefault()		{}
		virtual const Version*	getLatestVersion() const						{return NULL;}
		virtual IList*			getSubConfigs(Version ) const					{return NULL;}
		virtual IList*			getSubConfigs(const TString&, Version) const	{return NULL;}
		virtual IList*			addOrGetSubConfigs(Version, const TString& )	{return NULL;}
		virtual bool			clearSubConfigs()	{return false;}
		virtual void			backupValue()		{}
		virtual const TString&	getBackupValue() const	{return TString::EMPTY;}
		virtual void			restoreToBackup()	{}
		virtual void			setDataHandler(const HHANDLER&)	{}
		virtual const HHANDLER&	getDataHandler() const	{return HHANDLER::EMPTY;}
		virtual bool			updateData(bool, bool)							{return false;}
		virtual bool			bindTarget(Bindable* /*target*/)				{return false;}
		virtual bool			synchronize(Bindable* /*target*/) const			{return false;}
		virtual HCONFIG			clone(CONFIG_ACCESS, const TString&) const		{return HCONFIG::EMPTY;}
	private:
		//ConfigFindHelper&	operator=(const ConfigFindHelper&);
		const TString& mNameRef;
	};//class ConfigFindHelper
#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(pop)
#endif

	typedef List<HCONFIG>											SubConfigList;
	typedef Map< HCONFIG, SubConfigList::iterator, FnHConfigLess >	SubConfigSet;

	class ConfigList : public IConfig::IList, public Allocatable
	{
	protected:
		SubConfigList	mList;
		SubConfigSet	mSet;
	public:
		virtual ~ConfigList()	{}

		/** @brief  */
		virtual size_t		getCount() const
		{
			assert( mList.size() == mSet.size() );
			return mList.size();
		}
		/** @brief  */
		virtual const HCONFIG&	getConfig(index_t index) const
		{
			assert( mList.size() == mSet.size() );
			if( index < mList.size() )
			{
				SubConfigList::const_iterator i = mList.begin();
				std::advance(i, index);
				return *i;
			}
			return HCONFIG::EMPTY;
		}
		/** @brief  */
		virtual const HCONFIG&	getConfig(const TString& name) const
		{
			assert( mList.size() == mSet.size() );
			ConfigFindHelper helper(name);
			SubConfigSet::const_iterator i = mSet.find( helper );
			if( i != mSet.end() )
				return i->first;
			else
				return HCONFIG::EMPTY;

		}
		/** @brief  */
		virtual bool			addConfig(const HCONFIG& config)
		{
			assert( mList.size() == mSet.size() );
			std::pair<SubConfigSet::iterator,bool> ret = mSet.insert( std::make_pair(config, mList.end()) );
			if( ret.second )
			{
				mList.push_back(config);
				ret.first->second = --mList.end();
				return true;
			}
			else
				return false;
		}
		/** @brief  */
		virtual bool			removeConfig(const TString& name)
		{
			assert( mList.size() == mSet.size() );
			ConfigFindHelper finder(name);
			SubConfigSet::iterator iter = mSet.find(finder);
			if( iter != mSet.end() )
			{
				mList.erase( iter->second );
				mSet.erase(iter);
				return true;
			}
			else
				return false;
		}

		//////////////////////////////////////////////////////////////////////////
		void			clear()
		{
			assert( mList.size() == mSet.size() );
			mSet.clear();
			mList.clear();
		}
		//////////////////////////////////////////////////////////////////////////
		void			setToDefault() const
		{
			assert( mList.size() == mSet.size() );
			for(SubConfigList::const_iterator i = mList.begin(); i != mList.end(); ++i)
				(*i)->setToDefault();
		}
		//////////////////////////////////////////////////////////////////////////
		void			makeClone(CONFIG_ACCESS caf)
		{
			assert( mList.size() == mSet.size() );
			mSet.clear();
			for(SubConfigList::iterator i = mList.begin(); i != mList.end(); ++i)
			{
				*i = (*i)->clone(caf);
				bool ret = mSet.insert( std::make_pair(*i, i) ).second;
				assert(ret);
				BLADE_UNREFERENCED(ret);
			}
		}
		//////////////////////////////////////////////////////////////////////////
		void			backupValue() const
		{
			assert( mList.size() == mSet.size() );
			for(SubConfigList::const_iterator i = mList.begin(); i != mList.end(); ++i)
				(*i)->backupValue();
		}
		//////////////////////////////////////////////////////////////////////////
		void			restoreToBackup() const
		{
			assert( mList.size() == mSet.size() );
			for(SubConfigList::const_iterator i = mList.begin(); i != mList.end(); ++i)
				(*i)->restoreToBackup();
		}
		//////////////////////////////////////////////////////////////////////////
		void				updateData(bool updateChild, bool force/* = false*/) const
		{
			assert( mList.size() == mSet.size() );
			for(SubConfigList::const_iterator i = mList.begin(); i != mList.end(); ++i)
				(*i)->updateData(updateChild, force);
		}
		//////////////////////////////////////////////////////////////////////////
		bool				bindTarget(Bindable* target) const
		{
			assert( mList.size() == mSet.size() );
			bool ret = true;
			for(SubConfigList::const_iterator i = mList.begin(); i != mList.end(); ++i)
			{
				if( !(*i)->bindTarget(target) )
					ret = false;
			}
			return ret;
		}
		//////////////////////////////////////////////////////////////////////////
		bool				synchronize(Bindable* target) const
		{
			assert( mList.size() == mSet.size() );
			bool ret = true;
			for(SubConfigList::const_iterator i = mList.begin(); i != mList.end(); ++i)
			{
				if( !(*i)->synchronize(target) )
					ret = false;
			}
			return ret;
		}
	};//class ConfigList

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	ConfigBase::ConfigBase(const TString& name)
		:mName(name)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	ConfigBase::~ConfigBase()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	const TString&		ConfigBase::getName() const
	{
		return mName;
	}

	//////////////////////////////////////////////////////////////////////////
	const CONFIG_ATTR&	ConfigBase::getAttribute() const
	{
		return mAttribute;
	}

	//////////////////////////////////////////////////////////////////////////
	void				ConfigBase::setUIHint(const CONFIG_UIHINT& hint)
	{
		mAttribute.mUIHint = hint;
	}

	//////////////////////////////////////////////////////////////////////////
	void				ConfigBase::setReadOnly(bool readyOnly)
	{
		if( readyOnly )
			mAttribute.mAccess = CONFIG_ACCESS(mAttribute.mAccess & (~CAF_WRITE));
		else
			mAttribute.mAccess = CONFIG_ACCESS(mAttribute.mAccess | CAF_WRITE);
	}

	//////////////////////////////////////////////////////////////////////////
	bool		ConfigBase::setOptions(const TString* opts, size_t count, index_t newIndex/* = INVALID_INDEX*/)
	{
		if( opts == NULL || count == 0 || 
			(count > 1 && (this->getUIHint() != CUIH_LIST && this->getUIHint() != CUIH_INDEX 
			&& this->getUIHint() != CUIH_INPUT_RANGE && this->getUIHint() != CUIH_RANGE
			&& this->getUIHint() != CUIH_CHECK) )
			)
		{
			assert(false);
			return false;
		}

		mOptions.clear();
		mOptions.reserve(count);

		for(size_t i = 0; i < count; ++i)
			mOptions.push_back( opts[i] );

		if( newIndex >= count )
		{
			newIndex = mOptions.find(mValue);	//try using original index
			newIndex = (newIndex == INVALID_INDEX) ? 0 : newIndex;
		}
		mBackup = mDefaultValue = mValue = mOptions[newIndex];
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	const TString&		ConfigBase::getValue() const
	{
		return mValue;
	}

	//////////////////////////////////////////////////////////////////////////
	const TString&		ConfigBase::getDefaultValue() const
	{
		return mDefaultValue;
	}

	//////////////////////////////////////////////////////////////////////////
	const Variant&		ConfigBase::getBinaryValue() const
	{
		return mBinaryValue;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				ConfigBase::setValue(const TString& value, CONFIG_ACCESS verify/* = CAF_READWRITE*/)
	{
		if( mAttribute.mAccess & verify )
		{
			//note: binding data may change after bindTarget, read data from target first
			this->updateData(true, false);

			TString val = value;
			index_t index = INVALID_INDEX;
			if (mAttribute.mUIHint == CUIH_INDEX || mAttribute.mUIHint == CUIH_RANGE)
			{
				//string first
				index = mOptions.find(val);
				//string not found, guess it a number
				if (index == INVALID_INDEX)
				{
					index = TStringHelper::toSizeT(val);
					assert(index < mOptions.size());
				}
				if(mBinaryValue.getType() == VI_TSTRING)
					val = mOptions[index];
			}

			if(mValue != val)
			{
				if ((mAttribute.mUIHint == CUIH_INDEX || mAttribute.mUIHint == CUIH_RANGE) && mBinaryValue.getType() != VI_TSTRING)
				{
					//ignore the binary type (maybe string or uint32 etc.), set by string value
					mBinaryValue.fromTString(TStringHelper::fromUInt(index));
					mValue = val;
				}
				else
				{
					mBinaryValue.fromTString(val);
					mValue = mBinaryValue.toTString();
					assert(mValue == val
						//TODO: verify vector/quaternion/aabb etc, wich multiple element and encapsuled with ()
						|| Math::Equal(TStringHelper::toF64(mValue), TStringHelper::toF64(val), 1e-3) );
				}
				this->notify(verify);
			}
			return true;
		}
		else
			return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				ConfigBase::setBinaryValue(const Variant& Val, CONFIG_ACCESS verify/* = CAF_READWRITE*/)
	{
		if( mAttribute.mAccess & verify )
		{
			if( mBinaryValue != Val )
			{
				mBinaryValue = Val;

				if( mAttribute.mUIHint == CUIH_INDEX )
				{
					size_t val = TStringHelper::toSizeT(Val.toTString());
					if( val < mOptions.size() )
						mValue = mOptions[val];
					else
						assert(false);
				}
				else
					mValue = Val.toTString();

				this->notify(verify);
			}
			return true;
		}
		else
			return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				ConfigBase::updateData(bool /*updateChild*/, bool force/* = false*/)
	{
		if( mDataHandler == NULL || mDataHandler->getTarget() == NULL)
			return false;

		if( !this->isDirty() && !force )
			return false;

		bool ret = mDataHandler->getData(mBinaryValue.getBinaryData(), mBinaryValue.getSize());
		if(ret)
		{
			mValue = mBinaryValue.toTString();
			if((mAttribute.mUIHint == CUIH_INDEX || mAttribute.mUIHint == CUIH_RANGE) && mBinaryValue.getType() != VI_TSTRING)
			{
				size_t val = TStringHelper::toSizeT(mValue);
				if( val < mOptions.size() )
					mValue = mOptions[val];
				else
					assert(false);
			}
		}
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				ConfigBase::bindTarget(Bindable* target)
	{
		if( mDataHandler == NULL )
		{
			//assert(false);
			return false;
		}
		mDataHandler->bindTarget(target);
		//update config data after binding
		this->updateData(false);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				ConfigBase::synchronize(Bindable* target) const
	{
		if( mDataHandler == NULL || target == NULL)
			return false;
		else
		{
			mDataHandler->bindTarget(target);
			this->notify(CAF_READWRITE);
			mDataHandler->bindTarget(NULL);
			return true;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool				ConfigBase::initValue(const TString& val, bool addToOption /* = true*/, size_t optionCount/* = 1*/)
	{
		//value must be set first
		if( mOptions.size() != 0 )
		{
			assert(false);
			return false;
		}

		mValue = val;
		mDefaultValue = val;
		mBackup = val;
		mOptions.reserve(optionCount);
		if( addToOption )
			mOptions.push_back(val);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				ConfigBase::addOption(const TString& option)
	{
		if( mOptions.find(option) != INVALID_INDEX )
			return false;
		else
		{
			mOptions.push_back(option);
			return true;
		}	
	}

	//////////////////////////////////////////////////////////////////////////
	void				ConfigBase::forceAddOption(const TString& option)
	{
		mOptions.push_back(option);
	}

	//////////////////////////////////////////////////////////////////////////
	void				ConfigBase::notify(CONFIG_ACCESS verify) const
	{
		if (mDataHandler != NULL && (mDataHandler->getTarget() != NULL || mDataHandler->hasDelegate()) && ((mAttribute.mAccess&verify)&(CAF_WRITE | CAF_SERIALIZE)) != 0)
			mDataHandler->setData(this->getName(), mBinaryValue.getBinaryData(), mBinaryValue.getSize(), verify);
	}

	//////////////////////////////////////////////////////////////////////////
	bool				ConfigBase::isDirty() const
	{
		if( mDataHandler == NULL || mDataHandler->getTarget() == NULL)
			return false;

		const void* data = mBinaryValue.getBinaryData();
		size_t bytes = mBinaryValue.getSize();
		return !mDataHandler->compareData(data, bytes);
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void ConfigAtom::Range::generateOptions(TStringParam& options)
	{
		options.clear();
		options.push_back(mMin.toTString());
		options.push_back(mMax.toTString());
		options.push_back(mStep.toTString());
	}

	//////////////////////////////////////////////////////////////////////////
	ConfigAtom::ConfigAtom(const TString& name)
		:ConfigBase(name)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	ConfigAtom::ConfigAtom(const TString& name, const Range& rangeDef, CONFIG_ACCESS caf/* = CAF_NORMAL*/)
		:ConfigBase(name)
	{
		mAttribute.mUIHint = CUIH_INPUT_RANGE;
		mAttribute.mAccess = CONFIG_ACCESS(caf | CAF_READWRITE);
		mBinaryValue.reset(rangeDef.mVal);

		this->initValue( rangeDef.mVal.toTString(), false, 3);
		this->forceAddOption(rangeDef.mMin.toTString());
		this->forceAddOption(rangeDef.mMax.toTString());
		this->forceAddOption(rangeDef.mStep.toTString());
	}

	//////////////////////////////////////////////////////////////////////////
	ConfigAtom::ConfigAtom(const TString& name,const TStringParam& list,CONFIG_ACCESS caf/* = CAF_NORMAL*/)
		:ConfigBase(name)
	{
		mAttribute.mUIHint = CUIH_LIST;
		mAttribute.mAccess = CONFIG_ACCESS(caf | CAF_READWRITE);
		if( list.size() > 0 )
		{
			mBinaryValue.reset( list[0] );
			this->initValue( list[0], true, list.size() );

			for(size_t i = 1; i < list.size(); ++i)
			{
				this->addOption( list[i] );
			}	
		}
		else
		{
			mBinaryValue.reset( TString::EMPTY );
			this->initValue( TString::EMPTY, false, list.size() );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	ConfigAtom::ConfigAtom(const TString& name,const TStringParam& list, uint32 index, CONFIG_ACCESS caf/* = CAF_NORMAL*/)
		:ConfigBase(name)
	{
		mAttribute.mUIHint = CUIH_INDEX;
		mAttribute.mAccess = CONFIG_ACCESS(caf | CAF_READWRITE);
		assert(list.size() > 0 && index < list.size());
		if( index >= list.size() )
			index = (uint32)list.size()-1;

		mBinaryValue.reset(index);
		this->initValue(list[index], false, list.size());

		for(size_t i = 0; i < list.size(); ++i)
			this->addOption( list[i] );
	}

	//////////////////////////////////////////////////////////////////////////
	ConfigAtom::ConfigAtom(const TString& name, const Variant* list, size_t count, CONFIG_ACCESS caf/* = CAF_NORMAL*/)
		:ConfigBase(name)
	{
		mAttribute.mUIHint = CUIH_LIST;
		mAttribute.mAccess = CONFIG_ACCESS(caf | CAF_READWRITE);
		mBinaryValue.reset( list[0] );
		this->initValue( list[0].toTString(), true, count);
		for(size_t i = 1; i < count; ++i)
			this->addOption( list[i].toTString() );
	}

	//////////////////////////////////////////////////////////////////////////
	ConfigAtom::ConfigAtom(const TString& name, const Variant* list, size_t count, uint32 index, CONFIG_ACCESS caf/* = CAF_NORMAL*/)
		:ConfigBase(name)
	{
		mAttribute.mUIHint = CUIH_INDEX;
		mAttribute.mAccess = CONFIG_ACCESS(caf | CAF_READWRITE);
		assert(index < count);
		if( index >= count )
			index = (uint32)count - 1;

		mBinaryValue.reset( index );
		this->initValue(list[index], false, count);
		for(size_t i = 0; i < count; ++i)
		{
			this->addOption( list[i].toTString() );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	ConfigAtom::ConfigAtom(const TString& name, const Variant& val, CONFIG_ACCESS caf/* = CAF_NORMAL*/)
		:ConfigBase(name)
	{
		EConfigUIHint hint;
		switch( val.getType() )
		{
		case VI_BOOL:
			hint = CUIH_CHECK;
			break;
		case VI_COLOR:
			hint = CUIH_COLOR;
			break;
		default:
			hint = CUIH_INPUT;
		}
		mAttribute.mUIHint = hint;
		mAttribute.mAccess = caf;
		mBinaryValue.reset( val );

		if(caf & CAF_READWRITE )
		{
			this->initValue( val.toTString() );
			if( hint == CUIH_CHECK )
			{
				bool defTrueFalse = val;
				this->addOption( TStringHelper::fromBool(!defTrueFalse) );
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	ConfigAtom::~ConfigAtom()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void				ConfigAtom::setToDefault()
	{
		if( mValue != mDefaultValue )
		{
			mValue = mDefaultValue;
			mBinaryValue.fromTString(mValue);
			this->notify(CAF_READWRITE);
		}
		else
			mValue = mDefaultValue;
	}

	//////////////////////////////////////////////////////////////////////////
	void				ConfigAtom::backupValue()
	{
		mBackup = mValue;
	}

	//////////////////////////////////////////////////////////////////////////
	const TString&		ConfigAtom::getBackupValue() const
	{
		return mBackup;
	}

	//////////////////////////////////////////////////////////////////////////
	void				ConfigAtom::restoreToBackup()
	{
		if( mValue != mBackup )
		{
			mValue = mBackup;
			mBinaryValue.fromTString(mValue);
			this->notify(CAF_READWRITE);
		}
		else
			mValue = mBackup;
	}

	//////////////////////////////////////////////////////////////////////////
	HCONFIG				ConfigAtom::clone(CONFIG_ACCESS caf/* = CAF_NONE*/, const TString& name/* = TString::EMPTY*/) const
	{
		//no binding allowed on clone
		assert( mDataHandler == NULL || mDataHandler->getTarget() == NULL );
		ConfigAtom* atom = BLADE_NEW ConfigAtom(*this);
		if( caf != CAF_NONE )
			atom->mAttribute.mAccess = caf;
		if( name != TString::EMPTY )
			atom->mName = name;
		if( atom->mDataHandler != NULL )
			atom->mDataHandler = atom->mDataHandler->clone();
		return HCONFIG(atom);
	}

	//////////////////////////////////////////////////////////////////////////
	bool		ConfigAtom::initStringValue(const TString& val,CONFIG_UIHINT uiHint,CONFIG_ACCESS caf/* = CAF_NORMAL*/)
	{
		if( uiHint == CUIH_INPUT || uiHint == CUIH_FILE || uiHint == CUIH_PATH || uiHint == CUIH_IMAGE || (uiHint == CUIH_NONE && !(caf&CAF_WRITE)) )
		{
			mAttribute.mAccess = caf;
			mAttribute.mUIHint = uiHint;
			mBinaryValue.reset(val);

			if( caf & CAF_READWRITE )
				this->initValue( val );
			return true;
		}
		else
		{
			assert(false);
			return false;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool		ConfigAtom::initBinaryValue(const Variant& val, CONFIG_UIHINT uiHint, CONFIG_ACCESS caf/* = CAF_NORMAL*/)
	{
		mAttribute.mAccess = caf;
		mAttribute.mUIHint = uiHint;
		mBinaryValue.reset(val);

		if( caf & CAF_READWRITE )
			this->initValue( val.toTString() );
		return true;
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	typedef Map<Version, ConfigList >		VersionConfig;
	typedef TStringMap< VersionConfig >		ValueRelatedConfig;
	namespace Impl
	{
		class ConfigGroupListImpl : public ValueRelatedConfig, public Allocatable {};
	}

	//////////////////////////////////////////////////////////////////////////
	ConfigGroup::ConfigGroup(EConfigAccessFlag caf/* = CAF_NORMAL*/)
		:ConfigAtom(TString::EMPTY)
		,mData( BLADE_NEW Impl::ConfigGroupListImpl() )
	{
		mAttribute.mAccess = caf;
	}

	//////////////////////////////////////////////////////////////////////////
	ConfigGroup::ConfigGroup(const TString& name,CONFIG_ACCESS caf/* = CAF_NORMAL*/, bool collection/* = false*/)
		:ConfigAtom(name)
		,mData( BLADE_NEW Impl::ConfigGroupListImpl() )
	{
		mAttribute.mAccess = caf;
		mAttribute.mUIHint = CUIH_NONE;
		if (collection)
		{
			mBinaryValue.setCollection();
			mValue = mBinaryValue.toTString();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	ConfigGroup::ConfigGroup(const TString& name,const Variant& val,CONFIG_ACCESS caf/* = CAF_NORMAL*/)
		:ConfigAtom(name, val, caf)
		,mData( BLADE_NEW Impl::ConfigGroupListImpl() )
	{
	}


	//////////////////////////////////////////////////////////////////////////
	ConfigGroup::ConfigGroup(const TString& name,const TStringParam& list,CONFIG_ACCESS caf/* = CAF_NORMAL*/)
		:ConfigAtom(name,list,caf)
		,mData( BLADE_NEW Impl::ConfigGroupListImpl() )
	{
	}

	//////////////////////////////////////////////////////////////////////////
	ConfigGroup::ConfigGroup(const TString& name, const Variant* list, size_t count, CONFIG_ACCESS caf/* = CAF_NORMAL*/)
		:ConfigAtom(name, list, count, caf)
		,mData( BLADE_NEW Impl::ConfigGroupListImpl() )
	{
	}

	//////////////////////////////////////////////////////////////////////////
	ConfigGroup::ConfigGroup(const TString& name, const TStringParam& list, uint32 index, CONFIG_ACCESS caf)
		: ConfigAtom(name, list, index, caf)
		, mData(BLADE_NEW Impl::ConfigGroupListImpl())
	{

	}

	//////////////////////////////////////////////////////////////////////////
	ConfigGroup::ConfigGroup(const TString& name, const Variant* list, size_t count, uint32 index, CONFIG_ACCESS caf)
		: ConfigAtom(name, list, count, index, caf)
		, mData(BLADE_NEW Impl::ConfigGroupListImpl())
	{

	}

	//////////////////////////////////////////////////////////////////////////
	ConfigGroup::~ConfigGroup()
	{

	}

	/************************************************************************/
	/* IConfig interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool			ConfigGroup::setValue(const TString& value, CONFIG_ACCESS verify/* = CAF_READWRITE*/)
	{
		bool ret = ConfigAtom::setValue(value, verify);

		//prepare sub configs immediately, especially for serialization
		if ((verify&CAF_SERIALIZE))
		{
			if (ret && mBinaryValue.isCollection())
				this->updateData(true, false);
		}
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			ConfigGroup::setBinaryValue(const Variant& value, CONFIG_ACCESS verify/* = CAF_READWRITE*/)
	{
		bool ret = ConfigAtom::setBinaryValue(value, verify);

		//prepare sub configs immediately, especially for serialization
		if ((verify&CAF_SERIALIZE))
		{
			if (ret && mBinaryValue.isCollection())
				this->updateData(true, false);
		}
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	void			ConfigGroup::setToDefault()
	{
		if (mValue != mDefaultValue)
		{
			mValue = mDefaultValue;
			mBinaryValue.fromTString(mValue);
			this->notify(CAF_READWRITE);
		}

		for(ValueRelatedConfig::const_iterator it = (*mData).begin(); it != (*mData).end(); ++it )
		{
			const VersionConfig& verConfig = it->second;
			for(VersionConfig::const_iterator i = verConfig.begin(); i != verConfig.end(); ++i )
			{
				i->second.setToDefault();
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	const Version*	ConfigGroup::getLatestVersion() const
	{
		ValueRelatedConfig::const_iterator it = (*mData).find(mBinaryValue.isCollection() ? TString::EMPTY : mValue );
		if( it == (*mData).end() )
			return NULL;
		else
		{
			const VersionConfig& verConfig = it->second;
			if( verConfig.size() == 0 )
				return NULL;
			else
				return &(verConfig.rbegin()->first);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	IConfig::IList*			ConfigGroup::getSubConfigs(Version version/* = Version::MAX*/) const
	{
		ValueRelatedConfig::const_iterator it = (*mData).find( mBinaryValue.isCollection() ? TString::EMPTY : mValue );
		if( it == (*mData).end() || it->second.size() == 0 )
			return NULL;
		else
		{
			const VersionConfig& verConfig = it->second;
			VersionConfig::const_iterator i = (version == Version::MAX) ? --verConfig.end() : verConfig.find( version );
			if( i == verConfig.end() )
				return NULL;
			else
				return const_cast<ConfigList*>(&i->second);	//FIXME: dirty code
		}
	}

	//////////////////////////////////////////////////////////////////////////
	IConfig::IList*			ConfigGroup::getSubConfigs(const TString& RelatedValue, Version version/* = Version::MAX*/) const
	{
		TString Value = RelatedValue;
		if (mBinaryValue.isCollection())
			Value = TString::EMPTY;
		else if( Value == TString::EMPTY)
			Value = mValue;

		ValueRelatedConfig::const_iterator it = (*mData).find( Value );
		if( it == (*mData).end() || it->second.size() == 0 )
			return NULL;
		else
		{
			const VersionConfig& verConfig = it->second;
			VersionConfig::const_iterator i = (version == Version::MAX) ? --verConfig.end() : verConfig.find( version );
			if( i == verConfig.end() )
				return NULL;
			else
				return const_cast<ConfigList*>(&i->second);	//FIXME: dirty code
		}
	}

	//////////////////////////////////////////////////////////////////////////
	IConfig::IList*			ConfigGroup::addOrGetSubConfigs(Version version, const TString& RelatedValue/* = TString::EMPTY*/)
	{
		TString Val = RelatedValue;
		if( mBinaryValue.isCollection() )
		{
			//containers don't support version
			version = Version::MAX;
			Val = TString::EMPTY;
		}

		VersionConfig& verConfig = (*mData)[Val];
		if( version == Version::MAX )
		{
			if( verConfig.size() == 0 )
				//add one minimal version and return
				return &verConfig[Version::MIN];
			else
				//get the latest
				return &verConfig.rbegin()->second;
		}
		else
			return &verConfig[version];
	}

	//////////////////////////////////////////////////////////////////////////
	bool				ConfigGroup::clearSubConfigs()
	{
		mData->clear();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void				ConfigGroup::backupValue()
	{
		mBackup = mValue;

		for(ValueRelatedConfig::const_iterator it = (*mData).begin(); it != (*mData).end(); ++it )
		{
			const VersionConfig& verConfig = it->second;
			for(VersionConfig::const_iterator i = verConfig.begin(); i != verConfig.end(); ++i )
			{
				i->second.backupValue();
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	const TString&		ConfigGroup::getBackupValue() const
	{
		return mBackup;
	}


	//////////////////////////////////////////////////////////////////////////
	void				ConfigGroup::restoreToBackup()
	{
		this->setValue(mBackup);	//restore: add/delete sub configs

		for(ValueRelatedConfig::const_iterator it = (*mData).begin(); it != (*mData).end(); ++it )
		{
			const VersionConfig& verConfig = it->second;
			for(VersionConfig::const_iterator i = verConfig.begin(); i != verConfig.end(); ++i )
			{
				i->second.restoreToBackup();
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool				ConfigGroup::updateData(bool updateChild, bool force/* = false*/)
	{
		bool result;
		if( !mBinaryValue.isCollection() )
			result = this->ConfigAtom::updateData(updateChild, force);
		else
		{
			result = false;
			//update collections
			IDataHandler* handler = mDataHandler;
			size_t elementCount = handler != NULL ? handler->getCount() : 0;
			//if collection is writable, always check elements
			bool forceElements = force || ((mAttribute.mAccess&CAF_COLLECTION_WRITE) == CAF_COLLECTION_WRITE);

			if( handler != NULL)
			{
				ConfigList* list = static_cast<ConfigList*>( this->addOrGetSubConfigs(Version::MAX) );

				//check element reallocation. reallocation will make element config invalid
				bool elementChanged = handler->isElementReallocated(list);
				if (elementChanged)
					list->clear();

				result = list->getCount() != elementCount;

				if (result || forceElements)
				{
					mBinaryValue = (uint32)elementCount;
					mValue = mBinaryValue.toTString();
					size_t originalCount = list->getCount();

					//update element count: create new elements or delete old
					for (size_t i = originalCount; i < elementCount; ++i)
					{
						CONFIG_ACCESS access = CONFIG_ACCESS(mAttribute.mAccess);
						HCONFIG config = handler->createElementConfig(i, access);
						config->setUIHint(mAttribute.mUIHint);
						if(this->getOptions().size() != 0)
							config->setOptions(&this->getOptions()[0], this->getOptions().size());
						//assert(config->getDataHandler() != NULL);
						list->addConfig(config);
					}
					for (size_t i = elementCount; i < originalCount; ++i)
						list->removeConfig(list->getConfig(list->getCount()-1)->getName());

					assert(list->getCount() == elementCount);
				}
				//update element data
				for(size_t i = 0; i < list->getCount(); ++i )
				{
					if( list->getConfig(i)->updateData(false) )
						result = true;
				}
			}
			else
				result = true;
		}

		if( updateChild )
		{
			for(ValueRelatedConfig::const_iterator it = (*mData).begin(); it != (*mData).end(); ++it )
			{
				const VersionConfig& verConfig = it->second;
				for(VersionConfig::const_iterator i = verConfig.begin(); i != verConfig.end(); ++i )
				{
					i->second.updateData(updateChild, force);
				}
			}
		}
		return result;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				ConfigGroup::bindTarget(Bindable* target)
	{
		//bind this if has a handler attached
		this->ConfigAtom::bindTarget(target);

		bool result = true;
		for(ValueRelatedConfig::const_iterator it = (*mData).begin(); it != (*mData).end(); ++it )
		{
			const VersionConfig& verConfig = it->second;
			for(VersionConfig::const_iterator i = verConfig.begin(); i != verConfig.end(); ++i )
			{
				if( !i->second.bindTarget(target) )
					result = false;
			}
		}
		return result;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				ConfigGroup::synchronize(Bindable* target) const
	{
		bool result = target != NULL;
		if( result )
		{
			for(ValueRelatedConfig::const_iterator it = (*mData).begin(); it != (*mData).end(); ++it )
			{
				const VersionConfig& verConfig = it->second;
				for(VersionConfig::const_iterator i = verConfig.begin(); i != verConfig.end(); ++i )
				{
					if( !i->second.synchronize(target) )
						result = false;
				}
			}
		}
		return result;
	}

	//////////////////////////////////////////////////////////////////////////
	HCONFIG						ConfigGroup::clone(CONFIG_ACCESS caf/* = CAF_NONE*/, const TString& name/* = TString::EMPTY*/) const
	{
		ConfigGroup* group = BLADE_NEW ConfigGroup(name == TString::EMPTY ? mName : name, caf == CAF_NONE ? mAttribute.mAccess : caf );
		group->mOptions = this->mOptions;
		group->mAttribute.mUIHint = this->mAttribute.mUIHint;
		group->mBackup = this->mBackup;
		group->mBinaryValue.reset( this->mBinaryValue );
		group->mDefaultValue = this->mDefaultValue;
		group->mDataHandler = mDataHandler;
		group->mValue = this->mValue;
		*(group->mData) = *mData;

		//no binding allowed on clone
		assert( mDataHandler == NULL || mDataHandler->getTarget() == NULL );
		//collection's sub element configs should be empty, since there's no binding target
		//if the assertion below failed, maybe the collection's updateData() is not called after directly call IDataHandler::bindTarget(NULL)
		//TODO: make IDataHandler::bindTarget & IConfig::updateData a built-in serial call
		//assert( (!mBinaryValue.isCollection() || this->getSubConfigCount() == 0) );	//array collection can have sub element configs

		if( group->mDataHandler != NULL )
			group->mDataHandler = group->mDataHandler->clone();

		for(ValueRelatedConfig::iterator it = (*group->mData).begin(); it != (*group->mData).end(); ++it )
		{
			VersionConfig& verConfig = it->second;
			for(VersionConfig::iterator i = verConfig.begin(); i != verConfig.end(); ++i )
			{
				ConfigList& list = i->second;
				list.makeClone(caf);
			}
		}
		return HCONFIG(group);
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	
}//namespace Blade