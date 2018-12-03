/********************************************************************
	created:	2015/09/05
	filename: 	InternalStringHelper.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_InternalStringHelper_h__
#define __Blade_InternalStringHelper_h__
#include "InternalString.h"
#include <utility/BladeContainer.h>
#include <BladeException.h>
#if BLADE_COMPILER_GNU_EXTENSION
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#endif

namespace Blade
{
	namespace Impl
	{

		/************************************************************************/
		/* string operator+ proxy                                                                      */
		/************************************************************************/
		template <typename CharT, typename AllocT>
		class BasicStringConcat :
			public TList< BasicString<CharT, Char_Traits<CharT>, AllocT>, TempAllocator<BasicString<CharT, Char_Traits<CharT>, AllocT> >, TempAllocatable>
		{
		public:
			typedef BasicString<CharT,Char_Traits<CharT>, AllocT>	string_type;
			typedef typename string_type::traits_type				traits_type;
			typedef TList< BasicString<CharT, Char_Traits<CharT>, AllocT>, TempAllocator<BasicString<CharT, Char_Traits<CharT>, AllocT> >, TempAllocatable>
				base_type;

			/** @brief  */
			BasicStringConcat()
			{
				mSize = 0;
			}

			/** @brief  */
			void	add(const string_type& s)
			{
				this->push_back(s);
				mSize += s.size();
			}

			/** @brief  */
			size_t	count() const
			{
				return this->size();
			}

			/** @brief  */
			size_t	total_size() const
			{
				return mSize;
			}

			/** @brief  */
			const string_type& at(index_t index) const
			{
				return base_type::operator [](index);
			}

		protected:
			size_t	mSize;
		};//class BasicStringConcat

		class StringConcatImpl : public BasicStringConcat<char, TempStringAllocator>
		{
		public:
			inline StringConcatImpl()	{}
#if BLADE_DEBUG && 0
			inline StringConcatImpl(const char* file, int line) :BasicStringConcat<char, TempStringAllocator>(file,line){}
#endif
		};
		class WStringConcatImpl : public BasicStringConcat<wchar, TempStringAllocator>
		{
		public:
			inline WStringConcatImpl()	{}
#if BLADE_DEBUG && 0
			inline WStringConcatImpl(const char* file, int line) :BasicStringConcat<wchar, TempStringAllocator>(file,line){}
#endif
		};


		/************************************************************************/
		/* BasicStringTokenizerImpl                                                                     */
		/************************************************************************/
		//to boost the split function (avoid temp string object data allocation),first just record the substring information(do not copy buffer),
		//then perform a final pass to allocation buffer
