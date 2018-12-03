/********************************************************************
	created:	2011/04/23
	filename: 	IDataSourceDataBase.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_IDataSourceDataBase_h__
#define __Blade_IDataSourceDataBase_h__
#include <Factory.h>
#include <utility/Bindable.h>
#include <interface/InterfaceSingleton.h>
#include <BladeFramework.h>
#include <interface/IConfig.h>

namespace Blade
{
	///note: type_info returned by typeid has lifetime of the app (std C++)
#if BLADE_COMPILER == BLADE_COMPILER_CLANG

	template<typename T>
	const std::type_info* BLADE_ALWAYS_INLINE Blade_DS_Signature()
	{
		typedef typename BindableType<T>::type TYPE;
		return &typeid(TYPE);
	}
#define DS_SIGNATURE(_TYPE) (Blade_DS_Signature<_TYPE>())

#else
#define DS_SIGNATURE(_TYPE) (&typeid(BindableType<_TYPE>::type))	//TODO: decay
#endif
#define DS_INVALID_SIGNATURE (NULL)

	///note: data source is a complete data binding set of single struct/class
	class IDataSource;
	typedef Handle<IDataSource> HDATASOURCE;
	class BLADE_FRAMEWORK_API IDataSource
	{
	public:
		typedef uint BID;
		typedef const std::type_info*	signature_t;
		static const BID DEFAULT_BID = 0;
		static const BID INVALID_BID = uint(-1);

		virtual ~IDataSource()	{}

		/** @note: the difference between signature & typeName: typeName is constant among all compilers
		 * so it is used for persistent serialization, while signature maybe different among different run-times
		 * it is only used at run time to get source data from type info.
		 Object of the same type may have different signature instances among different libraries(.so/.DLL),
		 but they are equal (using std::type_info::before).
		 */
		virtual signature_t			getSignature() const = 0;

		/** @brief  */
		virtual const TString&		getName() const = 0;

		/** @brief bind target to all version  */
		virtual BID					bindTarget(Bindable* bindable) = 0;

		/** @brief  */
		virtual bool				unbindTarget(BID bid) = 0;

		/** @brief  */
		virtual const HCONFIG&		getConfig(BID bid) const = 0;

		/** @brief  */
		virtual HDATASOURCE			clone(const TString& name = TString::EMPTY, CONFIG_ACCESS caf = CAF_NONE) const = 0;

		/** @brief  */
		virtual HCONFIG				cloneConfig(CONFIG_ACCESS caf, const TString& name) const = 0;

	};//IDataSource

	/** @brief  */
	struct FnDataSourceLessName
	{
		bool operator()(const IDataSource* lhs,const IDataSource* rhs) const
		{
			return FnTStringFastLess::compare(lhs->getName(),rhs->getName());
		}
	};

	struct FnDataSourceLessSignature
	{
		bool operator()(const IDataSource* lhs,const IDataSource* rhs) const
		{
			//std c++: type_info::before should return bool, but MSVC returns int
#if BLADE_COMPILER == BLADE_COMPILER_MSVC
			return lhs->getSignature()->before( *rhs->getSignature() ) != 0;
#else
			return lhs->getSignature()->before( *rhs->getSignature() );
#endif
		}
	};

	struct FnDataSourceSignatureLess
	{
		bool operator()(IDataSource::signature_t lhs, IDataSource::signature_t rhs) const
		{
#if BLADE_COMPILER == BLADE_COMPILER_MSVC
			return lhs->before( *rhs ) != 0;
#else
			return lhs->before( *rhs );
#endif
		}
	};

	/** @brief data source data base */
	class IDataSourceDataBase: public InterfaceSingleton<IDataSourceDataBase>
	{
	public:
		virtual ~IDataSourceDataBase()		{}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void			shutdown() = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool			registerDataSource(const HDATASOURCE& source) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual IDataSource*	getDataSource(IDataSource::signature_t signature) const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual IDataSource*	getDataSource(const TString& typeName) const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual size_t			getDataSourceCount() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual IDataSource*	getDataSource(index_t index) const = 0;

		/**
		@describe 
		@param 
		@return handle to a ConfigAtom type
		*/
		virtual const HCONFIG&	getAtomConfig(IDataSource::signature_t signature) const = 0;

	};//class IDataSourceDataBase
	extern template class BLADE_FRAMEWORK_API Factory<IDataSourceDataBase>;

	//data binding helper
	class DataBinder : public NonCopyable, public NonAssignable, public Allocatable
	{
	public:
		/** @brief  */
		inline DataBinder()	{mDataSource = NULL;mBID = IDataSource::INVALID_BID;}
		/** @brief  */
		inline DataBinder(const TString& typeName, Bindable* target, bool permanent = false)
		{
			mDataSource = NULL;
			mBID = IDataSource::INVALID_BID;
			this->bindTarget(typeName, target, permanent);
		}

		/** @brief  */
		template<typename T>
		inline DataBinder(T* target, bool permanent = false)
		{
			mDataSource = NULL;
			mBID = IDataSource::INVALID_BID;
			this->bindTarget(DS_SIGNATURE(T), target, permanent);
		}

		/** @brief  */
		inline ~DataBinder()
		{
			if( !mPermanent )
				this->unbindTarget();
		}

		/** @brief  */
		const HCONFIG&	getConfig() const
		{
			if( mDataSource != NULL )
				return mDataSource->getConfig(mBID);
			else
				return HCONFIG::EMPTY;
		}

		/** @brief  */
		bool bindTarget(const TString& typeName, Bindable* target, bool permanent = false)
		{
			if( this->isBound() )
			{
				assert(false);
				return false;
			}
			mDataSource = IDataSourceDataBase::getSingleton().getDataSource(typeName);
			if( mDataSource != NULL )
			{
				mBID = mDataSource->bindTarget(target);
				if( mBID == IDataSource::INVALID_BID )
					mDataSource = NULL;
			}
			mPermanent = permanent;
			return mDataSource != NULL;
		}

		/** @brief  */
		bool bindTarget(const IDataSource::signature_t signature, Bindable* target, bool permanent = false)
		{
			if( this->isBound() )
			{
				assert(false);
				return false;
			}
			mDataSource = IDataSourceDataBase::getSingleton().getDataSource(signature);
			if( mDataSource != NULL )
			{
				mBID = mDataSource->bindTarget(target);
				if( mBID == IDataSource::INVALID_BID )
					mDataSource = NULL;
			}
			mPermanent = permanent;
			return mDataSource != NULL;
		}

		/** @brief  */
		bool	unbindTarget()
		{
			if( this->isBound() )
			{
				assert( !mPermanent );
				bool ret = mDataSource->unbindTarget(mBID);
				if( ret )
					mDataSource = NULL;
				return ret;
			}
			else
				return false;
		}

		/** @brief  */
		inline bool		isBound() const		{return mDataSource != NULL;}

		/** @brief  */
		IDataSource*	getDataSource() const	{return mDataSource;}
	protected:
		IDataSource*		mDataSource;
		IDataSource::BID	mBID;
		bool				mPermanent;
	};//DataBinder

}//namespace Blade


#endif //  __Blade_IDataSourceDataBase_h__