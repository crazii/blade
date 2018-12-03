/********************************************************************
	created:	2009/04/06
	filename: 	IStream.h
	author:		Crazii
	
	purpose:	stream abstraction.
				thread safe for reading only.
				although the reading is thread safe, you can only perform asynchronous reading via different instances of IStream,
				multi-thread access files via the same IStream pointer is not thread safe.
*********************************************************************/
#ifndef __Blade_IStream_h__
#define __Blade_IStream_h__
#include <utility/String.h>
#include <Handle.h>

namespace Blade
{
	class IStream
	{
	public:
		enum EStreamPos
		{
			SP_BEGIN = 0,
			SP_END,
			SP_CURRENT,
			SP_EOF   = 0xFFFFFFFF,
		};

		enum EAccessMode
		{
			AM_UNDEFINED = 0,	//when stream not open

			//separate read/write mode to restrict access, this may avoid careless mistakes
			AM_READ		= 0x0001,
			AM_WRITE	= 0x0002,
			AM_OVERWRITE= 0x0004 | AM_WRITE,
			AM_READWRITE = AM_READ | AM_WRITE,
			AM_READOVERWRITE = AM_READ | AM_OVERWRITE,
		};

		typedef int64	Off;
		typedef int64	Size;

		virtual ~IStream()		{}

		/** @brief get the stream name: path of the stream */
		virtual const TString& getName() = 0;

		/** @brief  */
		virtual bool		isValid() const = 0;

		/** @brief  */
		virtual void		close() = 0;

		/** @brief  */
		virtual Off			seek(EStreamPos origin, Off off) = 0;

		/** @brief  */
		virtual Off			tell() const = 0;

		/** @brief  */
		virtual bool		eof() const = 0;

		/** @brief  */
		virtual Size		readData(void* destBuffer,Size size) = 0;

		/** @brief  */
		virtual Size		writeData(const void* srcBuffer,Size size) = 0;

		/** @brief  */
		virtual Size		getSize() const = 0;

		/** @brief  */
		virtual int			getAccesMode() const = 0;

		/** @brief  */
		virtual void		flush(bool hardFlush = false) = 0;
		
		/** @brief  */
		virtual bool		truncate(Size size) = 0;

		/** @brief reset stream pointer */
		inline void rewind()
		{
			this->seek(SP_BEGIN,0);
		}

		///common type approaches: reading
		template <typename T>
		inline Size read(T* dest,size_t count = 1)
		{
			return this->readData(dest, Blade::IStream::Size( sizeof(T)*count) );
		}

		template <typename T>
		inline Size write(const T* src,size_t count = 1)
		{
			return this->writeData(src, Blade::IStream::Size( sizeof(T)*count) );
		}

		template <typename T>
		inline Size readAtom(T& dest)
		{
			return this->read(&dest, 1);
		}

		template<typename T>
		inline Size writeAtom(const T src)
		{
			return this->write(&src, 1);
		}

	protected:
		/** @brief  */
		inline Size	readUtf8(String& dest)
		{
			dest = String::EMPTY;
			static const size_t BUFF_SIZE = 64;

			Size sz = 0;
			char* buff = BLADE_TMP_ALLOCT(char, BUFF_SIZE + 1);

			do
			{
				std::memset(buff, 0, BUFF_SIZE + 1);
				Size s = this->readData(buff, BUFF_SIZE);
				sz += s;
				dest += buff;

				if (s < BUFF_SIZE)
					break;
			} while (dest.size() == (size_t)sz);

			this->seek(SP_CURRENT, (Size)dest.size() - sz);
			BLADE_TMP_FREE(buff);
			return sz;
		}
		/** @brief  */
		inline Size	writeUtf8(const String& src)
		{
			this->write(src.c_str(), src.size());
			this->writeAtom<char>('\0');
			return Size(src.size() + 1);
		}
	};//class IStream

	typedef Handle<IStream> HSTREAM;

	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////
	template<>
	inline IStream::Size	IStream::read<bool>(bool* dest, size_t count/* = 1*/)
	{
		uint8* data = BLADE_TMP_ALLOCT(uint8, count);
		Size ret = this->readData(data, (IStream::Size)(sizeof(uint8)*count));

		for (size_t i = 0; i < count; ++i)
			dest[i] = (data[i] != 0);
		BLADE_TMP_FREE(data);
		return ret;
	}

	template<>
	inline IStream::Size	IStream::write<bool>(const bool* src, size_t count/* = 1*/)
	{
		uint8* data = BLADE_TMP_ALLOCT(uint8, count);
		for (size_t i = 0; i < count; ++i)
			data[i] = (uint8)src[i];

		Size ret = this->writeData(data, (IStream::Size)(sizeof(uint8)*count));
		BLADE_TMP_FREE(data);
		return ret;
	}

	template<>
	inline IStream::Size	IStream::read<String>(String* dest, size_t count)
	{
		Size sz = 0;
		String utf8s;
		for (size_t i = 0; i < count; ++i)
		{
			sz += this->readUtf8(utf8s);
			dest[i] = StringConverter::UTF8StringToString(utf8s);
		}
		return sz;
	}

	template<>
	inline IStream::Size	IStream::write<String>(const String* src, size_t count)
	{
		Size s = 0;
		for (size_t i = 0; i < count; ++i)
		{
			String utf8s = StringConverter::StringToUTF8String(src->c_str(), src->size());
			s += this->writeUtf8(utf8s);
		}
		return s;
	}

	template<>
	inline IStream::Size	IStream::read<WString>(WString* dest, size_t count)
	{
		Size sz = 0;
		String utf8s;
		for (size_t i = 0; i < count; ++i)
		{
			sz += this->readUtf8(utf8s);
			dest[i] = StringConverter::UTF8StringToWString(utf8s);
		}
		return sz;
	}

	template<>
	inline IStream::Size	IStream::write<WString>(const WString* src, size_t count)
	{
		Size s = 0;
		for (size_t i = 0; i < count; ++i)
		{
			String utf8s = StringConverter::WStringToUTF8String(src->c_str(), src->size());
			s += this->writeUtf8(utf8s);
		}
		return s;
	}
	
}//namespace Blade

#endif // __Blade_IStream_h__