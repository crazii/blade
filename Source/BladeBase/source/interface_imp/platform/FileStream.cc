/********************************************************************
	created:	2010/03/24
	filename: 	FileStream.cc
	author:		Crazii
	
	purpose:	default file stream implementation
*********************************************************************/
#include <BladePCH.h>
#include "../../BladeWin32API.h"
#include "FileStream.h"

namespace Blade
{
#if BLADE_ARCHITECTURE == BLADE_ARCHITECTURE_32BIT
	static const size_t BUFFER_SIZE = 4*1024;
#else
	static const size_t BUFFER_SIZE = 8*1024;
#endif

	//////////////////////////////////////////////////////////////////////////
	FileStream::FileStream(const TString& name, int mode)
		:mFileNamePath(name)
		,mSize(0)
		,mOffset(0)
		,mAccessMode(mode)
		//note: File stream may be hold for reading/writing through out the lifetime of the program,
		//so don't use temp buffer
		,mBuffer( Memory::getResourcePool() )
		,mWriteDirty(false)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	FileStream::~FileStream()
	{
		this->freeBuffer();
	}

	//////////////////////////////////////////////////////////////////////////
	const TString& FileStream::getName()
	{
		return mFileNamePath;
	}

	//////////////////////////////////////////////////////////////////////////
	IStream::Off FileStream::seek(EStreamPos origin, Off off)
	{
		if( this->isValid() )
		{
			//note: on write only mode, buffer content is not synced from file, seek buffer will cause un-initialized buffer content written to file.
			//so we need write content on each seek to avoid gaps.
			//on read & write mode, buffer content is synced from file.
			if( (mAccessMode&AM_WRITE) && !(mAccessMode&AM_READ) )
				this->flush();

			Off offset = 0;

			if( origin == SP_BEGIN )
				offset = off;
			else if( origin == SP_CURRENT )
				offset = this->tell() + off;
			else
				offset = mSize + off;

			if( offset > mSize )
			{
				assert(false);
				BLADE_EXCEPT(EXC_FILE_READ,BTString("cannot seek()\"") + mFileNamePath + TEXT("\". overflow."));
			}

			if( offset >= mOffset && offset < mOffset + (Off)mBuffer.getSize() )
				//re-use buffer
				mBuffer.seek(SP_BEGIN, offset - mOffset);
			else//invalidate buffer
			{
				if( (mAccessMode&AM_WRITE) )
					this->flush();
				mBuffer.seek(SP_BEGIN, 0);
				mBuffer.resize(0, BUFFER_SIZE);
				mOffset = offset;
			}
			return mOffset + mBuffer.tell();
		}
		else
			BLADE_EXCEPT(EXC_FILE_READ,BTString("cannot seek()\"") + mFileNamePath + TEXT("\".not opened."));
	}

	//////////////////////////////////////////////////////////////////////////
	IStream::Off FileStream::tell() const
	{
		if( this->isValid() )
			return mOffset + mBuffer.tell();
		else
			BLADE_EXCEPT(EXC_FILE_READ,BTString("cannot tell()\"")+ mFileNamePath + TEXT("\".not opened."));
	}

	//////////////////////////////////////////////////////////////////////////
	IStream::Size FileStream::readData(void* destBuffer, Size size)
	{
		if( this->isValid() )
		{
			assert( mBuffer.getCapacity() == BUFFER_SIZE );
			assert( (Off)mBuffer.getSize() >= mBuffer.tell() );
			if( mOffset + size > mSize )
				size = mSize - mOffset;

			Size copied = 0;
			Size leftBytes = (Size)mBuffer.getSize() - mBuffer.tell();
			assert(leftBytes >= 0);
			if( leftBytes != 0 )
			{
				Size count = std::min<Size>(leftBytes, size);
				bool ret = mBuffer.readData(destBuffer, count);
				assert(ret);
				BLADE_UNREFERENCED(ret);

				copied = count;
				size -= count;
				leftBytes -= copied;
				if( leftBytes == 0 )
				{
					//advance to next buffer block
					mOffset += mBuffer.getSize();
					mBuffer.rewind();
					mBuffer.resize(0, BUFFER_SIZE);
				}
				else
					assert(size == 0);
			}

			//still more to read
			while( size >= BUFFER_SIZE )
			{
				assert( leftBytes == 0 );
				Size bytes = this->readDataImpl( (char*)destBuffer+copied, BUFFER_SIZE, mOffset);
				//some lib/codes will try to read more that file size
				//assert( bytes == BUFFER_SIZE );
				mOffset += bytes;

				copied += BUFFER_SIZE;
				size -= BUFFER_SIZE;

				//some lib/codes will try to read more that file size
				//if( bytes < BUFFER_SIZE )
				//	return copied;
			}

			if( size > 0 )//more needed (less than one buffer block)
			{
				assert( size < BUFFER_SIZE );
				assert( mBuffer.getSize() == 0 );
				Size toRead = std::min<Size>( (Size)mBuffer.getCapacity(), mSize - mOffset );
				Size bytes = this->readDataImpl(mBuffer.getData(), toRead, mOffset);
				mBuffer.resize((size_t)bytes, BUFFER_SIZE);
				Size toCopy = std::min<Size>( bytes, size );
				if( toCopy != 0 )
				{
					bool ret = mBuffer.readData((char*)destBuffer + copied, toCopy);
					assert(ret);
					BLADE_UNREFERENCED(ret);
					copied += toCopy;
				}
			}
			return copied;
		}
		else
			BLADE_EXCEPT(EXC_FILE_READ,BTString("cannot read()\"")+ mFileNamePath + TEXT("\".not opened."));
	}

