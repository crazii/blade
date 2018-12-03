/********************************************************************
	created:	2010/04/13
	filename: 	IIndexBuffer.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_IIndexBuffer_h__
#define __Blade_IIndexBuffer_h__
#include "IGraphicsBuffer.h"
#include <math/BladeSIMD.h>
#include <interface/IPlatformManager.h>
#include <Handle.h>

namespace Blade
{
	class IGraphicsResourceManager;

	class IIndexBuffer : public IGraphicsBuffer
	{
	public:
		enum EIndexType
		{
			IT_16BIT,
			IT_32BIT,
		};

		virtual ~IIndexBuffer()		{}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual EIndexType	getIndexType() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual size_t		getIndexCount() const = 0;

		/**
		@describe get size of one single index in bytes
		@param 
		@return 
		*/
		virtual size_t		getIndexSize() const = 0;

		/**
		@describe get the whole buffer size in bytes
		@param 
		@return 
		*/
		virtual size_t		getSizeInBytes() const = 0;

	};//class  IIndexBuffer

	typedef Handle<IIndexBuffer> HIBUFFER;

	///helper class acts like a index buffer array
	///usgae: IndexBufferHelper index();
	///index[i++] = 0;
	///*(index++) = 1;
	class IndexBufferHelper : public NonAssignable, public NonAllocatable
	{
	protected:
		///for operator++(int)
		IndexBufferHelper(const IndexBufferHelper& src)
			:mType(src.mType)
			,mPointer(src.mPointer)
			,mBuffer(src.mBuffer)
			,mIndexBuffer(NULL)
		{
		}
	public:
		///calculate the best format according to vertex count
		static IIndexBuffer::EIndexType calcIndexType(size_t vertexCount)
		{
			return vertexCount > uint32( uint16(-1) ) ? IIndexBuffer::IT_32BIT : IIndexBuffer::IT_16BIT;
		}
		static size_t calcIndexSize(IIndexBuffer::EIndexType type)
		{
			return type == IIndexBuffer::IT_32BIT ? 4u : 2u;
		}

		inline IndexBufferHelper(IIndexBuffer* buffer, bool writable = true)
			:mType(buffer->getIndexType())
			,mPointer(0)
			,mBuffer(buffer->lock(writable ? IGraphicsBuffer::GBLF_NORMAL : IGraphicsBuffer::GBLF_READONLY))
			,mIndexBuffer(buffer)
		{

		}

		inline IndexBufferHelper(void* buffer, IIndexBuffer::EIndexType type)
			:mType(type)
			,mPointer(0)
			,mBuffer(buffer)
			,mIndexBuffer(NULL)
		{
		}

		inline ~IndexBufferHelper()
		{
			if(mIndexBuffer != NULL )
				mIndexBuffer->unlock();
		}

		/** @brief  */
		inline void operator=(uint32 idata)
		{
			if( mType == IIndexBuffer::IT_16BIT )
				*(mBuffer16 + mPointer) = static_cast<uint16>(idata);
			else
				*(mBuffer32 + mPointer) = idata;
		}

		/** @brief  */
		inline uint32 operator+(uint32 idata)
		{
			uint32 data;
			if( mType == IIndexBuffer::IT_16BIT )
				data = *(mBuffer16 + mPointer);
			else
				data = *(mBuffer32 + mPointer);

			return data+idata;
		}

		/** @brief  */
		inline void operator+=(uint32 idata)
		{
			if( mType == IIndexBuffer::IT_16BIT )
				*(mBuffer16 + mPointer) += static_cast<uint16>(idata);
			else
				*(mBuffer32 + mPointer) += idata;
		}

		/** @brief  */
		inline IndexBufferHelper&	operator++()
		{
			++mPointer;
			return *this;
		}

		/** @brief  */
		inline IndexBufferHelper	operator++(int)
		{
			IndexBufferHelper ret(mBuffer,mType);
			ret.mPointer = mPointer;
			++mPointer;
			return ret;
		}

		/** @brief  */
		inline IndexBufferHelper& operator*()
		{
			return *this;
		}

		/** @brief  */
		inline IndexBufferHelper& operator[](index_t index)
		{
			mPointer = index;
			return *this;
		}

		/** @brief  */
		inline uint32	operator[](index_t index) const
		{
			if( mType == IIndexBuffer::IT_16BIT )
				return (uint32)mBuffer16[index];
			else
				return mBuffer32[index];
		}

		/** @brief  */
		inline index_t getCursor() const
		{
			return mPointer;
		}


		static inline void copyIndices(void* dest, IIndexBuffer::EIndexType destType,
			const void* src, IIndexBuffer::EIndexType srcType, size_t baseOffset, size_t count)
		{
			if (destType == srcType)
			{
				if (destType == IIndexBuffer::IT_16BIT)
				{
					assert(srcType == IIndexBuffer::IT_16BIT);
					uint16* dest16 = (uint16*)dest;
					const uint16* src16 = (const uint16*)src;

#if BLADE_ENABLE_SIMD
					size_t _uint16x32 = count / 32;
					count = count % 32;
#if BLADE_SIMD == BLADE_SIMD_SSE
					const __m128i offset16x8 = _mm_set1_epi16((short)(uint32)baseOffset);
					for (size_t i = 0; i < _uint16x32; ++i)
					{
						IPlatformManager::prefetch<PM_READ>(src16);
						IPlatformManager::prefetch<PM_WRITE>(dest16);

						{
							__m128i v = _mm_loadu_si128((__m128i*)src16);
							v = _mm_add_epi16(v, offset16x8);
							_mm_storeu_si128((__m128i*)dest16, v);
							src16 += 8;
							dest16 += 8;
						}

						{
							__m128i v = _mm_loadu_si128((__m128i*)src16);
							v = _mm_add_epi16(v, offset16x8);
							_mm_storeu_si128((__m128i*)dest16, v);
							src16 += 8;
							dest16 += 8;
						}

						{
							__m128i v = _mm_loadu_si128((__m128i*)src16);
							v = _mm_add_epi16(v, offset16x8);
							_mm_storeu_si128((__m128i*)dest16, v);
							src16 += 8;
							dest16 += 8;
						}

						{
							__m128i v = _mm_loadu_si128((__m128i*)src16);
							v = _mm_add_epi16(v, offset16x8);
							_mm_storeu_si128((__m128i*)dest16, v);
							src16 += 8;
							dest16 += 8;
						}
					}

					size_t _uint16x8 = count / 8;
					count = count % 8;
					for (size_t i = 0; i < _uint16x8; ++i)
					{
						__m128i v = _mm_loadu_si128((__m128i*)src16);

						v = _mm_add_epi16(v, offset16x8);
						_mm_storeu_si128((__m128i*)dest16, v);

						src16 += 8;
						dest16 += 8;
					}

#elif BLADE_SIMD == BLADE_SIMD_NEON
					const uint16x8_t offset16x8 = vmovq_n_u16((uint16_t)baseOffset);
					for (size_t i = 0; i < _uint16x32; ++i)
					{
						IPlatformManager::prefetch<PM_READ>(src16);
						IPlatformManager::prefetch<PM_WRITE>(dest16);

						{
							uint16x8_t v = vld1q_u16((const uint16_t*)src16);
							v = vaddq_u16(v, offset16x8);
							vst1q_u16((uint16_t*)dest16, v);
							src16 += 8;
							dest16 += 8;
						}

						{
							uint16x8_t v = vld1q_u16((const uint16_t*)src16);
							v = vaddq_u16(v, offset16x8);
							vst1q_u16((uint16_t*)dest16, v);
							src16 += 8;
							dest16 += 8;
						}

						{
							uint16x8_t v = vld1q_u16((const uint16_t*)src16);
							v = vaddq_u16(v, offset16x8);
							vst1q_u16((uint16_t*)dest16, v);
							src16 += 8;
							dest16 += 8;
						}

						{
							uint16x8_t v = vld1q_u16((const uint16_t*)src16);
							v = vaddq_u16(v, offset16x8);
							vst1q_u16((uint16_t*)dest16, v);
							src16 += 8;
							dest16 += 8;
						}

					}

					size_t _uint16x8 = count / 8;
					count = count % 8;
					for (size_t i = 0; i < _uint16x8; ++i)
					{
						uint16x8_t v = vld1q_u16((const uint16_t*)src16);

						v = vaddq_u16(v, offset16x8);
						vst1q_u16((uint16_t*)dest16, v);

						src16 += 8;
						dest16 += 8;
					}

#else
#error not implemented.
#endif
#endif
					for (size_t i = 0; i < count; ++i)
						*(dest16++) = (uint16)(*(src16++) + (uint16)baseOffset);
				}
				else
				{
					assert(destType == IIndexBuffer::IT_32BIT && srcType == IIndexBuffer::IT_32BIT);
					uint32* dest32 = (uint32*)dest;
					const uint32* src32 = (const uint32*)src;
#if BLADE_ENABLE_SIMD
					size_t _uint32x16 = count / 16;
					count = count % 16;
#if BLADE_SIMD == BLADE_SIMD_SSE
					const __m128i offset32x4 = _mm_set1_epi32((int)(uint32)baseOffset);
					for (size_t i = 0; i < _uint32x16; ++i)
					{
						IPlatformManager::prefetch<PM_READ>(src32);
						IPlatformManager::prefetch<PM_WRITE>(dest32);

						{
							__m128i v = _mm_loadu_si128((__m128i*)src32);
							v = _mm_add_epi32(v, offset32x4);
							_mm_storeu_si128((__m128i*)dest32, v);
							src32 += 4;
							dest32 += 4;
						}

						{
							__m128i v = _mm_loadu_si128((__m128i*)src32);
							v = _mm_add_epi32(v, offset32x4);
							_mm_storeu_si128((__m128i*)dest32, v);
							src32 += 4;
							dest32 += 4;
						}

						{
							__m128i v = _mm_loadu_si128((__m128i*)src32);
							v = _mm_add_epi32(v, offset32x4);
							_mm_storeu_si128((__m128i*)dest32, v);
							src32 += 4;
							dest32 += 4;
						}

						{
							__m128i v = _mm_loadu_si128((__m128i*)src32);
							v = _mm_add_epi32(v, offset32x4);
							_mm_storeu_si128((__m128i*)dest32, v);
							src32 += 4;
							dest32 += 4;
						}
					}

					size_t _uint32x4 = count / 4;
					count = count % 4;
					for (size_t i = 0; i < _uint32x4; ++i)
					{
						__m128i v = _mm_loadu_si128((__m128i*)src32);

						v = _mm_add_epi32(v, offset32x4);
						_mm_storeu_si128((__m128i*)dest32, v);

						src32 += 4;
						dest32 += 4;
					}
#elif BLADE_SIMD == BLADE_SIMD_NEON
					const uint32x4_t offset32x4 = vdupq_n_u32((uint32_t)baseOffset);
					for (size_t i = 0; i < _uint32x16; ++i)
					{
						IPlatformManager::prefetch<PM_READ>(src32);
						IPlatformManager::prefetch<PM_WRITE>(dest32);

						{
							uint32x4_t v = vld1q_u32((const uint32_t*)src32);
							v = vaddq_u32(v, offset32x4);
							vst1q_u32((uint32_t*)dest32, v);
							src32 += 4;
							dest32 += 4;
						}

						{
							uint32x4_t v = vld1q_u32((const uint32_t*)src32);
							v = vaddq_u32(v, offset32x4);
							vst1q_u32((uint32_t*)dest32, v);
							src32 += 4;
							dest32 += 4;
						}

						{
							uint32x4_t v = vld1q_u32((const uint32_t*)src32);
							v = vaddq_u32(v, offset32x4);
							vst1q_u32((uint32_t*)dest32, v);
							src32 += 4;
							dest32 += 4;
						}

						{
							uint32x4_t v = vld1q_u32((const uint32_t*)src32);
							v = vaddq_u32(v, offset32x4);
							vst1q_u32((uint32_t*)dest32, v);
							src32 += 4;
							dest32 += 4;
						}
					}

					size_t _uint32x4 = count / 4;
					count = count % 4;
					for (size_t i = 0; i < _uint32x4; ++i)
					{
						uint32x4_t v = vld1q_u32((const uint32_t*)src32);

						v = vaddq_u32(v, offset32x4);
						vst1q_u32((uint32_t*)dest32, v);

						src32 += 4;
						dest32 += 4;
					}
#else
#error not implemented.
#endif
#endif
					for (size_t i = 0; i < count; ++i)
						*(dest32++) = *(src32++) + (uint32)baseOffset;
				}
			}
			else
			{
				assert(destType == IIndexBuffer::IT_32BIT && srcType == IIndexBuffer::IT_16BIT);
				uint32* dest32 = (uint32*)dest;
				const uint16* src16 = (const uint16*)src;
#if BLADE_ENABLE_SIMD
				size_t _uint16x32 = count / 32;	//loop unroll: process 4 16x8 in one pass (1 src cache block, 2 dest cache block)
				count = count % 32;
#if BLADE_SIMD == BLADE_SIMD_SSE
				const __m128i offset32x4 = _mm_set1_epi32((int)(uint32)baseOffset);
				const __m128i zero16x8 = _mm_set1_epi16(0);
				for (size_t i = 0; i < _uint16x32; ++i)
				{
					IPlatformManager::prefetch<PM_READ>(src16);
					IPlatformManager::prefetch<PM_WRITE>(dest32);

					{
						__m128i v = _mm_loadu_si128((__m128i*)src16);
						__m128i unpack = _mm_unpacklo_epi16(v, zero16x8);
						unpack = _mm_add_epi32(unpack, offset32x4);
						_mm_storeu_si128((__m128i*)dest32, unpack);

						unpack = _mm_unpackhi_epi16(v, zero16x8);
						unpack = _mm_add_epi32(unpack, offset32x4);
						_mm_storeu_si128((__m128i*)(dest32 + 4), unpack);

						src16 += 8;
						dest32 += 8;
					}

					{
						__m128i v = _mm_loadu_si128((__m128i*)src16);

						__m128i unpack = _mm_unpacklo_epi16(v, zero16x8);
						unpack = _mm_add_epi32(unpack, offset32x4);
						_mm_storeu_si128((__m128i*)dest32, unpack);

						unpack = _mm_unpackhi_epi16(v, zero16x8);
						unpack = _mm_add_epi32(unpack, offset32x4);
						_mm_storeu_si128((__m128i*)(dest32 + 4), unpack);

						src16 += 8;
						dest32 += 8;
					}

					IPlatformManager::prefetch<PM_WRITE>(dest32);

					{
						__m128i v = _mm_loadu_si128((__m128i*)src16);
						__m128i unpack = _mm_unpacklo_epi16(v, zero16x8);
						unpack = _mm_add_epi32(unpack, offset32x4);
						_mm_storeu_si128((__m128i*)dest32, unpack);

						unpack = _mm_unpackhi_epi16(v, zero16x8);
						unpack = _mm_add_epi32(unpack, offset32x4);
						_mm_storeu_si128((__m128i*)(dest32 + 4), unpack);

						src16 += 8;
						dest32 += 8;
					}

					{
						__m128i v = _mm_loadu_si128((__m128i*)src16);

						__m128i unpack = _mm_unpacklo_epi16(v, zero16x8);
						unpack = _mm_add_epi32(unpack, offset32x4);
						_mm_storeu_si128((__m128i*)dest32, unpack);

						unpack = _mm_unpackhi_epi16(v, zero16x8);
						unpack = _mm_add_epi32(unpack, offset32x4);
						_mm_storeu_si128((__m128i*)(dest32 + 4), unpack);

						src16 += 8;
						dest32 += 8;
					}
				}

				size_t _uint16x8 = count / 8;
				count = count % 8;
				for (size_t i = 0; i < _uint16x8; ++i)
				{
					__m128i v = _mm_loadu_si128((__m128i*)src16);

					__m128i unpack = _mm_unpacklo_epi16(v, zero16x8);
					unpack = _mm_add_epi32(unpack, offset32x4);
					_mm_storeu_si128((__m128i*)dest32, unpack);

					unpack = _mm_unpackhi_epi16(v, zero16x8);
					unpack = _mm_add_epi32(unpack, offset32x4);
					_mm_storeu_si128((__m128i*)(dest32 + 4), unpack);

					src16 += 8;
					dest32 += 8;
				}

#elif BLADE_SIMD == BLADE_SIMD_NEON
				const uint32x4_t offset32x4 = vdupq_n_u32((uint32_t)baseOffset);
				for (size_t i = 0; i < _uint16x32; ++i)
				{
					IPlatformManager::prefetch<PM_READ>(src16);
					IPlatformManager::prefetch<PM_WRITE>(dest32);

					{
						uint16x4_t v = vld1_u16((const uint16_t*)src16);
						uint32x4_t unpack = vmovl_u16(v);
						unpack = vaddq_u32(unpack, offset32x4);
						vst1q_u32(dest32, unpack);

						v = vld1_u16((const uint16_t*)(src16 + 4));
						unpack = vmovl_u16(v);
						unpack = vaddq_u32(unpack, offset32x4);
						vst1q_u32((dest32 + 4), unpack);

						src16 += 8;
						dest32 += 8;
					}

					{
						uint16x4_t v = vld1_u16((const uint16_t*)src16);
						uint32x4_t unpack = vmovl_u16(v);
						unpack = vaddq_u32(unpack, offset32x4);
						vst1q_u32(dest32, unpack);

						v = vld1_u16((const uint16_t*)(src16 + 4));
						unpack = vmovl_u16(v);
						unpack = vaddq_u32(unpack, offset32x4);
						vst1q_u32((dest32 + 4), unpack);

						src16 += 8;
						dest32 += 8;
					}

					IPlatformManager::prefetch<PM_WRITE>(dest32);

					{
						uint16x4_t v = vld1_u16((const uint16_t*)src16);
						uint32x4_t unpack = vmovl_u16(v);
						unpack = vaddq_u32(unpack, offset32x4);
						vst1q_u32(dest32, unpack);

						v = vld1_u16((const uint16_t*)(src16 + 4));
						unpack = vmovl_u16(v);
						unpack = vaddq_u32(unpack, offset32x4);
						vst1q_u32((dest32 + 4), unpack);

						src16 += 8;
						dest32 += 8;
					}

					{
						uint16x4_t v = vld1_u16((const uint16_t*)src16);
						uint32x4_t unpack = vmovl_u16(v);
						unpack = vaddq_u32(unpack, offset32x4);
						vst1q_u32(dest32, unpack);

						v = vld1_u16((const uint16_t*)(src16 + 4));
						unpack = vmovl_u16(v);
						unpack = vaddq_u32(unpack, offset32x4);
						vst1q_u32((dest32 + 4), unpack);

						src16 += 8;
						dest32 += 8;
					}
				}

				size_t _uint16x8 = count / 8;
				count = count % 8;
				for (size_t i = 0; i < _uint16x8; ++i)
				{
					uint16x4_t v = vld1_u16((const uint16_t*)src16);
					uint32x4_t unpack = vmovl_u16(v);
					unpack = vaddq_u32(unpack, offset32x4);
					vst1q_u32(dest32, unpack);

					v = vld1_u16((const uint16_t*)(src16 + 4));
					unpack = vmovl_u16(v);
					unpack = vaddq_u32(unpack, offset32x4);
					vst1q_u32((dest32 + 4), unpack);

					src16 += 8;
					dest32 += 8;
				}
#else
#error not implemented.
#endif
#endif
				for (size_t i = 0; i < count; ++i)
					*(dest32++) = (uint32)*(src16++) + (uint32)baseOffset;
			}
		}


	protected:
		IIndexBuffer::EIndexType	mType;
		index_t				mPointer;
		union
		{
			void*			mBuffer;
			uint16*			mBuffer16;
			uint32*			mBuffer32;
		};
		IIndexBuffer*		mIndexBuffer;
	};

	template<IIndexBuffer::EIndexType TYPE>
	struct IndexTypeBinding
	{
		typedef void IndexType;
	};

	template<>
	struct IndexTypeBinding<IIndexBuffer::IT_16BIT>
	{
		typedef uint16 IndexType;
	};

	template<>
	struct IndexTypeBinding<IIndexBuffer::IT_32BIT>
	{
		typedef uint32 IndexType;
	};

}//namespace Blade


#endif //__Blade_IIndexBuffer_h__