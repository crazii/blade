/********************************************************************
	created:	2013/12/01
	filename: 	DataBindingEx.h
	author:		Crazii
	purpose:	this file is only for final data binding usage; DO NOT include it in a public header

	update log:	7/29/2017 decouple data binding from collection access
*********************************************************************/
#ifndef __Blade_DataBindingEx_h__
#define __Blade_DataBindingEx_h__
#include "DataBinding.h"
#include "StringList.h"
#include "BladeContainer.h"

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4127) //conditional expression is constant
#endif

namespace Blade
{
	namespace DataBindingUtil
	{

		template<typename T, bool isbindable>
		struct ElementBindableBase
		{
			static Bindable*	getBindable(T*)
			{
				return NULL;
			}

			static bool compare(const T& lhs, const T& rhs)
			{
				return lhs == rhs;
			}
		};

		template<typename T>
		struct ElementBindableBase<T, true>
		{
			static Bindable*	getBindable(T* ptr)
			{
				//static_cast is not available, using C style cast
				return static_cast<Bindable*>(ptr);
			}

			static bool compare(const T& lhs, const T& rhs)
			{
				return lhs == rhs;
			}
		};

		template<typename T>
		struct ElementBindableBase<T*, true>
		{
			static Bindable*	getBindable(T** ptr)
			{
				//static_cast is not available, using C style cast
				return static_cast<Bindable*>(*ptr);
			}

			//compare bindable object?
			static bool compare(T const*& lhs, T const*& rhs)
			{
				return *lhs == *rhs;
			}
		};

		template<typename T>
		struct ElementBindable : public ElementBindableBase<T, IsBindable<T>::value > {};

		/************************************************************************/
		/*                                                                      */
		/************************************************************************/
		//collection
		typedef bool		(*PREPARE_FUNC)(void* binary, size_t count);
		typedef bool		(*GETELEMENT_FUNC)(const void* binary, index_t index, void* outData, size_t bytes);
		typedef bool		(*SETELEMENT_FUNC)(void* binary, index_t index, const void* data, size_t bytes);
		typedef bool		(*CMPELEMENT_FUNC)(const void* binary, index_t index, const void* data, size_t bytes);
		typedef index_t		(*ADDELEMENT_FUNC)(void* binary, const void* data, size_t bytes);
		typedef bool		(*DELELEMENT_FUNC)(void* binary, index_t index);
		typedef size_t		(*COUNT_FUNC)(const void* binary);
		typedef bool		(*ISBINDELEMENT_FUNC)();
		typedef Bindable*	(*GETBINDELEMENT_FUNC)(void* binary, index_t index);

		typedef bool		(*MAP_GETELEMENT_FUNC)(const void* binary, index_t index, void* outKey, size_t keySize, void* outVal, size_t valSize);
		typedef bool		(*MAP_GETVALUE_FUNC)(const void* binary, const void* keyData, size_t keySize, void* outVal, size_t valSize);
		typedef index_t		(*MAP_ADDELEMENT_FUNC)(void* binary, const void* keyData, size_t keySize, const void* valData, size_t valSize);
		typedef bool		(*MAP_DELELEMENT_FUNC)(void* binary, const void* keyData, size_t keySize);

		struct CollectionBindingFuc
		{
			PREPARE_FUNC		FnPrepare;
			GETELEMENT_FUNC		FnGetElem;
			SETELEMENT_FUNC		FnSetElem;
			ADDELEMENT_FUNC		FnAddElem;
			DELELEMENT_FUNC		FnDelElem;
			CMPELEMENT_FUNC		FnCmpElem;
			COUNT_FUNC			FnCount;
			ISBINDELEMENT_FUNC	FnIsElemBind;
			GETBINDELEMENT_FUNC	FnGetBindElem;

			MAP_GETELEMENT_FUNC	FnGetMapElem;
			MAP_GETVALUE_FUNC	FnGetMapVal;
			MAP_ADDELEMENT_FUNC	FnAddMapElem;
			MAP_DELELEMENT_FUNC	FnDelMapElem;
		};

		/************************************************************************/
		/* TStringList binding                                                                     */
		/************************************************************************/
		/** @brief prepare element */
		template<typename T>
		static bool prepareTStringElement(void* binary, size_t count)
		{
			TStringList& list = *(TStringList*)binary;
			list.reserve(count);
			return true;
		}

		/** @brief get element */
		template<typename T>
		static bool getTStringElement(const void* binary, index_t index, void* outData, size_t bytes)
		{
			if( bytes == sizeof(TString) )
			{
				const TStringList& list = *(const TStringList*)binary;
				if( index < list.size() )
				{
					*((TString*)outData) = list[index];
					return true;
				}
			}
			return false;
		}

		/** @brief set element */
		template<typename T>
		static bool setTStringElement(void* binary, index_t index, const void* data, size_t bytes)
		{
			if( bytes == sizeof(TString) )
			{
				TStringList& list = *(TStringList*)binary;
				if( index < list.size() )
				{
					list[index] = *((const TString*)data);
					return true;
				}
			}
			return false;
		}

