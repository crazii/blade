/********************************************************************
	created:	2010/03/28
	filename: 	Log.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_Log_h__
#define __Blade_Log_h__
#include <interface/ILog.h>
#include <utility/TextFile.h>
#include <utility/StringStream.h>
#include <threading/Thread.h>

namespace Blade
{
	//internal wrap for string stream object,because string stream should not be hold persistent
	//we just delete it after each use
	//it's kinda dirty though.
	class TStreamWrap : public TempAllocatable
	{
	public:
		TStringStream mStream;

		/** @brief  */
		template<typename T>
		TStringStream& operator<<(const T& t)	{ return mStream << t;}

		/** @brief  */
		const TString& str() const { return mStream.str(); }
	};
	class TStreamWrapBuffer
	{
	public:
		TStreamWrapBuffer():mWrap(NULL)	{}

		/** @brief  */
		template<typename T>
		TStringStream& operator<<(const T& t)
		{
			if( mWrap == NULL )
				mWrap = BLADE_NEW TStreamWrap();

			return (*mWrap) << t;
		}

		/** @brief  */
		TStringStream& get()
		{
			if( mWrap == NULL )
				mWrap = BLADE_NEW TStreamWrap();
			return mWrap->mStream;
		}

		/** @brief  */
		const TString& str() const
		{
			if( mWrap == NULL )
				return TString::EMPTY;
			else
				return mWrap->str();
		}

		/** @brief  */
		void	clear()
		{
			BLADE_DELETE mWrap;
			mWrap = NULL;
		}
		TStreamWrap* mWrap;
	};

	class Log : public ILog, public Allocatable
	{
	private:
		Log(const Log&);
	public:
		Log(ELogLevel level,bool bShareLog = true);
		~Log();

		/************************************************************************/
		/* ILog interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual		ILog&	operator<<(const WString& text);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual		ILog&	operator<<(const String& text);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual		ILog&	operator<<(const wchar*	text);

		/*
		@describe
		@param
		@return
		*/
		virtual		ILog&	operator<<(const char* text);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual		ILog&	operator<<(bool	yesno);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual		ILog&	operator<<(int32	integer);

		/*
		@describe
		@param
		@return
		*/
		virtual		ILog&	operator<<(uint32 uinteger);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual		ILog&	operator<<(int64	integer);


		/*
		@describe
		@param
		@return
		*/
		virtual		ILog&	operator<<(uint64 uinteger);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual		ILog&	operator<<(fp32 real);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual		ILog&	operator<<(fp64 real);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual		ILog&	operator<<( ILog::Manipulators  );

		/*
		@describe enable time stamp, level info, etc.
		@param 
		@return 
		*/
		virtual void				enablePrefix(bool enable);

		/*
		@describe enable time stamp, level info, etc. for this output before flush
		@param 
		@return 
		*/
		virtual void				enablePrefixOnce(bool enable);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void				setEnable(bool enabled);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool				getEnabled() const { return mEnabled; }

		/*
		@describe 
		@param 
		@return 
		*/
		virtual ILog::ELogLevel		getLevel() const { return (ELogLevel)mLevel; }

		/*
		@describe
		@param
		@return
		*/
		virtual void				flush();

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		void						checkNewLine();

		/*
		@describe
		@param
		@return
		*/
		void						redirectLog(const HLOGREDIRECTOR& redirectStream, bool reserve);

	protected:
		TextFile			mOwnLog;
		Lock				mOwnLock;
		TStreamWrapBuffer	mOStreamBuffer;
		HLOGREDIRECTOR		mRedirector;

		Thread::ID			mPID;
		uint8				mLevel;			//ELogLevel: detail level
		bool				mShared : 1;	//share out put log file
		bool				mReserveOnRedirect : 1;
		bool				mEnabled : 1;
		bool				mPrefix : 1;
		bool				mPrefixOnce : 1;
		bool				mIsNewLine : 1;

		static StaticLock	msSharedLock;
		static TextFile		msSharedLog;
		
		friend class LogManager;
	public:
		/** @brief  */
		static Log			msDebugOutput;
		/** @brief  */
		static Log			msInformation;
		/** @brief  */
		static Log			msWarning;
		/** @brief  */
		static Log			msError;
	};
	
}//namespace Blade


#endif //__Blade_Log_h__
