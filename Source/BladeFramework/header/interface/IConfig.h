/********************************************************************
	created:	2009/03/29
	filename: 	IConfig.h
	author:		Crazii
	
	purpose:	
	log:		add new method to backup/restore values
				and also a method to set to default

				2011/11/26 now IConfgurable acts not only as an adapter for ui & data, \n
				but also as an adapter for serialization & data
*********************************************************************/
#ifndef __Blade_IConfig_h__
#define __Blade_IConfig_h__
#include <Handle.h>
#include <utility/StringList.h>
#include <utility/Version.h>
#include <utility/Delegate.h>
#include <BladeFramework.h>

namespace Blade
{

	/** @brief IConfig types enum */
	/* the type is optional, it is useful to auto generate UI controls */
	enum EConfigUIHint
	{
		CUIH_NONE,
		CUIH_START = CUIH_NONE,
		CUIH_LIST,			///list of strings(default type)
		CUIH_INDEX,			///list of strings for index value

		CUIH_INPUT,			///normal input
		CUIH_INPUT_RANGE,	///single number with range limit and with normal input box
		CUIH_RANGE,			///list of strings using range
		CUIH_COLOR,			///color picking
		CUIH_FILE,			///file path
		CUIH_PATH,			///directory path
		CUIH_IMAGE,
		CUIH_CHECK,
		CUIH_CUSTOM,		///invoke custom UI to config: UIHint contains the delegate of custom function: call to custom config routine
							//you should implement a model dialog in the custom routine, and return a result string, the result is customized and can be empty.
		
		//add new type here
		//CUIH_RADIO,

		//
		CUIH_COUNT,
		CUIH_DEFAULT = CUIH_LIST,
	};

	class BLADE_FRAMEWORK_API IConfigUIData
	{
	public:
		virtual ~IConfigUIData()	{}
	};
	typedef Handle<IConfigUIData> HCONFIGUIDATA;

	typedef class ConfigUIHint
	{
	public:
		Delegate		mCustomCall;		//CUIH_CUSTOM
		EConfigUIHint	mHint;
		TString			mUserData;
		HCONFIGUIDATA	mUIData;			//reserved for UI module, this is a little bit dirty.TODO: make it better

		ConfigUIHint(EConfigUIHint hint)
			:mHint(hint)		{}

		ConfigUIHint(EConfigUIHint hint, const TString& userdata)
			:mHint(hint),mUserData(userdata)	{}

		ConfigUIHint(const ConfigUIHint& hint)
			:mCustomCall(hint.mCustomCall),mHint(hint.mHint),mUserData(hint.mUserData),mUIData(hint.mUIData)	{}

		operator EConfigUIHint() const					{return mHint;}
		bool operator==(EConfigUIHint hint) const		{return mHint == hint;}
		ConfigUIHint&	operator=(EConfigUIHint hint)	{mHint = hint;return *this;}
		const TString&	getUserData() const				{return mUserData;}
		const Delegate&	getCustomMethod() const			{return mCustomCall;}
	}CONFIG_UIHINT;//class ConfigUIHint


	typedef enum EConfigAccessFlag
	{
		CAF_NONE		= 0x00,
		CAF_READ		= 0x01,					///whether visible to the UI
		CAF_WRITE		= 0x02,					///whether configurable through UI
		CAF_READWRITE	= CAF_WRITE | CAF_READ,	///whether visible or configurable through UI
		CAF_SERIALIZE	= 0x04,
		CAF_NOCASCADE	= 0x8,					///don't inherit the 'read only' access from parent config (group): 
												///by default if parent is read only, this config will be read only too, otherwise this use its own access
												///use this flag to ignore parent's access
												///if parent is not ready only and child is read only, this flag doesn't take any effect.

		CAF_COLLECTION_WRITE = 0x10 | CAF_WRITE,///CAF_WRITE for collection will only affect elements in collection. 
												///CAF_COLLECTION_WRITE makes collection itself writable: element count can be changed.
		CAF_DEVMODE		= 0x20,					///developer access, only visible on developer mode see @CosntDef::EnvBool::DEVELOPER_MODE

		CAF_NORMAL		= CAF_READWRITE | CAF_SERIALIZE,
		CAF_NONE_CONFIG	= CAF_READ | CAF_SERIALIZE,
		CAF_NON_VISIBLE = CAF_SERIALIZE,		///none-configurable that only support serialization

		CAF_NORMAL_DEVMODE = CAF_NORMAL | CAF_DEVMODE,
		CAF_READ_DEVMODE = CAF_READ | CAF_DEVMODE,
		CAF_WRITE_DEVMODE = CAF_WRITE | CAF_DEVMODE,
		CAF_NONE_CONFIG_DEVMODE = CAF_NONE_CONFIG | CAF_DEVMODE,
	}CONFIG_ACCESS;

