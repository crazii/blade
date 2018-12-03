/********************************************************************
	created:	2011/09/17
	filename: 	ConfigDataHandler.h
	author:		Crazii
	purpose:	this file is only for final data binding usage; DO NOT include it in a public header
*********************************************************************/
#ifndef __Blade_ConfigDataHandler_h__
#define __Blade_ConfigDataHandler_h__
#include <Pimpl.h>
#include <utility/DataBindingEx.h>
#include <utility/DataBridge.h>
#include <interface/IConfig.h>
#include <databinding/IDataSourceDataBase.h>

namespace Blade
{
	/************************************************************************/
	/* config handler wrap                                                                      */
	/************************************************************************/
	class BLADE_FRAMEWORK_API ConfigDataHandler : public IConfig::IDataHandler, public Allocatable
	{
	public:
		template<typename T, typename V>
		ConfigDataHandler(V T::*pmdata, T* ptr = NULL, void (T::*pmEvent)(void*) = NULL)
			:mBridge(pmdata, ptr, pmEvent)	{}

		template<typename T, typename V>
		ConfigDataHandler(const V& (T::*getter)(index_t) const, T* ptr = NULL, bool (T::*setter)(index_t, const V&) = NULL)
			:mBridge(getter, ptr, setter) {}

		ConfigDataHandler(const DataBinding& binding)
			:mBridge(binding) {}

		~ConfigDataHandler()				{}

		/************************************************************************/
		/* IConfig::IDataHandler interface                                                                     */
		/************************************************************************/
		/** @brief config's value is changed, maybe update handler's value from config */
		virtual bool	setData(const TString& name, const void* binary, size_t bytes, CONFIG_ACCESS verify, index_t index = INVALID_INDEX) const;

		/** @brief get the handler's binary data in memory */
		virtual bool	getData(void* binary, size_t bytes, index_t index = INVALID_INDEX) const;

		/** @brief  */
		virtual bool	compareData(const void* binary, size_t bytes, index_t index = INVALID_INDEX) const;

		/** @brief get sub element count */
		virtual size_t	getCount() const	{return 0;}

		/** @brief  */
		virtual void	prepareData(size_t /*count*/) const {}

		/** @brief  */
		virtual void	bindTarget(Bindable* target);

		/** @brief  */
		virtual Bindable* getTarget() const;

		/** @brief  */
		virtual void	addDelegate(const Delegate& d) { this->getBridge().addDelegate(d); }

		/** @brief  */
		virtual bool	hasDelegate() const { return this->getBridge().getDelegates().size() > 0; }

		/** @brief  */
		virtual HCDHANDLER clone() const;

		/************************************************************************/
		/* custom method                                                                     */
		/************************************************************************/
	protected:
		/** @brief  */
		DataBridge&		getBridge() { return mBridge; }
		const DataBridge&	getBridge() const { return mBridge; }

		DataBridge	mBridge;
	};//class ConfigDataHandler

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	template<typename> class Vector;
	class BLADE_FRAMEWORK_API CollectionConfigDataHandler : public IConfig::IDataHandler, public Allocatable
	{
	protected:
		CollectionConfigDataHandler()		{}
	public:
		template<typename T>
		CollectionConfigDataHandler(const DataBinding& binding, T* t, TStringList* v)
			:mBinding(binding, t, v)	{mElementSignature = DS_SIGNATURE(TString); mIsArray = false; mReallocationCheckCount = size_t(-1);}

		template<typename T, typename V>
		CollectionConfigDataHandler(const DataBinding& binding, T* t, Vector<V>* v)
			:mBinding(binding, t, v)	{mElementSignature = DS_SIGNATURE(V); mIsArray = false; mReallocationCheckCount = std::is_pointer<V>::value ? size_t(-1) : 1;}

		template<typename T, typename V>
		CollectionConfigDataHandler(const DataBinding& binding, T* t, List<V>* v)
			:mBinding(binding, t, v)	{ mElementSignature = DS_SIGNATURE(V); mIsArray = false; mReallocationCheckCount = size_t(-1); }