		/** @brief set element */
		template<typename T>
		static bool compareTStringElement(const void* binary, index_t index, const void* data, size_t bytes)
		{
			if( bytes == sizeof(TString) )
			{
				const TStringList& list = *(const TStringList*)binary;
				if( index < list.size() )				
					return list[index] == *((const TString*)data);
			}
			return true;
		}

		/** @brief add element */
		template<typename T>
		static index_t addTStringElement(void* binary, const void* data, size_t bytes)
		{
			if (data == NULL)
			{
				data = &TString::EMPTY;
				bytes = sizeof(TString);
			}

			if( bytes == sizeof(TString) )
			{
				TStringList& list = *(TStringList*)binary;
				list.push_back( *(const TString*)(data) );
				return list.size()-1;
			}
			assert(false);
			return INVALID_INDEX;
		}

		/** @brief delete element */
		template<typename T>
		static bool	deleteTStringElement(void* binary, index_t index)
		{
			TStringList& list = *(TStringList*)binary;
			if (index < list.size())
			{
				list.erase(index);
				return true;
			}
			return false;
		}

		/** @brief get element count */
		template<typename T>
		size_t		getTStringCount(const void* binary)
		{
			if (binary == NULL)
				return 0;
			const TStringList& list = *(const TStringList*)binary;
			return list.size();
		}

		/** @brief is element is bindable */
		template<typename T>
		static bool			isEmptyElementBindable()
		{
			return false;
		}

		/** @brief get element as bindable */
		template<typename T>
		static Bindable*	getEmptyElementBindable(void* /*binary*/, index_t /*index*/)
		{
			assert(false);
			return NULL;
		}

		/************************************************************************/
		/* Common container binding                                                                     */
		/************************************************************************/
		/** @brief get element */
		template<typename T, typename V>
		static bool getElement(const void* binary, index_t index, void* outData, size_t bytes)
		{
			typedef V CollectionType;
			typedef typename V::value_type ElementType;
			if( bytes == sizeof(ElementType) )
			{
				const CollectionType& collection = *(const CollectionType*)binary;
				if( index < collection.size() )
				{
					typename CollectionType::const_iterator iter = collection.begin();
					std::advance(iter, index);
					*((ElementType*)outData) = *iter;
					return true;
				}
			}
			return false;
		}

		/** @brief set element */
		template<typename T, typename V>
		static bool setElement(void* binary, index_t index, const void* data, size_t bytes)
		{
			typedef V CollectionType;
			typedef typename V::value_type ElementType;
			if( bytes == sizeof(ElementType) )
			{
				CollectionType& collection = *(CollectionType*)binary;
				if( index < collection.size() )
				{
					typename CollectionType::iterator iter = collection.begin();
					std::advance(iter, index);
					*iter = *((const ElementType*)data);
					return true;
				}
			}
			return false;
		}

		/** @brief set element */
		template<typename T, typename V>
		static bool compareElement(const void* binary, index_t index, const void* data, size_t bytes)
		{
			typedef V CollectionType;
			typedef typename V::value_type ElementType;
			if( bytes == sizeof(ElementType) )
			{
				const CollectionType& collection = *(const CollectionType*)binary;
				if( index < collection.size() )
				{
					typename CollectionType::const_iterator iter = collection.begin();
					std::advance(iter, index);
					return ElementBindable<ElementType>::compare(*iter, *((const ElementType*)data));
				}
			}
			return true;
		}

		/** @brief add element */
		template<typename T, typename V>
		static index_t addElement(void* binary, const void* data, size_t bytes)
		{
			typedef V CollectionType;
			typedef typename V::value_type ElementType;

			ElementType def = ElementType();
			//add default value
			if (data == NULL)
			{
				data = &def;
				bytes = sizeof(ElementType);
			}

			if( bytes == sizeof(ElementType) )
			{
				CollectionType& collection = *(CollectionType*)binary;
				collection.push_back( *(const ElementType*)(data) );
				return collection.size()-1;
			}
			assert(false);
			return INVALID_INDEX;
		}

		/** @brief delete element */
		template<typename T, typename V>
		static bool	deleteElement(void* binary, index_t index)
		{
			typedef V CollectionType;
			//typedef typename V::value_type ElementType;

			CollectionType& collection = *(CollectionType*)binary;
			if (index < collection.size())
			{
				typename CollectionType::iterator it = collection.begin();
				std::advance(it, index);
				collection.erase(it);
				return true;
			}
			return false;
		}

		/** @brief get element count */
		template<typename T, typename V>
		size_t		getCount(const void* binary)
		{
			typedef V CollectionType;
			//typedef typename V::value_type ElementType;

			if (binary == NULL)
				return 0;
			const CollectionType& collection = *(const CollectionType*)binary;
			return collection.size();
		}

