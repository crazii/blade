/********************************************************************
	created:	2014/11/06
	filename: 	BlockQuery.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include<interface/ITerrainConfigManager.h>
#include "BlockQuery.h"

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	bool			BlockQuery::raycastPoint(BlockVolumeQuery& rayQuery) const
	{
#if 0
		scalar start = 0, end = 0;
		const Ray& ray = rayQuery.mLocalRay;

		if( mQueryIndex == NULL )
		{
			//ray marching
			scalar step = scalar(0.5);
			bool lastAbove = false;
			bool lastBelow = false;
			scalar t = start;
			while(t <= end)
			{
				POINT3 pt = ray.getRayPoint(t);
				scalar terrainHeight = this->getInterpolatedHeight((int)pt.x, (int)pt.z);
				bool above = false, below = false;

				if( pt.y > terrainHeight)
					above = true;
				else
					below = true;

				if( t < rayQuery.mDistance &&
					( (lastAbove && below && (rayQuery.mSide == PS_POSITIVE || rayQuery.mSide == PS_BOTH))
					|| (lastBelow && above && (rayQuery.mSide == PS_NEGATIVE || rayQuery.mSide == PS_BOTH)) )
					)
				{
					if( step <= Math::LOW_EPSILON )
					{
						rayQuery.mDistance = t;
						//rayQuery.mTriangle
						return true;
					}
					step /= 2;
					continue;
				}

				t += step;
				lastAbove = above;
				lastBelow = below;
			}
			return false;
		}
		else
#endif
		{
			uint32 indexStart, indexCount;
			IIndexBuffer* ib = mQueryIndex->getIndexBuffer(rayQuery.mTargetLOD, rayQuery.mDiffIndex, 0, indexStart, indexCount);
			const IndexBufferHelper queryIndices(ib);

			scalar blockSize = (scalar)ITerrainConfigManager::getSingleton().getTerrainBlockSize();
			const QueryIndexQuad* quad = mQueryIndex->getQueryIndexbyLOD(rayQuery.mTargetLOD);
			assert(quad != NULL);

			RayCastContext context(queryIndices, rayQuery);
			context.modify(quad, mBlockX, mBlockZ, blockSize);

			return this->raycastPointRecursive(context);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool			BlockQuery::intersect(BlockVolumeQuery& qvolume, POS_VOL contentPos) const
	{
		if( mQueryIndex == NULL )
			return false;

		uint32 indexStart, indexCount;
		IIndexBuffer* ib = mQueryIndex->getIndexBuffer(qvolume.mTargetLOD, qvolume.mDiffIndex, 0, indexStart, indexCount);
		const IndexBufferHelper queryIndices(ib);

		scalar blockSize = (scalar)ITerrainConfigManager::getSingleton().getTerrainBlockSize();
		const QueryIndexQuad* quad = mQueryIndex->getQueryIndexbyLOD(qvolume.mTargetLOD);
		assert(quad != NULL);

		VolumeQueryConext context(queryIndices, qvolume);
		context.modify(quad, mBlockX, mBlockZ, blockSize);

		return this->volumeQueryRecurisve(context, contentPos);
	}

	//////////////////////////////////////////////////////////////////////////
	scalar			BlockQuery::getInterpolatedHeight(int /*x*/, int /*z*/) const
	{
		BLADE_EXCEPT(EXC_INTERNAL_ERROR, BTString("function not implemented."));
		//TODO: bilinear interpolation
		//return 0;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			BlockQuery::raycastPointRecursive(RayCastContext& context) const
	{
		bool result = false;
		BlockVolumeQuery& query = context.query;
		const IndexBufferHelper& indices = context.indices;

		const QueryIndexQuad* quad = context.quad;
		scalar dist = 0;
		if( quad->mSubQuad != NULL )
		{
			AABB localAABB(context.x, mMinY, context.z, context.x + context.size, mMaxY, context.z + context.size);
			if( !query.mQuery.intersect(localAABB, dist) || dist > query.mDistance )
				return result;

			scalar halfSize = context.size/2;
			const scalar subX[4] = {context.x,	context.x+halfSize,	context.x,			context.x+halfSize};
			const scalar subZ[4] = {context.z,	context.z,			context.z+halfSize,	context.z+halfSize};

			for(size_t i = 0; i < 4; ++i)
			{
				//modify context
				context.modify(&quad->mSubQuad[i], subX[i], subZ[i], halfSize);
				if( this->raycastPointRecursive(context) )
					//if result is true ,we don't stop
					//we need to check if there are more nearer one
					result = true;
			}
			return result;
		}

		//leaf: recursion end
		//test all triangles

		index_t indexStart = quad->mIndicesByLODDiff[ query.mDiffIndex ].mStartIndex;
		size_t indexEnd = indexStart + quad->mIndicesByLODDiff[ query.mDiffIndex ].mTriangleCount*3;

		scalar minY = FLT_MAX, maxY = -FLT_MAX;
		for(index_t index = indexStart; index < indexEnd; ++index)
		{
			scalar height = mHeight[ indices[index] ].getHeight();
			if( height < minY )
				minY = height;
			if( height > maxY )
				maxY = height;
		}
		//early out on bounds - AAB intersection is much faster
		AABB localAABB(context.x, minY, context.z, context.x + context.size, maxY, context.z + context.size);
		if( !query.mQuery.intersect(localAABB, dist) || dist > query.mDistance )
			return result;

		for(index_t index = indexStart; index < indexEnd; index += 3)
		{
			const TERRAIN_POSITION_DATA_XZ& hpos0 = mHorizontalPos[ indices[index] ];
			const TERRAIN_POSITION_DATA_XZ& hpos1 = mHorizontalPos[ indices[index+1] ];
			const TERRAIN_POSITION_DATA_XZ& hpos2 = mHorizontalPos[ indices[index+2] ];
			const TERRAIN_POSITION_DATA_Y& vpos0 = mHeight[ indices[index] ];
			const TERRAIN_POSITION_DATA_Y& vpos1 = mHeight[ indices[index+1] ];
			const TERRAIN_POSITION_DATA_Y& vpos2 = mHeight[ indices[index+2] ];

			Vector3 vertices[3] =
			{ 
				Vector3((scalar)hpos0.getX(), vpos0.getHeight(), (scalar)hpos0.getZ()),
				Vector3((scalar)hpos1.getX(), vpos1.getHeight(), (scalar)hpos1.getZ()),
				Vector3((scalar)hpos2.getX(), vpos2.getHeight(), (scalar)hpos2.getZ()),
			};

			if( query.mQuery.intersect(vertices[0], vertices[1], vertices[2], dist) && dist < query.mDistance )
			{
				result = true;
				//set the new limit to check if there's nearer triangle
				query.mDistance = dist;
			}
		}

		return result;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			BlockQuery::volumeQueryRecurisve(VolumeQueryConext& context, POS_VOL pos) const
	{
		BlockVolumeQuery& query = context.query;
		const IndexBufferHelper& indices = context.indices;

		const QueryIndexQuad*		quad = context.quad;

		bool result = false;
		if( pos == PV_OUTSIDE )
			return result;

		if( pos == PV_INSIDE || quad->mSubQuad == NULL )
		{
			bool skipTriangle = (pos == PV_INSIDE);
			if( skipTriangle || !query.mQuery.needTriangles())
				result = true;

			index_t indexStart = quad->mIndicesByLODDiff[ query.mDiffIndex ].mStartIndex;
			size_t indexEnd = indexStart + quad->mIndicesByLODDiff[ query.mDiffIndex ].mTriangleCount*3;
			for(index_t index = indexStart; index < indexEnd; index += 3)
			{
				const TERRAIN_POSITION_DATA_XZ& hpos0 = mHorizontalPos[ indices[index] ];
				const TERRAIN_POSITION_DATA_XZ& hpos1 = mHorizontalPos[ indices[index+1] ];
				const TERRAIN_POSITION_DATA_XZ& hpos2 = mHorizontalPos[ indices[index+2] ];
				const TERRAIN_POSITION_DATA_Y& vpos0 = mHeight[ indices[index] ];
				const TERRAIN_POSITION_DATA_Y& vpos1 = mHeight[ indices[index+1] ];
				const TERRAIN_POSITION_DATA_Y& vpos2 = mHeight[ indices[index+2] ];

				Vector3 vtx0((scalar)hpos0.getX(), vpos0.getHeight(), (scalar)hpos0.getZ());
				Vector3 vtx1((scalar)hpos1.getX(), vpos1.getHeight(), (scalar)hpos1.getZ());
				Vector3 vtx2((scalar)hpos2.getX(), vpos2.getHeight(), (scalar)hpos2.getZ());
				
				scalar distance;
				if(query.mQuery.needTriangles() && (skipTriangle || query.mQuery.intersect(vtx0, vtx1, vtx2, distance)))
				{
					//output triangles in world space
					vtx0 += query.mTranslation;
					vtx1 += query.mTranslation;
					vtx2 += query.mTranslation;

					query.mQuery.addTriangle(vtx0,vtx1,vtx2);
					result = result || true;
				}
			}
			return result;
		}

		scalar halfSize = context.size/2;
		const scalar subX[4] = {context.x,	context.x+halfSize,	context.x,			context.x+halfSize};
		const scalar subZ[4] = {context.z,	context.z,			context.z+halfSize,	context.z+halfSize};

		AABB localAABB(context.x, mMinY, context.z, context.x + context.size, mMaxY, context.z + context.size);
		scalar distance = query.mQuery.getDistanceLimit();
		POS_VOL subPos = query.mQuery.intersect(localAABB, distance);

		for(size_t i = 0; i < 4; ++i)
		{
			//modify context
			context.modify(&quad->mSubQuad[i], subX[i], subZ[i], halfSize);
			if( this->volumeQueryRecurisve(context, subPos) )
				//if result is true ,we don't stop
					//we need to check if there are more nearer one
						result = true;
		}
		return  result;
	}

}//namespace Blade