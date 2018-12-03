/********************************************************************
	created:	2010/03/25
	filename: 	ConfigTypes.h
	author:		Crazii
	
	purpose:	IConfig interface implementations, using GOF 's composite pattern to setup a tree structure
*********************************************************************/
#ifndef __Blade_ConfigTypes_h__
#define __Blade_ConfigTypes_h__
#include <utility/StringList.h>
#include <utility/Variant.h>
#include <BladeFramework.h>
#include <interface/IConfig.h>

namespace Blade
{
	class Vector4;
	class Quaternion;

	class BLADE_FRAMEWORK_API ConfigBase : public IConfig
	{
	public:
		//ConfigBase();
		ConfigBase(const TString& name);
		//ConfigBase(const TString& name,const TString& value);
		virtual ~ConfigBase() = 0;

		/** @brief get the configurable object's name */
		virtual const TString&	getName() const;

		/** @brief get the configurable attribute */
		virtual const CONFIG_ATTR&getAttribute() const;

		/** @brief  */
		virtual void			setUIHint(const CONFIG_UIHINT& hint);

		/** @brief  */
		virtual void			setReadOnly(bool readyOnly);

		/** @brief get option */
		virtual const Option&	getOptions() const		{return mOptions;}

		/** @brief reset option */
		virtual bool			setOptions(const TString* opts, size_t count, index_t newIndex = INVALID_INDEX);

		/** @brief get the value of this configurable object */
		/* @remark return value could be NULL string ,especially type is CT_NONE
		*/
		virtual const TString&	getValue() const;

		/** @brief get default(initial) value of this config */
		virtual const TString&	getDefaultValue() const;

		/** @brief  */
		virtual const Variant&	getBinaryValue() const;

		/** @brief set the value of this configurable object */
		virtual bool			setValue(const TString& value, CONFIG_ACCESS verify = CAF_READWRITE);

		/** @brief  */
		virtual bool			setBinaryValue(const Variant& value, CONFIG_ACCESS verify = CAF_READWRITE);

		/** @brief  */
		virtual void			setDataHandler(const HHANDLER& handler)	{mDataHandler = handler;}

		/** @brief  */
		virtual const HHANDLER&	getDataHandler() const	{return mDataHandler;}

		/**
		@describe update data from data handler
		@param 
		@return 
		*/
		virtual bool			updateData(bool updateChild, bool force = false);

		/**
		@describe bind data handler to a data target
		@param 
		@return 
		*/
		virtual bool			bindTarget(Bindable* target);

		/**
		@describe update data into target
		@param 
		@return 
		*/
		virtual bool			synchronize(Bindable* target) const;

	protected:
		/** @brief  */
		bool				initValue(const TString& val, bool addToOption = true, size_t optionCount = 1);
		/** @brief  */
		bool				addOption(const TString& option);
		/** @brief  */
		void				forceAddOption(const TString& option);
		/** @brief notify the data handler */
		void				notify(CONFIG_ACCESS verify) const;
		/** @brief  */
		bool				isDirty() const;

		/** @brief the name of this config */
		TString				mName;
		/** @brief the value of this config,in string format */
		TString				mValue;
		/** @brief this stores the default value */
		TString				mDefaultValue;
		/** @brief this stores the back up value,if values changes,it can still roll back */
		TString				mBackup;
		/** @brief  */
		Variant			mBinaryValue;

		/** @brief the range of this config,in string format */
		Option				mOptions;
		HHANDLER			mDataHandler;

