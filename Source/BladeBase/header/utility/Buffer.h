/********************************************************************
	created:	2009/04/07
	filename: 	Buffer.h
	author:		Crazii
	
	purpose:	temporary buffers
*********************************************************************/
#ifndef __Blade_Buffer_h__
#define __Blade_Buffer_h__
#include <BladeBase.h>

namespace Blade
{
	class IPool;

	class BLADE_BASE_API Buffer : public NonCopyable, public NonAssignable, public NonAllocatable
	{
	protected:
		~Buffer();
	public:
		typedef struct SBufferData
		{
			IPool* mPool;
			void*  mData;		
			size_t mCapacity;	///total capacity
			size_t mSize;		///really used size. default: 0 (not used), set by user if needed
		}DATA;
	public:
		Buffer(IPool* pool = NULL);
		Buffer(size_t size, IPool* pool = NULL);

		/** @brief allocate Buffer in bytes */
		bool		reserve(size_t buffersize, bool copyOld = false);

		/** @brief clear Buffer */
		void		clear();

		/** @brief detach internal Buffer data */
		DATA		detachData();

		/** @brief  */
		bool		attachData(const DATA& data);

		/** @brief get internal data object */
		inline DATA&		getInternalData()		{return mInternalData;}

		/** @brief get allocated size in bytes, maybe we also need to get the used size. */
		inline size_t		getCapacity() const		{return mData->mCapacity;}

		/** @brief get the internal Buffer data */
		inline void*		getData()				{return mData->mData;}
		inline const void*	getData() const			{return mData->mData;}

		/** @brief get used size, default: 0, set by user if needed */
		inline size_t		getSize() const			{return mData->mSize;}

		/** @brief set used size */
		bool				setSize(size_t size);

		/** @brief  */
		static bool freeData(const DATA& data);

		/* @note: data share is not thread safe  */
		/** @brief  */
		inline void			shareData(DATA& data)	{ this->unshareData(); this->clear(); mData = &data;}
		/** @brief  */
		inline void			unshareData()			{ mData = &mInternalData; }

	private:
		DATA	mInternalData;
		DATA*	mData;
	};//class Buffer

	class TempBuffer : public Buffer	{};

}//namespace Blade

#endif // __Blade_Buffer_h__