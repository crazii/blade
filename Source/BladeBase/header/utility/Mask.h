/********************************************************************
	created:	2010/08/08
	filename: 	Mask.h
	author:		Crazii
	purpose:	
	change:		09/08/2013
				use Mask (constant 32bit) for all platforms, 
				to cross platform without affect logic change (64bit MaskT are not available on 32bit platforms)
*********************************************************************/
#ifndef __Blade_MaskT_h__
#define __Blade_MaskT_h__
#include <Lock.h>

namespace Blade
{
	template<typename INT_T>
	struct MaskTraits
	{
		static INT_T compareAndSwap(volatile INT_T* /*data*/, INT_T /*val*/, INT_T /*comparand*/) { return 0; }
	};
	template<>
	struct MaskTraits<uint32>
	{
		static uint32 compareAndSwap(volatile uint32* data, uint32 val, uint32 comparand)
		{
			return (uint32)Lock::compareAndSwap32((volatile int32*)data, (int32)val, (int32)comparand);
		}
	};
	template<>
	struct MaskTraits<uint64>
	{
		static uint64 compareAndSwap(volatile uint64* data, uint64 val, uint64 comparand)
		{
			return (uint64)Lock::compareAndSwap64((volatile int64*)data, (int64)val, (int64)comparand);
		}
	};


	template<typename INT_T>
	class MaskT
	{
	public:
#if BLADE_STDCPP >= BLADE_STDCPP11 || (defined(_MSC_VER) && _MSC_VER >= 1900)
		static_assert(std::is_unsigned<INT_T>::value, "need unsigned type.");
#endif
		/** @brief  */
		inline MaskT()
			:mMask(0)		{}

		/** @brief  */
		inline MaskT(INT_T uiMaskT)
			:mMask(uiMaskT)	{}


		/** @brief  */
		inline ~MaskT()		{}
		
		/** @brief  */
		inline INT_T getMask() const
		{
			return mMask;
		}

		/** @brief  */
		inline bool operator<(const MaskT& rhs) const
		{
			return mMask < rhs.mMask;
		}

		/** @brief  */
		inline bool operator==(const MaskT& rhs) const
		{
			return mMask == rhs.mMask;
		}

		/** @brief  */
		inline bool operator==(INT_T uiMaskT) const
		{
			return mMask == uiMaskT;
		}

		/** @brief  */
		inline bool operator!=(const MaskT& rhs) const
		{
			return mMask != rhs.mMask;
		}

		/** @brief  */
		inline bool operator!=(INT_T uiMaskT) const
		{
			return mMask != uiMaskT;
		}

		/** @brief  */
		inline bool		checkBits(INT_T uiMaskTBit) const
		{
			return (mMask&uiMaskTBit) != 0;
		}

		/** @brief  */
		inline bool		checkBitsEqual(INT_T uiMaskT) const
		{
			return (mMask&uiMaskT) == uiMaskT;
		}

		/** @brief  */
		inline	INT_T	operator&(INT_T uiMaskTBit) const
		{
			return mMask&uiMaskTBit;
		}

		/** @brief  */
		inline	INT_T	operator&(MaskT MaskTBit) const
		{
			return mMask&MaskTBit.mMask;
		}

		/** @brief  */
		inline MaskT&	operator&=(INT_T uiMaskTBit)
		{
			mMask &= uiMaskTBit;
			return *this;
		}

		/** @brief  */
		inline void		raiseBits(INT_T uiMaskTBit)
		{
			mMask |= uiMaskTBit;
		}

		/** @brief  */
		inline INT_T	operator|(INT_T uiMaskTBit) const
		{
			return mMask | uiMaskTBit;
		}

		/** @brief  */
		inline MaskT& operator|=(INT_T uiMaskTBit)
		{
			mMask |= uiMaskTBit;
			return *this;
		}

		/** @brief  */
		inline INT_T	operator|(MaskT MaskTBit) const
		{
			return mMask | MaskTBit.mMask;
		}

		/** @brief  */
		inline MaskT& operator|=(MaskT MaskTBit)
		{
			mMask |= MaskTBit.mMask;
			return *this;
		}

		/** @brief  */
		inline void		clearBits(INT_T uiMaskTBit)
		{
			mMask &= (~uiMaskTBit);
		}

		/** @brief  */
		inline bool		checkBitAtIndex(index_t bitIndex) const
		{
			assert(bitIndex < sizeof(INT_T) * 8);
			return (mMask&((INT_T)1<<(INT_T)bitIndex)) != 0;	//make compiler happy
		}

		/** @brief  */
		inline void		raiseBitAtIndex(index_t bitIndex)
		{
			assert(bitIndex < sizeof(INT_T) * 8);
			mMask |= ( (INT_T)1u<<(INT_T)bitIndex);
		}

		/** @brief  */
		inline void		clearBitAtIndex(index_t bitIndex)
		{
			assert(bitIndex < sizeof(INT_T) * 8);
			mMask &= ~( (INT_T)1u<<(INT_T)bitIndex);
		}

		/** @brief  */
		inline void		updateBitAtIndex(index_t bitIndex, bool raise)
		{
			if(raise)
				this->raiseBitAtIndex(bitIndex);
			else
				this->clearBitAtIndex(bitIndex);
		}

		/** @brief  */
		inline void		updateBits(INT_T MaskTBits, bool raise)
		{
			if (raise)
				this->raiseBits(MaskTBits);
			else
				this->clearBits(MaskTBits);
		}

		/** @brief  */
		inline bool		safeRaiseBits(INT_T uiMaskTBit)
		{
			//lock-free style set
			INT_T mask;
			do 
			{
				mask = (INT_T)mMask;
				if( (mask&uiMaskTBit) )
					return false;
			} while (MaskTraits<INT_T>::compareAndSwap(&mMask, mask|(INT_T)uiMaskTBit, mask) != mask);
			return true;
		}

		/** @brief  */
		inline bool		safeClearBits(INT_T uiMaskTBit)
		{
			//lock-free style set
			INT_T mask;
			do 
			{
				mask = (INT_T)mMask;
				if( !(mask&uiMaskTBit) )
					return false;
			} while ( MaskTraits<INT_T>::compareAndSwap(&mMask, mask&(~(INT_T)uiMaskTBit), mask) != mask);
			return true;
		}
	protected:
		INT_T	mMask;
	};//class MaskT

	/** @brief  */
	template<typename INT_T>
	inline INT_T operator&(INT_T uim, MaskT<INT_T> MaskT)
	{
		return (MaskT&uim);
	}

	typedef MaskT<uint32> Mask;
	typedef MaskT<uint64> Mask64;
	
}//namespace Blade


#endif //__Blade_MaskT_h__