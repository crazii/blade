/********************************************************************
	created:	2010/09/04
	filename: 	MemoryStream.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <utility/MemoryStream.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	MemoryStream::MemoryStream(const TString& name, IPool* pool/* = NULL*/)
		:mName(name)
		,mMode(0)
		,mBuffer(pool)
		,mOpen(false)
	{
		mLock.constructInstance();
	}

	//////////////////////////////////////////////////////////////////////////
	MemoryStream::MemoryStream(IOBuffer& buffer, int mode/* = AM_READ*/, const TString& name/* = TString::EMPTY*/)
		:mName(name)
		,mMode(mode)
		,mBuffer( buffer.getInternalData().mPool )
	{
		mBuffer.shareData( buffer.getInternalData() );
		mBuffer.seek(IStream::SP_BEGIN, buffer.tell());
		mOpen = true;
		if( mName == TString::EMPTY )
			mName = buffer.getName();
	}

	//////////////////////////////////////////////////////////////////////////
	MemoryStream::MemoryStream(const TString& name, IOBuffer& buffer, RWLOCK lock, int mode/* = AM_READ*/)
		:mName(name)
		,mMode(mode)
		,mLock(lock)
		,mOpen(true)
	{
		assert( mLock != NULL );
		mBuffer.shareData( buffer.getInternalData() );
		mBuffer.seek(IStream::SP_BEGIN, buffer.tell());
		mLock->lock( (mMode&AM_WRITE) == 0 );
	}

	//////////////////////////////////////////////////////////////////////////
	MemoryStream::~MemoryStream()
	{
		this->close();
	}

	//////////////////////////////////////////////////////////////////////////
	const TString& MemoryStream::getName()
	{
		return mName;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		MemoryStream::isValid() const
	{
		return mOpen;
	}

	//////////////////////////////////////////////////////////////////////////
	void		MemoryStream::close()
	{
		bool open = mOpen;
		//disable further operations first
		mOpen = false;

		//unlock
		if( open && mLock != NULL )
			mLock->unlock( (mMode&AM_WRITE) == 0 );
		mBuffer.unshareData();
	}

	//////////////////////////////////////////////////////////////////////////
	IStream::Off	MemoryStream::seek(EStreamPos origin,Off off)
	{
		if( mOpen )
			return mBuffer.seek(origin,off);
		else
		{
			assert(false);
			return 0;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	IStream::Off	MemoryStream::tell() const
	{
		if( mOpen )
			return mBuffer.tell();
		else
		{
			assert(false);
			return 0;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool		MemoryStream::eof() const
	{
		if( mOpen )
			return mBuffer.eof();
		else
		{
			assert(false);
			return false;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	IStream::Size		MemoryStream::readData(void* destBuffer,Size size)
	{
		if( mOpen )
		{
			if( !(mMode&AM_READ) )
			{
				assert(false);
				return 0;
			}

			if( mBuffer.readData(destBuffer, size) )
				return size;
			else
			{
				assert(false);
				return 0;
			}
		}
		else
		{
			assert(false);
			return 0;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	IStream::Size		MemoryStream::writeData(const void* srcBuffer,Size size)
	{
		if( mOpen )
		{
			if( !(mMode&AM_WRITE) )
			{
				assert(false);
				return false;
			}
			//note: write data may need buffer re-allocation
			//thus previous buffer pointer becomes invalid.
			//but shared buffer are guarded by shared ReadWriteLock: mLock
			//if no writes, there can be multiple instance reading the memory stream at the same time
			//but if write mode used, there's should be only one instance writing the memory stream at the same time, and no reading allowed.
			//so this gonna be OK.
			//memory stream close() is the same.
			mBuffer.writeData(srcBuffer,size);
			return size;
		}
		else
		{
			assert(false);
			return 0;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	IStream::Size	MemoryStream::getSize() const
	{
		if( mOpen )
			return (IStream::Size)mBuffer.getSize();
		else
		{
			assert(false);
			return 0;
		}

	}

	//////////////////////////////////////////////////////////////////////////
	int			MemoryStream::getAccesMode() const
	{
		if( mOpen )
			return mMode;
		else
			return 0;
	}

	//////////////////////////////////////////////////////////////////////////
	void		MemoryStream::flush(bool /*hardFlush = false*/)
	{
		assert( mOpen );
	}

	//////////////////////////////////////////////////////////////////////////
	bool		MemoryStream::truncate(Size size)
	{
		if( (mMode&AM_WRITE) && mOpen )
			return mBuffer.resize( (size_t)size);
		else
			return false;
	}

	//////////////////////////////////////////////////////////////////////////
	MemoryStream* MemoryStream::clone(const TString& name, int mode, bool share)
	{
		if( share )
		{
			MemoryStream* stream = BLADE_NEW MemoryStream(name, mBuffer, mLock, mode);
			stream->mName = name;
			return stream;
		}
		else
		{
			MemoryStream* stream = BLADE_NEW MemoryStream( mBuffer, mode, name );
			stream->mBuffer.unshareData();
			stream->mBuffer.resize( mBuffer.getSize(), mBuffer.getCapacity() );
			std::memcpy(stream->mBuffer.getData(), mBuffer.getData(), mBuffer.getSize() );
			return stream;
		}
	}
	
}//namespace Blade