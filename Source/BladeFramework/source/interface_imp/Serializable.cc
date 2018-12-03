/********************************************************************
	created:	2013/03/05
	filename: 	SyncCallback.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <interface/public/ISerializable.h>

namespace Blade
{
	namespace Impl
	{
		struct SPercentData
		{
			scalar	mRate;
			scalar	mPercent;

			explicit SPercentData(scalar rate)
			{
				mPercent = 0.0;
				mRate = rate;
			}
		};

		class ProgressNotifierImpl : public List<SPercentData>, public Allocatable	{};

		struct SSerializableEntry
		{
			TString			name;
			ISerializable*	data;
			bool operator<(const SSerializableEntry& rhs) const
			{
				return FnTStringFastLess::compare(name, rhs.name);
			}

			SSerializableEntry(const TString& _name, ISerializable* _data) :name(_name), data(_data)	{}
		};

		class SerializableMapImpl : public Allocatable
		{
		public:
			typedef Set<SSerializableEntry>		SerializableSet;
			//TODO: use List instead of Vector?
			typedef Vector<const SSerializableEntry*>	SerializableList;

			static const size_t INIT_LIST = 32;
			SerializableMapImpl()
			{
				mList.reserve(INIT_LIST);
			}

			/** @brief  */
			bool		add(const TString& name, ISerializable* data)
			{
				assert( mList.size() == mSet.size() );
				std::pair<SerializableSet::iterator,bool> ret = mSet.insert( SSerializableEntry(name, data) );
				if( ret.second )
					mList.push_back( &(*(ret.first)) );
				return ret.second;
			}

			/** @brief  */
			size_t		count() const
			{
				assert( mList.size() == mSet.size() );
				return mList.size();
			}

			/** @brief  */
			ISerializable*	get(const TString& name) const
			{
				assert( mList.size() == mSet.size() );
				SerializableSet::iterator i = mSet.find( SSerializableEntry(name, NULL) );
				if( i != mSet.end() )
					return i->data;
				else
					return NULL;
			}
			
			/** @brief  */
			const SSerializableEntry*	get(index_t index) const
			{
				assert( mList.size() == mSet.size() );
				if( index < mList.size() )
					return mList[index];
				else
					return NULL;
			}
			SerializableSet		mSet;
			SerializableList	mList;
		};
	}
	using namespace Impl;


	const ProgressNotifier ProgressNotifier::EMPTY(false);

	//////////////////////////////////////////////////////////////////////////
	ProgressNotifier::ProgressNotifier()
		:mPrecent(0)
		,mImpl( BLADE_NEW Impl::ProgressNotifierImpl() )
	{

	}

	//////////////////////////////////////////////////////////////////////////
	ProgressNotifier::ProgressNotifier(IProgressCallback* rhs)
		:mPrecent(0)
		,mImpl(BLADE_NEW Impl::ProgressNotifierImpl())
	{
		mRef = rhs;
		if (rhs)
			rhs->setRef(this);
	}

	//////////////////////////////////////////////////////////////////////////
	ProgressNotifier::ProgressNotifier(bool)
		:mImpl(NULL)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	ProgressNotifier::~ProgressNotifier()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	void ProgressNotifier::beginStep(scalar rate) const
	{
		if(mImpl != NULL)
			mImpl->push_back( Impl::SPercentData(rate) );
	}

	//////////////////////////////////////////////////////////////////////////
	void ProgressNotifier::onStep(size_t index, size_t count) const
	{
		if (mImpl != NULL)
		{
			//avoid floating point precision issue
			scalar percent = index + 1 == count ? scalar(1.0) : (scalar)(index + 1) / (scalar)count;
			this->onNotify(percent);
		}
	}
	
	//////////////////////////////////////////////////////////////////////////
	void	ProgressNotifier::onNotify(scalar rate) const
	{
		if( this->getRef() == NULL )
			return;

		IProgressCallback* callback = static_cast<IProgressCallback*>( this->BiRef::getRef() );
		if(mImpl->size() == 0 )
		{
			mPrecent += rate;
			return callback->onNotify( scalar(mPrecent) );
		}
		else
		{
			scalar base = (*mImpl).front().mRate;
			for(Impl::ProgressNotifierImpl::iterator i = ++mImpl->begin(); i != mImpl->end(); ++i)
				base *= (*i).mRate;

			scalar percent = rate;
			callback->onNotify( scalar(mPrecent + base*percent) );

			if (rate >= scalar(1.0) - Math::HIGH_EPSILON) //step finished
			{
				mPrecent += base;
				scalar lastRate = mImpl->back().mRate;
				mImpl->pop_back();

				while (mImpl->size() > 0)
				{
					mImpl->back().mPercent += lastRate;
					if (mImpl->back().mPercent > 0.999)
					{
						lastRate = mImpl->back().mRate;
						mImpl->pop_back();
					}
					else
						break;
				}
			}
		}
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	SerializableMap::SerializableMap()
		:mData( BLADE_NEW SerializableMapImpl() )
	{

	}

	//////////////////////////////////////////////////////////////////////////
	SerializableMap::~SerializableMap()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	ISerializable*	SerializableMap::getSerializable(const TString& name) const
	{
		if(name == TString::EMPTY )
		{
			assert(false);
			return NULL;
		}
		return mData->get(name);
	}

	//////////////////////////////////////////////////////////////////////////
	bool			SerializableMap::addSerializable(const TString& name, ISerializable* serializable)
	{
		if(name == TString::EMPTY || serializable == NULL )
		{
			assert(false);
			return false;
		}
		return mData->add(name, serializable);
	}

	//////////////////////////////////////////////////////////////////////////
	size_t			SerializableMap::getCount() const
	{
		return mData->count();
	}

	//////////////////////////////////////////////////////////////////////////
	bool			SerializableMap::getSerializable(index_t index, TString& name, ISerializable*& serialziable) const
	{
		const SSerializableEntry* entry = mData->get(index);
		if( entry == NULL )
			return false;
		else
		{
			name = entry->name;
			serialziable = entry->data;
			return true;
		}
	}


}//namespace Blade