		/** @brief is element bindable */
		template<typename T, typename ElementV>
		static bool	isElementBinableE()
		{
			return IsBindable<ElementV>::value;
		}

		/** @brief is element bindable */
		template<typename T, typename CollectionV>
		static bool	isElementBinable()
		{
			typedef CollectionV CollectionType;
			typedef typename CollectionType::value_type ValueType;
			return IsBindable<ValueType>::value;
		}

		/** @brief get element as bindable */
		template<typename T, typename V>
		static Bindable*	getElementBinable(void* binary, index_t index)
		{
			typedef V CollectionType;
			typedef typename CollectionType::value_type ValueType;
			if (!IsBindable<ValueType>::value)
				return NULL;

			CollectionType& collection = *(CollectionType*)binary;
			if( index < collection.size() )
			{
				typename CollectionType::iterator iter = collection.begin();
				std::advance(iter, index);

				return ElementBindable<ValueType>::getBindable(&*iter);
			}
			return NULL;
		}

		/** @brief prepare element */
		template<typename T, typename V>
		static bool prepareElement(void* /*binary*/, size_t /*count*/)
		{
			return true;
		}

		/************************************************************************/
		/* Vector binding                                                                    */
		/************************************************************************/
		/** @brief prepare element */
		template<typename T, typename V>
		static bool prepareElementV(void* binary, size_t count)
		{
			typedef V CollectionType;
			CollectionType& collection = *(CollectionType*)binary;
			collection.reserve(count);
			return true;
		}

		/************************************************************************/
		/* Set binding                                                                     */
		/************************************************************************/
		/** @brief can not change set key */
		template<typename T, typename V>
		static bool setElementS(void* binary, index_t index, const void* data, size_t bytes)
		{
			assert(false);
			return false;
		}

		/** @brief insertion */
		template<typename T, typename V>
		static index_t insertElement(void* binary, const void* data, size_t bytes)
		{
			typedef V CollectionType;
			typedef typename V::value_type ElementType;
			if( data != NULL && bytes == sizeof(ElementType) )
			{
				CollectionType& collection = *(CollectionType*)binary;
				std::pair<typename CollectionType::iterator,bool> ret = collection.insert( *(const ElementType*)(data) );
				if( !ret.second )
				{
					assert(false);	//is insertion failure a expected result?
					return INVALID_INDEX;
				}
				return (index_t)std::distance(collection.begin(), ret.first);
			}
			assert(false);
			return INVALID_INDEX;
		}

		/************************************************************************/
		/* Map binding                                                                     */
		/************************************************************************/
		template<typename T, typename V>
		static bool getMapKey(const void* binary, index_t index, void* outKey, size_t bytes)
		{
			bool ret = false;
			typedef V CollectionType;
			if( bytes == sizeof(typename CollectionType::key_type) )
			{
				const CollectionType& collection = *(const CollectionType*)binary;
				if( index < collection.size() )
				{
					ret = true;
					typename CollectionType::const_iterator iter = collection.begin();
					std::advance(iter, index);
					*((typename CollectionType::key_type*)outKey) = iter->first;
				}
			}
			return ret;
		}

		/** @brief get element */
		template<typename T, typename V>
		static bool getMapElement(const void* binary, index_t index, void* outKey, size_t keySize, void* outVal, size_t valSize)
		{
			bool ret = false;
			typedef V CollectionType;
			if( keySize == sizeof(typename CollectionType::key_type) && valSize == sizeof(typename CollectionType::mapped_type))
			{
				const CollectionType& collection = *(const CollectionType*)binary;
				if( index < collection.size() )
				{
					ret = true;
					typename CollectionType::const_iterator iter = collection.begin();
					std::advance(iter, index);
					*((typename CollectionType::key_type*)outKey) = iter->first;
					*((typename CollectionType::mapped_type*)outVal) = iter->second;
				}
			}
			return ret;
		}

		/** @brief set element */
		template<typename T, typename V>
		static bool setMapElement(void* binary, index_t index, const void* data, size_t bytes)
		{
			bool ret = false;
			typedef V CollectionType;
			if( bytes == sizeof(typename CollectionType::mapped_type) )
			{
				CollectionType& collection = *(CollectionType*)binary;
				if( index < collection.size() )
				{
					ret = true;
					typename CollectionType::iterator iter = collection.begin();
					std::advance(iter, index);
					iter->second = *((const typename CollectionType::mapped_type*)data);
				}
			}
			return ret;
		}

		/** @brief set element */
		template<typename T, typename V>
		static bool compareMapElement(const void* binary, index_t index, const void* data, size_t bytes)
		{
			typedef V CollectionType;
			if( bytes == sizeof(typename CollectionType::mapped_type) )
			{
				const CollectionType& collection = *(const CollectionType*)binary;
				if( index < collection.size() )
				{
					typename CollectionType::const_iterator iter = collection.begin();
					std::advance(iter, index);
					return ElementBindable<const typename CollectionType::mapped_type>::compare(iter->second, *((const typename CollectionType::mapped_type*)data));
				}
			}
			return true;
		}