		/** @brief the type of this config */
		CONFIG_ATTR			mAttribute;
	};//class ConfigBase

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class BLADE_FRAMEWORK_API ConfigAtom : public ConfigBase, public Allocatable
	{
	public:
		class Range
		{
		public:
			inline Range() {}
			inline explicit Range(const Variant& min, const Variant& max, const Variant& step, const Variant& val)
			{
				this->init(min, max, step, val);
			}
			inline explicit Range(const Variant& min, const Variant& max, const Variant& step)
			{
				this->init(min, max, step, min);
			}

			/** @brief  */
			inline void reset()
			{
				mMin.reset();  mMax.reset();  mStep.reset(); mVal.reset();
			}

			inline void init(const Variant& min, const Variant& max, const Variant& step, const Variant& val)
			{
				assert(min.getType() == max.getType() && max.getType() == step.getType() && step.getType() == val.getType());
				//assert(min.getSize() == max.getSize() && max.getSize() == step.getSize() && step.getSize() == val.getSize());
				assert(min.getType() == VI_UINT || min.getType() == VI_INT || min.getType() == VI_FLOAT);
				mMin.reset(min); mMax.reset(max); mStep.reset(step); mVal.reset(val);
			}

			BLADE_FRAMEWORK_API void generateOptions(TStringParam& options);

			Variant mMin;
			Variant	mMax;
			Variant mStep;
			Variant mVal;
		};

	public:
		ConfigAtom(const TString& name);

		//common string value (input)
		//ConfigAtom(const TString& name,const TString& value,CONFIG_ACCESS caf = CAF_NORMAL);
		
		///input range
		/// UI hint is a CUIH_INPUT_RANGE
		///note: eAccess is forced appended the CAF_CONFIG flag because range input must be write-able or it should be a single valued data
		ConfigAtom(const TString& name, const Range& rangeDef, CONFIG_ACCESS caf = CAF_NORMAL);

		///CUIH_LIST: list of string (single selection)
		ConfigAtom(const TString& name,const TStringParam& list, CONFIG_ACCESS caf);
		///CUIH_INDEX: index instead of list content string
		ConfigAtom(const TString& name,const TStringParam& list, uint32 index, CONFIG_ACCESS caf);
		///CUIH_LIST: list (of any type)
		ConfigAtom(const TString& name, const Variant* list, size_t count, CONFIG_ACCESS caf);
		///CUIH_INDEX: index (of any type)
		ConfigAtom(const TString& name, const Variant* list, size_t count, uint32 index, CONFIG_ACCESS caf);

		///CUIH_INPUT or read-only or pure serialization support
		///CUIH_COLOR/CUIH_CHECK based on variable type
		ConfigAtom(const TString& name, const Variant& val, CONFIG_ACCESS caf = CAF_NORMAL);

		~ConfigAtom();

		/**
		@describe set to default values, this behavior depend on sub class implementation
		@param 
		@return 
		*/
		virtual void			setToDefault();

		/** @brief  */
		virtual const Version*	getLatestVersion() const				{return NULL;}

		/** @brief  */
		virtual IList*			getSubConfigs(Version version ) const	{return NULL;}

		/** @brief  */
		virtual IList*			getSubConfigs(const TString& /*RelatedValue*/, Version /*version = Version::MAX*/) const {return NULL;}

		/** @brief  */
		virtual IList*			addOrGetSubConfigs(Version version, const TString& ){return NULL;}
		
		/** @brief  */
		inline virtual bool		setOptions(const TString* opts, size_t count, index_t newIndex = INVALID_INDEX)
		{
			return ConfigBase::setOptions(opts, count, newIndex);
		}

		/**
		@describe remove all
		@param 
		@return 
		*/
		virtual bool			clearSubConfigs()								{return true;}

		/**
		@describe	backup current value, this allows rolling back if config changes via GUI, when GUI canceled applying.
		@param 
		@return 
		*/
		virtual void			backupValue();

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const TString&	getBackupValue() const;

		/**
		@describe restore backuped value
		@param 
		@return 
		*/
		virtual void			restoreToBackup();

		/**
		@describe clone config object
		@param [in] caf: the new access of cloned object, specify CAF_NONE to remain access unchanged
		@return 
		@note: the data handler object is NOT cloned (will be set to NULL), because we may not simply clone it.
		*/
		virtual HCONFIG			clone(CONFIG_ACCESS caf = CAF_NONE, const TString& name = TString::EMPTY) const;


		/** @brief common string value, this is delay init of default constructor */
		//CUIH_INPUT, CUIH_FILE, CUIH_PATH, CUIH_IMAGE, CUIH_NONE
		bool		initStringValue(const TString& val, CONFIG_UIHINT uiHint, CONFIG_ACCESS caf = CAF_NORMAL);

		/** @brief  */
		bool		initBinaryValue(const Variant& val, CONFIG_UIHINT uiHint, CONFIG_ACCESS caf = CAF_NORMAL);

		/** @brief add string option for CUIH_LIST/CUIH_INDEX type
		if the original value not in the new options, set the new value as the string indexed at newVal in the list */
		/* @note the config type should be common string value */
		inline bool	setOptions(const TStringParam& list, index_t newVal = INVALID_INDEX)
		{
			const TString* arrayStr = list.getBuffer();
			return ConfigBase::setOptions(arrayStr, list.size(), newVal);
		}

		inline bool setOptions(const Variant* list, size_t count, index_t newVal = INVALID_INDEX)
		{
			TStringParam strList;
			strList.reserve(count);
			for (size_t i = 0; i < count; ++i)
				strList.push_back(list[i].toTString());
			return this->setOptions(strList, newVal);
		}
		
	protected:

	};



	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	namespace Impl
	{
		class ConfigGroupListImpl;
	}