	typedef struct SConfigAttribute
	{
		CONFIG_UIHINT		mUIHint;
		CONFIG_ACCESS		mAccess;
		SConfigAttribute()	:mUIHint(CUIH_INPUT),mAccess(CAF_NORMAL)	{}
	}CONFIG_ATTR;

	class Bindable;
	class Variant;

	class IConfig
	{
	public:
		typedef TStringList Option;

		class IList
		{
		public:
			/** @brief get the number config */
			virtual size_t		getCount() const = 0;

			/** @brief  */
			virtual const HCONFIG&getConfig(index_t index) const = 0;

			/** @brief  */
			virtual const HCONFIG&getConfig(const TString& name) const = 0;

			/** @brief  */
			virtual bool		addConfig(const HCONFIG& config) = 0;

			/** @brief a internal smart pointer will create to bind the naked pointer
			* so DO NOT bind it to a smart pointer outside,nor delete it outside. */
			inline bool addConfigPtr(IConfig* config)
			{
				if (config == NULL)
					return false;
				return this->addConfig(HCONFIG(config));
			}

			/** @brief  */
			virtual bool		removeConfig(const TString& name) = 0;
		};

		///the real-time notification class
		///the object is informed instantly when the config changes
		///for loading/saving, the target data is get/set through onConfigChange()
		class IDataHandler
		{
		public:
			virtual ~IDataHandler()	{}

			/** @brief config's value is changed, maybe update handler's value from config */
			virtual bool	setData(const TString& name, const void* binary, size_t bytes, CONFIG_ACCESS verify, index_t index = INVALID_INDEX) const = 0;

			/** @brief get the handler's binary data in memory (index is ignored
			 * or get sub element data specified by index */
			virtual bool	getData(void* binary, size_t bytes, index_t index = INVALID_INDEX) const = 0;

			/** @brief  */
			virtual bool	compareData(const void* binary, size_t bytes, index_t index = INVALID_INDEX) const = 0;

			/** @brief get sub element count */
			virtual size_t	getCount() const = 0;

			/** @brief used for collection only data to prepare elements */
			virtual void	prepareData(size_t count) const = 0;

			/** @brief  */
			virtual void	bindTarget(Bindable* target) = 0;

			/** @brief  */
			virtual Bindable* getTarget() const = 0;

			/** @brief  */
			virtual void	addDelegate(const Delegate& d) = 0;

			/** @brief  */
			virtual bool	hasDelegate() const = 0;

			/** @brief create config for elements, bound to an element by index */
			/* note: this function is valid only when getCount() > 0 */
			virtual HCONFIG	createElementConfig(index_t index, CONFIG_ACCESS caf) const { assert(false); BLADE_UNREFERENCED(index); BLADE_UNREFERENCED(caf); return HCONFIG::EMPTY; }

			/** @brief  */
			/* note: this function is valid only when getCount() > 0 */
			virtual bool	isElementReallocated(IList* elementConfigList) const { BLADE_UNREFERENCED(elementConfigList);return false; }

			/** @brief  */
			virtual Handle<IDataHandler> clone() const = 0;
		};
		typedef Handle<IDataHandler> HHANDLER;

		class IVisitor
		{
		public:
			virtual void visit(IConfig* config) = 0;
			virtual ~IVisitor() {}
		};

	public:
		/** @brief virtual dector */
		virtual ~IConfig()	{}