		/** @brief  */
		template<typename T, typename V>
		static index_t addMapElement2(void* /*binary*/, const void* /*data*/, size_t /*bytes*/)
		{
			assert(false);
			return false;
		}

		/** @brief add element */
		template<typename T, typename V>
		static index_t addMapElement(void* binary, const void* keyData, size_t keySize, const void* valData, size_t valSize)
		{
			typedef V CollectionType;
			if( keySize == sizeof(typename CollectionType::key_type) && valSize == sizeof(typename CollectionType::mapped_type) )
			{
				CollectionType& collection = *(CollectionType*)binary;

				const typename CollectionType::key_type& key = *(const typename CollectionType::key_type*)keyData;
				const typename CollectionType::mapped_type& val = *(const typename CollectionType::mapped_type*)valData;

				collection.insert( typename CollectionType::value_type(key, val) );
				return collection.size() - 1;

			}
			assert(false);
			return INVALID_INDEX;
		}

		/** @brief  */
		template<typename T, typename V>
		bool		getMapValue(const void* binary, const void* keyData, size_t keySize, void* outVal, size_t valSize)
		{
			bool ret = false;
			typedef V CollectionType;
			if( keySize == sizeof(typename CollectionType::key_type) && valSize == sizeof(typename CollectionType::mapped_type))
			{
				const CollectionType& collection = *(const CollectionType*)binary;
				typename CollectionType::const_iterator iter = collection.find( *((const typename CollectionType::key_type*)keyData) );
				ret = iter != collection.end();
				if(ret)
					*((typename CollectionType::mapped_type*)outVal) = iter->second;
			}
			assert(ret);
			return ret;
		}

		/** @brief  */
		template<typename T, typename V>
		bool		deleteMapKey(void* binary, const void* keyData, size_t keySize)
		{
			bool ret = false;
			typedef V CollectionType;
			if (keySize == sizeof(typename CollectionType::key_type))
			{
				CollectionType& collection = *(CollectionType*)binary;
				typename CollectionType::iterator iter = collection.find(*((const typename CollectionType::key_type*)keyData));
				ret = iter != collection.end();
				if (ret)
					collection.erase(iter);
			}
			assert(ret);
			return ret;
		}

		/** @brief is element bindable */
		template<typename T, typename V>
		static bool	isMapElementBinable()
		{
			typedef V CollectionType;
			return IsBindable<typename CollectionType::mapped_type>::value;
		}

		/** @brief get element as bindable */
		template<typename T, typename V>
		static Bindable*	getMapElementBinable(void* binary, index_t index)
		{
			typedef V CollectionType;
			CollectionType& collection = *(CollectionType*)binary;
			if( index < collection.size() )
			{
				typename CollectionType::iterator iter = collection.begin();
				std::advance(iter, index);
				return ElementBindable<typename CollectionType::mapped_type>::getBindable(&(iter->second));
			}
			else
				return NULL;
		}


		/************************************************************************/
		/* array binding                                                                     */
		/************************************************************************/
		/** @brief get element */
		template<typename T, typename V, size_t N>
		static bool getArrayElement(const void* binary, index_t index, void* outData, size_t bytes)
		{
			if (bytes == sizeof(V) && index < N)
			{
				const V(&_array)[N] = *(const V(*)[N])binary;
				*((V*)outData) = _array[index];
				return true;
			}
			return false;
		}

		/** @brief set element */
		template<typename T, typename V, size_t N>
		static bool setArrayElement(void* binary, index_t index, const void* data, size_t bytes)
		{
			if (bytes == sizeof(V) && index < N)
			{
				V(&_array)[N] = *(V(*)[N])binary;
				_array[index]  = *((const V*)data);
				return true;
			}
			return false;
		}

		/** @brief set element */
		template<typename T, typename V, size_t N>
		static bool compareArrayElement(const void* binary, index_t index, const void* data, size_t bytes)
		{
			if (bytes == sizeof(V) && index < N)
			{
				const V(&_array)[N] = *(const V(*)[N])binary;
				return *((V*)data) == _array[index];
			}
			return false;
		}

		/** @brief add element */
		template<typename T, typename V, size_t N>
		static index_t addArrayElement(void* /*binary*/, const void* /*data*/, size_t /*bytes*/)
		{
			return INVALID_INDEX;
		}

		/** @brief delete element */
		template<typename T, typename V, size_t N>
		static bool	deleteArrayElement(void* /*binary*/, index_t /*index*/)
		{
			return false;
		}

		/** @brief get element count */
		template<typename T, typename V, size_t N>
		size_t		getArrayCount(const void* /*binary*/)
		{
			return N;
		}

		/** @brief get element as bindable */
		template<typename T, typename V, size_t N>
		static Bindable*	getArrayElementBinable(void* binary, index_t index)
		{
			if (!IsBindable<V>::value)
				return NULL;

			if (index < N)
			{
				V(&_array)[N] = *(V(*)[N])binary;
				return ElementBindable<V>::getBindable(&_array[index]);
			}

			assert(false);
			return NULL;
		}

