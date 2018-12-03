/********************************************************************
	created:	2013/11/23
	filename: 	DataSource.h
	author:		Crazii
	purpose:	data source hold config / data binding info for one struct/class
		note:	this file is not exported and is compiled at user end, so
				use std templates directly is OK.
*********************************************************************/
#ifndef __Blade_DataSource_h__
#define __Blade_DataSource_h__
#include <BladeFramework.h>
#include <utility/DataBinding.h>
#include <databinding/IDataSourceDataBase.h>
#include <Pimpl.h>

namespace Blade
{
	namespace Impl
	{
		class SubDataSourceSetImpl;
	}//namespace Impl

	class BLADE_FRAMEWORK_API DataSource : public IDataSource, public Allocatable, public NonCopyable
	{
	protected:
		DataSource(signature_t signature, const HCONFIG& config);
		DataSource(const TString& name, signature_t signature, const HCONFIG& config, CONFIG_ACCESS access);
	public:
		DataSource(const TString& name, signature_t signature, IDataSource* derived = NULL);
		~DataSource();

		/************************************************************************/
		/*                                                                      */
		/************************************************************************/
		/** @note: the difference between signature & typeName: typeName is constant among all compilers
		 * so it is used for persistent serialization, while signature maybe different among different run-times
		 * it is only used at run time to get source data from type info.
		 */
		virtual signature_t			getSignature() const{return mSignature;}

		/** @brief  */
		virtual const TString&		getName() const		{return mConfig->getName();}

		/** @brief  */
		virtual const HCONFIG&		getConfig(BID) const{return mConfig;}

		/** @brief  */
		virtual BID					bindTarget(Bindable* bindable);

		/** @brief  */
		virtual bool				unbindTarget(BID)
		{
			return mTarget == NULL || this->bindTarget(NULL) == DEFAULT_BID;
		}

		/** @brief  */
		virtual HDATASOURCE			clone(const TString& name = TString::EMPTY, CONFIG_ACCESS caf = CAF_NONE) const;

		/** @brief  */
		virtual HCONFIG				cloneConfig(CONFIG_ACCESS caf, const TString& name) const
		{
			return mConfig->clone(caf, name);
		}

	public:
		/************************************************************************/
		/*                                                                      */
		/************************************************************************/
		/** @brief  */
		virtual const HCONFIG&		getConfig() const { return mConfig; }
		/** @brief  */
		Bindable*		getTarget() const	{return mTarget;}
		/** @brief get super/base source */
		IDataSource*	getSuper() const	{return mSuper;}

		/** @brief  */
		bool	addSubSource(Version version, const HDATASOURCE& sub, const DataBinding& this2Sub);

		/** @brief  */
		bool	addSubSource(const signature_t signature, const DataBinding& this2Sub, Version version, const HCONFIG& subConfig);

	protected:

		signature_t			mSignature;
		Bindable*			mTarget;
		HCONFIG				mConfig;
		IDataSource*		mSuper;
		LazyPimpl<Impl::SubDataSourceSetImpl>	mSubSource;
	};

}//namespace Blade


#endif //  __Blade_DataSource_h__