/********************************************************************
	created:	2013/11/23
	filename: 	DataSourceDataBase.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "DataSourceDataBase.h"
#include <ConfigTypes.h>

namespace Blade
{
	template class Factory<IDataSourceDataBase>;

#if BLADE_COMPILER_GNU_EXTENSION
	const IDataSource::BID IDataSource::DEFAULT_BID;
	const IDataSource::BID IDataSource::INVALID_BID;
#endif

	class DataSourceFinder : public IDataSource, public NonAssignable
	{
	public:
		const TString&	mNameRef;
		signature_t		mSignature;
		DataSourceFinder(const TString& nameRef) : mNameRef(nameRef)	{mSignature = DS_INVALID_SIGNATURE;}
		DataSourceFinder(signature_t signature)	:mNameRef(TString::EMPTY),mSignature(signature)	{}
		virtual signature_t			getSignature() const	{return mSignature;}
		virtual const TString&		getName() const			{return mNameRef;}
		virtual BID					bindTarget(Bindable* )	{return false;}
		virtual bool				unbindTarget(BID)		{return false;}
		virtual const HCONFIG&		getConfig(BID) const	{return HCONFIG::EMPTY;}
		virtual HDATASOURCE			clone(const TString&, CONFIG_ACCESS) const{return HDATASOURCE::EMPTY;}
		virtual HCONFIG				cloneConfig(CONFIG_ACCESS, const TString&) const	{return HCONFIG::EMPTY;}
	};

	//////////////////////////////////////////////////////////////////////////
	class ThreadDataSource : public IDataSource, public Allocatable
	{
		static const uint MAX_PARALLEL = 4;
	public:
		ThreadDataSource(const HDATASOURCE& hSource)
		{
			BLADE_TS_VERIFY(TS_MAIN_SYNC);
			mParallelSource[0] = hSource;
			for(uint i = 1; i < MAX_PARALLEL; ++i)
				mParallelSource[i] = hSource->clone();
		}

		/** @brief  */
		virtual signature_t			getSignature() const	{return mParallelSource[0]->getSignature();}

		/** @brief  */
		virtual const TString&		getName() const			{return mParallelSource[0]->getName();}

		/** @brief bind target to all version  */
		virtual BID				bindTarget(Bindable* bindable)
		{
			for(uint i = 0; ; ++i)
			{
				BID bid = i % MAX_PARALLEL;
				if( mLock[bid].tryLock() )
				{
					BID ret = mParallelSource[bid]->bindTarget(bindable);
					if(ret != INVALID_BID)
						return bid;
					else
						assert(false);
				}
			}
		}

		/** @brief  */
		virtual bool				unbindTarget(BID bid)
		{
			if( bid >= MAX_PARALLEL || !mLock[bid].isLocked() )
			{
				assert(false);
				return false;
			}
			bool ret = mParallelSource[bid]->unbindTarget(DEFAULT_BID);
			if(ret)
				mLock[bid].unlock();
			return ret;
		}

		/** @brief  */
		virtual const HCONFIG&		getConfig(BID bid) const
		{
			if( bid >= MAX_PARALLEL || !mLock[bid].isLocked() )
			{
				assert(false);
				return HCONFIG::EMPTY;
			}
			return mParallelSource[bid]->getConfig(DEFAULT_BID);
		}

		/** @brief  */
		virtual HDATASOURCE			clone(const TString& name/* = TString::EMPTY*/, CONFIG_ACCESS caf/* = CAF_NONE*/) const
		{
			BLADE_TS_VERIFY(TS_MAIN_SYNC);
			return mParallelSource[0]->clone(name, caf);
		}

		/** @brief  */
		virtual HCONFIG				cloneConfig(CONFIG_ACCESS caf, const TString& name) const
		{
			BLADE_TS_VERIFY(TS_MAIN_SYNC);
			return mParallelSource[0]->cloneConfig(caf, name);
		}
	protected:

		HDATASOURCE	mParallelSource[MAX_PARALLEL];
		Lock		mLock[MAX_PARALLEL];
	};


	//////////////////////////////////////////////////////////////////////////
	DataSourceDataBase::DataSourceDataBase()
	{
		//init atom configs:
		mAtomSet[ DS_SIGNATURE(int8) ].bind( BLADE_NEW ConfigAtom(TString::EMPTY, Variant( (int8)0 ) ) );
		mAtomSet[ DS_SIGNATURE(uint8) ].bind( BLADE_NEW ConfigAtom(TString::EMPTY, Variant( (uint8)0 ) ) );

		mAtomSet[ DS_SIGNATURE(int16) ].bind( BLADE_NEW ConfigAtom(TString::EMPTY, Variant( (int16)0 ) ) );
		mAtomSet[ DS_SIGNATURE(uint16) ].bind( BLADE_NEW ConfigAtom(TString::EMPTY, Variant( (uint16)0 ) ) );

		mAtomSet[ DS_SIGNATURE(int32) ].bind( BLADE_NEW ConfigAtom(TString::EMPTY, Variant( (int32)0 ) ) );
		mAtomSet[ DS_SIGNATURE(uint32) ].bind( BLADE_NEW ConfigAtom(TString::EMPTY, Variant( (uint32)0 ) ) );

		mAtomSet[ DS_SIGNATURE(int64) ].bind( BLADE_NEW ConfigAtom(TString::EMPTY, Variant( (int64)0 ) ) );
		mAtomSet[ DS_SIGNATURE(uint64) ].bind( BLADE_NEW ConfigAtom(TString::EMPTY, Variant( (uint64)0 ) ) );

		mAtomSet[ DS_SIGNATURE(fp32) ].bind( BLADE_NEW ConfigAtom(TString::EMPTY, Variant( (fp32)0 ) ) );
		mAtomSet[ DS_SIGNATURE(fp64) ].bind( BLADE_NEW ConfigAtom(TString::EMPTY, Variant( (fp64)0 ) ) );

		mAtomSet[ DS_SIGNATURE(bool) ].bind( BLADE_NEW ConfigAtom(TString::EMPTY, Variant( false ) ) );
		mAtomSet[ DS_SIGNATURE(String) ].bind( BLADE_NEW ConfigAtom(TString::EMPTY, Variant( String::EMPTY ) ) );
		mAtomSet[ DS_SIGNATURE(WString) ].bind( BLADE_NEW ConfigAtom(TString::EMPTY, Variant( WString::EMPTY ) ) );
		mAtomSet[ DS_SIGNATURE(void*) ].bind( BLADE_NEW ConfigAtom(TString::EMPTY, Variant( (void*)NULL ) ) );
		mAtomSet[ DS_SIGNATURE(Color) ].bind( BLADE_NEW ConfigAtom(TString::EMPTY, Variant( Color::WHITE ) ) );
		mAtomSet[ DS_SIGNATURE(Vector2) ].bind( BLADE_NEW ConfigAtom(TString::EMPTY, Variant( Vector2::ZERO ) ) );
		mAtomSet[ DS_SIGNATURE(Vector3) ].bind( BLADE_NEW ConfigAtom(TString::EMPTY, Variant( Vector3::ZERO ) ) );
		mAtomSet[ DS_SIGNATURE(Vector4) ].bind( BLADE_NEW ConfigAtom(TString::EMPTY, Variant( Vector4::ZERO ) ) );
		mAtomSet[ DS_SIGNATURE(Quaternion) ].bind( BLADE_NEW ConfigAtom(TString::EMPTY, Variant( Quaternion::IDENTITY) ) );
	}

	//////////////////////////////////////////////////////////////////////////
	DataSourceDataBase::~DataSourceDataBase()
	{

	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void			DataSourceDataBase::shutdown()
	{
		//note: data sources usually hold data from other libraries, clear them
		assert(mDataList.size() == mDatabase.size() );
		mDataList.clear();
		mDatabase.clear();
		mDataSignature.clear();
		mAtomSet.clear();
	}

	//////////////////////////////////////////////////////////////////////////
	bool			DataSourceDataBase::registerDataSource(const HDATASOURCE& source)
	{
		BLADE_TS_VERIFY(TS_MAIN_SYNC);
		assert(mDataList.size() == mDatabase.size() );
		if( source == NULL || source->getName() == TString::EMPTY || source->getSignature() == NULL )
		{
			assert(false);
			return false;
		}
		HDATASOURCE hSource;
		hSource.bind( BLADE_NEW ThreadDataSource(source)  );
		std::pair<DataSourceSet::iterator, bool> ret = mDatabase.insert( hSource );
		if( ret.second )
		{
			mDataList.push_back(hSource);
			bool result = mDataSignature.insert( hSource ).second;
			assert(result);
			BLADE_UNREFERENCED(result);
		}
		return ret.second;
	}

	//////////////////////////////////////////////////////////////////////////
	IDataSource*	DataSourceDataBase::getDataSource(IDataSource::signature_t signature) const
	{
		if( signature == NULL )
			return NULL;

		DataSourceFinder finder(signature);
		DataSourceSignatureSet::const_iterator i = mDataSignature.find( &finder );
		if( i != mDataSignature.end() )
			return *i;
		else
			return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	IDataSource*	DataSourceDataBase::getDataSource(const TString& typeName) const
	{
		assert(mDataList.size() == mDatabase.size() );
		DataSourceFinder finder(typeName);
		DataSourceSet::const_iterator i = mDatabase.find( &finder);
		if( i != mDatabase.end() )
			return *i;
		else
			return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t			DataSourceDataBase::getDataSourceCount() const
	{
		assert(mDataList.size() == mDatabase.size() );
		return mDatabase.size();
	}

	//////////////////////////////////////////////////////////////////////////
	IDataSource*	DataSourceDataBase::getDataSource(index_t index) const
	{
		assert(mDataList.size() == mDatabase.size() );
		if( index < mDataList.size() )
		{
			DataSourceList::const_iterator i = mDataList.begin();
			std::advance(i, index);
			return *i;
		}
		else
			return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	const HCONFIG&	DataSourceDataBase::getAtomConfig(IDataSource::signature_t signature) const
	{
		if( signature != NULL )
		{
			AtomConfigSet::const_iterator i = mAtomSet.find(signature);
			if( i != mAtomSet.end() )
				return i->second;
		}
		return HCONFIG::EMPTY;
	}
	
}//namespace Blade