		/************************************************************************/
		/*                                                                      */
		/************************************************************************/
		template<typename T, typename V>
		struct DefCollectionBindingFuc : public CollectionBindingFuc
		{
			DefCollectionBindingFuc()
			{
				FnPrepare = &prepareElement<T, V>;
				FnGetElem = &getElement<T, V>;
				FnSetElem = &setElement<T, V>;
				FnCmpElem = &compareElement<T, V>;
				FnAddElem = &addElement<T, V>;
				FnDelElem = &deleteElement<T, V>;
				FnCount = &getCount<T, V>;
				FnIsElemBind = &isElementBinable<T, V>;
				FnGetBindElem = &getElementBinable<T, V>;

				FnGetMapElem = NULL;
				FnAddMapElem = NULL;
				FnGetMapVal = NULL;
				FnDelMapElem = NULL;
			}

			static const DefCollectionBindingFuc<T,V>* BINDINGFUC()
			{
				return &msFunc;
			}
			static DefCollectionBindingFuc<T,V> msFunc;
		};
		template<typename T, typename V>
		DefCollectionBindingFuc<T,V> DefCollectionBindingFuc<T,V>::msFunc;


		template<typename T>
		struct DefCollectionBindingFuc<T, TStringList> : public CollectionBindingFuc
		{
			DefCollectionBindingFuc()
			{
				FnPrepare = &prepareTStringElement<T>;
				FnGetElem = &getTStringElement<T>;
				FnSetElem = &setTStringElement<T>;
				FnCmpElem = &compareTStringElement<T>;
				FnAddElem = &addTStringElement<T>;
				FnDelElem = &deleteTStringElement<T>;
				FnCount = &getTStringCount<T>;
				FnIsElemBind = &isEmptyElementBindable<T>;
				FnGetBindElem = &getEmptyElementBindable<T>;

				FnGetMapElem = NULL;
				FnAddMapElem = NULL;
				FnGetMapVal = NULL;
				FnDelMapElem = NULL;
			}

			static const DefCollectionBindingFuc<T, TStringList>* BINDINGFUC()
			{
				return &msFunc;
			}
			static DefCollectionBindingFuc<T, TStringList> msFunc;
		};
		template<typename T>
		DefCollectionBindingFuc<T, TStringList> DefCollectionBindingFuc<T, TStringList>::msFunc;

		template<typename T, typename V>
		struct DefCollectionBindingFuc<T, Vector<V> > : public CollectionBindingFuc
		{
			DefCollectionBindingFuc()
			{
				FnPrepare = &prepareElementV<T, Vector<V> >;
				FnGetElem = &getElement<T, Vector<V> >;
				FnSetElem = &setElement<T, Vector<V> >;
				FnCmpElem = &compareElement<T, Vector<V> >;
				FnAddElem = &addElement<T, Vector<V> >;
				FnDelElem = &deleteElement<T, Vector<V> >;
				FnCount = &getCount<T, Vector<V> >;
				FnIsElemBind = &isElementBinableE<T, V>;
				FnGetBindElem = &getElementBinable<T, Vector<V> >;

				FnGetMapElem = NULL;
				FnAddMapElem = NULL;
				FnGetMapVal = NULL;
				FnDelMapElem = NULL;
			}

			static const DefCollectionBindingFuc<T, Vector<V> >* BINDINGFUC()
			{
				return &msFunc;
			}
			static DefCollectionBindingFuc<T, Vector<V> > msFunc;
		};
		template<typename T, typename V>
		DefCollectionBindingFuc<T, Vector<V> > DefCollectionBindingFuc<T, Vector<V> >::msFunc;


		template<typename T, typename V>
		struct DefCollectionBindingFuc<T, Set<V> > : public CollectionBindingFuc
		{
			DefCollectionBindingFuc()
			{
				FnPrepare = &prepareElementV<T, Set<V> >;
				FnGetElem = &getElement<T, Set<V> >;
				FnSetElem = &setElementS<T, Set<V> >;
				FnCmpElem = &compareElement<T, Set<V> >;
				FnAddElem = &insertElement<T, Set<V> >;
				FnDelElem = &deleteElement<T, Set<V> >;
				FnCount = &getCount<T, Set<V> >;
				FnIsElemBind = &isElementBinableE<T, V>;
				FnGetBindElem = &getElementBinable<T, Set<V> >;

				FnGetMapElem = NULL;
				FnAddMapElem = NULL;
				FnGetMapVal = NULL;
				FnDelMapElem = NULL;
			}

			static const DefCollectionBindingFuc<T, Set<V> >* BINDINGFUC()
			{
				return &msFunc;
			}
			static DefCollectionBindingFuc<T, Set<V> > msFunc;
		};
		template<typename T, typename V>
		DefCollectionBindingFuc<T, Set<V> > DefCollectionBindingFuc<T, Set<V> >::msFunc;

