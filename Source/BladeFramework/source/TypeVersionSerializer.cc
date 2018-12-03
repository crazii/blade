/********************************************************************
	created:	2013/04/01
	filename: 	TypeVersionSerializer.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <TypeVersionSerializer.h>

namespace Blade
{

	namespace TypeVersionSerializerUtil
	{
		//////////////////////////////////////////////////////////////////////////
		bool	readSerializerInfo(const HSTREAM& stream, TString& type, Version& version)
		{
			TString header;
			{
				TempVector<char> tempString;
				char c = char();
				do
				{
					stream->read(&c,1);

					//error handling
					if( c == '\0')
						break;

					tempString.push_back( (char)c );
				}while( c != '\n' );
				tempString.push_back( char() ) ;
				header = StringConverter::UTF8StringToTString( &tempString[0], tempString.size() );
			}

			header = TStringHelper::trim(header);
			if( !TStringHelper::isStartWith(header, BTString("<!--"))
				|| !TStringHelper::isEndWith(header, BTString("-->")) 
				)
			{
				assert(false);
				return false;
			}
			header = TStringHelper::trim( header.substr_nocopy(4, header.size()-7) );

			TStringTokenizer tokenizer;
			tokenizer.tokenize(header, TEXT(",") );
			if( tokenizer.size() != 2 )
				return false;

			type = tokenizer[0];
			version = Version( tokenizer[1] );
			return true;
		}


		//////////////////////////////////////////////////////////////////////////
		bool	readSerializerInfo(const HSTREAM& stream, Version& version)
		{
			TString header;
			{
				TempVector<char> tempString;
				char c = char();
				do
				{
					stream->read(&c,1);

					//error handling
					if( c == '\0')
						break;
					tempString.push_back( (char)c );
				}while( c != '\n' );

				tempString.push_back( char() ) ;
				header = StringConverter::UTF8StringToTString( &tempString[0], tempString.size() );
			}

			header = TStringHelper::trim(header);
			if( !TStringHelper::isStartWith(header, BTString("<!--"))
				|| !TStringHelper::isEndWith(header, BTString("-->")) 
				)
			{
				assert(false);
				return false;
			}

			header = TStringHelper::trim( header.substr_nocopy(4, header.size()-7) );
			version = Version( header );
			return true;
		}

		//////////////////////////////////////////////////////////////////////////
		bool	writeSerializerInfo(const HSTREAM& stream, const TString& type, const Version& version)
		{
			const TString header = BTString("<!-- ") 
				+ type
				+ BTString(",") 
				+ version.getVersionString()
				+ BTString(" -->\n");

			TempBuffer utf8Buffer;
			size_t utf8c = 0;
			const char* utf8s = StringConverter::TStringToUTF8String(utf8Buffer, header, &utf8c);
			stream->write( utf8s, utf8c );
			//stream->writeAtom( (int8)0 );
			
			return true;
		}


		//////////////////////////////////////////////////////////////////////////
		bool	writeSerializerInfo(const HSTREAM& stream, const Version& version)
		{
			const TString header = BTString("<!-- ") + version.getVersionString() + BTString(" -->\n");

			TempBuffer utf8Buffer;
			size_t utf8c = 0;
			const char* utf8s = StringConverter::TStringToUTF8String(utf8Buffer, header, &utf8c);
			stream->write( utf8s, utf8c );
			//stream->writeAtom( (int8)0 );

			return true;
		}


	}//namespace TypeVersionSerializerUtil
	
}//namespace Blade