		/** @brief get the configurable object's name */
		virtual const TString&	getName() const = 0;

		/** @brief get the configurable attribute */
		virtual const CONFIG_ATTR&getAttribute() const = 0;

		/** @brief  */
		virtual void			setUIHint(const CONFIG_UIHINT& hint) = 0;

		/** @brief  */
		virtual void			setReadOnly(bool readyOnly) = 0;

		/** @brief get option */
		virtual const Option&	getOptions() const = 0;

		/** @brief reset option */
		virtual bool			setOptions(const TString* opts, size_t count, index_t newIndex = INVALID_INDEX) = 0;

		/** @brief get the value of this configurable object */
		/* @remark return value could be NULL string, especially type is CT_NONE
		*/
		virtual const TString&	getValue() const = 0;

		/** @brief get default(initial) value of this config */
		virtual const TString&	getDefaultValue() const = 0;

		/** @brief  */
		virtual const Variant&	getBinaryValue() const = 0;

		/** @brief set the value of this configurable object */
		virtual bool			setValue(const TString& value, CONFIG_ACCESS verify = CAF_READWRITE) = 0;

		/** @brief  */
		virtual bool			setBinaryValue(const Variant& value, CONFIG_ACCESS verify = CAF_READWRITE) = 0;

		/** @brief  */
		virtual void			setToDefault() = 0;

		/** @brief  */
		virtual const Version*	getLatestVersion() const = 0;

		/** @brief use Version::MAX to get the latest version */
		virtual IList*			getSubConfigs(Version version = Version::MAX) const = 0;

		/** @brief use Version::MAX to get the latest version  */
		virtual IList*			getSubConfigs(const TString& RelatedValue, Version version = Version::MAX) const = 0;

		/** @brief  */
		virtual IList*			addOrGetSubConfigs(Version version, const TString& RelatedValue = TString::EMPTY) = 0;

		/**
		@describe remove all
		@param 
		@return 
		*/
		virtual bool			clearSubConfigs() = 0;

		/**
		@describe	backup current value , this allows rolling back if config changes via GUI,when GUI canceled applying.
		@param 
		@return 
		*/
		virtual void			backupValue() = 0;

		/** @brief  */
		virtual const TString&	getBackupValue() const = 0;

		/** @brief  */
		virtual void			restoreToBackup() = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void			setDataHandler(const HHANDLER& handler) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const HHANDLER&	getDataHandler() const = 0;

		/**
		@describe update data from data handler
		@param 
		@return true if the data source data is dirty & data updated
		*/
		virtual bool			updateData(bool updateChild, bool force = false) = 0;

		/**
		@describe bind data handler to a data target
		@param 
		@return 
		@note: be careful: this function will bind all sub config to the same target, but the config may 
		contain sub config of another type.
		if there's a data source, use the data source's bind function instead of this one
		use this one only when there's a config, and data source containing this config
		*/
		virtual bool			bindTarget(Bindable* target) = 0;

		/**
		@describe update data into target
		@param 
		@return 
		*/
		virtual bool			synchronize(Bindable* target) const = 0;

		/**
		@describe clone config object
		@param [in] caf: the new access of cloned object, specify CAF_NONE to remain access unchanged
		@param [in] name: the new name of cloned instance.
		@return 
		@note: the data handler object is cloned too.
		*/
		virtual HCONFIG			clone(CONFIG_ACCESS caf = CAF_NONE, const TString& name = TString::EMPTY) const = 0;

		/** @brief  */
		inline const CONFIG_UIHINT&	getUIHint() const	{return this->getAttribute().mUIHint;}
		inline CONFIG_ACCESS		getAccess() const	{return this->getAttribute().mAccess;}
		inline bool					isReadOnly() const	{return (this->getAccess() & CAF_WRITE) == 0;}

