/********************************************************************
	created:	2010/03/28
	filename: 	ILog.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_ILog_h__
#define __Blade_ILog_h__
#include <Handle.h>
#include <utility/String.h>

namespace Blade
{
	class ILog
	{
	public:
		typedef ILog& (*Manipulators)(ILog&);

		enum ELogLevel
		{
			LL_NONE = 0,		///no logs
			LL_CRITICAL,		///only log critical errors
			LL_CAREFUL,			///critical errors plus some warning		
			LL_DETAIL,			///log some more details
			LL_DEBUG,			///all possible informations : default

			LL_COUNT,
			LL_BEGIN = LL_NONE,
			LL_DEFAULT = LL_DEBUG,
		};

		virtual		~ILog()	{}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual		ILog&	operator<<(const WString& text) = 0;

		/**
		* @describe 
		* @param 
		* @return 
		*/
		virtual		ILog&	operator<<(const String& text) = 0;

		/**
		* @describe 
		* @param 
		* @return 
		*/
		virtual		ILog&	operator<<(const wchar*	text) = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual		ILog&	operator<<(const char* text) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual		ILog&	operator<<(bool	yesno) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual		ILog&	operator<<(int32	integer) = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual		ILog&	operator<<(uint32 uinteger) = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual		ILog&	operator<<(int64	integer) = 0;


		/**
		@describe
		@param
		@return
		*/
		virtual		ILog&	operator<<(uint64 uinteger) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual		ILog&	operator<<(fp32 real) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual		ILog&	operator<<(fp64 real) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual		ILog&	operator<<( ILog::Manipulators  ) = 0;

		/**
		@describe enable time stamp, level info, etc.
		@param 
		@return 
		*/
		virtual void				enablePrefix(bool enable) = 0;

		/**
		@describe enable time stamp, level info, etc. for output this time only once, before flush
		@param 
		@return 
		*/
		virtual void				enablePrefixOnce(bool enable) = 0;

		/**
		* @describe 
		* @param 
		* @return 
		*/
		virtual void				setEnable(bool enabled) = 0;

		/**
		* @describe 
		* @param 
		* @return 
		*/
		virtual bool				getEnabled() const = 0;

		/*
		* @describe 
		* @param 
		* @return 
		*/
		virtual ILog::ELogLevel		getLevel() const = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual void				flush() = 0;

		/**
		* @describe 
		* @param 
		* @return 
		*/
		static ILog&				flushLog(ILog& log)		{ log.flush(); return log; }
		
		static ILog&				skipPrefix(ILog& log)	{ log.enablePrefixOnce(false); return log; }

		static const BLADE_BASE_API Manipulators	endLog;
		static const BLADE_BASE_API Manipulators	noPrefix;

		/** @brief  */
		static BLADE_BASE_API ILog&	DebugOutput;
		/** @brief  */
		static BLADE_BASE_API ILog&	Information;
		/** @brief  */
		static BLADE_BASE_API ILog&	Warning;
		/** @brief  */
		static BLADE_BASE_API ILog&	Error;
	};

	class HLOG : public Handle<ILog>
	{
	public:
		inline HLOG() {}
		inline HLOG(ILog* log) :Handle<ILog>(log) {}

		/** @brief  */
		inline void	validate() const
		{
			assert(mPtr != NULL);
		}

		//wrappers
		/** @brief  */
		inline void				setEnable(bool enabled)
		{
			this->validate();
			if (mPtr != NULL)
				mPtr->setEnable(enabled);
		}

		/** @brief  */
		inline bool				getEnabled() const
		{
			this->validate();
			return mPtr != NULL && mPtr->getEnabled();
		}

		///operator << wrappers
		/** @brief  */
		const HLOG&	operator<<(const WString& value) const
		{
			this->validate();
			mPtr->operator <<(value);
			return *this;
		}

		/** @brief  */
		const HLOG&	operator<<(const String& value) const
		{
			this->validate();
			mPtr->operator <<(value);
			return *this;
		}

		/** @brief  */
		const HLOG&	operator<<(const wchar* value) const
		{
			this->validate();
			mPtr->operator <<(value);
			return *this;
		}

		/** @brief  */
		const HLOG&	operator<<(const char* value) const
		{
			this->validate();
			mPtr->operator <<(value);
			return *this;
		}

		/** @brief  */
		const HLOG&	operator<<(bool	value) const
		{
			this->validate();
			mPtr->operator <<(value);
			return *this;
		}

		/** @brief  */
		const HLOG&	operator<<(int32 value) const
		{
			this->validate();
			mPtr->operator <<(value);
			return *this;
		}

		/** @brief  */
		const HLOG&	operator<<(uint32 value) const
		{
			this->validate();
			mPtr->operator <<(value);
			return *this;
		}

		/** @brief  */
		const HLOG&	operator<<(int64 value) const
		{
			this->validate();
			mPtr->operator <<(value);
			return *this;
		}

		/** @brief  */
		const HLOG&	operator<<(uint64 value) const
		{
			this->validate();
			mPtr->operator <<(value);
			return *this;
		}

		/** @brief  */
		const HLOG&	operator<<(fp32 value) const
		{
			this->validate();
			mPtr->operator <<(value);
			return *this;
		}

		/** @brief  */
		const HLOG&	operator<<(fp64 value) const
		{
			this->validate();
			mPtr->operator <<(value);
			return *this;
		}

		/** @brief  */
		const HLOG&	operator<<( ILog::Manipulators  value) const
		{
			this->validate();
			mPtr->operator <<(value);
			return *this;
		}

	};//class HLOG


	class ILogRedirector
	{
	public:
		virtual ~ILogRedirector()	{}

		/**
		@describe redirect original log content for specified level channel
		@param
		@return
		*/
		virtual void outputLog(ILog::ELogLevel level, const tchar* content) = 0;
	};

	typedef Handle<ILogRedirector> HLOGREDIRECTOR;

	//output to any log objects
#define BLADE_LOGU(_log, _content) do { if(_log.getEnabled()) _log << _content << Blade::ILog::endLog; } while(false)
#define BLADE_DETAIL_LOGU(_log, _content) BLADE_LOGU(_log, BLADE_TFUNCTION << TEXT("\n") << TEXT(__FILE__) << TEXT("(") << __LINE__ << TEXT("):") << _content)

	//output to built-in ILog objects
#define BLADE_LOG(_log, _content) BLADE_LOGU(Blade::ILog::_log, _content)
#define BLADE_DETAIL_LOG(_log, _content) BLADE_DETAIL_LOGU(Blade::ILog::_log, _content)

}//namespace Blade

#endif //__Blade_ILog_h__