	class BLADE_FRAMEWORK_API ConfigGroup : public ConfigAtom
	{
	public:
		
		/* @note the caf flag is forced appended the CAF_CONFIG flag because the ConfigGroup is need to be write-able  */
		ConfigGroup(EConfigAccessFlag caf = CAF_NORMAL);
		ConfigGroup(const TString& name, CONFIG_ACCESS caf = CAF_NORMAL, bool collection = false);
		ConfigGroup(const TString& name,const Variant& val,CONFIG_ACCESS caf = CAF_NORMAL);
		ConfigGroup(const TString& name,const TStringParam& list,CONFIG_ACCESS caf = CAF_NORMAL);
		ConfigGroup(const TString& name, const Variant* list, size_t count, CONFIG_ACCESS caf = CAF_NORMAL);
		ConfigGroup(const TString& name, const TStringParam& list, uint32 index, CONFIG_ACCESS caf);
		ConfigGroup(const TString& name, const Variant* list, size_t count, uint32 index, CONFIG_ACCESS caf);

		~ConfigGroup();

		/************************************************************************/
		/* IConfig interface                                                                     */
		/************************************************************************/
		/** @brief set the value of this configurable object */
		virtual bool			setValue(const TString& value, CONFIG_ACCESS verify = CAF_READWRITE);

		/** @brief  */
		virtual bool			setBinaryValue(const Variant& value, CONFIG_ACCESS verify = CAF_READWRITE);

		/**
		@describe set to default values,this behavior depend on sub class implementation
		@param 
		@return 
		*/
		virtual void			setToDefault();

		/** @brief  */
		virtual const Version*	getLatestVersion() const;

		/** @brief  */
		virtual IList*			getSubConfigs(Version version = Version::MAX) const;

		/** @brief  */
		virtual IList*			getSubConfigs(const TString& RelatedValue, Version version = Version::MAX) const;

		/** @brief  */
		virtual IList*			addOrGetSubConfigs(Version version, const TString& RelatedValue = TString::EMPTY);

		/**
		@describe remove all
		@param 
		@return 
		*/
		virtual bool			clearSubConfigs();

		/**
		@describe	backup current value , this allows rolling back if config changes via GUI,when GUI canceled applying.
		@param 
		@return 
		*/
		virtual void			backupValue();

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const TString&	getBackupValue() const;

		/**
		@describe restore backuped value
		@param 
		@return 
		*/
		virtual void			restoreToBackup();

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool			updateData(bool updateChild, bool force = false);

		/**
		@describe bind data handler to a data target
		@param 
		@return 
		*/
		virtual bool			bindTarget(Bindable* target);

		/**
		@describe update data into target
		@param 
		@return 
		*/
		virtual bool			synchronize(Bindable* target) const;

		/**
		@describe clone config object
		@param [in] caf: the new access of cloned object, specify CAF_NONE to remain access unchanged
		@return 
		@note: the data handler object is NOT cloned (will be set to NULL), because we may not simply clone it.
		*/
		virtual HCONFIG			clone(CONFIG_ACCESS caf = CAF_NONE, const TString& name = TString::EMPTY) const;

		/** @brief  */
		using ConfigBase::addOption;

	private:
		Pimpl<Impl::ConfigGroupListImpl>	mData;

		ConfigGroup(const ConfigGroup&);
		ConfigGroup& operator=(const ConfigGroup&);
	};//class ConfigGroup
	
}//namespace Blade

#endif //__Blade_ConfigTypes_h__