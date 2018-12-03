/********************************************************************
	created:	2009/04/07
	filename: 	TextFile.h
	author:		Crazii
	
	purpose:	for access to pure text files,UNICODE support

	note:		this text file is for temporary use,that means it's not recommended to be
				created dynamically in heap.you should read its content into a persistent place,
				and destroy it as soon as possible.

				for UNICODE(wide char) build, text file uses WString (usually UTF16 on Windows, UTF32 on Linux) as runtime buffer
				on none-UNICODE build, text file uses (UTF8) String at runtime by default, you can change its format to MBCS.

	log:		2010/03/24 use IStream for I/O access instead of std FILE stream
				this could be useful when access a text file in special archive,i.e. zip file
*********************************************************************/
#ifndef __Blade_TextFile_h__
#define __Blade_TextFile_h__
#include <interface/public/file/IStream.h>
#include "Buffer.h"
#include "String.h"

namespace Blade
{
	class IOBuffer;

	class BLADE_BASE_API TextFile : public Allocatable
	{
	private:
		TextFile(const TextFile&);
	public:
		TextFile(TEXT_ENCODING bufferEncoding = TE_UNKNOWN);
		~TextFile();

		/**
		@describe open a text file for read/write on local file system
		@param [in] pathname the file to open
		@return true if opened successfully
		*/
		bool	open(const TString& pathname, IStream::EAccessMode mode = IStream::AM_READOVERWRITE);


		/**
		@describe open a text file for read/write on specific file system
		@param 
		@return 
		*/
		bool	open(const HSTREAM& hstream);

		/**
		@describe figure out the encoding format ,and read text to input buffer
		@param
		@return
		*/
		bool	readfile(const IOBuffer& buffer);

		/**
		@describe 
		@param 
		@return 
		*/
		bool	isOpened() const;

		/**
		* @describe set the desired encoding format,when saving file
		* @param [in] encoding the desired file encoding format to save
		* @param [in] bBOM whether to write BOM on some format,i.e. UTF8
		* @return none
		*/
		void	setDestEncoding(TEXT_ENCODING encoding,bool bBOM = true);

		/**
		@describe
		@param
		@return
		*/
		bool	write(const tchar* buffer,size_t size);

		inline bool	write(const TString& text)
		{
			return write(text.c_str(),text.size());
		}

		/** @brief  */
		void	close();

		/** @brief  get the full content data */
		const TString getText() const;

		/** @brief write output buffer to file */
		bool	flushObuffer();

		/** @brief  */
		inline TEXT_ENCODING	getFileEncoding() const
		{
			return mFileEncoding;
		}

		/** @brief  */
		inline TEXT_ENCODING	getDestEncoding() const
		{
			return mDestEncoding;
		}

	protected:

		//note: when the text file utility is written, IOBuffer doesn't exist.
		//TODO: use IOBuffer directly and remove buffer operation in text file.
		TempBuffer			mInBuffer;
		TempBuffer			mOutBuffer;

	private:

		/** @brief  */
		bool writeBOM();

		/** @brief data position of the buffer,in tchars */
		size_t				mInPos;
		size_t				mOutPos;

		/** @brief written bytes */
		size_t				mBytesWritten;

		/** @brief text size in (t)characters */
		size_t				mTextSize;

		/** @brief this is the original encoding of a open file */
		TEXT_ENCODING		mFileEncoding;
		/** @brief this is the desired format to save */
		TEXT_ENCODING		mDestEncoding;
		/** @brief runtime encoding for text buffer, UTF16/UTF32 on unicode build, MBCS on non unicode build */
		TEXT_ENCODING		mBufferEncoding;
		/** @brief whether to write BOM for utf8 (utf16/32 are mandatory) */
		bool				mWriteBOM;
	protected:
		/** @brief a abstract stream for a file */
		HSTREAM				mFileStream;
	};//class TextFile
	
}//namespace Blade

#endif // __Blade_TextFile_h__