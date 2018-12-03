/********************************************************************
	created:	2010/03/28
	filename: 	Exception.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_Exception_h__
#define __Blade_Exception_h__
#include <exception>
#include <utility/String.h>

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
//fix warning for VC14
namespace std {
	class BLADE_BASE_API exception;
}//namespace std
#endif

namespace Blade
{

	enum EExceptionCode
	{
		__EXC_FW__ = 0xFFFFFFFF,

		EXC_UNKNOWN = 0,		//unknown exception
		EXC_REDUPLICATE,		//reduplicate key/values/data
		EXC_FILE_WRITE,			//file write exception
		EXC_FILE_READ,			//file read exception

		EXC_OUT_OF_MEMORY,
		EXC_OUT_OF_RANGE,
		EXC_INVALID_PARAM,
		EXC_INVALID_OPERATION,

		EXC_INTERNAL_ERROR,
		EXC_API_ERROR,

		//added new exceptions here:
		EXC_NEXIST,		//function/data requested not exist

		//keep it as last
		EXC_COUNT,
	};

	class BLADE_BASE_API Exception : public std::exception
	{
	public:
		Exception(const tchar* name,uint code,const TString& description, const TString& src);
		Exception(const tchar* name,uint code,const TString& description, const TString& src, const TString& file, ulong line);
		Exception(const Exception& src);
		~Exception() throw() {}

		void operator = (const Exception& rhs);

		///compatibility
		/** @brief std::exception::what override */
		const char*		what() const noexcept;

		/** @brief  */
		const tchar*	getTypeName() const noexcept;
		/** @brief  */
		uint			getCode() const noexcept;
		/** @brief  */
		const TString&	getDescription() const;
		/** @brief  */
		const TString&	getSource() const noexcept;
		/** @brief  */
		const TString&	getFile() const noexcept;
		/** @brief  */
		ulong			getLine() const noexcept;
		/** @brief  */
		const tchar*	getFullDescription() const;
	protected:
		const tchar*	mName;
		uint			mCode;
		TString			mDescription;
		TString			mSource;
		TString			mFile;
		ulong			mLine;
		///std compatible what string
		mutable String	_what;
		mutable TString	mFullDescription;
	};

	class BLADE_BASE_API ExceptionUnknown : public Exception
	{
	public:
		ExceptionUnknown(const TString& description,const TString& src, const TString& file, ulong line)
			:Exception(TEXT("Unknown Exception"),EXC_UNKNOWN, description, src, file, line){}
	};

	class BLADE_BASE_API ExceptionReDuplicate : public Exception
	{
	public:
		ExceptionReDuplicate(const TString& description,const TString& src,const TString& file,ulong line)
			:Exception(TEXT("ReDuplicate Value/Operation Exception"), EXC_REDUPLICATE, description, src, file, line){}
	};

	class BLADE_BASE_API ExceptionFileWrite : public Exception
	{
	public:
		ExceptionFileWrite(const TString& description,const TString& src,const TString& file,ulong line)
			:Exception(TEXT("File Write Exception"),EXC_FILE_WRITE,description,src,file,line){}
	};

	class BLADE_BASE_API ExceptionFileRead : public Exception
	{
	public:
		ExceptionFileRead(const TString& description,const TString& src,const TString& file,ulong line)
			:Exception(TEXT("File Read Exception"),EXC_FILE_READ,description,src,file,line){}
	};

	class BLADE_BASE_API ExceptionDataNotExist : public Exception
	{
	public:
		ExceptionDataNotExist(const TString& description,const TString& src,const TString& file,ulong line)
			:Exception(TEXT("Data Not Exist Exception"),EXC_NEXIST,description,src,file,line){}
	};


	template < uint excpcode >
	class ExceptionBind
	{
	public:
		typedef ExceptionUnknown ExceptionType;
	};

	template<>
	class ExceptionBind<EXC_UNKNOWN>
	{
	public:
		typedef ExceptionUnknown ExceptionType;
	};

	template<>
	class ExceptionBind<EXC_REDUPLICATE>
	{
	public:
		typedef ExceptionReDuplicate ExceptionType;
	};

	template<>
	class ExceptionBind<EXC_FILE_WRITE>
	{
	public:
		typedef ExceptionFileWrite ExceptionType;
	};

	template<>
	class ExceptionBind<EXC_FILE_READ>
	{
	public:
		typedef ExceptionFileRead ExceptionType;
	};

	template<>
	class ExceptionBind<EXC_NEXIST>
	{
	public:
		typedef ExceptionDataNotExist ExceptionType;
	};


///format: BLADE_EXCEPT_SOURCE(EXC_NEXIST,BTString(""),BTString("main"))
#define BLADE_EXCEPT_SOURCE(code,desc,src) throw ExceptionBind<code>::ExceptionType(desc, src, BLADE_TFILE, (ulong)__LINE__)

#define BLADE_EXCEPT(code,desc) BLADE_EXCEPT_SOURCE(code, desc, BLADE_TFUNCTION )
#define BLADE_EXCEPT_IF(condition, code,desc) if((condition)) BLADE_EXCEPT_SOURCE(code, desc, BLADE_TFUNCTION )

///for convenience
#define BLADE_DEFINE_EXCEPT(dll_exp_type,code,except_class,except_typedesc) \
	class dll_exp_type except_class : public Exception \
	{ \
	public: \
		except_class(const TString& description,const TString& src,const TString& file,ulong line) \
			:Exception(TEXT(except_typedesc),code,description,src,file,line){} \
	}; \
	template<> \
	class ExceptionBind<code> \
	{ \
	public: \
		typedef except_class ExceptionType; \
	}


	BLADE_DEFINE_EXCEPT(BLADE_BASE_API,EXC_OUT_OF_MEMORY,ExceptionOutOfMemory,"Out of Memory Exception");
	BLADE_DEFINE_EXCEPT(BLADE_BASE_API,EXC_OUT_OF_RANGE,ExceptionOutOfRange,"Out Of Range Exception");
	BLADE_DEFINE_EXCEPT(BLADE_BASE_API,EXC_INVALID_PARAM,ExceptionInvalidParam,"Invalid Parameter Exception");
	BLADE_DEFINE_EXCEPT(BLADE_BASE_API,EXC_INVALID_OPERATION,ExceptionInvalidOperation,"Invalid Operation Exception");
	BLADE_DEFINE_EXCEPT(BLADE_BASE_API,EXC_INTERNAL_ERROR,ExceptionInternalError,"Internal Error Exception");
	BLADE_DEFINE_EXCEPT(BLADE_BASE_API,EXC_API_ERROR,ExceptionAPIFail,"API Fail Exception");
	
	
	
}//namespace Blade




#endif //__Blade_Exception_h__