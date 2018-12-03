/********************************************************************
	created:	2010/03/28
	filename: 	Log.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "Log.h"
#include <iomanip>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	Log::Log(ELogLevel level,bool bShareLog)
		:mPID(Thread::INVALID_ID)
		,mLevel((uint8)level)
		,mShared(bShareLog)
		,mReserveOnRedirect(false)
		,mEnabled(true)
		,mPrefix(true)
		,mPrefixOnce(true)
		,mIsNewLine(true)
		
	{

	}

	//////////////////////////////////////////////////////////////////////////
	Log::~Log()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	ILog&			Log::operator<<(const WString& text)
	{
		if( mEnabled )
		{
			TextFile& logfile = mShared ? msSharedLog : mOwnLog;
			if( logfile.isOpened() || mRedirector != NULL)
			{
				this->checkNewLine();
#if BLADE_UNICODE
				//text may be a local const buffer which size() is smaller thant '\0' string length
				for(size_t c = 0; c < text.size(); ++c)
					mOStreamBuffer << text[c];
#else
				String val = StringConverter::WStringToTString(text.c_str(),text.size());
				mOStreamBuffer << val.c_str();
#endif
			}
		}
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	ILog&			Log::operator<<(const String& text)
	{
		if( mEnabled )
		{
			TextFile& logfile = mShared ? msSharedLog : mOwnLog;
			if( logfile.isOpened() || mRedirector != NULL)
			{
				this->checkNewLine();
#if BLADE_UNICODE
				TString val = StringConverter::StringToTString(text.c_str(),text.size());
				mOStreamBuffer << val.c_str();
#else
				//text may be a local const buffer which size() is smaller thant '\0' string length
				for (size_t c = 0; c < text.size(); ++c)
					mOStreamBuffer << text[c];
#endif
			}
		}
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	ILog&			Log::operator<<(const wchar*	text)
	{
		if( mEnabled )
		{
			TextFile& logfile = mShared ? msSharedLog : mOwnLog;
			if( logfile.isOpened() || mRedirector != NULL )
			{
				this->checkNewLine();
#if BLADE_UNICODE
				mOStreamBuffer << text;
#else
				String val = StringConverter::WStringToTString(text);
				mOStreamBuffer << val.c_str();
#endif

			}
		}
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	ILog&			Log::operator<<(const char* text)
	{
		if( mEnabled )
		{
			TextFile& logfile = mShared ? msSharedLog : mOwnLog;
			if( logfile.isOpened() || mRedirector != NULL )
			{
				this->checkNewLine();
#if BLADE_UNICODE
				TString val = StringConverter::StringToTString(text);
				mOStreamBuffer << val.c_str();
				
#else
				mOStreamBuffer << text;
#endif

			}
		}
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	ILog&			Log::operator<<(bool	yesno)
	{
		if( mEnabled )
		{
			TextFile& logfile = mShared ? msSharedLog : mOwnLog;
			if( logfile.isOpened() || mRedirector != NULL )
			{
				this->checkNewLine();
				mOStreamBuffer << TStringHelper::fromBool(yesno, true).c_str();
			}
		}
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	ILog&			Log::operator<<(int32	integer)
	{
		if( mEnabled )
		{
			TextFile& logfile = mShared ? msSharedLog : mOwnLog;
			if( logfile.isOpened() || mRedirector != NULL )
			{
				this->checkNewLine();
				mOStreamBuffer <<  integer;
			}
		}
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	ILog&			Log::operator<<(uint32 uinteger)
	{
		if( mEnabled )
		{
			TextFile& logfile = mShared ? msSharedLog : mOwnLog;
			if( logfile.isOpened() || mRedirector != NULL )
			{
				this->checkNewLine();
				mOStreamBuffer <<  uinteger;
			}
		}
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	ILog&			Log::operator<<(int64	integer)
	{
		if( mEnabled )
		{
			TextFile& logfile = mShared ? msSharedLog : mOwnLog;
			if( logfile.isOpened() || mRedirector != NULL )
			{
				this->checkNewLine();
				mOStreamBuffer <<  integer;
			}
		}
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	ILog&			Log::operator<<(uint64 uinteger)
	{
		if( mEnabled )
		{
			TextFile& logfile = mShared ? msSharedLog : mOwnLog;
			if( logfile.isOpened() || mRedirector != NULL )
			{
				this->checkNewLine();
				mOStreamBuffer <<  uinteger;
			}
		}
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	ILog&			Log::operator<<(fp32 real)
	{
		if( mEnabled )
		{
			TextFile& logfile = mShared ? msSharedLog : mOwnLog;
			if( logfile.isOpened() || mRedirector != NULL )
			{
				this->checkNewLine();
				mOStreamBuffer << real;
			}
		}
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	ILog&			Log::operator<<(fp64 real)
	{
		if( mEnabled )
		{
			TextFile& logfile = mShared ? msSharedLog : mOwnLog;
			if( logfile.isOpened() || mRedirector != NULL )
			{
				this->checkNewLine();
				mOStreamBuffer << real;
			}
		}
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	ILog&			Log::operator<<( ILog::Manipulators  LogManip)
	{
		return (*LogManip)(*this);
	}

	//////////////////////////////////////////////////////////////////////////
	void			Log::enablePrefix(bool enable)
	{
		ScopedLock l(mOwnLock);
		mPrefix = enable;
		mPrefixOnce = enable;
	}

	//////////////////////////////////////////////////////////////////////////
	void			Log::enablePrefixOnce(bool enable)
	{
		ScopedLock l(mOwnLock);	//must be the first one or 
		mPrefixOnce = enable;
	}

	//////////////////////////////////////////////////////////////////////////
	void			Log::setEnable(bool enabled)
	{
		ScopedLock l(mOwnLock);
		mEnabled = enabled;
	}

	//////////////////////////////////////////////////////////////////////////
	void			Log::flush()
	{
		if( mEnabled )
		{
			Lock* lock = mShared ? &msSharedLock : &mOwnLock;

			//own lock should always be locked on starting a new line of log. @see checkNewLine()
			if (!mOwnLock.isLocked())
				mOwnLock.lock();

			ScopedPLock spl(lock != &mOwnLock ? lock : NULL);

			TextFile& logfile = mShared ? msSharedLog : mOwnLog;
			try
			{
				mOStreamBuffer << TEXT("\n");

				if (mRedirector != NULL)
				{
					mRedirector->outputLog((ELogLevel)mLevel, mOStreamBuffer.str().c_str());
					mIsNewLine = true;
				}

				if (logfile.isOpened() && (mRedirector == NULL || mReserveOnRedirect))
				{
					logfile.write(mOStreamBuffer.str().c_str(), mOStreamBuffer.str().size());
					logfile.flushObuffer();
					mIsNewLine = true;
				}

				mOStreamBuffer.clear();
				mPrefixOnce = mPrefix;
			}
			catch (...)
			{
				mPID = Thread::INVALID_ID;
				mOwnLock.unlock();
				throw;
			}

			mPID = Thread::INVALID_ID;
			mOwnLock.unlock();
		}
	}

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#	pragma warning(disable:4996)	//'localtime': This function or variable may be unsafe
#endif

	//////////////////////////////////////////////////////////////////////////
	void			Log::checkNewLine()
	{
		if (!mEnabled)
			return;

		if (mIsNewLine || mPID != Thread::getCurrentID())
		{
			mOwnLock.lock();
			if (mPID != Thread::getCurrentID())
			{
				mPID = Thread::getCurrentID();
				assert(mIsNewLine);
			}
			mIsNewLine = false;

			if (mPrefix && mPrefixOnce)
			{
				struct ::tm *pTime;
				::time_t ctTime;
				::time(&ctTime);
				pTime = ::localtime(&ctTime);
				mOStreamBuffer.get().format(TEXT("[%02d:%02d:%02d]"), pTime->tm_hour, pTime->tm_min, pTime->tm_sec);

				switch (mLevel)
				{
				case LL_NONE:
					break;
				case LL_CRITICAL:
					mOStreamBuffer.get().appendFormat(TEXT("%-15s"), TEXT("<Error>"));
					break;
				case LL_CAREFUL:
					mOStreamBuffer.get().appendFormat(TEXT("%-15s"), TEXT("<Warning>"));
					break;
				case LL_DETAIL:
					mOStreamBuffer.get().appendFormat(TEXT("%-15s"), TEXT("<Information>"));
					break;
				case LL_DEBUG:
					mOStreamBuffer.get().appendFormat(TEXT("%-15s"), TEXT("<Debug>"));
					break;
				case LL_COUNT:
					break;
				}
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void						Log::redirectLog(const HLOGREDIRECTOR& redirectStream,bool reserve)
	{
		ScopedLock ls(mOwnLock);
		mRedirector = redirectStream;
		mReserveOnRedirect = reserve;
	}

}//namespace Blade