		template<typename T, typename MAP>
		struct DefMapBindingFuc : public CollectionBindingFuc
		{
			DefMapBindingFuc()
			{
				FnPrepare = &prepareElement<T, MAP>;
				FnGetElem = &getMapKey<T, MAP>;
				FnSetElem = &setMapElement<T, MAP>;
				FnCmpElem = &compareMapElement<T, MAP>;
				FnAddElem = &addMapElement2<T, MAP>;
				FnDelElem = &deleteElement<T, MAP>;
				FnCount = &getCount<T, MAP>;
				FnIsElemBind = &isMapElementBinable<T, MAP>;
				FnGetBindElem = &getMapElementBinable<T, MAP>;

				FnGetMapElem = &getMapElement<T, MAP>;
				FnAddMapElem = &addMapElement<T, MAP>;
				FnGetMapVal = &getMapValue<T, MAP>;
				FnDelMapElem = &deleteMapKey<T, MAP>;
			}

			static const DefMapBindingFuc<T, MAP>* BINDINGFUC()
			{
				return &msFunc;
			}
			static DefMapBindingFuc<T, MAP> msFunc;
		};
		template<typename T, typename MAP>
		DefMapBindingFuc<T, MAP> DefMapBindingFuc<T, MAP>::msFunc;


		template<typename T, typename V, size_t N>
		struct DefCollectionBindingFuc<T, V[N] > : public CollectionBindingFuc
		{
			DefCollectionBindingFuc()
			{
				FnPrepare = &prepareElement<T, V[N]>;
				FnGetElem = &getArrayElement<T, V, N>;
				FnSetElem = &setArrayElement<T, V, N>;
				FnCmpElem = &compareArrayElement<T, V, N>;
				FnAddElem = &addArrayElement<T, V, N>;
				FnDelElem = &deleteArrayElement<T, V, N>;
				FnCount = &getArrayCount<T, V, N>;
				FnIsElemBind = &isElementBinableE<T, V>;
				FnGetBindElem = &getArrayElementBinable<T, V, N>;

				FnGetMapElem = NULL;
				FnAddMapElem = NULL;
				FnGetMapVal = NULL;
				FnDelMapElem = NULL;
			}

			static const DefCollectionBindingFuc<T, V[N]>* BINDINGFUC()
			{
				return &msFunc;
			}
			static DefCollectionBindingFuc<T, V[N]> msFunc;
		};
		template<typename T, typename V, size_t N>
		DefCollectionBindingFuc<T, V[N]> DefCollectionBindingFuc<T, V[N]>::msFunc;

	}//namespace DataBindingUtil


	class BLADE_BASE_API CollectionDataBinding
	{
	protected:
		typedef DataBindingUtil::CollectionBindingFuc BindingFunc;
		const BindingFunc*mFunc;
		DataBinding	mCollectionValueBinding;
	public:
		static const CollectionDataBinding EMPTY;

		inline CollectionDataBinding()
		{
#if BLADE_STDCPP >= BLADE_STDCPP11
			static_assert( std::is_trivially_copyable<CollectionDataBinding>::value, "data binding structure should be trivially copyable" );
#endif
			mFunc = NULL;
		}

		template<typename T, typename V>
		inline CollectionDataBinding(DataBinding valueBinding, T*, V*)
			:mCollectionValueBinding(valueBinding)
		{
			mFunc = DataBindingUtil::DefCollectionBindingFuc<T, V>::BINDINGFUC();
		}

		template<typename T, typename V, size_t N>
		inline CollectionDataBinding(DataBinding valueBinding, T*, V(*)[N])
			:mCollectionValueBinding(valueBinding)
		{
			mFunc = DataBindingUtil::DefCollectionBindingFuc<T, V[N]>::BINDINGFUC();
		}

		template<typename T>
		inline CollectionDataBinding(T* ptr, TStringList T::*pmdata, void (T::*pmEvent)(void*) = NULL)
			:mCollectionValueBinding(ptr, pmdata, pmEvent)
		{
			mFunc = DataBindingUtil::DefCollectionBindingFuc<T, TStringList>::BINDINGFUC();
		}

		template<typename T, typename V>
		inline CollectionDataBinding(T* ptr, Vector<V> T::*pmdata, void (T::*pmEvent)(void*) = NULL)
			:mCollectionValueBinding(ptr, pmdata, pmEvent)
		{
			mFunc = DataBindingUtil::DefCollectionBindingFuc<T,  Vector<V> >::BINDINGFUC();
		}

		template<typename T, typename V>
		inline CollectionDataBinding(T* ptr, List<V> T::*pmdata, void (T::*pmEvent)(void*) = NULL)
			:mCollectionValueBinding(ptr, pmdata, pmEvent)
		{
			mFunc = DataBindingUtil::DefCollectionBindingFuc<T,  List<V> >::BINDINGFUC();
		}

