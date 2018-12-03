/********************************************************************
	created:	2014/02/16
	filename: 	BPKStream.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_BPKStream_h__
#define __Blade_BPKStream_h__
#include <interface/public/file/IFileDevice.h>
#include <utility/BPKFile.h>

namespace Blade
{
	class BPKStream : public IStream, public Allocatable
	{
	public:
		BPKStream(const TString& name, IStream::EAccessMode mode, const BPKFile* package, BPKFile::FILE* fp);
		~BPKStream();

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
		virtual Off			seek(EStreamPos origin, Off off);

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
		TString			mName;
		const BPKFile*	mPackage;
		BPKFile::FILE*	mFile;
		int				mAccessMode;
	};//class BPKStream
	
}//namespace Blade

#endif //  __Blade_BPKStream_h__