//#define TOKENIZER_DEBUG
#ifndef TOKENIZER_DEBUG
#	define substr substr_nocopy
#endif

		template <typename CharT >
		class BasicStringTokenizerImpl : public TList< BasicString<CharT, Char_Traits<CharT>, TempStringAllocator>, TempAllocator<BasicString<CharT, Char_Traits<CharT>, TempStringAllocator> >,  TempAllocatable>
		{
		public:
			typedef BasicStringTokenizerImpl<CharT>									this_type;
			typedef BasicString<CharT, Char_Traits<CharT>, TempStringAllocator >	string_type;
			typedef TList< BasicString<CharT, Char_Traits<CharT>, TempStringAllocator>, TempAllocator<BasicString<CharT, Char_Traits<CharT>, TempStringAllocator> >, TempAllocatable> base_type;
			typedef BasicStringConcat<CharT, TempStringAllocator>					concat_type;
			typedef BasicStringHelper<CharT, TempStringAllocator>					helper_type;

			typedef typename base_type::iterator							iterator;
			typedef typename base_type::const_iterator						const_iterator;
			typedef typename string_type::traits_type						traits_type;

			BasicStringTokenizerImpl()	{}

			//////////////////////////////////////////////////////////////////////////
			inline void reserve(size_t _count)
			{
				this->base_type::reserve(_count);
				mDoubleToken.reserve(_count);
			}

			//////////////////////////////////////////////////////////////////////////
			void	tokenize(const string_type& inString, const CharT* tokens, bool inplace)
			{
#ifdef TOKENIZER_DEBUG
				BLADE_UNREFERENCED(inplace);
#endif
				index_t start = 0,pos;
				size_t	TokenSize = traits_type::length(tokens);

				do 
				{
					pos = inString.find_first_of(tokens, start, TokenSize);

					if( pos != INVALID_INDEX )
					{
						//continuous tokens
						if (pos != start)
						{
							this->push_back(inString.substr(start, pos - start));
#ifndef TOKENIZER_DEBUG
							if (!inplace)
								this->back().make_const_temp();
#endif
						}

						start = pos + 1;
					}
					else
					{
						if( start != INVALID_INDEX && inString.size() != 0)
						{
							//to the end of the string: this is the last one
							string_type lastString = inString.substr(start);
							//if( lastString.size() != 0 )
							this->push_back( lastString );
#ifndef TOKENIZER_DEBUG
							if(!inplace)
								this->back().make_const_temp();
#endif
						}
						break;
					}
					//find the next data after token
					start = inString.find_first_not_of(tokens,start,TokenSize);
				} while (pos != INVALID_INDEX );
			}

			//////////////////////////////////////////////////////////////////////////
			bool	tokenize(const string_type& inString,const CharT* doubleTokens,const CharT* tokens, bool inplace)
			{
				if( inString.size() == 0 )
					return true;

				index_t start = 0,pos;
				size_t	TokenSize = traits_type::length(tokens);
				const string_type DoubleTokenString(doubleTokens,traits_type::length(doubleTokens), string_type::CONST_HINT);

				string_type left;
				left.resize( (DoubleTokenString.size() + 1)/2 );
				for(size_t i = 0; i < DoubleTokenString.size(); i+=2 )
					left[i/2] = DoubleTokenString[i];

				bool hasDoubleToken = true;
				//note: double tokenize DOESN'T support nesting
				//i.e. (())
				do 
				{
					if (hasDoubleToken)
					{
						pos = inString.find_first_of(left.c_str(), start, left.size());
						if (pos != INVALID_INDEX)
						{
							bool rightMatch = false;
							for (size_t i = 0; i < left.size(); ++i)
							{
								if(inString[pos] != left[i])
									continue;

								CharT right = DoubleTokenString[i * 2 + 1];

								size_t pos_end = inString.find_first_of(right, pos + 1);
								if (pos_end == INVALID_INDEX)
									continue;

								rightMatch = true;
								const string_type before = inString.substr_nocopy(start, pos - start);
								this->tokenize(before, tokens, inplace);

								this->push_back(inString.substr(pos + 1, pos_end - (pos + 1)));
#ifndef TOKENIZER_DEBUG
								if (!inplace)
									this->back().make_const_temp();
#endif
								this->setDoubleToken(this->size() - 1);

								pos = pos_end + 1;
								start = inString.find_first_not_of(tokens, pos, TokenSize);
							}

							if (!rightMatch)
								hasDoubleToken = false;
							continue;
						}
						else
							hasDoubleToken = false;
					}

					pos = inString.find_first_of(tokens, start, TokenSize);

					if( pos != INVALID_INDEX )
					{
						//continuous tokens
						if (pos != start)
						{
							this->push_back(inString.substr(start, pos - start));
#ifndef TOKENIZER_DEBUG
							if (!inplace)
								this->back().make_const_temp();
#endif
						}

						start = pos + 1;
					}
					else
					{
						if( start != INVALID_INDEX )
						{
							//to the end of the string: this is the last one
							string_type lastString = inString.substr(start);
							//if( lastString.size() != 0 )
							this->push_back( lastString );
#ifndef TOKENIZER_DEBUG
							if (!inplace)
								this->back().make_const_temp();
#endif
						}
						break;
					}
					//find the next data after token
					start = inString.find_first_not_of(tokens,start,TokenSize);
				} while (pos != INVALID_INDEX );

				//full fill the mask
				if( mDoubleToken.size() < this->size() )
					this->setDoubleToken( this->size() - 1 , false );
				return true;
			}
#ifndef TOKENIZER_DEBUG
#	undef substr
#endif

			//////////////////////////////////////////////////////////////////////////
			bool	isDoubleToken(index_t stringIndex) const
			{
				assert( mDoubleToken.size() == this->size() );
				if( stringIndex < this->size() )
					return mDoubleToken[stringIndex];
				else
					BLADE_EXCEPT(EXC_OUT_OF_RANGE,BTString("index out of range.") );
			}

			//////////////////////////////////////////////////////////////////////////
			void	splitLines(const string_type& inString)
			{
				index_t start = 0, pos;

				do
				{
					//\r\n, \n\r, \n
					pos = inString.find_first_of(CharT('\n'), start);

					if( pos != INVALID_INDEX )
					{
						size_t CRLF = 1;
						//note: check prev char first, if check next first, \r\n\r\n will be recognized as \n\r
						if (pos > 0 && inString[pos - 1] == CharT('\r'))
						{
							--pos;
							++CRLF;
						}
						else if (pos + 1 < inString.size() && inString[pos + 1] == CharT('\r'))
							++CRLF;

						//should record continuous lines
						this->push_back( inString.substr(start, pos - start) );
#ifndef TOKENIZER_DEBUG
						this->back().make_const_temp();
#endif
						start = pos + CRLF;
					}
					else
					{
						if( start != INVALID_INDEX && inString.size() != 0)
						{
							//to the end of the string: this is the last one
							string_type lastString = inString.substr(start);
							//if( lastString.size() != 0 )
							this->push_back( lastString );
#ifndef TOKENIZER_DEBUG
							this->back().make_const_temp();
#endif
						}
						break;
					}
				} while (pos != INVALID_INDEX );
			}

			//////////////////////////////////////////////////////////////////////////
			void	removeComments(const CharT* lineCommentWord, const CharT* blockCommentWord, const CharT* blockCommentEndWord)
			{
				if( lineCommentWord == NULL)
					lineCommentWord = traits_type::null();
				if( blockCommentWord == NULL )
					blockCommentWord = traits_type::null();
				if( blockCommentEndWord == NULL )
					blockCommentEndWord = traits_type::null();

				if( *lineCommentWord == CharT(0) && *blockCommentWord == CharT(0) && *blockCommentEndWord == CharT(0) )
					return;

				size_t lineCommentSize = traits_type::length(lineCommentWord);
				size_t blockCommentSize = traits_type::length(blockCommentWord);
				size_t blockCommentEndSize = traits_type::length(blockCommentEndWord);

				bool inBlockComment = false;
				const string_type lineCommentString = string_type(lineCommentWord, lineCommentSize, string_type::CONST_HINT);
				const string_type blockStart = string_type(blockCommentWord, blockCommentSize, string_type::CONST_HINT);
				const string_type blockEnd = string_type(blockCommentEndWord, blockCommentEndSize, string_type::CONST_HINT);

				for(index_t lineNO = 0; lineNO < this->size(); ++lineNO)
				{
					string_type rawLine = helper_type::trim(this->at(lineNO));
					if( rawLine.empty() )
						continue;

					index_t quotation = rawLine.find(CharT('"'));
					index_t lineComment = *lineCommentWord == CharT(0) ? string_type::npos : rawLine.find(lineCommentString);
					index_t blockComment = *blockCommentWord == CharT(0) ? string_type::npos : rawLine.find(blockStart);
					index_t comment = std::min<index_t>(lineComment, blockComment);

					index_t blockCommentEnd = *blockCommentEndWord == CharT(0) ? string_type::npos : rawLine.find(blockEnd);
					while( blockCommentEnd != string_type::npos && blockCommentEnd > 0 )
					{
						size_t prevComment = rawLine.rfind(blockStart, blockCommentEnd);
						//overlap check
						if(prevComment != string_type::npos && prevComment + blockCommentSize > blockCommentEnd )
							blockCommentEnd = rawLine.find(blockEnd, blockCommentEnd+blockCommentEndSize);
						else
							break;
					}
					if( inBlockComment )
					{
						if( blockCommentEnd != string_type::npos )
						{
							inBlockComment = false;
							rawLine = rawLine.substr(blockCommentEnd+blockCommentEndSize);
						}
						else
						{
							//clear comment line
							(*this)[lineNO] = string_type::EMPTY;
							continue;
						}
					}

					string_type testLine = rawLine.substr_nocopy();

					//INTERNAL HACK: same memory layout of StringConcat
					Pimpl<concat_type> validText(BLADE_NEW concat_type() );
					static_assert(sizeof(validText) == sizeof(typename string_type::concat_type), "memory layout mismatch" );

					if( comment == string_type::npos )
						validText->add(rawLine);

					size_t quoteCount = 0;
					while( comment != string_type::npos )
					{
						while(quotation < comment)
						{
							++quoteCount;
							quotation = testLine.find(CharT('"'), quotation+1);
						}

						//not "///**/"
						if( quoteCount%2 == 0 )
						{
							string_type text = testLine.substr_nocopy(0, comment);
							validText->add(testLine.substr_nocopy(0, comment));
							if( comment == lineComment )
								break;
							assert(comment == blockComment);
							blockCommentEnd = testLine.find(blockEnd, blockComment+blockCommentSize);
							if(blockCommentEnd == string_type::npos )
							{
								inBlockComment = true;
								break;
							}
							if(blockCommentEnd + blockCommentEndSize >= testLine.size())
							{
								assert(blockCommentEnd + blockCommentEndSize == testLine.size());
								break;
							}

							testLine = testLine.substr_nocopy(blockCommentEnd+blockCommentSize);
						}
						else if( quotation != string_type::npos )
						{
							++quoteCount;
							testLine = testLine.substr_nocopy(quotation+1);
						}
						else
							break;
						assert(quoteCount%2 == 0);

						blockComment = *blockCommentWord == CharT(0) ? string_type::npos : testLine.find(blockStart);
						lineComment = *lineCommentWord == CharT(0) ? string_type::npos : testLine.find(lineCommentString);
						quotation = testLine.find(CharT('"'));
						comment = std::min<index_t>(lineComment, blockComment);
					}//while

					(*this)[lineNO] = reinterpret_cast<const typename string_type::concat_type&>(validText);
				}//for
			}

			//////////////////////////////////////////////////////////////////////////
			const string_type&	at(index_t index) const
			{
				if( index < this->size() )
					return (*this)[index];
				else
					BLADE_EXCEPT(EXC_OUT_OF_RANGE,BTString("index out of range.") );
			}

			//////////////////////////////////////////////////////////////////////////
			string_type&	at(index_t index)
			{
				if( index < this->size() )
					return (*this)[index];
				else
					BLADE_EXCEPT(EXC_OUT_OF_RANGE,BTString("index out of range.") );
			}

			//////////////////////////////////////////////////////////////////////////
			void		erase(index_t index)
			{
				if( index < this->size() )
				{
					iterator i = this->begin();
					base_type::erase( i + (indexdiff_t)index );
				}
				else
					BLADE_EXCEPT(EXC_OUT_OF_RANGE,BTString("index out of range.") );
			}

		protected:

			/** @brief  */
			void	setDoubleToken(index_t index, bool doubled = true)
			{
				if( mDoubleToken.size() > index )
					mDoubleToken[index] = doubled;
				else
				{
					if (mDoubleToken.capacity() <= index)
						mDoubleToken.reserve((index+1)*2);

					while( mDoubleToken.size() < index )
						mDoubleToken.push_back(false);

					mDoubleToken.push_back(doubled);
				}					
			}

			typedef TempVector<bool>	DoubleTokenMask;
			DoubleTokenMask				mDoubleToken;
		};//class BasicStringTokenizerImpl

		class StringTokenizerImpl : public BasicStringTokenizerImpl<char> {};
		class WStringTokenizerImpl : public BasicStringTokenizerImpl<wchar> {};	

		
	}//namespace Impl
	
}//namespace Blade


#endif // __Blade_InternalStringHelper_h__