		template<typename T, typename V, typename C>
		inline CollectionDataBinding(T* ptr, Set<V,C> T::*pmdata, void (T::*pmEvent)(void*) = NULL)
			:mCollectionValueBinding(ptr, pmdata, pmEvent)
		{
			mFunc = DataBindingUtil::DefCollectionBindingFuc<T,  Set<V,C> >::BINDINGFUC();
		}

		template<typename T, typename V, size_t N>
		inline CollectionDataBinding(T* ptr, V(T::*pmdata)[N], void (T::*pmEvent)(void*) = NULL)
			:mCollectionValueBinding(ptr, pmdata, pmEvent)
		{
			mFunc = DataBindingUtil::DefCollectionBindingFuc<T, V[N]>::BINDINGFUC();
		}

		/** @brief  */
		inline bool		operator==(const CollectionDataBinding& rhs) const
		{
			return ::memcmp(this, &rhs, sizeof(CollectionDataBinding) ) == 0;
		}

		/** @brief  */
		inline bool		isEmpty() const
		{
			return *this == EMPTY;
		}

		/** @brief  */
		inline bool		getElementData(index_t index, void* pdata, size_t bytes) const
		{
			if( this->isBound() )
				return mFunc->FnGetElem(mCollectionValueBinding.getDataPtr(), index, pdata, bytes);
			else
			{
				assert(false);
				return false;
			}
		}

		/** @brief  */
		inline bool		setElementData(index_t index, const void* data, size_t bytes, bool callEvt = true) const
		{
			if( this->isBound() )
			{
				bool ret = mFunc->FnSetElem(mCollectionValueBinding.getDataPtr(), index, data, bytes);
				if( ret && callEvt)
					this->callEvent();
				return ret;
			}
			else
			{
				assert(false);
				return false;
			}
		}

		/** @brief  */
		inline bool		addElement(const void* data, size_t bytes, bool callEvt = true) const
		{
			if (this->isBound())
			{
				bool ret = mFunc->FnAddElem(mCollectionValueBinding.getDataPtr(), data, bytes) != INVALID_INDEX;
				if (ret && callEvt)
					this->callEvent();
				return ret;
			}
			else
			{
				assert(false);
				return false;
			}
		}

		/** @brief  */
		inline bool		deleteElement(index_t index, bool callEvt = true) const
		{
			if (this->isBound())
			{
				bool ret = mFunc->FnDelElem(mCollectionValueBinding.getDataPtr(), index);
				if (ret && callEvt)
					this->callEvent();
				return ret;
			}
			else
			{
				assert(false);
				return false;
			}
		}

		/** @brief  */
		inline void	callEvent() const
		{
			mCollectionValueBinding.callEvent();
		}

		/** @brief  */
		inline bool		compareElementData(index_t index, const void* pdata, size_t bytes) const
		{
			if( this->isBound() )
				return mFunc->FnCmpElem(mCollectionValueBinding.getDataPtr(), index, pdata, bytes);
			else
				return true;
		}

		/** @brief  */
		inline size_t	getElementCount() const
		{
			//arrays have fixed count, check bound state in internal functions
			if( this->isBound() )
				return mFunc->FnCount(mCollectionValueBinding.getDataPtr());
			else
				return mFunc->FnCount(NULL);
		}

		/** @brief  */
		inline void		prepare(size_t count) const
		{
			if (this->isBound() && mFunc->FnPrepare != NULL)
				mFunc->FnPrepare(mCollectionValueBinding.getDataPtr(), count);
		}

		/** @brief  */
		inline void*		getDataPtr() const
		{
			return mCollectionValueBinding.getDataPtr();
		}

		/** @brief  */
		inline bool			isBound() const
		{
			return mCollectionValueBinding.isBound();
		}

		/** @brief  */
		inline Bindable*		getTarget() const
		{
			return mCollectionValueBinding.getTarget();
		}

		/** @brief  */
		inline bool		rebind(Bindable* target)
		{
			return mCollectionValueBinding.rebind(target);
		}

		/** @brief  */
		inline bool			isElementBindable() const
		{
			return mFunc->FnIsElemBind();
		}

		/** @brief  */
		inline Bindable*	getElementBinable(index_t index) const
		{
			if( !this->isBound() || mFunc->FnGetBindElem == NULL )
				return NULL;
			return mFunc->FnGetBindElem(mCollectionValueBinding.getDataPtr(), index);
		}
	};//class CollectionDataBinding



	class BLADE_BASE_API MapDataBinding
	{
	protected:
		typedef DataBindingUtil::CollectionBindingFuc BindingFunc;
		DataBinding mCollectionValueBinding;
		const BindingFunc*mFunc;
	public:
		static const MapDataBinding EMPTY;

		inline MapDataBinding()
		{
#if BLADE_STDCPP >= BLADE_STDCPP11
			static_assert( std::is_trivially_copyable<DataBinding>::value, "data binding structure should be trivially copyable" );
#endif
			mFunc = NULL;
		}

