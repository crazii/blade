/********************************************************************
	created:	2010/09/04
	filename: 	MemoryStream.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_MemoryStream_h__
#define __Blade_MemoryStream_h__
#include "IOBuffer.h"
#include <threading/ReadWriteLock.h>

namespace Blade
{
	class BLADE_BASE_API MemoryStream : public IStream , public Allocatable
	{
	private:
		MemoryStream(const MemoryStream&);
	public:
		MemoryStream(const TString& name, IPool* pool = NULL);
		/** @brief construct a opened stream: attach read only data from outside, the data is not released by memory stream */
		MemoryStream(IOBuffer& buffer, int mode = AM_READ, const TString& name = TString::EMPTY);
		MemoryStream(const TString& name, IOBuffer& buffer, RWLOCK lock, int mode = AM_READ);
		~MemoryStream();

		/** @brief  */
		virtual const TString& getName();

		/** @brief  */
		virtual bool		isValid() const;

		/** @brief  */
		virtual void		close();

		/** @brief  */
		virtual Off	seek(EStreamPos origin,Off off);

		/** @brief  */
		virtual Off	tell() const;

		/** @brief  */
		virtual bool	eof() const;

		/** @brief  */
		virtual Size	readData(void* destBuffer,Size size);

		/** @brief  */
		virtual Size	writeData(const void* srcBuffer,Size size);

		/** @brief  */
		virtual Size	getSize() const;

		/** @brief  */
		virtual int		getAccesMode() const;

		/** @brief  */
		virtual void	flush(bool hardFlush = false);

		/** @brief  */
		virtual bool	truncate(Size size);

		/************************************************************************/
		/*                                                                      */
		/************************************************************************/
		/** @brief  */
		MemoryStream* clone(const TString& name, int mode, bool share);

	protected:
		TString		mName;
		int			mMode;
		RWLOCK		mLock;
		IOBuffer	mBuffer;
		bool		mOpen;
	};//class MemoryStream 
	
}//namespace Blade


#endif //__Blade_MemoryStream_h__