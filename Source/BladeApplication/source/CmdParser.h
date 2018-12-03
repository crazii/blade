/********************************************************************
	created:	2011/08/28
	filename: 	CmdParser.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_CmdParser_h__
#define __Blade_CmdParser_h__
#include <ICmdParser.h>
#include <utility/BladeContainer.h>

namespace Blade
{
	class CmdOptionImpl : public CmdOption
	{
	public:
		CmdOptionImpl(const TString& name,const TString& desc,tchar shortcut,int val);
		CmdOptionImpl(const TString& name,const TString& desc,tchar shortcut,float val);
		CmdOptionImpl(const TString& name,const TString& desc,tchar shortcut,bool val);
		CmdOptionImpl(const TString& name,const TString& desc,tchar shortcut,const TString& val);
		CmdOptionImpl(const TString& name,const TString& desc,tchar shortcut,CMD_ARG_TYPE type,const TString& valString);

		Variant&	getValueRef()	const {return mValue;}
		bool mHidden;
	};


	class CmdParser : public ICmdParser , public Singleton<CmdParser>
	{
	public:

		CmdParser();
		~CmdParser();

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const CmdOption&	addOption(const TString& name,const TString& desc,tchar shortcut,CMD_ARG_TYPE type,const TString& defaultVal = TString::EMPTY);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const CmdOption*	getOption(const TString& name) const;

		/*
		@describe get the argv[0] of executed app, in original. (may have relative/absolute path prefix)
		@param 
		@return 
		*/
		virtual const TString&		getCommand() const	{return mCommand;}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool	initialize(IAppConfig& config,int argc,char* argv[]);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool	initialize(IAppConfig& config,int argc,wchar* argv[]);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool	initialize(IAppConfig& config,int argc,const TString& argv);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void	showInformation(IAppConfig& cfg, bool force = false);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const TStringList&	getFileList() const		{return mFileList;}

	protected:

		/*
		@describe 
		@param 
		@return 
		*/
		const CmdOptionImpl*	getOptionImpl(const TString& name);


		struct SCmdOptionCMP
		{
		public:
			bool	operator()(const CmdOptionImpl& lhs,const CmdOptionImpl& rhs) const
			{
				return lhs.getName() < rhs.getName();
			}
		};

		typedef Set<CmdOptionImpl,SCmdOptionCMP>	OptionSet;
		typedef Map<tchar,TString>					ShortcutMap;

		OptionSet	mCmdOptions;
		ShortcutMap	mShortcutMap;
		TStringList	mFileList;
		TString		mCommand;
	};
	

}//namespace Blade



#endif // __Blade_CmdParser_h__