		template<typename T, typename K, typename V, typename C>
		inline MapDataBinding(const DataBinding& valueBinding, T*, K*, V*, C*)
			:mCollectionValueBinding(valueBinding)
		{
			mFunc = DataBindingUtil::DefMapBindingFuc<T, Map<K, V, C> >::BINDINGFUC();
		}

		template<typename T, typename K, typename V, typename C>
		inline MapDataBinding(T* ptr, Map<K, V, C> T::*pmdata, void (T::*pmEvent)(void*) = NULL)
			:mCollectionValueBinding(ptr, pmdata, pmEvent)
		{
			mFunc = DataBindingUtil::DefMapBindingFuc<T, Map<K,V,C> >::BINDINGFUC();
		}

		/** @brief  */
		inline bool		operator==(const MapDataBinding& rhs) const
		{
			return ::memcmp(this, &rhs, sizeof(MapDataBinding) ) == 0;
		}

		/** @brief  */
		inline bool		isEmpty() const
		{
			return *this == EMPTY;
		}

		/** @brief  */
		inline bool		getElementData(index_t index, void* key, size_t keySize, void* val, size_t valSize) const
		{
			//TODO: index is not stable for map
			if( this->isBound() )
				return mFunc->FnGetMapElem(mCollectionValueBinding.getDataPtr(), index, key, keySize, val, valSize);
			else
			{
				assert(false);
				return false;
			}
		}

		/** @brief  */
		inline bool		getKeyData(index_t index, void* key, size_t keySize) const
		{
			//TODO: index is not stable for map
			if( this->isBound() )
				return mFunc->FnGetElem(mCollectionValueBinding.getDataPtr(), index, key, keySize);
			else
			{
				assert(false);
				return false;
			}
		}

		/** @brief  */
		inline bool		setElementData(index_t index, const void* data, size_t bytes, bool callEvt = true) const
		{
			if( this->isBound() )
			{
				//TODO: index is not stable for map
				bool ret = mFunc->FnSetElem(mCollectionValueBinding.getDataPtr(), index, data, bytes);
				if (ret && callEvt)
					this->callEvent();
				return ret;
			}
			else
			{
				assert(false);
				return false;
			}
		}

		/** @brief  */
		inline bool		addElement(const void* keyData, size_t keyBytes, const void* valData, size_t valBytes, bool callEvt = true) const
		{
			if (this->isBound())
			{
				bool ret = mFunc->FnAddMapElem(mCollectionValueBinding.getDataPtr(), keyData, keyBytes, valData, valBytes) != INVALID_INDEX;
				if (ret && callEvt)
					this->callEvent();
				return ret;
			}
			else
			{
				assert(false);
				return false;
			}
		}

		/** @brief  */
		inline bool		deleteElement(index_t index, bool callEvt = true) const
		{
			if (this->isBound())
			{
				bool ret = mFunc->FnDelElem(mCollectionValueBinding.getDataPtr(), index);
				if (ret && callEvt)
					this->callEvent();
				return ret;
			}
			else
			{
				assert(false);
				return false;
			}
		}

		/** @brief  */
		inline void	callEvent() const
		{
			mCollectionValueBinding.callEvent();
		}

		/** @brief  */
		inline bool		compareElementData(index_t index, const void* pdata, size_t bytes) const
		{
			if( this->isBound() )
				return mFunc->FnCmpElem(mCollectionValueBinding.getDataPtr(), index, pdata, bytes);
			else
				return true;
		}

		/** @brief  */
		inline size_t	getElementCount() const
		{
			if( this->isBound() )
				return mFunc->FnCount(mCollectionValueBinding.getDataPtr());
			else
				return mFunc->FnCount(NULL);
		}

		/** @brief  */
		inline void		prepare(size_t count) const
		{
			if (this->isBound() && mFunc->FnPrepare != NULL)
				mFunc->FnPrepare(mCollectionValueBinding.getDataPtr(), count);
		}

		/** @brief  */
		inline void*		getDataPtr() const
		{
			return mCollectionValueBinding.getDataPtr();
		}

		/** @brief  */
		inline bool			isBound() const
		{
			return mCollectionValueBinding.isBound();
		}

		/** @brief  */
		inline Bindable*		getTarget() const
		{
			return mCollectionValueBinding.getTarget();
		}

		/** @brief  */
		inline bool		rebind(Bindable* target)
		{
			return mCollectionValueBinding.rebind(target);
		}

		/** @brief  */
		inline bool			isElementBindable() const
		{
			return mFunc->FnIsElemBind();
		}

		/** @brief  */
		inline Bindable*	getElementBinable(index_t index) const
		{
			if( !this->isBound() || mFunc->FnGetBindElem == NULL )
				return NULL;
			return mFunc->FnGetBindElem(mCollectionValueBinding.getDataPtr(), index);
		}
	};//class MapDataBinding 
	
}//namespace Blade

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(pop)
#endif

#endif //  __Blade_DataBindingEx_h__