	//////////////////////////////////////////////////////////////////////////
	IStream::Size FileStream::writeData(const void* srcBuffer, Size size)
	{
		if( this->isValid() )
		{
			if( (mAccessMode&AM_WRITE) )
			{
				assert( mBuffer.getCapacity() == BUFFER_SIZE );
				assert( (Off)mBuffer.getSize() >= mBuffer.tell() );
				assert( mSize >= mOffset );

				if( mBuffer.getSize() == 0 )
				{
					mBuffer.resize(BUFFER_SIZE, BUFFER_SIZE);
					if( (mAccessMode&AM_READ) )//read back data
					{
						Size left = mSize - mOffset;
						left = std::min<Size>(left, (Size)mBuffer.getSize() );
						if( left != 0)
						{
							Size bytes = this->readDataImpl(mBuffer.getData(), left, this->tell() );
							assert( bytes == left );
							BLADE_UNREFERENCED(bytes);
						}
					}
				}

				Size copied = 0;
				Size leftBytes = (Size)mBuffer.getSize() - mBuffer.tell();
				if( leftBytes != 0 )
				{
					Size count = std::min<Size>(leftBytes, size);
					bool ret = mBuffer.writeData(srcBuffer, count);
					assert(ret);
					mWriteDirty = ret;

					copied = count;
					size -= count;
					leftBytes -= copied;

					if( leftBytes == 0 )
					{
						Size bytes = this->writeDataImpl(mBuffer.getData(), (Size)mBuffer.getSize(), mOffset);
						assert(bytes == (Off)mBuffer.getSize() );
						//advance to next buffer block
						mOffset += bytes;
						mBuffer.rewind();
						mWriteDirty = false;
					}
					else
						assert(size == 0);
				}
				else
					assert(!mWriteDirty);

				//still more to write
				while( size >= BUFFER_SIZE )
				{
					assert( !mWriteDirty );
					assert( leftBytes == 0 );
					Size bytes = this->writeDataImpl( (const char*)srcBuffer + copied, BUFFER_SIZE, mOffset);
					assert( bytes == BUFFER_SIZE );
					mOffset += bytes;

					copied += BUFFER_SIZE;
					size -= BUFFER_SIZE;
				}

				if( size > 0 )//more needed (less than one buffer block)
				{
					assert( size < BUFFER_SIZE );
					assert( mBuffer.tell() == 0 );
					//assert( mBuffer.getSize() == BUFFER_SIZE );
					assert( !mWriteDirty );
					if( (mAccessMode&AM_READ) )//read back data
					{
						Size left = mSize <= mOffset ? 0 : mSize - mOffset;
						left = std::min<Size>(left, (Size)mBuffer.getSize() );
						if( left != 0 )
						{
							Size bytes = this->readDataImpl(mBuffer.getData(), left, mOffset);
							assert( bytes == left );
							BLADE_UNREFERENCED(bytes);
						}
					}
					bool ret = mBuffer.writeData((const char*)srcBuffer + copied, size);
					assert(ret);
					mWriteDirty = ret;
					copied += size;
				}

				Off offset = mOffset + mBuffer.tell();
				if( offset > mSize )
					mSize = offset;

				return copied;
			}
			else
				BLADE_EXCEPT(EXC_FILE_WRITE,BTString("cannot write()\"")+ mFileNamePath + TEXT("\".not writable."));
		}
		else
			BLADE_EXCEPT(EXC_FILE_WRITE,BTString("cannot write()\"")+ mFileNamePath + TEXT("\".not opened."));
	}

	//////////////////////////////////////////////////////////////////////////
	IStream::Size	FileStream::getSize() const
	{
		if( this->isValid() )
		{
			return mSize;
		}
		else
			BLADE_EXCEPT(EXC_FILE_READ,BTString("cannot getSize()\"")+ mFileNamePath + TEXT("\"not opened."));
	}

	//////////////////////////////////////////////////////////////////////////
	int				FileStream::getAccesMode() const
	{
		return mAccessMode;
	}

	//////////////////////////////////////////////////////////////////////////
	void	FileStream::flush(bool hardFlush/* = false*/)
	{
		//TODO: flush buffer
		if( (mAccessMode&AM_WRITE) )
		{
			assert( (Off)mBuffer.getSize() >= mBuffer.tell() );
			
			if( mWriteDirty )
			{
				if( !(mAccessMode&AM_READ) )
				{
					assert( mBuffer.tell() != 0 );
					//note: write buffer is always start at the beginning because we flush on each seek
					Size bytes = this->writeDataImpl(mBuffer.getData(), mBuffer.tell(), mOffset);
					assert(bytes == mBuffer.tell() );
					mBuffer.rewind();
					mBuffer.resize(0, BUFFER_SIZE);
					mOffset += bytes;
				}
				else
				{
					//avoid overflow
					if( mOffset + (IStream::Size)mBuffer.getSize() > mSize )
						mBuffer.resize( (size_t)mBuffer.tell(), BUFFER_SIZE);
					//write the whole buffer because seek may happen within the buffer,
					//in that case the modification is in-consecutive
					Size bytes = this->writeDataImpl(mBuffer.getData(), (Size)mBuffer.getSize(), mOffset);
					assert(bytes == (Off)mBuffer.getSize() );
					mBuffer.rewind();
					mBuffer.resize(0, BUFFER_SIZE);
					mOffset += bytes;
				}
				mWriteDirty = false;
			}
			else
			{
				if( !(mAccessMode&AM_READ) )
					assert(mBuffer.tell() == 0);
			}

			if( hardFlush )
				this->flushImpl();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void			FileStream::prepareBuffer()
	{
		mBuffer.reserve(BUFFER_SIZE);
	}

	//////////////////////////////////////////////////////////////////////////
	void			FileStream::freeBuffer()
	{
		mBuffer.clear();
	}

}//namespace Blade