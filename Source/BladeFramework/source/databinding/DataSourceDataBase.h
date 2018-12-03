/********************************************************************
	created:	2013/11/23
	filename: 	DataSourceDataBase.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_DataSourceDataBase_h__
#define __Blade_DataSourceDataBase_h__
#include <BladePCH.h>
#include <Singleton.h>
#include <utility/BladeContainer.h>
#include <databinding/IDataSourceDataBase.h>

namespace Blade
{
	class DataSourceDataBase : public IDataSourceDataBase, public Singleton<DataSourceDataBase>
	{
	public:
		DataSourceDataBase();
		~DataSourceDataBase();

		/************************************************************************/
		/*                                                                      */
		/************************************************************************/

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			shutdown();

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			registerDataSource(const HDATASOURCE& source);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual IDataSource*	getDataSource(IDataSource::signature_t signature) const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual IDataSource*	getDataSource(const TString& typeName) const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual size_t			getDataSourceCount() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual IDataSource*	getDataSource(index_t index) const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const HCONFIG&	getAtomConfig(IDataSource::signature_t signature) const;

	protected:
		typedef Set<IDataSource*, FnDataSourceLessName>			DataSourceSet;
		typedef List<HDATASOURCE> DataSourceList;
		typedef Set<IDataSource*, FnDataSourceLessSignature>	DataSourceSignatureSet;
		typedef Map<IDataSource::signature_t, HCONFIG, FnDataSourceSignatureLess> AtomConfigSet;

		DataSourceSet			mDatabase;
		DataSourceList			mDataList;
		DataSourceSignatureSet	mDataSignature;
		AtomConfigSet			mAtomSet;
	};//DataSourceDataBase
	
}//namespace Blade

#endif //  __Blade_DataSourceDataBase_h__