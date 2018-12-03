/********************************************************************
	created:	2010/08/10
	filename: 	StringHelper.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "InternalStringHelper.h"

namespace Blade
{
	using namespace Impl;

	/************************************************************************/
	/* StringTokenizer                                                                     */
	/************************************************************************/
	StringTokenizer::StringTokenizer()
		:mTokenizer( BLADE_NEW StringTokenizerImpl() )
	{
		mTokenizer->reserve(32);
	}

	StringTokenizer::~StringTokenizer()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	void			StringTokenizer::tokenize(const String& inString,const char* tokens, bool inplace/* = false*/)
	{
		mTokenizer->tokenize( reinterpret_cast<const StringTokenizerImpl::string_type&>(inString), tokens, inplace);
	}

	//////////////////////////////////////////////////////////////////////////
	bool			StringTokenizer::tokenize(const String& inString,const char* doubleTokens,const char* tokens, bool inplace/* = false*/)
	{
		return mTokenizer->tokenize( reinterpret_cast<const StringTokenizerImpl::string_type&>(inString), doubleTokens, tokens, inplace);
	}

	//////////////////////////////////////////////////////////////////////////
	bool			StringTokenizer::isDoubleToken(index_t stringIndex) const
	{
		return mTokenizer->isDoubleToken(stringIndex);
	}

	//////////////////////////////////////////////////////////////////////////
	void			StringTokenizer::splitLines(const String& inString)
	{
		mTokenizer->splitLines(reinterpret_cast<const StringTokenizerImpl::string_type&>(inString));
	}

	//////////////////////////////////////////////////////////////////////////
	void			StringTokenizer::removeComments(const char* lineCommnet/* = "//"*/, const char* blockComment, const char* blockCommentEnd)
	{
		mTokenizer->removeComments(lineCommnet, blockComment, blockCommentEnd);
	}

	//////////////////////////////////////////////////////////////////////////
	const String&	StringTokenizer::at(index_t index) const
	{
		return reinterpret_cast<const String&>( mTokenizer->at(index) );
	}

	//////////////////////////////////////////////////////////////////////////
	index_t			StringTokenizer::find(const String& str) const
	{
		StringTokenizerImpl::string_type val = reinterpret_cast<const StringTokenizerImpl::string_type&>(str);
		StringTokenizerImpl::const_iterator i = std::find( mTokenizer->begin(), mTokenizer->end(), val);
		if( i == mTokenizer->end() )
			return INVALID_INDEX;
		else
			return (index_t)(i - mTokenizer->begin());
	}

	//////////////////////////////////////////////////////////////////////////
	void			StringTokenizer::erase(index_t index)
	{
		mTokenizer->erase(index);
	}

	//////////////////////////////////////////////////////////////////////////
	bool			StringTokenizer::set(index_t index, const String& Val)
	{
		if( mTokenizer->size() <= index )
		{
			assert(false);
			return false;
		}
		(*mTokenizer)[index] = reinterpret_cast<const StringTokenizerImpl::string_type&>(Val);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void			StringTokenizer::push_back(const String& inString)
	{
		return mTokenizer->push_back( reinterpret_cast<const StringTokenizerImpl::string_type&>(inString) );
	}

	//////////////////////////////////////////////////////////////////////////
	size_t			StringTokenizer::size()
	{
		return mTokenizer->size();
	}

	//////////////////////////////////////////////////////////////////////////
	void			StringTokenizer::clear()
	{
		mTokenizer->clear();
	}

	/************************************************************************/
	/* WStringTokenizer                                                                     */
	/************************************************************************/
	WStringTokenizer::WStringTokenizer()
		:mTokenizer( BLADE_NEW Impl::WStringTokenizerImpl() )
	{
		mTokenizer->reserve(32);
	}

	WStringTokenizer::~WStringTokenizer()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	void			WStringTokenizer::tokenize(const WString& inWString, const wchar* tokens, bool inplace/* = false*/)
	{
		mTokenizer->tokenize( reinterpret_cast<const WStringTokenizerImpl::string_type&>(inWString), tokens, inplace);
	}

	//////////////////////////////////////////////////////////////////////////
	bool			WStringTokenizer::tokenize(const WString& inWString, const wchar* doubleTokens, const wchar* tokens, bool inplace/* = false*/)
	{
		return mTokenizer->tokenize( reinterpret_cast<const WStringTokenizerImpl::string_type&>(inWString),doubleTokens, tokens, inplace);
	}

	//////////////////////////////////////////////////////////////////////////
	bool			WStringTokenizer::isDoubleToken(index_t WStringIndex) const
	{
		return mTokenizer->isDoubleToken(WStringIndex);
	}

	//////////////////////////////////////////////////////////////////////////
	void			WStringTokenizer::splitLines(const WString& inWString)
	{
		mTokenizer->splitLines(reinterpret_cast<const WStringTokenizerImpl::string_type&>(inWString));
	}

	//////////////////////////////////////////////////////////////////////////
	void			WStringTokenizer::removeComments(const wchar* lineCommnet/* = L"//"*/, const wchar* blockComment, const wchar* blockCommentEnd)
	{
		mTokenizer->removeComments(lineCommnet, blockComment, blockCommentEnd);
	}

	//////////////////////////////////////////////////////////////////////////
	const WString&	WStringTokenizer::at(index_t index) const
	{
		return reinterpret_cast<const WString&>( mTokenizer->at(index) );
	}

	//////////////////////////////////////////////////////////////////////////
	index_t			WStringTokenizer::find(const WString& str) const
	{
		WStringTokenizerImpl::string_type val = reinterpret_cast<const WStringTokenizerImpl::string_type&>(str);
		WStringTokenizerImpl::const_iterator i = std::find( mTokenizer->begin(), mTokenizer->end(), val);
		if( i == mTokenizer->end() )
			return INVALID_INDEX;
		else
			return (index_t)(i - mTokenizer->begin());
	}

	//////////////////////////////////////////////////////////////////////////
	void			WStringTokenizer::erase(index_t index)
	{
		mTokenizer->erase(index);
	}

	//////////////////////////////////////////////////////////////////////////
	bool			WStringTokenizer::set(index_t index, const WString& Val)
	{
		if( mTokenizer->size() <= index )
		{
			assert(false);
			return false;
		}
		(*mTokenizer)[index] = reinterpret_cast<const WStringTokenizerImpl::string_type&>(Val);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void			WStringTokenizer::push_back(const WString& inWString)
	{
		return mTokenizer->push_back( reinterpret_cast<const WStringTokenizerImpl::string_type&>(inWString) );
	}

	//////////////////////////////////////////////////////////////////////////
	size_t			WStringTokenizer::size()
	{
		return mTokenizer->size();
	}

	//////////////////////////////////////////////////////////////////////////
	void			WStringTokenizer::clear()
	{
		mTokenizer->clear();
	}

}//namespace Blade