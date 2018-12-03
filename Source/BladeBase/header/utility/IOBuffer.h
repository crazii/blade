/********************************************************************
	created:	2010/05/23
	filename: 	IOBuffer.h
	author:		Crazii
	purpose:	Buffer with read/write features
	note:		IOBuffer will use Buffer's size info, so you should use getSize() to get the IOBuffer's real size
				the getCapacity() will get the buffer's total size, which is usually larger thant getSize()
*********************************************************************/
#ifndef __Blade_IOBuffer_h__
#define __Blade_IOBuffer_h__
#include "Buffer.h"
#include <interface/public/file/IStream.h>

namespace Blade
{

	//////////////////////////////////////////////////////////////////////////
	class BLADE_BASE_API IOBuffer : public Buffer
	{
	public:
		IOBuffer(IPool* pool = NULL);
		IOBuffer(IStream* stream);
		IOBuffer(const HSTREAM& stream);

		/** @brief get the buffer name
		usually a stream file path if it is loaded from a stream */
		inline const TString&	getName() const	{ return mName; }

		/** @brief  */
		bool					saveToStream(IStream* stream);

		/** @brief load the whole stream into buffer, the buffer pointer is pointed where as in the stream  */
		bool					loadFromStream(IStream* stream);

		/************************************************************************/
		/* stream style methods                                                                     */
		/************************************************************************/
		/** @brief  */
		bool					writeData(const void* data, IStream::Size size);

		template<typename T>
		inline bool				write(const T* src, size_t count = 1)
		{
			return this->writeData(src, (IStream::Size)(sizeof(T)*count) );
		}

		inline bool			write(const bool* src, size_t count = 1)
		{
			uint8* data = BLADE_TMP_ALLOCT(uint8, count);
			for(size_t i = 0; i < count; ++i)
				data[i] = (uint8)src[i];
			bool ret = this->writeData(data, (IStream::Size)(sizeof(uint8)*count));
			BLADE_TMP_FREE(data);
			return ret;
		}

		template<typename T>
		inline bool writeAtom(const T src)
		{
			return this->write(&src, 1);
		}
		inline bool writeAtom(const bool src)
		{
			return this->write(&src, 1);
		}

		/** @brief  */
		bool					readDataPtr(void const*& pointer,IStream::Size size);

		template<typename T>
		inline bool				readPtr(T const*& pointer,size_t count = 1)
		{
			void const*& p = (void const*&)(pointer);
			return this->readDataPtr( p, (IStream::Size)(sizeof(T)*count) );
		}

		inline bool				readPtr(bool const*& /*pointer*/, size_t /*count = 1*/)
		{
			assert(false);
			return false;
		}

		/** @brief  */
		bool					readData(void* pointer,IStream::Size size);

		template<typename T>
		inline bool				read(T* pointer,size_t count = 1)
		{
			return this->readData( pointer, (IStream::Size)(sizeof(T)*count) );
		}

		inline bool				read(bool* pointer, size_t count)
		{
			uint8* data = BLADE_TMP_ALLOCT(uint8, count);
			bool ret = this->readData(data, (IStream::Size)(sizeof(uint8)*count));

			for (size_t i = 0; i < count; ++i)
				pointer[i] = (data[i] != 0);
			BLADE_TMP_FREE(data);
			return ret;
		}

		template <typename T>
		inline bool readAtom(T& dest)
		{
			return this->read(&dest, 1);
		}
		inline bool readAtom(bool& dest)
		{
			return this->read(&dest, 1);
		}

		//////////////////////////////////////////////////////////////////////////
		///String serialization with internal encode (to cross platform UTF8)
		///this is mainly for saving/loading
		bool					writeWString(const WString& wstr);
		bool					readWString(WString& outWStr);
		bool					writeString(const String& str);
		bool					readString(String& outStr);
#if BLADE_UNICODE
		inline bool				writeTString(const TString& tstr)
		{
			return this->writeWString(tstr);
		}
		inline bool				readTString(TString& tstr)
		{
			return this->readWString(tstr);
		}
#else
		inline bool				writeTString(const TString& tstr)
		{
			return this->writeString(tstr);
		}
		inline bool				readTString(TString& tstr)
		{
			return this->readString(tstr);
		}
#endif

