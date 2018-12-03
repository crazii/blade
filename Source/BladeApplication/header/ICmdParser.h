/********************************************************************
	created:	2011/08/28
	filename: 	ICmdParser.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_ICmdParser_h__
#define __Blade_ICmdParser_h__
#include <BladeApplication.h>
#include <utility/StringList.h>
#include <interface/InterfaceSingleton.h>
#include <utility/Variant.h>
#include <IAppConfig.h>

namespace Blade
{
	typedef enum ECmdArgType
	{

		CAT_INT,
		CAT_FLOAT,
		CAT_BOOL,
		CAT_STRING,

	}CMD_ARG_TYPE;

	class BLADE_APPLICATION_API CmdOption
	{
	public:
		/** @brief  */
		const TString&	getName() const		{return mName;}

		/** @brief  */
		const TString&	getDesc() const		{return mDesc;}

		/** @brief  */
		const tchar		getShortcut() const	{return mShorcut;}

		/** @brief  */
		CMD_ARG_TYPE	getType() const		{return mType;}

		/** @brief  */
		const Variant&	getValue() const	{return mValue;}

	protected:
		//option name "--name"
		TString				mName;
		//
		TString				mDesc;
		//short name  "-n"
		tchar				mShorcut;
		//
		CMD_ARG_TYPE		mType;
		//
		mutable Variant	mValue;
	};

	class ICmdParser : public InterfaceSingleton<ICmdParser>
	{
	public:

		virtual ~ICmdParser()	{}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const CmdOption&	addOption(const TString& name,const TString& desc,tchar shortcut,CMD_ARG_TYPE type,const TString& defaultVal = TString::EMPTY) = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const CmdOption*	getOption(const TString& name) const = 0;

		/*
		@describe get the argv[0] of executed app, in original. (may have relative/absolute path prefix)
		@param 
		@return 
		*/
		virtual const TString&		getCommand() const = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool	initialize(IAppConfig& config,int argc,char* argv[]) = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool	initialize(IAppConfig& config,int argc,wchar* argv[]) = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool	initialize(IAppConfig& config,int argc,const TString& argv) = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void	showInformation(IAppConfig& cfg, bool force = false) = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const TStringList&	getFileList() const = 0;

	};

	extern template class BLADE_APPLICATION_API Factory<ICmdParser>;

	

}//namespace Blade



#endif // __Blade_ICmdParser_h__