/********************************************************************
	created:	2014/01/13
	filename: 	ZipStream.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_ZipStream_h__
#define __Blade_ZipStream_h__
#define BLADE_ENABLE_ZIPFILE 0

#if BLADE_ENABLE_ZIPFILE
#include <interface/public/file/IFileDevice.h>

#include <zzip/zzip.h>
#include <zzip/plugin.h>


namespace Blade
{
	class ZipStream : public IStream, public TempAllocatable
	{
	public:
		ZipStream(const TString& name, ZZIP_FILE* zipFile, IStream::EAccessMode mode, IStream::Size decompressSize);
		~ZipStream();

		/************************************************************************/
		/* IStream interface                                                                     */
		/************************************************************************/
		/** @brief get the stream name: path of the stream */
		virtual const TString& getName();

		/** @brief  */
		virtual bool		isValid() const;

		/** @brief  */
		virtual void		close();

		/** @brief  */
		virtual Off			seek(EStreamPos origin,Off off);

		/** @brief  */
		virtual Off			tell() const;

		/** @brief  */
		virtual bool		eof() const;

		/** @brief  */
		virtual Size		readData(void* destBuffer,Size size);

		/** @brief  */
		virtual Size		writeData(const void* srcBuffer,Size size);

		/** @brief  */
		virtual Size		getSize() const;

		/** @brief  */
		virtual int			getAccesMode() const;

		/** @brief  */
		virtual void		flush(bool hardFlush = false);

		/** @brief  */
		virtual bool		truncate(Size size);

	protected:
		TString		mName;
		ZZIP_FILE*	mZipFile;
		Size		mDecompressSize;
		EAccessMode	mMode;
	};//class ZipStream

	
}//namespace Blade
#endif//BLADE_ENABLE_ZIPFILE

#endif // __Blade_ZipStream_h__
