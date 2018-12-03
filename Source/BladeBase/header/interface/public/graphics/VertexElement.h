/********************************************************************
	created:	2010/04/22
	filename: 	VertexElement.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_VertexElement_h__
#define __Blade_VertexElement_h__
#include "Color.h"

namespace Blade
{
	///stream/attribute semantics
	enum EVertexUsage
	{
		/// Position
		VU_POSITION				= 0,
		/// Blending weights
		VU_BLEND_WEIGHTS		= 1,
		/// Blending indices
		VU_BLEND_INDICES		= 2,
		/// Normal, 3 reals per vertex
		VU_NORMAL				= 3,
		/// Diffuse color
		VU_COLOR				= 4,
		/// Texture coordinates
		VU_TEXTURE_COORDINATES	= 5,
		/// Binormal (Y axis if normal is Z)
		VU_BINORMAL				= 6,
		/// Tangent (X axis if normal is Z)
		VU_TANGENT				= 7,
		
		//
		VU_COUNT,
		VU_BEGIN = VU_POSITION,
		VU_UNDEFINED = -1,
	};

	enum EVertexElementType
	{
		VET_FLOAT1	= 0,
		VET_FLOAT2	= 1,
		VET_FLOAT3	= 2,
		VET_FLOAT4	= 3,
		VET_COLOR	= 4,
		VET_SHORT2	= 5,
		VET_SHORT4	= 6,
		VET_UBYTE4	= 7,
		VET_UBYTE4N	= 8,
		VET_SHORT2N	= 9,
		VET_SHORT4N = 10,
		VET_USHORT2N= 11,
		VET_USHORT4N= 12,
		VET_UDEC3	= 13,	///D3DDECLTYPE_UDEC3 for dx9, DXGI_FORMAT_R10G10B10A2_UINT for dx11
		VET_DEC3N	= 14,
		VET_HALF2	= 15,	///float16x2
		VET_HALF4	= 16,	///float16x4

		//
		VET_COUNT,
		VET_BEGIN = 0,
	};


	class VertexElement
	{
	public:
		inline VertexElement() {}
		inline VertexElement(uint16 source,uint16 offset,EVertexElementType type,EVertexUsage usage,uint8 index)
			:mStreamSource(source)
			,mOffset(offset)
			,mType( (uint8)type)
			,mUsage( (uint8)usage)
			,mUsageIndex(index)	{}

		/** @brief  */
		inline uint16				getSource() const
		{
			return mStreamSource;
		}

		/** @brief  */
		inline uint16				getOffset() const
		{
			return mOffset;
		}

		/** @brief  */
		inline EVertexElementType	getType() const
		{
			return (EVertexElementType)mType;
		}

		/** @brief  */
		inline EVertexUsage			getUsage() const
		{
			return (EVertexUsage)mUsage;
		}

		/** @brief  */
		inline uint8				getIndex() const
		{
			return mUsageIndex;
		}

		/** @brief  */
		inline void*				getElement(void* vertexBase) const
		{
			return static_cast<char*>(vertexBase) + mOffset;
		}

		/** @brief  */
		inline float*				getFloatElement(void* vertexBase) const
		{
			return reinterpret_cast<float*>( static_cast<char*>(vertexBase) + mOffset );
		}

		/** @brief  */
		inline char*				getByteElement(void* vertexBase) const
		{
			return static_cast<char*>(vertexBase) + mOffset;
		}

		/** @brief  */
		inline short*				getShortElement(void* vertexBase) const
		{
			return reinterpret_cast<short*>( static_cast<char*>(vertexBase) + mOffset );
		}

		/** @brief  */
		inline Color::COLOR*		getColorElment(void* vertexBase) const
		{
			return reinterpret_cast<Color::COLOR*>( static_cast<char*>(vertexBase) + mOffset );
		}

		/*
		@describe 
		@param 
		@return 
		*/
		inline	void				getElement(void* vertexBase,void*& element) const
		{
			element = this->getElement(vertexBase);
		}

		inline	void				getElement(void* vertexBase,float*& element) const
		{
			element = this->getFloatElement(vertexBase);
		}

		inline void					getElement(void* vertexBase,char*& element) const
		{
			element = this->getByteElement(vertexBase);
		}

		inline void					getElement(void* vertexBase,short*& element) const
		{
			element = this->getShortElement(vertexBase);
		}

		inline void					getElement(void* vertexBase,Color::COLOR*& element) const
		{
			element = this->getColorElment(vertexBase);
		}

		/** @brief  */
		uint16						getSize() const
		{
			return VertexElement::getSize(this->getType());
		}

		/** @brief STL compatibility */
		bool						operator<(const VertexElement& rhs) const
		{
			return (mStreamSource < rhs.mStreamSource)
				|| (mStreamSource == rhs.mStreamSource && mUsage < rhs.mUsage)
				|| (mStreamSource == rhs.mStreamSource  && mUsage == rhs.mUsage && mUsageIndex < rhs.mUsageIndex);
		}

		/** @brief  */
		BLADE_BASE_API static uint16 getSize(EVertexElementType type);

		/** @brief  */
		BLADE_BASE_API static Color::COLOR			getColorValue(const Color& color,const EVertexElementType color_type);

	protected:
		uint16	mStreamSource;
		uint16	mOffset;
		uint8	mType;
		uint8	mUsage;
		uint8	mUsageIndex;
	};//class VertexElement
	
}//namespace Blade


#endif //__Blade_VertexElement_h__