		/** @brief get the number sub config */
		inline size_t			getSubConfigCount(Version version = Version::MAX) const
		{
			const IList* list = this->getSubConfigs(version);
			return list == NULL ? 0 : list->getCount();
		}
		inline const HCONFIG&	getSubConfig(index_t index, Version version = Version::MAX) const
		{
			const IList* list = this->getSubConfigs(version);
			return list == NULL ? HCONFIG::EMPTY : list->getConfig(index);
		}
		inline const HCONFIG&	getSubConfigByName(const TString& name, Version version = Version::MAX) const
		{
			const IList* list = this->getSubConfigs(version);
			return list == NULL ? HCONFIG::EMPTY : list->getConfig(name);
		}
		inline const HCONFIG&	findConfig(const TString& name, Version version = Version::MAX) const
		{
			TStringParam options = this->getOptions();
			if (options.size() == 0)
				options.push_back(TString::EMPTY);
			else
			{
				//option based configs may have configs with the same name
				//search & choose current value first if there are multiple entry
				index_t index = options.find(this->getValue());
				if (index != INVALID_INDEX && index != 0)
					std::swap(options[index], options[0]);
			}

			for (size_t j = 0; j < options.size(); ++j)
			{
				const IList* list = this->getSubConfigs(options[j], version);
				const HCONFIG& ret = list == NULL ? HCONFIG::EMPTY : list->getConfig(name);
				if (ret != NULL)
					return ret;
				if (list != NULL)
				{
					for (size_t i = 0; i < list->getCount(); ++i)
					{
						const HCONFIG& subret = list->getConfig(i)->findConfig(name);
						if (subret != NULL)
							return subret;
					}
				}
			}
			return HCONFIG::EMPTY;
		}
		inline void traverse(IVisitor* visitor, Version version = Version::MAX)
		{
			TStringParam options = this->getOptions();
			if (options.size() == 0)
				options.push_back(TString::EMPTY);

			for (size_t j = 0; j < options.size(); ++j)
			{
				const IList* list = this->getSubConfigs(options[j], version);
				if (list != NULL)
				{
					for (size_t i = 0; i < list->getCount(); ++i)
					{
						IConfig* cfg = list->getConfig(i);
						visitor->visit(cfg);
						cfg->traverse(visitor, version);
					}
				}
			}
		}

		/**
		@describe add a sub config
		@param [in] config a pointer to IConfig
		@param [in] RelatedValue self value IConfig object is related with
		@return true if succeed
		@remark theoretically, each value could be mapped with a set of config \n
		* i.e. add a sub config with a related value "TRUE",then the sub config is only available \n
		* only when this object 's value is "TRUE", if this object's value is not "TRUE"
		* the added related sub config will not be contained in getSubConfig() result
		*/
		inline bool				addSubConfig(const HCONFIG& config, const TString& RelatedValue = TString::EMPTY, Version version = Version::MAX)
		{
			IList* list = this->addOrGetSubConfigs(version, RelatedValue);
			return list != NULL && list->addConfig(config);
		}

		/** @brief a internal smart pointer will create to bind the naked pointer,so DO NOT bind it to a smart pointer outside,nor delete it outside. */
		inline bool				addSubConfigPtr(IConfig* config, const TString& RelatedValue = TString::EMPTY, Version version = Version::MAX)
		{
			if( config == NULL )
				return false;
			return this->addSubConfig(HCONFIG(config),RelatedValue);
		}

		/** @brief  */
		inline bool				removeSubConfig(const TString& name,const TString& RelatedValue = TString::EMPTY, Version version = Version::MAX)
		{
			IList* list = this->getSubConfigs(RelatedValue, version);
			return list == NULL ? false : list->removeConfig(name);
		}

	};//class IConfig
	typedef IConfig::HHANDLER HCDHANDLER;

	/** @brief  */
	struct FnHConfigLess
	{
		bool operator()(const HCONFIG& lhs,const HCONFIG& rhs) const
		{
			return FnTStringFastLess::compare(lhs->getName(),rhs->getName());
		}
	};

	struct FnHConfigFindByName
	{
		FnHConfigFindByName(const TString& toFind) :targetRef(toFind)	{}

		bool operator()(const HCONFIG& config)
		{
			return config->getName() == targetRef;
		}
	private:
		const TString& targetRef;
		FnHConfigFindByName& operator=(const FnHConfigFindByName&);
	};
	
}//namespace Blade

#endif // __Blade_IConfig_h__