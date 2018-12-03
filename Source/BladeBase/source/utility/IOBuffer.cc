/********************************************************************
	created:	2010/05/23
	filename: 	IOBuffer.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	IOBuffer::IOBuffer(IPool* pool/* = NULL*/)
		:Buffer(pool)
		,mPosition(0)
		,mLastPosition(0)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	IOBuffer::IOBuffer(IStream* stream)
		:mPosition(0)
		,mLastPosition(0)
	{
		this->loadFromStream(stream);
	}

	//////////////////////////////////////////////////////////////////////////
	IOBuffer::IOBuffer(const HSTREAM& stream)
		:mPosition(0)
		,mLastPosition(0)
	{
		this->loadFromStream(stream);
	}

	//////////////////////////////////////////////////////////////////////////
	bool					IOBuffer::saveToStream(IStream* stream)
	{
		if (stream != NULL)
			return stream->writeData(this->getData(), (IStream::Size)mPosition) > 0;
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool					IOBuffer::loadFromStream(IStream* stream)
	{
		if( stream != NULL )
		{
			mName = stream->getName();

			IStream::Size size = stream->getSize();
			IStream::Off off = stream->tell();

			this->reserve((size_t)size);
			std::memset(this->getData(), 0, this->getCapacity());
			this->setSize((size_t)size);

			//reset position
			stream->rewind();

			bool ret;
			if( size == 0 || stream->readData(this->getData(), (IStream::Size)this->getCapacity()) )
				ret = true;
			else
				ret = false;

			//restore
			stream->seek(IStream::SP_BEGIN, off);

			if(ret)
				mPosition = mLastPosition = off;
			else
				mPosition = mLastPosition = 0;

			return ret;
		}
		else 
			return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				IOBuffer::writeData(const void* data, IStream::Size size)
	{
		assert( (size_t)mPosition <= this->getCapacity() );
		mLastPosition = mPosition;
		mPosition += size;

		if( this->getCapacity() == 0 )
		{
			assert( mLastPosition == 0 );
			this->reserve( std::max<size_t>(32*1024, (size_t)size) );
		}
		else if( (size_t)mPosition > this->getCapacity() )
		{
			DATA original_data = this->detachData();
			this->reserve( ((size_t)mPosition)+((size_t)mPosition)/2 );

			if (this->getData() == NULL)
				return false;

			std::memcpy(this->getData(), original_data.mData, original_data.mSize);
			Buffer::freeData(original_data);
		}

		char* dest = (char*)this->getData() + mLastPosition;
		std::memcpy(dest, data, (size_t)size);

		if( this->eof() )
			this->setSize((size_t)mPosition);

		return true;
	}


	//////////////////////////////////////////////////////////////////////////
	bool					IOBuffer::readDataPtr(void const*& pointer,IStream::Size size)
	{
		//overflow check , check three value
		if( (size_t)mPosition > this->getSize() 
			|| (size_t)size > this->getSize()
			|| (size_t)(mPosition+size) > this->getSize()  )
		{
			assert(false);
			return false;
		}

		pointer = (char*)this->getData() + mPosition;
		mLastPosition = mPosition;
		mPosition += size;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool					IOBuffer::readData(void* pointer,IStream::Size size)
	{
		//overflow check , check three value
		if( (size_t)mPosition > this->getSize() 
			|| (size_t)size > this->getSize()
			|| (size_t)(mPosition+size) > this->getSize()  )
		{
			assert(false);
			return false;
		}

		std::memcpy(pointer, (const char*)this->getData()+mPosition, (size_t)size);
		mLastPosition = mPosition;
		mPosition += size;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool					IOBuffer::writeWString(const WString& wstr)
	{
		size_t utf8count = wstr.size()*3+3;
		char* UTF8String = (char*)BLADE_TMP_ALLOC(utf8count);
		utf8count = CodeConverter::StringtoUTF8<wchar>(wstr.c_str(), wstr.size(), UTF8String, utf8count);
		if(utf8count == INVALID_INDEX)
		{
			BLADE_TMP_FREE(UTF8String);
			assert(false);
			return false;
		}
		//'\0' not written
		this->writeData( UTF8String, (IStream::Size)utf8count);
		BLADE_TMP_FREE(UTF8String);

		//append ending
		char send = char();
		this->write(&send);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool					IOBuffer::readWString(WString& outWStr)
	{
		String utf8String( (char*)this->getCurrentData() );
		bool ret = mPosition + (IStream::Off)utf8String.size() + 1 <= (IStream::Off)this->getSize();
		this->seekForward( (IStream::Off)(utf8String.size()+1) );

		size_t wcharCount = utf8String.size()+1;
		wchar* buffer = (wchar*)BLADE_TMP_ALLOC(wcharCount*sizeof(wchar) );
		wcharCount = CodeConverter::UTF8toString<wchar>(utf8String.c_str(), utf8String.size(), buffer, wcharCount);

		if( wcharCount == INVALID_INDEX )
		{
			BLADE_TMP_FREE(buffer);
			assert(false);
			return false;
		}
		buffer[wcharCount] = wchar();
		outWStr = buffer;
		BLADE_TMP_FREE(buffer);
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	bool					IOBuffer::writeString(const String& str)
	{
		size_t utf8count = str.size()*3+3;
		char* UTF8String = (char*)BLADE_TMP_ALLOC(utf8count);
		utf8count = CodeConverter::StringtoUTF8<char>(str.c_str(),str.size(), UTF8String, utf8count);
		if(utf8count == INVALID_INDEX)
		{
			BLADE_TMP_FREE(UTF8String);
			assert(false);
			return false;
		}
		this->writeData( UTF8String, (IStream::Size)utf8count);
		BLADE_TMP_FREE(UTF8String);

		//avoid shared string that has no '\0' ending
		char send = char();
		this->write(&send);
		return true;

	}

	//////////////////////////////////////////////////////////////////////////
	bool					IOBuffer::readString(String& outStr)
	{
		String utf8String( (char*)this->getCurrentData() );
		bool ret = mPosition + (IStream::Off)utf8String.size() + 1 <= (IStream::Off)this->getSize();
		this->seekForward( (IStream::Off)(utf8String.size()+1) );

		size_t charCount = utf8String.size()+sizeof(char);
		char* buffer = (char*)BLADE_TMP_ALLOC(charCount );
		charCount = CodeConverter::UTF8toString<char>(utf8String.c_str(), utf8String.size(), buffer, charCount);

		if( charCount == INVALID_INDEX )
		{
			BLADE_TMP_FREE(buffer);
			assert(false);
			return false;
		}
		buffer[charCount] = char();
		outStr = buffer;
		BLADE_TMP_FREE(buffer);
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	bool					IOBuffer::writeWStringRAW(const WString& wstr)
	{
		//avoid shared string that has no '\0' ending
		this->writeData( wstr.c_str(), (IStream::Size)((wstr.size() )*sizeof(wchar)) );
		tchar send = wchar();
		this->write(&send);

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool					IOBuffer::readWStringRAW(WString& outWStr)
	{
		outWStr = WString( (wchar*)this->getCurrentData() );
		bool ret = mPosition + (IStream::Off)outWStr.size()*(IStream::Off)sizeof(wchar) + 1 <= (IStream::Off)this->getSize();
		this->seekForward( (IStream::Off)((outWStr.size()+1)*sizeof(wchar)) );
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	bool					IOBuffer::writeStringRAW(const String& str)
	{
		//avoid shared string that has no '\0' ending
		this->writeData( str.c_str(), (IStream::Size)((str.size() )*sizeof(char)) );
		char send = char();
		this->write(&send);

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool					IOBuffer::readStringRAW(String& outStr)
	{
		outStr = String( (char*)this->getCurrentData() );
		bool ret = mPosition + (IStream::Off)outStr.size() + 1 <= (IStream::Off)this->getSize();
		this->seekForward( (IStream::Off)((outStr.size()+1)*sizeof(char)) );
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	const void*				IOBuffer::getCurrentData() const
	{
		return (const char*)this->getData() + mPosition;
	}

	//////////////////////////////////////////////////////////////////////////
	void*					IOBuffer::getCurrentData()
	{
		return (char*)this->getData() + mPosition;
	}

	//////////////////////////////////////////////////////////////////////////
	bool					IOBuffer::eof() const
	{
		return mPosition >= (IStream::Off)this->getSize();
	}

	//////////////////////////////////////////////////////////////////////////
	void					IOBuffer::rewind()
	{
		mLastPosition = mPosition = 0;
	}

	//////////////////////////////////////////////////////////////////////////
	IStream::Off		IOBuffer::tell() const
	{
		return mPosition;
	}

	//////////////////////////////////////////////////////////////////////////
	IStream::Off		IOBuffer::seek(IStream::EStreamPos origin,IStream::Off off)
	{
		mLastPosition = mPosition;

		if( origin == IStream::SP_BEGIN )
			mPosition = off;
		else if( origin == IStream::SP_CURRENT )
			mPosition += off;
		else if( origin == IStream::SP_END )
			mPosition = (IStream::Off)this->getSize() + off;

		return mPosition;
	}

	//////////////////////////////////////////////////////////////////////////
	void					IOBuffer::seekBack()
	{
		mPosition = mLastPosition;
	}

	//////////////////////////////////////////////////////////////////////////
	void					IOBuffer::seekFromLastPos(IStream::Off size)
	{
		IStream::Off off = mLastPosition;
		mLastPosition = mPosition;
		mPosition = off + size;
	}
	
}//namespace Blade