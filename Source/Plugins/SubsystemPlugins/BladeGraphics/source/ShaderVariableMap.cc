/********************************************************************
	created:	2010/09/05
	filename: 	ShaderVariableMap.cc
	author:		Crazii
	purpose:	

*********************************************************************/
#include <BladePCH.h>
#include <ShaderVariableMap.h>
#include <interface/public/graphics/IGraphicsResourceManager.h>
#include "ShaderConstantTypeBind.h"

#define USE_VECTOR_FOR_INSTANCE_MAP 1

namespace Blade
{
	namespace Impl
	{
		class CustomVariable : public AutoShaderVariable
		{
		public:
			CustomVariable()
				:AutoShaderVariable(NULL, SCT_UNDEFINED, 0, SVU_CUSTOM_FLAG)
			{
			}

			~CustomVariable()
			{
			}

			CustomVariable(const CustomVariable& rhs)
				:AutoShaderVariable(rhs.mUpdater, rhs)
			{
				if (rhs.getBuffer() != NULL)
				{
					mData = mBuffer.alloc(EShaderConstantType(mType), mCount);
					assert(mData != NULL);
					assert(mDataDirty == false);
				}
				else
					assert(mType == SCT_UNDEFINED && mCount == 0 && mDataDirty == true);
			}

			/** @brief  */
			void		initialize(SHADER_CONSTANT_TYPE type, size_t count, EShaderVariableUsage usage)
			{
				assert(mBuffer.getBuffer() == NULL);
				mType = type;
				mCount = (uint16)count;
				mSize = (uint16)IGraphicsResourceManager::getSingleton().getShaderConstantSize(mType);
				mBytes = (uint32)mSize * (uint32)mCount;
				mData = mBuffer.alloc(type, mCount);
				mUsage = EShaderVariableUsage(usage | SVU_CUSTOM_FLAG);
				mDataDirty = false;
			}

			/** @brief  */
			void*		getBuffer() const
			{
				return mBuffer.getBuffer();
			}

		protected:
			ShaderVariableData	mBuffer;
		};

		class ShaderVariableMapDataImpl : public Allocatable
		{
		public:
			ShaderVariableMapDataImpl() {}
			~ShaderVariableMapDataImpl() {}

			/** @brief  */
			const HAUTOSHADERVARIABLE&	createSlot(const TString& name, SHADER_CONSTANT_TYPE type, size_t count, EShaderVariableUsage usage)
			{
				CustomVariable* var = BLADE_NEW CustomVariable();
				HAUTOSHADERVARIABLE hVar(var);
				std::pair<VariableMap::iterator, bool> result = mVarMap.insert(std::make_pair(name, hVar));
				if (!result.second)
					BLADE_EXCEPT(EXC_REDUPLICATE, BTString("variable with the same name\" ") + name + BTString("\" already exist."));

				var->initialize(type, count, usage);

				return result.first->second;
			}

			/** @brief  */
			const HAUTOSHADERVARIABLE&	getSlot(const TString& name) const
			{
				VariableMap::const_iterator i = mVarMap.find(name);
				if (i == mVarMap.end())
					return HAUTOSHADERVARIABLE::EMPTY;
				return i->second;
			}

			typedef TStringMap<HAUTOSHADERVARIABLE>	VariableMap;
			VariableMap	mVarMap;
		};

#if !USE_VECTOR_FOR_INSTANCE_MAP
		class CustmInstanceStorage : public ShaderVariableData
		{
		public:
			size_t				mIndex;
			uint16				mCount;
			ShaderConstantType	mType;

			CustmInstanceStorage() {}
			CustmInstanceStorage(size_t index, SHADER_CONSTANT_TYPE type, size_t count)
				:mIndex(index), mCount((uint16)count), mType(type) {}
		};

		class InstanceVariableMapDataImpl : public TStringMap<CustmInstanceStorage>, public Allocatable
		{
		public:
			index_t getIndex(iterator i) const
			{
				return i != this->end() ? i->second.mIndex : INVALID_INDEX;
			}
		};
#else
		class CustmInstanceStorage : public ShaderVariableData
		{
		public:
			uint16				mCount;
			ShaderConstantType	mType;

			CustmInstanceStorage() {}
			CustmInstanceStorage(size_t /*index*/, SHADER_CONSTANT_TYPE type, size_t count)
				:mCount((uint16)count), mType(type) {}
		};
		class InstanceVariableMapDataImpl : public Vector< std::pair<TString, CustmInstanceStorage> >, public Allocatable
		{
		public:
			struct FnFinder : public NonAssignable
			{
				const TString& nameRef;
				FnFinder(const TString& nameref) :nameRef(nameref) {}

				inline bool operator()(const std::pair<TString, CustmInstanceStorage>& target) const
				{
					return target.first == nameRef;
				}
			};