		//////////////////////////////////////////////////////////////////////////
		///String serialization without encoding
		///this is mainly used as helper for runtime string formating,etc.
		bool					writeWStringRAW(const WString& wstr);
		bool					readWStringRAW(WString& outWStr);
		bool					writeStringRAW(const String& str);
		bool					readStringRAW(String& outStr);
#if BLADE_UNICODE
		inline bool				writeTStringRAW(const TString& tstr)
		{
			return this->writeWStringRAW(tstr);
		}
		inline bool				readTStringRAW(TString& tstr)
		{
			return this->readWStringRAW(tstr);
		}
#else
		inline bool				writeTStringRAW(const TString& tstr)
		{
			return this->writeStringRAW(tstr);
		}
		inline bool				readTStringRAW(TString& tstr)
		{
			return this->readStringRAW(tstr);
		}
#endif


		/** @brief  get the data pointed by current stream pointer */
		const void*				getCurrentData() const;

		/** @brief  */
		void*					getCurrentData();


		/** @brief  */
		bool					eof() const;

		/** @brief  reset stream pointer */
		void					rewind();

		/** @brief  get current stream pointer */
		IStream::Off			tell() const;

		/** @brief  set current stream pointer */
		IStream::Off			seek(IStream::EStreamPos origin,IStream::Off offset);

		/** @brief  back to last pointer position */
		void					seekBack();

		/** @brief  */
		void					seekFromLastPos(IStream::Off offset);

		/** @brief  */
		inline void				seekForward(IStream::Off offset)
		{
			this->seek(IStream::SP_CURRENT,offset);
		}

		/** @brief  */
		inline bool				resize(size_t size, size_t capacity = 0)
		{
			capacity = capacity < size ? size : capacity;
			bool ret = Buffer::reserve(capacity, true);
			if( ret )
				this->setSize(size);
			return ret;
		}

	protected:
		/** @brief IOBuffer will use Buffer's size info as buffer's internal real size, so it is not accessible,
		use resize if needed. */
		using Buffer::setSize;

		IStream::Off		mPosition;
		IStream::Off		mLastPosition;
		TString				mName;
	};

	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////

	template<>
	inline bool	IOBuffer::read<String>(String* dest, size_t count)
	{
		bool ret = true;
		for (size_t i = 0; i < count; ++i)
			ret = this->readString(*dest) && ret;
		return ret;
	}
	template<>
	inline bool	IOBuffer::write<String>(const String* src, size_t count)
	{
		bool ret = true;
		for (size_t i = 0; i < count; ++i)
			ret = this->writeString(src[i]) && ret;
		return ret;
	}

	template<>
	inline bool	IOBuffer::read<WString>(WString* dest, size_t count)
	{
		bool ret = true;
		for (size_t i = 0; i < count; ++i)
			ret = this->readWString(*dest) && ret;
		return ret;
	}
	template<>
	inline bool	IOBuffer::write<WString>(const WString* src, size_t count)
	{
		bool ret = true;
		for (size_t i = 0; i < count; ++i)
			ret = this->writeWString(src[i]) && ret;
		return ret;
	}

	///IOBuffer that reference other data, which doesn't belong to the IOBuffer object itself
	///it will auto-detach the data
	///this is useful when doing exception-safe attach without detaching the data manually and re-throwing (like RAII idiom)
	class RefIOBuffer : public IOBuffer
	{
	public:
		inline explicit RefIOBuffer(const DATA& data)
		{
			this->attachData(data);
		}

		inline explicit RefIOBuffer(IOBuffer& buffer)
		{
			this->attachData( buffer.getInternalData() );
		}

		inline explicit RefIOBuffer(IOBuffer& buffer, void* data, size_t size)
		{
			DATA internalData = buffer.getInternalData();
			internalData.mData = data;
			internalData.mCapacity = size;
			internalData.mSize = size;
			this->attachData(internalData);
		}

		inline ~RefIOBuffer()
		{
			this->detachData();
		}
	};
	
}//namespace Blade


#endif //__Blade_IOBuffer_h__