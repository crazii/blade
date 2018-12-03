/********************************************************************
	created:	2010/03/27
	filename: 	StringHelper.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_StringHelper_h__
#define __Blade_StringHelper_h__
#include "StringStream.h"

namespace Blade
{
	/************************************************************************/
	/* string split function (tokenize)                                                                     */
	/************************************************************************/
	namespace Impl
	{
		class StringTokenizerImpl;
		class WStringTokenizerImpl;
	}//namespace Impl
	
	class BLADE_BASE_API StringTokenizer : public NonAssignable, public NonCopyable
	{
	public:
		StringTokenizer();
		~StringTokenizer();

		/** @brief  */
		void	tokenize(const String& inString, const char* tokens, bool inplace = false);

		/** @brief  */
		bool	tokenize(const String& inString ,const char* doubleTokens, const char* tokens, bool inplace = false);

		/** @brief  */
		bool	isDoubleToken(index_t stringIndex) const;

		/** @brief this is different from tokenize: successive line-end token will be make a entry as empty string */
		void	splitLines(const String& inString);

		/** @brief replace comment lines with empty line: keep line NO valid */
		void	removeComments(const char* lineCommnet = "//", const char* blockComment = "/*", const char* blockCommentEnd = "*/");

		/** @brief  */
		const String&	at(index_t index) const;

		/** @brief  */
		inline const String&	operator[](index_t index) const
		{
			return this->at(index);
		}

		/** @brief  */
		index_t				find(const String& str) const;

		/** @brief  */
		void				erase(index_t index);

		/** @brief  */
		bool				set(index_t index, const String& Val);

		/** @brief  */
		void				push_back(const String& inString);

		/** @brief  */
		size_t				size();

		/** @brief  */
		void				clear();

	private:
		Pimpl<Impl::StringTokenizerImpl> mTokenizer;
	};//class StringTokenizer

	
	class BLADE_BASE_API WStringTokenizer : public NonAssignable, public NonCopyable
	{
	public:
		WStringTokenizer();
		~WStringTokenizer();

		/** @brief  */
		void	tokenize(const WString& inWString, const wchar* tokens, bool inplace = false);

		/** @brief  */
		bool	tokenize(const WString& inWString, const wchar* doubleTokens, const wchar* tokens, bool inplace = false);

		/** @brief  */
		bool	isDoubleToken(index_t WStringIndex) const;

		/** @brief this is different from tokenize: successive line-end token will be make a entry as empty string(emptry line) */
		void	splitLines(const WString& inWString);

		/** @brief replace comment lines with empty line: keep line NO valid */
		void	removeComments(const wchar* lineCommnet = L"//", const wchar* blockComment = L"/*", const wchar* blockCommentEnd = L"*/");

		/** @brief  */
		const WString&	at(index_t index) const;

		/** @brief  */
		inline const WString&	operator[](index_t index) const
		{
			return this->at(index);
		}

		/** @brief  */
		index_t				find(const WString& str) const;

		/** @brief  */
		void				erase(index_t index);

		/** @brief  */
		bool				set(index_t index, const WString& Val);

		/** @brief  */
		void				push_back(const WString& inWString);

		/** @brief  */
		size_t				size();

		/** @brief  */
		void				clear();

	private:
		Pimpl<Impl::WStringTokenizerImpl> mTokenizer;
	};//class WStringTokenizer


	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	template<typename CharT>
	struct HelperTraits
	{
		typedef void StringType;
		typedef void StreamType;
		typedef void TokenizerType;
		typedef void ConcatType;
	};

	template<>
	struct HelperTraits<char>
	{
		typedef String StringType;
		typedef StringStream StreamType;
		typedef StringTokenizer TokenizerType;
		typedef StringConcat ConcatType;
	};

	template<>
	struct HelperTraits<wchar>
	{
		typedef WString StringType;
		typedef WStringStream StreamType;
		typedef WStringTokenizer TokenizerType;
		typedef WStringConcat ConcatType;
	};

	/************************************************************************/
	/*string conversion helper                                                                      */
	/************************************************************************/
	template <typename CharT, typename AllocT >
	class BasicStringHelper
	{
	public:
		typedef BasicString<CharT, Char_Traits<CharT>, AllocT> StringType;
		typedef Char_Traits<CharT>	TraitsType;
		typedef HelperTraits<CharT> HelperTraitsType;
		typedef typename HelperTraitsType::StreamType StreamType;
		typedef typename HelperTraitsType::TokenizerType TokenizerType;
		typedef typename HelperTraitsType::ConcatType ConcatType;

		/************************************************************************/
		/* converting operations                                                                     */
		/************************************************************************/
		/**
		@describe convert string to integer
		@param [in] value string to convert
		@return integer result
		*/
		static inline int32		toInt32(const StringType& Val)
		{
			int32 retval = 0;
			StreamType stream;
			stream << Val;
			stream >> retval;
			return retval;
		}

		/**
		@describe convert string to integer
		@param [in] value string to convert
		@return integer result
		*/
		static inline uint32		toUInt32(const StringType& Val)
		{
			uint32 retval = 0;
			StreamType stream;
			stream << Val;
			stream >> retval;
			return retval;
		}

		/**
		@describe convert string to integer
		@param [in] value string to convert
		@return integer result
		*/
		static inline int	toInt(const StringType& Val)
		{
			return toInt32(Val);
		}

		/**
		@describe convert string to unsigned integer
		@param [in] value string to convert
		@return integer result
		*/
		static inline uint	toUInt(const StringType& Val)
		{
			return toUInt32(Val);
		}

		/**
		@describe convert string to integer
		@param [in] value string to convert
		@return integer result
		*/
		static inline int64		toInt64(const StringType& Val)
		{
			int64 retval = 0;
			StreamType stream;
			stream << Val;
			stream >> retval;
			return retval;
		}

		/**
		@describe convert string to integer
		@param [in] value string to convert
		@return integer result
		*/
		static inline uint64		toUInt64(const StringType& Val)
		{
			uint64 retval = 0;
			StreamType stream;
			stream << Val;
			stream >> retval;
			return retval;
		}

		static inline uint32	toHex32(const StringType& Val)
		{
			StreamType stream;
			stream << Val;

			const CharT fmt[] = { CharT('0'), CharT('x'), CharT('%'),CharT('x'),CharT('\0') };
			int off = ( Val.size() > 2 && Val[0] == CharT('0') && Val[1] == CharT('x') ) ? 0 : 2;
			uint32 retVal = 0;
			stream.scanFormat(fmt + off, &retVal );
			return retVal;
		}
		static inline uint32	toOct32(const StringType& Val)
		{
			StreamType stream;
			stream << Val;

			const CharT fmt[] = { CharT('o'), CharT('%'),CharT('o'),CharT('\0') };
			int off = ( Val.size() > 1 && Val[0] == CharT('o') ) ? 0 : 1;
			uint32 retVal = 0;
			stream.scanFormat( fmt+off, &retVal );
			return retVal;
		}

		static inline uint64	toHex64(const StringType& Val)
		{
			StreamType stream;
			stream << Val;

			const CharT fmt[] = { CharT('0'), CharT('x'), CharT('%'),CharT('l'),CharT('l'),CharT('x'),CharT('\0') };
			int off = ( Val.size() > 1 && Val[0] == CharT('o') ) ? 0 : 2;
			uint64 retVal = 0;
			stream.scanFormat( fmt+off, &retVal );
			return retVal;
		}
		static inline uint64	toOct64(const StringType& Val)
		{
			StreamType stream;
			stream << Val;

			const CharT fmt[] = { CharT('o'), CharT('%'),CharT('l'),CharT('l'),CharT('o'),CharT('\0') };
			int off = ( Val.size() > 1 && Val[0] == CharT('o') ) ? 0 : 1;
			uint64 retVal = 0;
			stream.scanFormat( fmt+off, &retVal );
			return retVal;
		}
		static inline void*	toPointer(const StringType& Val)
		{
			StreamType stream;
			stream << Val;

			const CharT fmt[] = { CharT('%'), CharT('p') };
			void* retVal = NULL;
			stream.scanFormat( fmt, &retVal );
			return retVal;
		}
#if BLADE_ARCHITECTURE == BLADE_ARCHITECTURE_32BIT
		/**
		@describe convert string to size_t (make it less confussing)
		@param [in] value string to convert
		@return unsigned integer result
		*/
		static inline size_t	toSizeT(const StringType& Val)
		{
			return toUInt32(Val);
		}

		/**
		@describe convert string to size_t (make it less confussing)
		@param [in] value string to convert
		@return unsigned integer result
		*/
		static inline ptrdiff_t	toPtrdiffT(const StringType& Val)
		{
			return toInt32(Val);
		}

		static inline uint32	toHex(const StringType& Val)
		{
			return toHex32(Val);
		}
		static inline uint32	toOct(const StringType& Val)
		{
			return toOct32(Val);
		}
#else
		/**
		@describe convert string to size_t (make it less confussing)
		@param [in] value string to convert
		@return unsigned integer result
		*/
		static inline size_t	toSizeT(const StringType& Val)
		{
			return toUInt64(Val);
		}

		/**
		@describe convert string to size_t (make it less confussing)
		@param [in] value string to convert
		@return unsigned integer result
		*/
		static inline ptrdiff_t	toPtrdiffT(const StringType& Val)
		{
			return toInt64(Val);
		}

		static inline uint64	toHex(const StringType& Val)
		{
			return toHex64(Val);
		}
		static inline uint64	toOct(const StringType& Val)
		{
			return toOct64(Val);
		}
#endif
		/**
		* @describe convert string to scalar
		* @param [in] value string to convert
		* @return scalar result
		*/
		static inline fp32		toF32(const StringType& Val)
		{
			fp32 retval = 0;
			StreamType stream;
			stream << Val;
			stream >> retval;
			return retval;
		}

		/**
		* @describe convert string to scalar
		* @param [in] value string to convert
		* @return scalar result
		*/
		static inline fp64		toF64(const StringType& Val)
		{
			fp64 retval = 0;
			StreamType stream;
			stream << Val;
			stream >> retval;
			return retval;
		}

		/**
		* @describe convert string to scalar
		* @param [in] value string to convert
		* @return scalar result
		*/
		static inline scalar		toScalar(const StringType& Val)
		{
#if BLADE_DOUBLE_PRESITION
			return toF64(Val);
#else
			return toF32(Val);
#endif
		}

		/**
		* @describe convert string to bool
		* @param [in] value string to convert
		* @return bool result
		*/
		static inline bool			toBool(const StringType& Val, bool bCaseSensitive = true)
		{
			CharT trueString[] = {CharT('T'),CharT('R'),CharT('U'),CharT('E'),CharT('\0')  };
			CharT trueStringLC[] = {CharT('t'),CharT('r'),CharT('u'),CharT('e'),CharT('\0')  };
			CharT trueValue[] = {CharT('1'),CharT('\0')};
			//construct const string
			return (Val == trueValue) ||
				(bCaseSensitive && (Val == trueString || Val == trueStringLC) ) ||
				TraitsType::compare_no_case(Val.c_str(), Val.size(), trueString, 4) == 0;
		}

		/**
		* @describe convert integer to string
		* @param [in] value integer to convert
		* @return string result
		*/
		static inline StringType	fromInt32(int32 Val)
		{
			StreamType stream;
			stream << Val;
			return StringType( stream.str() );
		}

		/**
		* @describe convert integer to string
		* @param [in] value integer to convert
		* @return string result
		*/
		static inline StringType	fromUInt32(uint32 Val)
		{
			StreamType stream;
			stream << Val;
			return StringType( stream.str() );
		}

		/**
		* @describe convert integer to string
		* @param [in] value integer to convert
		* @return string resultddddd
		*/
		static inline StringType	fromInt64(int64 Val)
		{
			StreamType stream;
			stream << Val;
			return StringType( stream.str() );
		}

		/**
		* @describe convert integer to string
		* @param [in] value integer to convert
		* @return string result
		*/
		static inline StringType	fromUInt64(uint64 Val)
		{
			StreamType stream;
			stream << Val;
			return StringType( stream.str() );
		}

		static inline StringType	fromHex32(uint32 Val, bool prefix = true)
		{
			StreamType stream;
			CharT fmt[] = { CharT('0'), CharT('x'), CharT('%'), CharT('X'), CharT('\0') };
			int off = prefix ? 0 : 2;
			stream.format(fmt+off, Val);
			return StringType( stream.str() );
		}

		static inline StringType	fromOct32(uint32 Val, bool prefix = true)
		{
			StreamType stream;
			CharT fmt[] = { CharT('o'), CharT('%'), CharT('o'), CharT('\0') };
			int off = prefix ? 0 : 1;
			stream.format(fmt+off, Val);
			return StringType( stream.str() );
		}

		static inline StringType	fromHex64(uint64 Val, bool prefix = true)
		{
			StreamType stream;
			CharT fmt[] = { CharT('0'), CharT('x'), CharT('%'), CharT('l'), CharT('l'), CharT('X'), CharT('\0') };
			int off = prefix ? 0 : 2;
			stream.format(fmt+off, Val);
			return StringType( stream.str() );
		}

		static inline StringType	fromOct64(uint64 Val, bool prefix = true)
		{
			StreamType stream;
			CharT fmt[] = { CharT('o'), CharT('%'), CharT('l'), CharT('l'), CharT('o'), CharT('\0') };
			int off = prefix ? 0 : 1;
			stream.format(fmt+off, Val);
			return StringType( stream.str() );
		}

		static inline StringType	fromPointer(void* Val)
		{
			StreamType stream;
			CharT fmt[] = { CharT('%'), CharT('p'), CharT('\0') };
			stream.format(fmt, Val);
			return StringType( stream.str() );
		}
#if BLADE_ARCHITECTURE == BLADE_ARCHITECTURE_32BIT
		/**
		* @describe convert integer to string
		* @param [in] value integer to convert
		* @return string result
		*/
		static inline StringType	fromInt(int32 Val)
		{
			return fromInt32(Val);
		}

		/**
		* @describe convert integer to string
		* @param [in] value integer to convert
		* @return string result
		*/
		static inline StringType	fromUInt(uint32 Val)
		{
			return fromUInt32(Val);
		}

		static inline StringType	fromHex(uint32 Val)
		{
			return fromHex32(Val);
		}
		static inline StringType	fromOct(uint32 Val)
		{
			return fromOct32(Val);
		}
#else
		/**
		* @describe convert integer to string
		* @param [in] value integer to convert
		* @return string result
		*/
		static inline StringType	fromInt(int64 Val)
		{
			return fromInt64(Val);
		}

		/**
		* @describe convert integer to string
		* @param [in] value integer to convert
		* @return string result
		*/
		static inline StringType	fromUInt(uint64 Val)
		{
			return fromUInt64(Val);
		}

		static inline StringType	fromHex(uint64 Val)
		{
			return fromHex64(Val);
		}
		static inline StringType	fromOct(uint64 Val)
		{
			return fromOct64(Val);
		}
#endif
		/**
		* @describe convert scalar to string
		* @param [in] value scalar to convert
		* @return string result
		*/
		static inline StringType	fromF32(fp32 Val)
		{
			StreamType stream;
			stream << Val;
			return StringType( stream.str() );
		}

		/**
		* @describe convert scalar to string
		* @param [in] value scalar to convert
		* @return string result
		*/
		static inline StringType	fromF64(fp64 Val)
		{
			StreamType stream;
			stream << Val;
			return StringType( stream.str() );
		}

		/**
		* @describe convert scalar to string
		* @param [in] value scalar to convert
		* @return string result
		*/
		static inline StringType	fromScalar(scalar Val)
		{
			StreamType stream;
			stream << Val;
			return StringType( stream.str() );			
		}

		/**
		* @describe convert bool to string
		* @param [in] value bool to convert
		* @return string result
		* @remark the yesno idea come from the Ogre.
		*/
		static const StringType	fromBool(bool Val,bool yesno = false)
		{
			static CharT trueString[]	= {CharT('T'),CharT('R'),CharT('U'),CharT('E'),CharT('\0')  };
			static CharT falseString[]	= {CharT('F'),CharT('A'),CharT('L'),CharT('S'),CharT('E'),CharT('\0')  };
			static CharT yesString[]	= {CharT('Y'),CharT('E'),CharT('S'),CharT('\0')};
			static CharT noString[]		= {CharT('N'),CharT('O'),CharT('\0') };

			if( !yesno )
			{
				if( Val )
					return StringType( trueString , 4 , StringType::CONST_HINT);
				else
					return StringType( falseString , 5, StringType::CONST_HINT);
			}
			else
			{
				if( Val )
					return StringType( yesString , 3, StringType::CONST_HINT);
				else
					return StringType( noString , 2, StringType::CONST_HINT);
			}
		}

		/**
		@describe 
		@param
		@return
		*/
		static StringType	fromTime(const std::tm& time)
		{
			static CharT slashString[] = { CharT('/'), CharT('\0') };
			static CharT spaceString[] = { CharT(' '), CharT('\0') };
			static CharT colonString[] = { CharT(':'), CharT('\0') };
			StringType month = fromInt(time.tm_mon);
			StringType day = fromInt(time.tm_mday);
			StringType year = fromInt(time.tm_year+1900);
			StringType hour = fromInt(time.tm_hour);
			StringType min = fromInt(time.tm_min);
			return month + slashString + day +  slashString + year + spaceString + hour + colonString + min;
		}

		/**
		@describe remove the white spaces character of the string
		@param [in] value the string to operate with
		@param [in] left whether to remove left end spaces
		@param [in] right whether to remove right end spaces
		@return result after trim
		*/
		static StringType		trim(const StringType& value,bool left = true,bool right = true)
		{
			index_t leftcount = 0;
			index_t rightcount = value.size() - 1;

			CharT WhiteSpaces[] = {CharT(' '),CharT('\t'),CharT('\n'),CharT('\r')};

			if( left )
			{
				leftcount = value.find_first_not_of(WhiteSpaces,0,4);

				//all characters are spaces
				if( leftcount == INVALID_INDEX )
					return StringType(NULL);
			}

			if( right )
			{
				rightcount = value.find_last_not_of(WhiteSpaces,INVALID_INDEX,4);
				assert( rightcount != INVALID_INDEX );
			}

			return value.substr(leftcount,rightcount - leftcount  + 1);
		}

		/************************************************************************/
		/* file path operations                                                                     */
		/************************************************************************/

		/**
		@describe
		@param
		@return
		*/
		static void		getFilePathInfo(const StringType& fullpath,StringType& dir,StringType& filename)
		{
			CharT pathdel[] = {CharT('\\'),CharT('/'),CharT(':'),CharT('\0')};

			index_t i = fullpath.find_last_of( pathdel );
			if( i != StringType::npos )
			{
				dir = fullpath.substr(0,i);
				filename = fullpath.substr(i+1);
			}
			else
				filename = fullpath;
		}

		/**
		@describe
		@param
		@return
		*/
		static void		getFilePathInfo(const StringType& fullpath,StringType& dir,StringType& filename,StringType& filebase,StringType& extension)
		{
			CharT pathdel[] = {CharT('\\'),CharT('/'),CharT(':'),CharT('\0')};
			index_t i = fullpath.find_last_of( pathdel );
			if( i != StringType::npos )
			{
				dir = fullpath.substr(0,i);
				filename = fullpath.substr(i+1);
			}
			else
				filename = fullpath;

			i = filename.find_last_of(TEXT('.'));
			if( i != StringType::npos )
			{
				filebase = filename.substr(0,i);
				extension = filename.substr(i+1);
			}
			else
				filebase = filename.substr();
		}

		/**
		@describe
		@param
		@return
		*/
		static void getFileInfo(const StringType& filename,StringType& filebase,StringType& extension)
		{
			index_t i = filename.find_last_of(TEXT('.'));
			index_t start = 0;
			//index_t start = filename.find_first_of(TEXT("/\\"));
			//if(start == StringType::npos)
			//	start = 0;
			if( i != StringType::npos )
			{
				filebase = filename.substr(start,i);
				extension = filename.substr(i+1);
			}
			else
				filebase = filename.substr(start);
		}

		/**
		@describe check if the filename has the specified extension. if not,append one.
		@param
		@return
		*/
		static void appendFileExtension(StringType& filename,const StringType& extension)
		{
			index_t i = filename.find_last_of(CharT('.'));
			StringType ext;

			if( i != StringType::npos )
				ext = filename.substr_nocopy(i+1);

			if( ext != extension )
			{
				CharT dot[] = {CharT('.'),CharT('\0')};
				filename += dot + extension;
			}
		}

		/**
		@describe
		@param
		@return
		*/
		static StringType getRelativePath(const StringType& OriginalPath,const StringType& TargetPath)
		{
			static CharT splits[] = {CharT('\\'),CharT('/'),CharT(0)};
			TokenizerType originals;
			TokenizerType targets;
			originals.tokenize(OriginalPath, splits, true);
			targets.tokenize(TargetPath, splits, true);
			size_t i = 0;

			while( i < originals.size() && i < targets.size() && originals[i] == targets[i] )
				++i;

			if( i == 0 )
				return standardizePath(TargetPath);

			ConcatType result = StringType::EMPTY;

			static CharT prefix[] = { CharT('.'),CharT('.'),CharT('/'),CharT(0) };
			StringType prefixString = StringType(prefix, 3, StringType::CONST_HINT);

			if( i < originals.size() )
			{
				for(size_t n = i; n < originals.size(); ++n )
					result += prefixString;
			}
			else
			{
				static CharT dot[] =  { CharT('.'),CharT(0) };
				StringType dotString = StringType(dot, 1, StringType::CONST_HINT);
				result += dotString;
			}

			static CharT connect[] = { CharT('/'),CharT(0) };
			StringType connectString = StringType(connect, 1, StringType::CONST_HINT);
			for( size_t n = i; n < targets.size(); ++n )
			{
				result += connectString;
				result += targets[n];
			}

			return result;
		}

		/**
		@describe 
		@param 
		@return 
		*/
		static StringType getAbsolutePath(const StringType& OriginalPath, const StringType& relativePath)
		{
			static CharT splits[] = {CharT('\\'),CharT('/'),CharT(0)};
			static CharT current_node[] = {CharT('.'),CharT(0)};
			static CharT parent_node[] = {CharT('.'),CharT('.'),CharT(0)};
			static CharT slash[] = { CharT('/'),CharT(0) };

			TokenizerType originals;
			TokenizerType relatives;
			originals.tokenize(OriginalPath, splits, true);
			relatives.tokenize(relativePath, splits, true);
			bool isRoot = OriginalPath.size() >= 1 && OriginalPath[0] == CharT('/');

			size_t realNode = 0;
			for(size_t i = 0; i < originals.size(); ++i)
			{
				const StringType& node = originals[i];

				if( node == current_node && (i != 0 || isRoot) )
					originals.erase(i--);
				else if( realNode >= 1 && node == parent_node && originals[i-1] != parent_node )
				{
					originals.erase(--i);
					originals.erase(i--);
					--realNode;
				}
				else
					++realNode;
			}

			size_t relativeCount = relatives.size();
			for(size_t i = 0; i < relativeCount; ++i)
			{
				const StringType& node = relatives[i];
				if( node == current_node )
					continue;
				else if( node == parent_node && originals.size() > 1 )
					originals.erase( originals.size()-1 );
				else
					originals.push_back( relatives[i] );
			}

			ConcatType result = isRoot ? slash : TraitsType::null();
			size_t originalCount = originals.size();
			for(size_t i = 0; i < originalCount; ++i)
			{
				result += originals[i];
				if( i != originals.size()-1 )
					result += slash;
			}

			return result;
		}
		static inline StringType standardizePath(const StringType& OriginalPath)
		{
			return getAbsolutePath(OriginalPath, StringType::EMPTY);
		}

		/**
		@describe
		@param
		@return
		*/
		static inline bool	isAbsolutePath(const StringType& path)
		{
#if BLADE_IS_WINDOWS_CLASS_SYSTEM
			return (path.size() >= 2 && TraitsType::is_alpha(path[0]) && path[1] == CharT(':'))
				|| (path.size() >=1 && (path[0] == CharT('/')) || path[0] == CharT('\\') );
#else
			return path.size() >= 1 && path[0] == CharT('/');
#endif
		}

		/************************************************************************/
		/* misc string operations                                                                     */
		/************************************************************************/
		/**
		@describe
		@param
		@return
		*/
		static inline void	toLowerCase(StringType& in_str)
		{
			TraitsType::tolower( &in_str[0] );
		}

		/**
		@describe
		@param
		@return
		*/
		static inline StringType getLowerCase(const StringType& in_str)
		{
			StringType ret = in_str;
			TraitsType::tolower( &ret[0] );
			return ret;
		}

		/**
		@describe
		@param
		@return
		*/
		static inline void		toUpperCase(StringType& in_str)
		{
			TraitsType::toupper( &in_str[0] );
		}

		/**
		@describe
		@param
		@return
		*/
		static inline StringType getUpperCase(const StringType& in_str)
		{
			StringType ret = in_str;
			TraitsType::toupper( &ret[0] );
			return ret;
		}

		/**
		@describe
		@param
		@return
		*/
		static inline bool		isStartWith(const StringType& in_str,const StringType& start_str,bool bCaseSensitive = true)
		{
			const StringType start = in_str.substr_nocopy(0,start_str.size());
			if (!bCaseSensitive)
				return start.compare_no_case(start_str) == 0;
			else
				return start == start_str;
		}

		/**
		@describe
		@param
		@return
		*/
		static inline bool		isEndWith(const StringType& in_str,const StringType& end_str,bool bCaseSensitive = true)
		{
			const StringType end = in_str.substr_nocopy(in_str.size() - end_str.size() );
			if( !bCaseSensitive )
				return end.compare_no_case(end_str) == 0;
			else
				return end == end_str;
		}

		/**
		@describe 
		@param
		@return
		*/
		static size_t countSub(const StringType& in_str, const StringType& sub, size_t limit = -1)
		{
			size_t count = 0;
			StringType str = in_str.substr_nocopy();
			for( size_t i = 0 ; i < limit; ++i )
			{
				index_t index = str.find(sub);
				if( index == StringType::npos )
					break;
				str = str.substr_nocopy(index+sub.size());
				++count;
			}
			return count;
		}

		/**
		@describe
		@param
		@return
		*/
		static size_t replace(StringType& outReplaced, const StringType& in_str,const StringType& toReplace, const StringType& replaceWith,size_t count = -1)
		{
			size_t replacedCount = 0;
			StringType cin_str = in_str.substr_nocopy(0);
			ConcatType proxy( StringType::EMPTY );
			for( size_t i = 0 ; i < count; ++i )
			{
				index_t index = cin_str.find(toReplace);
				if( index == StringType::npos )
					break;
				proxy += cin_str.substr_nocopy(0,index);
				proxy += replaceWith;

				cin_str = cin_str.substr_nocopy(index+toReplace.size());
				++replacedCount;
			}
			proxy += cin_str;
			outReplaced = proxy;
			return replacedCount;
		}

		/**
		@describe
		@param
		@return
		*/
		static inline size_t replace(StringType& in_outReplaced, const StringType& toReplace, const StringType& replaceWith,size_t count = -1)
		{
			return replace(in_outReplaced, in_outReplaced, toReplace, replaceWith, count);
		}

		/**
		@describe
		@param
		@return
		*/
		static inline StringType	getReplace(const StringType& in_str,const StringType toReplace,const StringType& replaceWith,size_t count = -1)
		{
			StringType ret;
			replace(ret, in_str, toReplace, replaceWith, count);
			return ret;
		}

		/**
		@describe 
		@param 
		@return 
		*/
		static bool		wildcardMatch(const StringType& wildcard, const StringType& str, bool bCaseSensitive = true)
		{
			StringType pattern = wildcard;
			StringType input = str;
			if( !bCaseSensitive )
			{
				toUpperCase(pattern);
				toUpperCase(input);
			}

			size_t strIndex = 0;
			size_t patIndex = 0;
			size_t lastPatIndex = StringType::npos;
			while( strIndex < str.size() && patIndex < pattern.size() )
			{
				if( pattern[patIndex] == CharT('*') )
				{
					lastPatIndex = patIndex;
					++patIndex;

					//last * mathes all str
					if( patIndex == pattern.size() )
						strIndex = str.size();
					else
					{
						//find next *
						while(strIndex < str.size() && str[strIndex] != pattern[patIndex] )
							++strIndex;
					}
				}
				else
				{
					if( str[strIndex] != pattern[patIndex] )
					{
						if(lastPatIndex != StringType::npos )
						{
							//the last * can match this incorrect sequence,
							//rewind pattern to last wildcard
							patIndex = lastPatIndex;
							lastPatIndex = StringType::npos;
						}
						else //no wildcards left
							return false;
					}
					else
					{
						++strIndex;
						++patIndex;
					}
				}
			}
			// match if we reached the end of both the pattern
			return patIndex == pattern.size() && strIndex == str.size();
		}

	};//class BasicStringHelper

	typedef BasicStringHelper<char, StringAllocator>		StringHelper;
	typedef BasicStringHelper<wchar_t, StringAllocator>		WStringHelper;


#if BLADE_UNICODE
	typedef WStringTokenizer TStringTokenizer;
	typedef WStringHelper TStringHelper;
#else
	typedef StringHelper TStringHelper;
	typedef StringTokenizer TStringTokenizer;
#endif
	
}//namespace Blade


#endif //__Blade_StringHelper_h__