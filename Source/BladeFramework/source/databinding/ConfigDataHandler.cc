/********************************************************************
	created:	2011/09/17
	filename: 	ConfigDataHandler.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <utility/DataBindingEx.h>
#include <ConfigTypes.h>
#include <databinding/ConfigDataHandler.h>
#include "DataBindingHelper_Private.h"

namespace Blade
{
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool ConfigDataHandler::setData(const TString& name, const void* binary, size_t bytes, CONFIG_ACCESS /*verify*/, index_t /*index = INVALID_INDEX*/) const
	{
		//enable pure serialization event notification
		//TODO: which way to choose?
		bool callEvents = true;//(verify != CAF_SERIALIZE);

		bool ret = mBridge.setValue( binary, bytes, callEvents );
		if( ret )
		{
			//pure serialization doesn't call notification ?
			if( callEvents )
				mBridge.getDelegates().call( mBridge.getData() );

			DatabindingDelegateRouter::getSingleton().notify(this->getTarget(), name, /*unsafe*/(void*)binary);
		}
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	ConfigDataHandler::getData(void* binary, size_t bytes, index_t/* index = INVALID_INDEX*/) const
	{
		return mBridge.getValue(binary, bytes);
	}

	//////////////////////////////////////////////////////////////////////////
	bool	ConfigDataHandler::compareData(const void* binary, size_t bytes, index_t/* index = INVALID_INDEX*/) const
	{
		return mBridge.getBinding().compareData(binary, bytes);
	}

	//////////////////////////////////////////////////////////////////////////
	void	ConfigDataHandler::bindTarget(Bindable* target)
	{
		mBridge.rebind(target);
	}

	//////////////////////////////////////////////////////////////////////////
	Bindable* ConfigDataHandler::getTarget() const
	{
		return mBridge.getBinding().getTarget();
	}

	HCDHANDLER ConfigDataHandler::clone() const
	{
		assert( this->getTarget() == NULL );
		return HCDHANDLER(BLADE_NEW ConfigDataHandler(*this));
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	namespace Impl
	{
		//element data handler for atom(fundamental) types
		class CollectionElementDataHandler : public IConfig::IDataHandler, public Allocatable
		{
		public:
			CollectionElementDataHandler(const CollectionConfigDataHandler* container, index_t index) :mContainer(container),mIndex(index)	{}
			~CollectionElementDataHandler()		{}

			/** @brief config's value is changed, maybe update handler's value from config */
			virtual bool	setData(const TString& /*name*/, const void* binary, size_t bytes, CONFIG_ACCESS verify, index_t /*index = INVALID_INDEX*/) const
			{
				return mContainer->setElementData(binary, bytes, verify, mIndex);
			}

			/** @brief get the handler's binary data in memory (index is ignored
			 * or get sub element data specified by index */
			virtual bool	getData(void* binary, size_t bytes, index_t /*index = INVALID_INDEX*/) const
			{
				return mContainer->getElementData(binary, bytes, mIndex);
			}

			/** @brief  */
			virtual bool	compareData(const void* binary, size_t bytes, index_t/* index = INVALID_INDEX*/) const
			{
				return mContainer->compareElementData(binary, bytes, mIndex);
			}

			/** @brief get sub element count */
			virtual size_t	getCount()	const				{return 0;}

			/** @brief  */
			virtual void	prepareData(size_t count) const { mContainer->prepareElementData(count); }

			/** @brief  */
			virtual void	bindTarget(Bindable*)			{}

			/** @brief kind of hacking: fake target - always bound */
			virtual Bindable* getTarget() const				{return mContainer->getTarget();}

			/** @brief  */
			virtual void	addDelegate(const Delegate& )	{ assert(false); }

			/** @brief  */
			virtual bool	hasDelegate() const { return mContainer->hasDelegate(); }

			/** @brief  */
			virtual Handle<IDataHandler> clone() const							{assert(false);return HCDHANDLER::EMPTY;}

		protected:
			const CollectionConfigDataHandler*	mContainer;
			index_t		mIndex;
		};//class CollectionElementDataHandler

		//array element data handler for atom(fundamental) types
		class ArrayElementDataHandler : public CollectionConfigDataHandler
		{
		public:
			ArrayElementDataHandler(const CollectionConfigDataHandler* container, index_t index)
				:CollectionConfigDataHandler(*container), mIndex(index) {}

			~ArrayElementDataHandler() {}

			/** @brief config's value is changed, maybe update handler's value from config */
			virtual bool	setData(const TString& /*name*/, const void* binary, size_t bytes, CONFIG_ACCESS verify, index_t /*index = INVALID_INDEX*/) const
			{
				return CollectionConfigDataHandler::setElementData(binary, bytes, verify, mIndex);
			}

			/** @brief get the handler's binary data in memory (index is ignored
			* or get sub element data specified by index */
			virtual bool	getData(void* binary, size_t bytes, index_t /*index = INVALID_INDEX*/) const
			{
				return CollectionConfigDataHandler::getElementData(binary, bytes, mIndex);
			}

			/** @brief  */
			virtual bool	compareData(const void* binary, size_t bytes, index_t/* index = INVALID_INDEX*/) const
			{
				return CollectionConfigDataHandler::compareElementData(binary, bytes, mIndex);
			}

			/** @brief  */
			virtual Handle<IDataHandler> clone() const { return HCDHANDLER(BLADE_NEW ArrayElementDataHandler(*this)); }

		protected:
			index_t		mIndex;
		};//class ArrayElementDataHandler

		class BindableElementDataHandle : public IConfig::IDataHandler
		{
		public:
			BindableElementDataHandle(const CollectionConfigDataHandler* container, index_t index, HCDHANDLER original)
				:mContainer(container), mIndex(index),mOriginalHandler(original) {}

			BindableElementDataHandle(const BindableElementDataHandle& src) :mContainer(src.mContainer), mIndex(src.mIndex),mOriginalHandler(src.mOriginalHandler)
			{
				if (mOriginalHandler != NULL)
					mOriginalHandler->bindTarget(NULL);
			}

			~BindableElementDataHandle() {}

			/** @brief config's value is changed, maybe update handler's value from config */
			virtual bool	setData(const TString& name, const void* binary, size_t bytes, CONFIG_ACCESS verify, index_t index = INVALID_INDEX) const
			{
				bool ret = true;
				if (mOriginalHandler != NULL)
					ret = mOriginalHandler->setData(name, binary, bytes, verify, index);
				if (ret)
					mContainer->callEvent();
				return ret;
			}

			/** @brief get the handler's binary data in memory (index is ignored
			* or get sub element data specified by index */
			virtual bool	getData(void* binary, size_t bytes, index_t index = INVALID_INDEX) const
			{
				if (mOriginalHandler != NULL)
					return mOriginalHandler->getData(binary, bytes, index);
				return false;
			}

			/** @brief  */
			virtual bool	compareData(const void* binary, size_t bytes, index_t index = INVALID_INDEX) const
			{
				if (mOriginalHandler != NULL)
					return mOriginalHandler->compareData(binary, bytes, index);
				return false;
			}

			/** @brief get sub element count */
			virtual size_t	getCount()	const { return mOriginalHandler != NULL ? mOriginalHandler->getCount() : 0; }

			/** @brief  */
			virtual void	prepareData(size_t count) const { if (mOriginalHandler != NULL) mOriginalHandler->prepareData(count); }

			/** @brief  */
			virtual void	bindTarget(Bindable* target) { if (mOriginalHandler != NULL) mOriginalHandler->bindTarget(target); }

			/** @brief kind of hacking: fake target - always bound */
			virtual Bindable* getTarget() const { return mOriginalHandler != NULL ? mOriginalHandler->getTarget() : NULL; }

			/** @brief  */
			virtual void	addDelegate(const Delegate& d) { if (mOriginalHandler != NULL) mOriginalHandler->addDelegate(d); }

			/** @brief  */
			virtual bool	hasDelegate() const { return mOriginalHandler != NULL && mOriginalHandler->hasDelegate(); }

			/** @brief create config for elements, bound to an element by index */
			virtual HCONFIG	createElementConfig(index_t index, CONFIG_ACCESS caf) const { return mOriginalHandler != NULL ? mOriginalHandler->createElementConfig(index, caf) : HCONFIG::EMPTY; }

			/** @brief  */
			virtual Handle<IDataHandler> clone() const { return HCDHANDLER(BLADE_NEW BindableElementDataHandle(*this)); }

		protected:
			const CollectionConfigDataHandler* mContainer;
			index_t								mIndex;
			HCDHANDLER							mOriginalHandler;
		};//class BindableElementDataHandle

		struct BindableHandlerChanger : public IConfig::IVisitor
		{
			const CollectionConfigDataHandler* container;
			index_t						 index;
			virtual void visit(IConfig* config)
			{
				HCDHANDLER newHandler(BLADE_NEW BindableElementDataHandle(container, index, config->getDataHandler()));
				config->setDataHandler(newHandler);
			}
		};

		//////////////////////////////////////////////////////////////////////////
		template<typename BINDING>
		static bool	isElementReallocated(IConfig::IList* elementConfigList, const BINDING& binding, size_t count)
		{
			bool reallocated = false;

			//check element that is bindable. atom element (i.e. bool/int) don't need update, sin atom data handler directly get data from parent
			if (!binding.isElementBindable())
				return reallocated;

			size_t minCount = std::min(count, elementConfigList->getCount());

			//collection element reallocated? then address is different front element bound target (previous bound target probably dangling)
			if (minCount != 0)
			{
				assert(elementConfigList->getConfig(0)->getDataHandler() != NULL);
				assert(binding.getElementBinable(0) != NULL);

				//need check every element, especially for List/Map/Set, (Vector reallocation can be checked on the first element address)
				for (size_t i = 0; i < minCount; ++i)
				{
					IConfig* elementConfig = elementConfigList->getConfig(i);
					assert(elementConfig->getDataHandler() != NULL);
					//address changes
					if (elementConfig->getDataHandler()->getTarget() != binding.getElementBinable(i))
					{
						reallocated = true;
						break;
					}
				}
			}
			return reallocated;
		}


	}//namespace
	using namespace Impl;

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool CollectionConfigDataHandler::setData(const TString& name, const void* binary, size_t bytes, CONFIG_ACCESS /*verify*/, index_t /*index = INVALID_INDEX*/) const
	{
		//collection's data is hard coded to its size (uint32). TODO: better ways?
		assert(bytes == 4); BLADE_UNREFERENCED(bytes);
		uint32  count = *(const uint32*)binary;
		uint32 curCount = (uint32)this->getCount();
		if (count == curCount)
			return true;
		while (curCount > count)
		{
			bool ret = mBinding.deleteElement(curCount - 1, curCount - 1 == count);
			assert(ret); BLADE_UNREFERENCED(ret);
			--curCount;
		}
		//FIXME: container "Set" cannot insert default value
		bool ret = true;
		if (curCount == 0)
		{

		}
		while ((curCount < count) && ret)
		{
			ret = mBinding.addElement(NULL, 0, ++curCount == count);
		}

		if(ret)
			DatabindingDelegateRouter::getSingleton().notify(this->getTarget(), name, /*unsafe*/(void*)binary);
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	CollectionConfigDataHandler::getData(void* binary, size_t bytes, index_t /*index = INVALID_INDEX*/) const
	{
		assert(bytes == 4); BLADE_UNREFERENCED(bytes);
		uint32& count = *(uint32*)binary;
		count = (uint32)this->getCount();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	CollectionConfigDataHandler::compareData(const void* binary, size_t bytes, index_t /*index = INVALID_INDEX*/) const
	{
		assert(bytes == 4); BLADE_UNREFERENCED(bytes);
		uint32  count = *(const uint32*)binary;
		return (uint32)this->getCount() == count;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t	CollectionConfigDataHandler::getCount() const
	{
		return mBinding.getElementCount();
	}

	//////////////////////////////////////////////////////////////////////////
	void	CollectionConfigDataHandler::bindTarget(Bindable* target)
	{
		mBinding.rebind(target);
	}

	//////////////////////////////////////////////////////////////////////////
	Bindable* CollectionConfigDataHandler::getTarget() const
	{
		return mBinding.getTarget();
	}

	//////////////////////////////////////////////////////////////////////////
	HCONFIG CollectionConfigDataHandler::createElementConfig(index_t index, CONFIG_ACCESS caf) const
	{
		size_t count = this->getCount();
		if( index < count )
		{
			//data source is an unified interface for all types
			//whether the actual data is atom (fundamental type) or structures/classes
			assert(mElementSignature != NULL);
			const HCONFIG& config = IDataSourceDataBase::getSingleton().getAtomConfig(mElementSignature);
			if( config != NULL )
			{
				HCONFIG ret = config->clone(caf, TStringHelper::fromUInt(index) );

				//use the element handler and directly update in config group?
				//but that need access to child's binary value which is protected and maybe a sub-collection(CollectionConfigDataHandler)too.
				//so we better gives a interface, but this sub element handler SHOULD NOT be cloned

				//set sub index to the config, and the config knows its a sub element and call the right method with index
				IConfig::IDataHandler* elementHandler = mIsArray ? static_cast<IConfig::IDataHandler*>(BLADE_NEW ArrayElementDataHandler(this, index))
					: static_cast<IConfig::IDataHandler*>(BLADE_NEW CollectionElementDataHandler(this, index));
				ret->setDataHandler(HCDHANDLER(elementHandler));
				return ret;
			}
			else
			{
				IDataSource* datasource = IDataSourceDataBase::getSingleton().getDataSource(mElementSignature);
				assert(datasource != NULL );
				//lock binding
				IDataSource::BID bid = datasource->bindTarget(NULL);
				HCONFIG ret = datasource->cloneConfig(caf, TStringHelper::fromUInt(index) );
				ret->bindTarget( mBinding.getElementBinable(index) );
				datasource->unbindTarget(bid);

				//add sub config event handling for container. the container's owner will receive a data event when container element changes
				BindableHandlerChanger changer;
				changer.container = this;
				changer.index = index;
				ret->traverse(&changer);
				return ret;
			}
		}
		else
		{
			assert(false);
			return HCONFIG::EMPTY;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool	CollectionConfigDataHandler::isElementReallocated(IConfig::IList* elementConfigList) const
	{
		return Impl::isElementReallocated(elementConfigList, mBinding, std::min(mReallocationCheckCount, this->getCount()));
	}

	//////////////////////////////////////////////////////////////////////////
	HCDHANDLER CollectionConfigDataHandler::clone() const
	{
		assert( this->getTarget() == NULL );
		return HCDHANDLER(BLADE_NEW CollectionConfigDataHandler(*this));
	}

	//////////////////////////////////////////////////////////////////////////
	bool	CollectionConfigDataHandler::setElementData(const void* binary, size_t bytes, CONFIG_ACCESS verify, index_t index/* = INVALID_INDEX*/) const
	{
		//serialization doesn't call notification
		bool callEvents = true;// (verify != CAF_SERIALIZE);
		bool ret = mBinding.setElementData(index, binary, bytes, callEvents);
		if (ret)
		{
			if (callEvents)
				mDelegates.call(mBinding.getDataPtr());

			//TODO: notify on element change
			//DatabindingDelegateRouter::getSingleton().notify(this->getTarget(), name, /*unsafe*/(void*)binary);
		}
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	CollectionConfigDataHandler::getElementData(void* binary, size_t bytes, index_t index/* = INVALID_INDEX*/) const
	{
		return mBinding.getElementData(index, binary, bytes);
	}

	//////////////////////////////////////////////////////////////////////////
	bool	CollectionConfigDataHandler::compareElementData(const void* binary, size_t bytes, index_t index/* = INVALID_INDEX*/) const
	{
		return mBinding.compareElementData(index, binary, bytes);
	}

	//////////////////////////////////////////////////////////////////////////
	void	CollectionConfigDataHandler::prepareElementData(size_t count, index_t index/* = INVALID_INDEX*/) const
	{
		BLADE_UNREFERENCED(count);
		BLADE_UNREFERENCED(index);
		//TODO: cascade containers
	}

	//////////////////////////////////////////////////////////////////////////
	void		CollectionConfigDataHandler::callEvent() const
	{
		mBinding.callEvent();
		mDelegates.call(mBinding.getDataPtr());
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool MapConfigDataHandler::setData(const TString& name, const void* binary, size_t bytes, CONFIG_ACCESS /*verify*/, index_t /*index = INVALID_INDEX*/) const
	{
		//collection's data is hard coded to its size (uint32). TODO: better ways?
		assert(bytes == 4); BLADE_UNREFERENCED(bytes);
		uint32  count = *(const uint32*)binary;
		uint32 curCount = (uint32)this->getCount();
		if (count == curCount)
			return true;

		//FIXME: container "Map" cannot insert default value
		if (curCount < count)
			return false;

		bool ret = false;

		while (curCount > count)
		{
			ret = mBinding.deleteElement(curCount - 1, curCount - 1 == count);
			assert(ret); BLADE_UNREFERENCED(ret);
			--curCount;
		}

		if(ret)
			DatabindingDelegateRouter::getSingleton().notify(this->getTarget(), name, /*unsafe*/(void*)binary);

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	MapConfigDataHandler::getData(void* binary, size_t bytes, index_t /*index = INVALID_INDEX*/) const
	{
		assert(bytes == 4); BLADE_UNREFERENCED(bytes);
		uint32& count = *(uint32*)binary;
		count = (uint32)this->getCount();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	MapConfigDataHandler::compareData(const void* binary, size_t bytes, index_t /*index = INVALID_INDEX*/) const
	{
		assert(bytes == 4); BLADE_UNREFERENCED(bytes);
		uint32  count = *(const uint32*)binary;
		return (uint32)this->getCount() == count;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t	MapConfigDataHandler::getCount() const
	{
		return mBinding.getElementCount();
	}

	//////////////////////////////////////////////////////////////////////////
	void	MapConfigDataHandler::bindTarget(Bindable* target)
	{
		mBinding.rebind(target);
	}

	//////////////////////////////////////////////////////////////////////////
	Bindable* MapConfigDataHandler::getTarget() const
	{
		return mBinding.getTarget();
	}

	//////////////////////////////////////////////////////////////////////////
	HCONFIG MapConfigDataHandler::createElementConfig(index_t index, CONFIG_ACCESS caf) const
	{
		assert(mKeyConfig != NULL);

		size_t count = this->getCount();

		if( index < count )
		{
			//
			Variant key = mKeyConfig->getBinaryValue();
			mBinding.getKeyData(index, key.getBinaryData(), key.getSize());
			TString configName = key.toTString();

			//data source is an unified interface for all types
			//whether the actual data is atom (fundamental type) or structures/classes
			assert(mKeySignature != NULL && mElementSignature != NULL);
			const HCONFIG& config = IDataSourceDataBase::getSingleton().getAtomConfig(mElementSignature);
			if( config != NULL )
			{
				HCONFIG ret = config->clone(caf, BTString("[") + TStringHelper::fromUInt(index) + BTString("] ") + configName );

				//TODO:do NOT use the element handler and directly update in config group?
				//but that need access to child's binary value which is protected and maybe a sub-collection(CollectionConfigDataHandler)too.
				//so we better gives a interface, but this sub element handler SHOULD NOT be cloned

				//set sub index to the config, and the config knows its a sub element and call the right method with index
				ret->setDataHandler(HCDHANDLER(BLADE_NEW CollectionElementDataHandler(this, index)));
				return ret;
			}
			else
			{
				IDataSource* datasource = IDataSourceDataBase::getSingleton().getDataSource(mElementSignature);
				assert(datasource != NULL );
				//lock binding
				datasource->bindTarget(NULL);
				HCONFIG ret = datasource->cloneConfig(caf, BTString("[") + TStringHelper::fromUInt(index) + BTString("] ") + configName );
				ret->bindTarget( mBinding.getElementBinable(index) );

				//add sub config event handling for container. the container's owner will receive a data event when container element changes
				BindableHandlerChanger changer;
				changer.container = this;
				changer.index = index;
				ret->traverse(&changer);
				return ret;
			}
		}
		else
		{
			assert(false);
			return HCONFIG::EMPTY;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool	MapConfigDataHandler::isElementReallocated(IConfig::IList* elementConfigList) const
	{
		return Impl::isElementReallocated(elementConfigList, mBinding, std::min(mReallocationCheckCount, this->getCount()));
	}

	//////////////////////////////////////////////////////////////////////////
	HCDHANDLER MapConfigDataHandler::clone() const
	{
		assert( this->getTarget() == NULL );
		return HCDHANDLER(BLADE_NEW MapConfigDataHandler(*this));
	}

	//////////////////////////////////////////////////////////////////////////
	void		MapConfigDataHandler::initKeyConfig()
	{
		if( mKeyConfig != NULL )
			return;

		const HCONFIG& config = IDataSourceDataBase::getSingleton().getAtomConfig(mKeySignature);
		if( config != NULL )
			mKeyConfig = config->clone(CAF_READ, TString::EMPTY);
		else
		{
			IDataSource* datasource = IDataSourceDataBase::getSingleton().getDataSource(mElementSignature);
			assert(datasource != NULL );
			mKeyConfig = datasource->cloneConfig(CAF_READ, TString::EMPTY);
		}
		assert(mKeyConfig != NULL);
	}

	//////////////////////////////////////////////////////////////////////////
	bool	MapConfigDataHandler::setElementData(const void* binary, size_t bytes, CONFIG_ACCESS verify, index_t index/* = INVALID_INDEX*/) const
	{
		//serialization doesn't call notification
		bool callEvents = true;// (verify != CAF_SERIALIZE);
		bool ret = mBinding.setElementData(index, binary, bytes, callEvents);
		if (ret)
		{
			if (callEvents)
				mDelegates.call(mBinding.getDataPtr());

			//TODO: notify on element change
			//DatabindingDelegateRouter::getSingleton().notify(this->getTarget(), name, /*unsafe*/(void*)binary);
		}
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	MapConfigDataHandler::getElementData(void* binary, size_t bytes, index_t index/* = INVALID_INDEX*/) const
	{
		assert(mKeyConfig != NULL);
		Variant key = mKeyConfig->getBinaryValue();
		return mBinding.getElementData(index, key.getBinaryData(), key.getSize(), binary, bytes);
	}

	//////////////////////////////////////////////////////////////////////////
	bool	MapConfigDataHandler::compareElementData(const void* binary, size_t bytes, index_t index/* = INVALID_INDEX*/) const
	{
		return mBinding.compareElementData(index, binary, bytes);
	}

	//////////////////////////////////////////////////////////////////////////
	void	MapConfigDataHandler::prepareElementData(size_t count, index_t index/* = INVALID_INDEX*/) const
	{
		BLADE_UNREFERENCED(count);
		BLADE_UNREFERENCED(index);
		//TODO: cascade containers
	}

	//////////////////////////////////////////////////////////////////////////
	void	MapConfigDataHandler::callEvent() const
	{
		mBinding.callEvent();
		mDelegates.call(mBinding.getDataPtr());
	}

}//namespace Blade
