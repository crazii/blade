/********************************************************************
created:	2013/01/26
filename: 	Base64.h
author:		Crazii

purpose:	copied from http://base64.sourceforge.net/b64.c
			1.a littel bit change on encoding/decoding input params -
			(C to C++):
			change FILE* to std::stream.
			
			2.remove most error codes, messages, b64 function & main function.
			
			3.separate b64.c file into Base64.h & Base64.cc
			
			4.add prefix (b64_) to encode/decode function
*********************************************************************/
#include <BladePCH.h>
#include "Base64.h"

/*
** Translation Table as described in RFC1113
*/
static const char cb64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/*
** Translation Table to decode (created by author)
*/
static const char cd64[]="|$$$}rstuvwxyz{$$$$$$$>?@ABCDEFGHIJKLMNOPQRSTUVW$$$$$$XYZ[\\]^_`abcdefghijklmnopq";

/*
** encodeblock
**
** encode 3 8-bit binary bytes as 4 '6-bit' characters
*/
static void encodeblock( unsigned char *in, unsigned char *out, int len)
{
	out[0] = (unsigned char) cb64[ (int)(in[0] >> 2) ];
	out[1] = (unsigned char) cb64[ (int)(((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4)) ];
	out[2] = (unsigned char) (len > 1 ? cb64[ (int)(((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6)) ] : '=');
	out[3] = (unsigned char) (len > 2 ? cb64[ (int)(in[2] & 0x3f) ] : '=');
}

/*
** encode
**
** base64 encode a stream adding padding and line breaks as per spec.
*/
int b64_encode( B64IStream& infile, B64OStream& outfile, int linesize/* = B64_DEF_LINE_SIZE*/)
{
	unsigned char in[3];
	unsigned char out[4];
	int i, len, blocksout = 0;
	int retcode = 0;

	*in = (unsigned char) 0;
	*out = (unsigned char) 0;
	while( !infile.eof() ) {
		len = 0;
		for( i = 0; i < 3; i++ ) {
			infile.get( (char&)in[i] );

			if( !infile.eof() ) {
				len++;
			}
			else {
				in[i] = (unsigned char) 0;
			}
		}
		if( len > 0 ) {
			encodeblock( in, out, len );
			for( i = 0; i < 4; i++ ) {
				if( outfile.put((char)out[i]).fail()	){
					if( outfile.bad() )      {
						retcode = B64_FILE_IO_ERROR;
					}
					break;
				}
			}
			blocksout++;
		}

		if( blocksout >= (linesize/4) || infile.eof() ) {
			if( blocksout > 0 ) {
				outfile << "\r\n";
			}
			blocksout = 0;
		}
	}
	return( retcode );
}

/*
** decodeblock
**
** decode 4 '6-bit' characters into 3 8-bit binary bytes
*/
static void decodeblock( unsigned char *in, unsigned char *out )
{   
	out[ 0 ] = (unsigned char ) (in[0] << 2 | in[1] >> 4);
	out[ 1 ] = (unsigned char ) (in[1] << 4 | in[2] >> 2);
	out[ 2 ] = (unsigned char ) (((in[2] << 6) & 0xc0) | in[3]);
}

/*
** decode
**
** decode a base64 encoded stream discarding padding, line breaks and noise
*/
int b64_decode( B64IStream& infile, B64OStream& outfile)
{
	int retcode = 0;
	unsigned char in[4];
	unsigned char out[3];
	char v;
	int i, len;

	*in = (unsigned char) 0;
	*out = (unsigned char) 0;
	while( !infile.eof() ) {
		for( len = 0, i = 0; i < 4 && !infile.eof(); i++ ) {
			v = 0;
			while( !infile.eof() && v == 0 ) {
				infile.get(v);
				if( !infile.eof() ) {
					v = ((v < 43 || v > 122) ? 0 : (int) cd64[ v - 43 ]);
					if( v != 0 ) {
						v = ((v == (int)'$') ? 0 : v - 61);
					}
				}
			}
			if( !infile.eof() == 0 ) {
				len++;
				if( v != 0 ) {
					in[ i ] = (unsigned char) (v - 1);
				}
			}
			else {
				in[i] = (unsigned char) 0;
			}
		}
		if( len > 0 ) {
			decodeblock( in, out );
			for( i = 0; i < len - 1; i++ ) {
				if( outfile.put((char)out[i]).fail() ){
					if( outfile.bad() )      {
						retcode = B64_FILE_IO_ERROR;
					}
					break;
				}
			}
		}
	}
	return( retcode );
}