		template<typename T, typename V, typename C>
		CollectionConfigDataHandler(const DataBinding& binding, T* t, Set<V,C>* v)
			:mBinding(binding, t, v)	{mElementSignature = DS_SIGNATURE(V);mIsArray = false; mReallocationCheckCount = size_t(-1);}

		template<typename T, typename V, size_t N>
		CollectionConfigDataHandler(const DataBinding& binding, T* t, V(*v)[N])
			:mBinding(binding, t, v)	{mElementSignature = DS_SIGNATURE(V);mIsArray = true; mReallocationCheckCount = std::is_pointer<V>::value ? size_t(-1) : 0;}



		template<typename T>
		CollectionConfigDataHandler(TStringList T::*pmdata, T* ptr = NULL, void (T::*pmEvent)(void*) = NULL)
			:mBinding(ptr, pmdata, pmEvent)	{mElementSignature = DS_SIGNATURE(TString); mIsArray = false; mReallocationCheckCount = size_t(-1);}

		template<typename T, typename V>
		CollectionConfigDataHandler(Vector<V> T::*pmdata, T* ptr = NULL, void (T::*pmEvent)(void*) = NULL)
			:mBinding(ptr, pmdata, pmEvent)	{mElementSignature = DS_SIGNATURE(V); mIsArray = false; mReallocationCheckCount = std::is_pointer<V>::value ? size_t(-1) : 1;}

		template<typename T, typename V>
		CollectionConfigDataHandler(List<V> T::*pmdata, T* ptr = NULL, void (T::*pmEvent)(void*) = NULL)
			:mBinding(ptr, pmdata, pmEvent)	{mElementSignature = DS_SIGNATURE(V); mIsArray = false; mReallocationCheckCount = size_t(-1);}

		template<typename T, typename V, typename C>
		CollectionConfigDataHandler(Set<V,C> T::*pmdata, T* ptr = NULL, void (T::*pmEvent)(void*) = NULL)
			:mBinding(ptr, pmdata, pmEvent)	{mElementSignature = DS_SIGNATURE(V);mIsArray = false; mReallocationCheckCount = size_t(-1);}

		template<typename T, typename V, size_t N>
		CollectionConfigDataHandler(V(T::*pmdata)[N], T* ptr = NULL, void (T::*pmEvent)(void*) = NULL)
			:mBinding(ptr, pmdata, pmEvent)	{mElementSignature = DS_SIGNATURE(V);mIsArray = true; mReallocationCheckCount = std::is_pointer<V>::value ? size_t(-1) : 0;}

		~CollectionConfigDataHandler()		{}

		/************************************************************************/
		/* IConfig::IDataHandler interface                                                                     */
		/************************************************************************/
		/** @brief config's value is changed, maybe update handler's value from config */
		virtual bool	setData(const TString& name, const void* binary, size_t bytes, CONFIG_ACCESS verify, index_t index = INVALID_INDEX) const;

		/** @brief get the handler's binary data in memory */
		virtual bool	getData(void* binary, size_t bytes, index_t index = INVALID_INDEX) const;

		/** @brief  */
		virtual bool	compareData(const void* binary, size_t bytes, index_t index = INVALID_INDEX) const;

		/** @brief get sub element count */
		virtual size_t	getCount() const;

		/** @brief  */
		virtual void	prepareData(size_t count) const { mBinding.prepare(count); }

		/** @brief  */
		virtual void	bindTarget(Bindable* target);

		/** @brief  */
		virtual Bindable* getTarget() const;

		/** @brief  */
		virtual void	addDelegate(const Delegate& d) { mDelegates.push_back(d); }

		virtual bool	hasDelegate() const { return mDelegates.size() > 0; }

		/** @brief create config for elements, bound to an element by index */
		virtual HCONFIG	createElementConfig(index_t index, CONFIG_ACCESS caf) const;

		/** @brief  */
		virtual bool	isElementReallocated(IConfig::IList* elementConfigList) const;

