/********************************************************************
	created:	2013/10/18
	filename: 	GraphicsUtility.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <math/Half3.h>
#include <interface/public/graphics/GraphicsUtility.h>
#include <interface/public/graphics/IVertexDeclaration.h>
#include <interface/public/graphics/VertexElement.h>
#include <interface/public/graphics/IVertexBuffer.h>
#include <interface/public/graphics/IIndexBuffer.h>
#include <interface/public/graphics/IVertexSource.h>

namespace Blade
{
	namespace GraphicsUtility
	{
		class VertexHelper
		{
		public:
			VertexHelper(void* buffer, size_t vertexSize, size_t vertexCount,size_t offset, bool half4)
				:mBuffer((char*)buffer)
				,mVertexSize(vertexSize)
				,mVertexCount(vertexCount)
				,mOffset(offset)
				,mHalf4CacheIndex(0)
				,mHalf4(half4)
			{
			}

			/** @brief  */
			const Vector3& operator[](index_t index)
			{
				if( index < mVertexCount )
				{
					char* vertex = mBuffer + mVertexSize*index;
					char* element = vertex + mOffset;
					if(!mHalf4)
						return *(Vector3*)element;
					else
					{
						size_t cacheIndex = mHalf4CacheIndex++;
						if (mHalf4CacheIndex == 3)
							mHalf4CacheIndex = 0;
						mHalf4Cache[cacheIndex] = *(Half3*)element;
						return mHalf4Cache[cacheIndex];
					}
				}
				else
					BLADE_EXCEPT(EXC_OUT_OF_RANGE, BTString("index out of range.") );
			}

		private:
			char*				mBuffer;
			size_t				mVertexSize;
			size_t				mVertexCount;
			size_t				mOffset;
			Vector3				mHalf4Cache[3];
			size_t				mHalf4CacheIndex;
			bool				mHalf4;
		};

		//////////////////////////////////////////////////////////////////////////
		bool rayIntersectTriangles(const Ray& ray, const GraphicsGeometry& geometry, scalar& t, POS_SIDE side/* = PS_POSITIVE*/)
		{
			if (geometry.mPrimitiveType != GraphicsGeometry::GPT_TRIANGLE_LIST)
			{
				assert(false);
				return false;
			}
			if (geometry.mVertexDecl == NULL || geometry.mVertexSource == NULL)
			{
				assert(false);
				return false;
			}
			if (geometry.hasIndexBuffer() && geometry.mIndexBuffer == NULL)
			{
				assert(false);
				return false;
			}

			size_t elementCount = geometry.mVertexDecl->getElementCount();
			index_t posIndex = INVALID_INDEX;
			for(index_t i = 0; i < elementCount; ++i)
			{
				const VertexElement& elem = geometry.mVertexDecl->getElement(i);
				if( elem.getUsage() == VU_POSITION )
				{
					posIndex = i;
					break;
				}
			}

			if( posIndex == INVALID_INDEX )
				return false;

			const VertexElement& elem = geometry.mVertexDecl->getElement(posIndex);
			const HVBUFFER& hVertexBuffer = geometry.mVertexSource->getBuffer( elem.getSource() );
			if( elem.getType() != VET_FLOAT3 && elem.getType() != VET_FLOAT4 && elem.getType() != VET_HALF4)
				return false;
			
			bool intersected = false;
			scalar dist = t;
			char* data = (char*)hVertexBuffer->lock(IGraphicsBuffer::GBLF_READONLY);
			VertexHelper vertexHelper(data+hVertexBuffer->getVertexSize()*geometry.mVertexStart, hVertexBuffer->getVertexSize(), geometry.mVertexCount, elem.getOffset(), elem.getType() == VET_HALF4);
			if( geometry.hasIndexBuffer() )
			{
				data = (char*)geometry.mIndexBuffer->lock(IGraphicsBuffer::GBLF_READONLY);
				const IndexBufferHelper indexHelper(data + geometry.mIndexStart*geometry.mIndexBuffer->getIndexSize(), geometry.mIndexBuffer->getIndexType() );
				size_t indexCount = geometry.mIndexCount;
				scalar l = dist;
				for( size_t i = 0; i < indexCount; i+=3)
				{
					uint32 index0 = indexHelper[i];
					uint32 index1 = indexHelper[i+1];
					uint32 index2 = indexHelper[i+2];

					const Vector3& p0 = vertexHelper[index0];
					const Vector3& p1 = vertexHelper[index1];
					const Vector3& p2 = vertexHelper[index2];
					if( ray.intersect(p0, p1, p2, l, side, dist) )
					{
						intersected = true;
						dist = l;
					}	
				}
				geometry.mIndexBuffer->unlock();
			}
			else
			{
				for(size_t i = 0; i < geometry.mVertexCount; i+=3)
				{
					const Vector3& p0 = vertexHelper[i];
					const Vector3& p1 = vertexHelper[i+1];
					const Vector3& p2 = vertexHelper[i+2];
					scalar l = dist;
					if( ray.intersect(p0, p1, p2, l, side, dist) )
					{
						intersected = true;
						dist = l;
					}						
				}
			}
			hVertexBuffer->unlock();
			if(intersected)
			{
				assert(dist <= t);
				t = dist;
			}
			return intersected;
		}

	}//namespace GraphicsUtility

	
}//namespace Blade