			InstanceVariableMapDataImpl() { this->reserve(4); }
			iterator find(const TString& name)
			{
				return std::find_if(this->begin(), this->end(), FnFinder(name));
			}
			const_iterator find(const TString& name) const
			{
				return std::find_if(this->begin(), this->end(), FnFinder(name));
			}
			inline std::pair<iterator, bool> insert(const value_type& val)
			{
				iterator i = this->find(val.first);
				if (i != this->end())
					return std::make_pair(i, false);
				this->push_back(val);
				return std::make_pair(--this->end(), true);
			}
			index_t getIndex(iterator i) const
			{
				return index_t(i - this->begin());
			}
		};
#endif
	}//namespace Impl
	using namespace Impl;

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool ShaderVariableMapBase::Handles::setData(const void* data, size_t size)
	{
		size_t count = mVariables.size();
		if (count == 0)
			return false;
		for (size_t i = 0; i < count; ++i)
		{
			CustomVariable* v = mVariables.getAt<CustomVariable>(i);
			size_t sz = v->getRealSize();
			if (sz < size)
			{
				assert(false);
				return false;
			}
			assert(ShaderConstantType::getConstantSize(v->getType())*v->getCount() == size);
			ShaderVariableMapBase::writeData(v->getBuffer(), data, v->getType(), v->getCount());
		}
		return true;
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	ShaderVariableMapBase::ShaderVariableMapBase(EShaderVariableUsage usage)
		:mUsage(usage)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	ShaderVariableMapBase::~ShaderVariableMapBase()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	ShaderVariableMapBase::ShaderVariableMapBase(const ShaderVariableMapBase& rhs)
	{
		if( rhs.mData == NULL && mData != NULL)
			mData.destruct();
		else if( rhs.mData != NULL && mData != NULL )
			*mData = *rhs.mData;
		else if( rhs.mData != NULL && mData == NULL )
			*mData = *rhs.mData;

	}

	//////////////////////////////////////////////////////////////////////////
	ShaderVariableMapBase&	ShaderVariableMapBase::operator=(const ShaderVariableMapBase& rhs)
	{
		if( this == &rhs)
			return *this;

		if( rhs.mData == NULL && mData != NULL)
			mData.destruct();
		else if( rhs.mData != NULL && mData != NULL )
			*mData = *rhs.mData;
		else if( rhs.mData != NULL && mData == NULL )
			*mData = *rhs.mData;

		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	const HAUTOSHADERVARIABLE&	ShaderVariableMapBase::createVariable(const TString& name,SHADER_CONSTANT_TYPE type,size_t count)
	{
		const HAUTOSHADERVARIABLE& var = mData->createSlot(name, type, count, mUsage);
		return var;
	}

	//////////////////////////////////////////////////////////////////////////
	void			ShaderVariableMapBase::clear()
	{
		if( mData != NULL )
			mData.destruct();
	}

	//////////////////////////////////////////////////////////////////////////
	size_t			ShaderVariableMapBase::size() const
	{
		return mData == NULL ? 0 : mData->mVarMap.size();
	}

	//////////////////////////////////////////////////////////////////////////
	bool			ShaderVariableMapBase::writeVariable(const TString& name, SHADER_CONSTANT_TYPE type, size_t count, const void* source)
	{
		if(source == NULL)
		{
			assert(false);
			return false;
		}
		//shader constant maybe optimized out
		if (mData == NULL)
			return false;

		const HAUTOSHADERVARIABLE& hVar = this->getVariable(name);
		if( hVar == HAUTOSHADERVARIABLE::EMPTY )
		{
			//assert(false);
			BLADE_LOG(Error, BTString("custom shader variable not found:") << name);
			return false;
		}
		if( hVar->getType() != type || hVar->getCount() != count )
		{
			assert(false);
			return false;
		}

		Impl::CustomVariable* var = static_cast<Impl::CustomVariable*>(hVar);
		this->writeData(var->getBuffer(),source,type,count);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	const HAUTOSHADERVARIABLE&	ShaderVariableMapBase::getVariable(const TString& name) const
	{
		if( mData == NULL )
			return HAUTOSHADERVARIABLE::EMPTY;
		return mData->getSlot(name);
	}

	//////////////////////////////////////////////////////////////////////////
	bool			ShaderVariableMapBase::getVariableHandle(Handles& outHandle, const TString& name, SHADER_CONSTANT_TYPE type, size_t count) const
	{
		const HAUTOSHADERVARIABLE& v = this->getVariable(name);
		if (v == NULL || v->getType() != type || v->getCount() != count)
			return false;
		CustomVariable* cv = v;
		outHandle.mVariables.push_back(cv);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void			ShaderVariableMapBase::writeData(void* data, const void* source, SHADER_CONSTANT_TYPE type, size_t count)
	{
		const char* src = static_cast<const char*>(source);
		char* dst = static_cast<char*>(data);

		size_t srcStride = ShaderConstantType::getConstantSize(type);
		size_t dstStride = IGraphicsResourceManager::getSingleton().getShaderConstantSize(type);

		//size_t copyStride = std::min(srcStride,dstStride);
		assert( srcStride <= dstStride );
		if (srcStride == dstStride)
		{
			std::memcpy(dst, src, srcStride * count);
		}
		else for(size_t i = 0; i < count; ++i)
		{
			std::memcpy(dst,src,srcStride);
			src += srcStride;
			dst += dstStride;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void*			ShaderVariableMapBase::getVariableData(const HAUTOSHADERVARIABLE& hAsv)
	{
		return static_cast<Impl::CustomVariable*>(hAsv)->getBuffer();
	}

	//////////////////////////////////////////////////////////////////////////
	const ShaderVariableMap ShaderVariableMap::EMPTY;

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	InstanceVariableMap::InstanceVariableMap()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	InstanceVariableMap::~InstanceVariableMap()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void			InstanceVariableMap::clear()
	{
		if( mData != NULL )
			mData.destruct();
	}

	//////////////////////////////////////////////////////////////////////////
	size_t			InstanceVariableMap::size() const
	{
		return mData != NULL ? mData->size() : 0;
	}

	//////////////////////////////////////////////////////////////////////////
	void*			InstanceVariableMap::createStorage(const TString& name, SHADER_CONSTANT_TYPE type, size_t count)
	{
		size_t index = this->size();
		std::pair<InstanceVariableMapDataImpl::iterator, bool> ret = mData->insert(std::make_pair(name, CustmInstanceStorage(index, type, count)) );
		CustmInstanceStorage& storage = ret.first->second;
		if( !ret.second )
		{
			if( storage.mType == type && storage.mCount == count )
				return storage.getBuffer();
			else
			{
				assert(false);
				return NULL;
			}
		}
		return storage.alloc(type, count);
	}

	//////////////////////////////////////////////////////////////////////////
	bool			InstanceVariableMap::writeStorage(const TString& name, SHADER_CONSTANT_TYPE type, size_t count, const void* source)
	{
		if( mData == NULL )
			return false;

		InstanceVariableMapDataImpl::const_iterator i = mData->find(name);
		if( i != mData->end() )
		{
			const CustmInstanceStorage& storage = i->second;
			if( storage.mType != type || storage.mCount != count )
			{
				assert(false);
				return false;
			}

			const char* src = static_cast<const char*>(source);
			char* dst = static_cast<char*>(storage.getBuffer());

			size_t srcStride = ShaderConstantType::getConstantSize(type);
			size_t dstStride = IGraphicsResourceManager::getSingleton().getShaderConstantSize(type);

			//size_t copyStride = std::min(srcStride,dstStride);
			assert( srcStride <= dstStride );
			for(size_t j = 0; j < count; ++j)
			{
				std::memcpy(dst,src,srcStride);
				src += srcStride;
				dst += dstStride;
			}
			return true;
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	const void*		InstanceVariableMap::getStorage(const TString& name, SHADER_CONSTANT_TYPE type, size_t count) const
	{
		if( mData == NULL )
			return NULL;
		InstanceVariableMapDataImpl::const_iterator i = mData->find(name);
		if( i != mData->end() )
		{
			const CustmInstanceStorage& storage = i->second;
			if( storage.mType == type && storage.mCount == count )
				return i->second.getBuffer();
			else
				assert(false);
		}
		assert(false);
		return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	void*			InstanceVariableMap::getStorage(const TString& name, SHADER_CONSTANT_TYPE type, size_t count)
	{
		if( mData == NULL )
			return NULL;
		InstanceVariableMapDataImpl::iterator i = mData->find(name);
		if( i != mData->end() )
		{
			const CustmInstanceStorage& storage = i->second;
			if( storage.mType == type && storage.mCount == count )
				return i->second.getBuffer();
			else
				assert(false);
		}
		return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	index_t			InstanceVariableMap::getStorageIndex(const TString& name)
	{
		if (mData == NULL)
			return INVALID_INDEX;
		InstanceVariableMapDataImpl::iterator i = mData->find(name);
		if (i != mData->end())
		{
			return mData->getIndex(i);
		}
		return INVALID_INDEX;
	}

	//////////////////////////////////////////////////////////////////////////
	const void*		InstanceVariableMap::getStorage(index_t index) const
	{
		if (mData == NULL)
			return NULL;

		if (index < mData->size())
		{
			InstanceVariableMapDataImpl::const_iterator i = mData->begin();
			std::advance(i, index);
			return i->second.getBuffer();
		}
		return NULL;
	}

}//namespace Blade