		/** @brief  */
		virtual HCDHANDLER clone() const;

		/************************************************************************/
		/* custom method                                                                     */
		/************************************************************************/
		/** @brief config's value is changed, maybe update handler's value from config */
		virtual bool	setElementData(const void* binary, size_t bytes, CONFIG_ACCESS verify, index_t index = INVALID_INDEX) const;

		/** @brief get the handler's binary data in memory */
		virtual bool	getElementData(void* binary, size_t bytes, index_t index = INVALID_INDEX) const;

		/** @brief  */
		virtual bool	compareElementData(const void* binary, size_t bytes, index_t index = INVALID_INDEX) const;

		/** @brief  */
		virtual void	prepareElementData(size_t count, index_t index = INVALID_INDEX) const;

		/** @brief  */
		virtual void	callEvent() const;

	protected:
		CollectionDataBinding	mBinding;
		DelegateList			mDelegates;
		IDataSource::signature_t mElementSignature;
		size_t					mReallocationCheckCount;
		bool					mIsArray;
	};//class CollectionConfigDataHandler

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	template<typename, typename, typename> class Map;
	class BLADE_FRAMEWORK_API MapConfigDataHandler : public CollectionConfigDataHandler
	{
	public:
		template<typename T, typename K, typename V, typename C>
		MapConfigDataHandler(const DataBinding& binding, T*, Map<K, V, C>*)
			:mBinding(binding, (T*)NULL, (K*)NULL, (V*)NULL, (C*)NULL)
		{
			mKeySignature = DS_SIGNATURE(K);
			mElementSignature = DS_SIGNATURE(V);
			mReallocationCheckCount = size_t(-1);
			this->initKeyConfig();
		}

		template<typename T, typename K, typename V, typename C>
		MapConfigDataHandler(Map<K,V,C> T::*pmdata, T* ptr = NULL, void (T::*pmEvent)(void*) = NULL)
			:mBinding(ptr, pmdata, pmEvent)
		{
			mKeySignature = DS_SIGNATURE(K);
			mElementSignature = DS_SIGNATURE(V);
			this->initKeyConfig();
		}

		~MapConfigDataHandler()		{}

		/************************************************************************/
		/* IConfig::IDataHandler interface                                                                     */
		/************************************************************************/
		/** @brief config's value is changed, maybe update handler's value from config */
		virtual bool	setData(const TString& name, const void* binary, size_t bytes, CONFIG_ACCESS verify, index_t index = INVALID_INDEX) const;

		/** @brief get the handler's binary data in memory */
		virtual bool	getData(void* binary, size_t bytes, index_t index = INVALID_INDEX) const;

		/** @brief  */
		virtual bool	compareData(const void* binary, size_t bytes, index_t index = INVALID_INDEX) const;

		/** @brief get sub element count */
		virtual size_t	getCount() const;

		/** @brief  */
		virtual void	prepareData(size_t count) const { mBinding.prepare(count); }

		/** @brief  */
		virtual void	bindTarget(Bindable* target);

		/** @brief  */
		virtual Bindable* getTarget() const;

		/** @brief create config for elements, bound to an element by index */
		virtual HCONFIG	createElementConfig(index_t index, CONFIG_ACCESS caf) const;

		/** @brief  */
		virtual bool	isElementReallocated(IConfig::IList* elementConfigList) const;

		/** @brief  */
		virtual HCDHANDLER clone() const;

		/** @brief  */
		void			initKeyConfig();

		/************************************************************************/
		/* CollectionConfigDataHandler interface                                                                     */
		/************************************************************************/
		/** @brief config's value is changed, maybe update handler's value from config */
		virtual bool	setElementData(const void* binary, size_t bytes, CONFIG_ACCESS verify, index_t index = INVALID_INDEX) const;

		/** @brief get the handler's binary data in memory */
		virtual bool	getElementData(void* binary, size_t bytes, index_t index = INVALID_INDEX) const;

		/** @brief  */
		virtual bool	compareElementData(const void* binary, size_t bytes, index_t index = INVALID_INDEX) const;

