/********************************************************************
	created:	2009/04/08
	filename: 	TextFile.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "BladeUnicode.h"

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(disable:4127)		//while(true)
#pragma warning( disable:4996)		//deprecated
#endif

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	TextFile::TextFile(TEXT_ENCODING bufferEncoding/* = TE_UNKNOWN*/)
		:mInPos(0)
		,mOutPos(0)
		,mBytesWritten(0)
		,mTextSize(0)
		,mFileEncoding(TE_UNKNOWN)
		,mDestEncoding(TE_DEFAULT)
		,mBufferEncoding(bufferEncoding)
		,mWriteBOM(true)
	{
#if BLADE_UNICODE
		assert( bufferEncoding == TE_UNKNOWN && "change format on unicode build is not supported" );
		if(sizeof(wchar) == 2 )
			mBufferEncoding = TE_UTF16;
		else if( sizeof(wchar) == 4 )
			mBufferEncoding = TE_UTF32;
		else
			assert(false && "unsupported");
#else
		if( bufferEncoding == TE_UNKNOWN )
			mBufferEncoding = TE_UTF8;
		else if( bufferEncoding != TE_MBCS && bufferEncoding != TE_UTF8 )
		{
			assert( false && "only MBCS & UTF8 are supported" );
			mBufferEncoding = TE_UTF8;
		}
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	TextFile::~TextFile()
	{
		this->flushObuffer();
		this->close();
	}

	//////////////////////////////////////////////////////////////////////////
	bool TextFile::open(const TString& pathname, IStream::EAccessMode mode/* = IStream::AM_READOVERWRITE*/)
	{
		this->close();

		bool ret = false;

		TString dir, file;
		TStringHelper::getFilePathInfo(pathname, dir, file);
		IFileDevice* folder = BLADE_FACTORY_CREATE(IFileDevice, IFileDevice::DEFAULT_FILE_TYPE);

		if( folder->load(dir) && (mFileStream = folder->openFile(file, mode)) != NULL )
		{
			if( (mode & IStream::AM_READ) )
			{
				IOBuffer buffer;
				if( !buffer.loadFromStream(mFileStream) )
					ret = false;
				mFileStream->rewind();
				ret = this->readfile(buffer);
			}
			else
				ret = true;
		}
		BLADE_DELETE folder;
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	TextFile::open(const HSTREAM& stream)
	{
		mFileStream = stream;
		if( stream == NULL || !stream->isValid() )
			return false;

		if( (stream->getAccesMode() & IStream::AM_READ) )
		{
			IOBuffer buffer;
			if( !buffer.loadFromStream(mFileStream) )
				return false;
			return this->readfile(buffer);
		}
		else
			return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	TextFile::isOpened() const
	{
		return mFileStream != NULL && mFileStream->isValid();
	}

	//////////////////////////////////////////////////////////////////////////
	void TextFile::setDestEncoding(TEXT_ENCODING encoding, bool bBOM)
	{
		if( encoding == TE_UTF8 )
			mWriteBOM = bBOM;
		else if( encoding == TE_MBCS )
			mWriteBOM = false;
		else
			mWriteBOM = true;

		mDestEncoding = encoding;
	}

	//////////////////////////////////////////////////////////////////////////
	bool TextFile::readfile(const IOBuffer& buffer)
	{
		TempBuffer tempbuffer;

		/** @brief buffersize in bytes */
		size_t buffersize = buffer.getSize();

		if( buffersize == 0 )
			return true;

		//read file into buffer
		tempbuffer.reserve(buffersize + sizeof(UTF32) );
		char* bufferdata = static_cast<char*> ( tempbuffer.getData() );
		std::memcpy(bufferdata,buffer.getData(),buffersize);

		//check the source file encoding
		if( ::memcmp(bufferdata,UTF8_BOM,3) == 0)
		{
			mFileEncoding = TE_UTF8;
			bufferdata += 3;
			buffersize -=3;
		}
		else if( ::memcmp(bufferdata,UTF32LE_BOM,4) == 0 )
		{
			//skip bom
			bufferdata +=4;
			buffersize -=4;
			mFileEncoding = TE_UTF32LE;
		}
		else if( ::memcmp(bufferdata,UTF32BE_BOM,4) == 0 )
		{
			bufferdata +=4;
			buffersize -=4;
			mFileEncoding = TE_UTF32BE;
		}
		else if( ::memcmp(bufferdata,UTF16LE_BOM,2) == 0 )
		{
			bufferdata +=2;
			buffersize -=2;
			mFileEncoding = TE_UTF16LE;
		}
		else if( ::memcmp(bufferdata,UTF16BE_BOM,2) == 0 )
		{
			bufferdata +=2;
			buffersize -=2;
			mFileEncoding = TE_UTF16BE;
		}

		//here we should check if it is a UTF-8 file without BOM
		if( mFileEncoding == TE_UNKNOWN )
		{
			if( ::isLegalUTF8Sequence((const UTF8*)bufferdata, (const UTF8*)bufferdata+buffersize) )
				mFileEncoding = TE_UTF8;
			else
				mFileEncoding = TE_MBCS;
		}

#if BLADE_UNICODE
		if( !(mBufferEncoding == TE_UTF16 && sizeof(wchar) == 2 || mBufferEncoding == TE_UTF32 && sizeof(wchar) == 4) )
		{
			assert(false);
			return false;
		}
#else
		if( !(mBufferEncoding == TE_UTF8 || mBufferEncoding == TE_MBCS ) )
		{
			assert(false);
			return false;
		}
#endif

		if( mBufferEncoding != mFileEncoding )
		{
			mInBuffer.reserve( (buffersize+1)*3*sizeof(tchar) );
			tchar* dest = (tchar*)mInBuffer.getData();
			size_t bytes = CodeConverter::CodeConvert(mFileEncoding, mBufferEncoding, bufferdata, buffersize, (char*)dest, mInBuffer.getCapacity() );
			assert( bytes + sizeof(tchar) < mInBuffer.getCapacity() );
			mTextSize = bytes/sizeof(tchar);
			dest[mTextSize] = 0;
			return true;
		}
		else
		{
			mTextSize = buffersize;

			mInBuffer.reserve( buffersize+sizeof(tchar) );
			std::memcpy( mInBuffer.getData(), bufferdata, buffersize );
			tchar* dest = (tchar*)mInBuffer.getData();
			dest[mTextSize] = 0;
			return true;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void TextFile::close()
	{
		if( mFileStream != NULL )
		{
			this->flushObuffer();
			mFileStream->close();
		}

		mInBuffer.clear();
		mOutBuffer.clear();
		mInPos = 0;
		mOutPos = 0;
		mTextSize = 0;
	}

	//////////////////////////////////////////////////////////////////////////
	bool TextFile::write(const tchar* buffer, size_t size)
	{
		const size_t bytes = size*sizeof(tchar);

		if( mOutBuffer.getCapacity() == 0 )
			mOutBuffer.reserve(32*1024);	//32K output buffer

		//buffer left is not enough?
		if( mOutBuffer.getCapacity()/sizeof(tchar) - mOutPos < size )
		{
			if( !this->flushObuffer() )
			{
				assert(false);
				return false;
			}

			mOutBuffer.reserve(32 * 1024);
			//32K is not enough?
			if( mOutBuffer.getCapacity() < bytes)
				mOutBuffer.reserve(bytes);
		}

		tchar* data = (tchar*)mOutBuffer.getData() + mOutPos;
		std::memcpy(data, buffer, bytes );
		mOutPos += (long)size;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool TextFile::writeBOM()
	{
		//make sure there is no invalid call
		assert( mBytesWritten == 0);

		if( mDestEncoding == TE_UTF8 )
		{
			if( !mWriteBOM )
				return true;

			if( mFileStream->writeData(UTF8_BOM,3) )
				return true;
			else
			{
				assert(false);
				return false;
			}
		}
		else if( mDestEncoding == TE_MBCS )
			return true;
		else 
		{
			const char* bom;
			size_t size;
			switch( mDestEncoding )
			{
			default:
				assert(false);
			case TE_UTF16LE:
				bom = UTF16LE_BOM;
				size = 2;
				break;
			case TE_UTF16BE:
				bom = UTF16BE_BOM;
				size = 2;
				break;
			case TE_UTF32LE:
				bom = UTF32LE_BOM;
				size = 4;
				break;
			case TE_UTF32BE:
				bom = UTF32BE_BOM;
				size = 4;
				break;
			}

			if( mFileStream->writeData(bom,IStream::Size(size)) )
				return true;
			else
			{
				assert(false);
				return false;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool TextFile::flushObuffer()
	{
		if( mFileStream == NULL )
			return false;
		//nothing to write
		if( mOutPos == 0 )
			return true;

		//first write, encoding BOM
		if( mBytesWritten == 0 )
		{
			if( !this->writeBOM() )
			{
				mFileStream->rewind();
				BLADE_EXCEPT(EXC_FILE_WRITE,BTString("write BOM failed."));
				//return false;
			}
		}

		//bytes to write
		size_t bytes;
		if( mBufferEncoding == mDestEncoding )
		{
			 bytes = mOutPos*sizeof(tchar);
			if( !mFileStream->writeData(mOutBuffer.getData(), (IStream::Size)bytes) )
			{
				mFileStream->seek(IStream::SP_BEGIN,(IStream::Off)mBytesWritten);
				BLADE_EXCEPT(EXC_FILE_WRITE,BTString("write data failed."));
			}
		}
		else
		{
			//convert current encoding to dest encoding
			TempBuffer tempbuffer;
			//just make enough room
			bytes = mOutPos*4;
			tempbuffer.reserve(bytes+4);
			char* dest = (char*)tempbuffer.getData();
			bytes = CodeConverter::CodeConvert(mBufferEncoding, mDestEncoding, (const char*)mOutBuffer.getData(), mOutPos*sizeof(tchar), dest, bytes);
			if( bytes == size_t(-1) )
			{
				assert(false);
				return false;
			}
			assert( bytes+4 <= tempbuffer.getCapacity() );
			std::memset(dest+bytes, 0, 4);
			if( !mFileStream->writeData(tempbuffer.getData(),(IStream::Size)bytes ) )
			{
				mFileStream->seek(IStream::SP_BEGIN, (IStream::Off)mBytesWritten);
				BLADE_EXCEPT(EXC_FILE_WRITE,BTString("write data failed:")+mFileStream->getName() );
			}
		}

		mBytesWritten += bytes;
		mFileStream->flush();
		mOutPos = 0;
		mOutBuffer.clear();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	const TString TextFile::getText() const
	{
		if( mTextSize == 0)
			return TString::EMPTY;
		return TString( (tchar*)mInBuffer.getData() + mInPos, mTextSize, TString::CONST_HINT);
	}	
	
}//namespace Blade