		/** @brief  */
		virtual void	prepareElementData(size_t count, index_t index = INVALID_INDEX) const;

		/** @brief  */
		virtual void	callEvent() const;

	protected:
		MapDataBinding			mBinding;
		IDataSource::signature_t mKeySignature;
		HCONFIG					mKeyConfig;
	};//class CollectionConfigDataHandler

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class BLADE_FRAMEWORK_API DelegateOnlyHandler : public IConfig::IDataHandler, public Allocatable
	{
	public:
		DelegateOnlyHandler() {}
		DelegateOnlyHandler(const DelegateOnlyHandler& src) :mDelegates(src.mDelegates) {}
		virtual ~DelegateOnlyHandler() {}

		/** @brief config's value is changed, maybe update handler's value from config */
		virtual bool	setData(const TString& /*name*/, const void* /*binary*/, size_t /*bytes*/, CONFIG_ACCESS /*verify*/, index_t /*index = INVALID_INDEX*/) const
		{
			mDelegates.call();
			return true;
		}

		/** @brief get the handler's binary data in memory */
		virtual bool	getData(void* /*binary*/, size_t /*bytes*/, index_t /*index = INVALID_INDEX*/) const
		{
			return false;
		}

		/** @brief  */
		virtual bool	compareData(const void* /*binary*/, size_t /*bytes*/, index_t /*index = INVALID_INDEX*/) const
		{
			return false;
		}

		/** @brief get sub element count */
		virtual size_t	getCount() const { return 0; }

		/** @brief  */
		virtual void	prepareData(size_t /*count*/) const {}

		/** @brief  */
		virtual void	bindTarget(Bindable* /*target*/) {}

		/** @brief  */
		virtual Bindable* getTarget() const { return NULL; }

		/** @brief  */
		virtual void	addDelegate(const Delegate& d) { mDelegates.push_back(d); }

		/** @brief  */
		virtual bool	hasDelegate() const { return mDelegates.size() > 0; }

		/** @brief  */
		virtual HCDHANDLER clone() const { return HCDHANDLER(BLADE_NEW DelegateOnlyHandler(*this)); }

	protected:
		DelegateList			mDelegates;
	};//class DelegateOnlyHandler

	///group dymmy data handler, only record the bound target
	class GroupHandler : public IConfig::IDataHandler, public Allocatable
	{
	public:
		GroupHandler() :mTarget(NULL) {}

		/************************************************************************/
		/* IConfig::IDataHandler interface                                                                     */
		/************************************************************************/
		/** @brief config's value is changed, maybe update handler's value from config */
		virtual bool	setData(const TString& /*name*/, const void* /*binary*/, size_t /*bytes*/, CONFIG_ACCESS /*verify*/, index_t /*index = INVALID_INDEX*/) const
		{
			return false;
		}

		/** @brief get the handler's binary data in memory */
		virtual bool	getData(void* /*binary*/, size_t /*bytes*/, index_t /*index = INVALID_INDEX*/) const
		{
			return false;
		}

		/** @brief  */
		virtual bool	compareData(const void* /*binary*/, size_t /*bytes*/, index_t /*index = INVALID_INDEX*/) const
		{
			return true;
		}

		/** @brief get sub element count */
		virtual size_t	getCount() const { return 0; }

		/** @brief  */
		virtual void	prepareData(size_t /*count*/) const {}

		/** @brief  */
		virtual void	bindTarget(Bindable* target) { mTarget = target; }

		/** @brief  */
		virtual Bindable* getTarget() const { return mTarget; }

		/** @brief  */
		virtual void	addDelegate(const Delegate& /*d*/) { }

		/** @brief  */
		virtual bool	hasDelegate() const { return false; }

		/** @brief  */
		virtual HCDHANDLER clone() const { return HCDHANDLER(BLADE_NEW GroupHandler()); }
	protected:
		Bindable* mTarget;
	};

}//namespace Blade

#endif // __Blade_ConfigDataHandler_h__