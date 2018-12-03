/********************************************************************
	created:	2011/06/02
	filename: 	BlockQueryDataCompact.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "BlockQueryDataCompact.h"

namespace Blade
{

	//////////////////////////////////////////////////////////////////////////
	BlockQueryDataCompact::BlockQueryDataCompact()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	BlockQueryDataCompact::~BlockQueryDataCompact()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	void			BlockQueryDataCompact::initialize(uint16& index,BlockQueryDataCompact* pool,const QueryIndexQuad* quad,
		const TERRAIN_POSITION_DATA_Y* vposition, const TERRAIN_POSITION_DATA_XZ* hposition, const IndexBufferHelper& indices)
	{
		if(quad->mSubQuad != NULL )
		{
			mSubLeafIndex = index;
			index += 4;
			mMinY = FLT_MAX;
			mMaxY = FLT_MIN;
			for(size_t i = 0; i < 4; ++i)
			{
				pool[ mSubLeafIndex+i ].initialize(index,pool,&quad->mSubQuad[i],vposition,hposition,indices);

				if( mMinY > pool[ mSubLeafIndex+i ].mMinY )
					mMinY = pool[ mSubLeafIndex+i ].mMinY;

				if( mMaxY < pool[ mSubLeafIndex+i ].mMaxY )
					mMaxY = pool[ mSubLeafIndex+i ].mMaxY;
			}

			mSize = pool[mSubLeafIndex].mSize*2u;
			mX = pool[mSubLeafIndex].mX;
			mZ = pool[mSubLeafIndex].mZ;
		}
		else
		{
			mSubLeafIndex = uint16(-1);
			scalar minX = FLT_MAX;
			scalar maxX = FLT_MIN;
			scalar minZ = FLT_MAX;
			//scalar maxZ = FLT_MIN;
			mMinY = FLT_MAX;
			mMaxY = FLT_MIN;
			index_t indexStart = quad->mIndicesByLODDiff[LODDI_ALL].mStartIndex;
			size_t indexEnd = indexStart + quad->mIndicesByLODDiff[LODDI_ALL].mTriangleCount*3;
			for(index_t i = indexStart; i < indexEnd; ++i)
			{
				const TERRAIN_POSITION_DATA_XZ& hpos = hposition[ indices[i] ];
				const TERRAIN_POSITION_DATA_Y& vpos = vposition[ indices[i] ];
				Vector3 vtx((scalar)hpos.getX(), vpos.getHeight(), (scalar)hpos.getZ());
				
				if( minX > vtx.x )
					minX = vtx.x;
				else if( maxX < vtx.x )
					maxX = vtx.x;

				if( minZ > vtx.z )
					minZ = vtx.z;
				//else if( maxZ < vtx.z )
				//	maxZ = vtx.z;

				if( mMinY > vtx.y )
					mMinY = vtx.y;
				if( mMaxY < vtx.y )
					mMaxY = vtx.y;
			}
			mX = (uint16)minX;
			mZ = (uint16)minZ;
			mSize = (uint16)(maxX - minX);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool			BlockQueryDataCompact::raycastPoint(const QueryIndexQuad* quad,SBlockRayQueryCompact& rayQuery)
	{
		scalar distance;
		bool result = false;
		scalar max_distance;
		AABB localAABB( (scalar)mX, mMinY.getFloat(), (scalar)mZ, (scalar)(mX+mSize), mMaxY.getFloat(), (scalar)(mZ+mSize));

		if( rayQuery.mLocalRay.intersect( localAABB,distance,max_distance) && distance < rayQuery.mDistance )
		{
			if(quad->mSubQuad != NULL )
			{

				for(size_t i = 0; i < 4; ++i)
				{
					if( rayQuery.mQueryDataPool[mSubLeafIndex+i].raycastPoint(&quad->mSubQuad[i],rayQuery) )
						//if result is true ,we don't stop
						//we need to check if there are more nearer one
						result = true;
				}
			}
			else
				//leaf : test all triangles to find the nearest
			{
				index_t indexStart = quad->mIndicesByLODDiff[ rayQuery.mDiffIndex ].mStartIndex;
				size_t indexEnd = indexStart + quad->mIndicesByLODDiff[ rayQuery.mDiffIndex ].mTriangleCount*3;
				for(index_t index = indexStart; index < indexEnd; index += 3)
				{
					const TERRAIN_POSITION_DATA_XZ& hpos0 = rayQuery.mHorizVertices[ rayQuery.mIndices[index] ];
					const TERRAIN_POSITION_DATA_XZ& hpos1 = rayQuery.mHorizVertices[ rayQuery.mIndices[index+1] ];
					const TERRAIN_POSITION_DATA_XZ& hpos2 = rayQuery.mHorizVertices[ rayQuery.mIndices[index+2] ];
					const TERRAIN_POSITION_DATA_Y& vpos0 = rayQuery.mVertVertices[ rayQuery.mIndices[index] ];
					const TERRAIN_POSITION_DATA_Y& vpos1 = rayQuery.mVertVertices[ rayQuery.mIndices[index+1] ];
					const TERRAIN_POSITION_DATA_Y& vpos2 = rayQuery.mVertVertices[ rayQuery.mIndices[index+2] ];

					Vector3 vtx0((scalar)hpos0.getX(), vpos0.getHeight(), (scalar)hpos0.getZ());
					Vector3 vtx1((scalar)hpos1.getX(), vpos1.getHeight(), (scalar)hpos1.getZ());
					Vector3 vtx2((scalar)hpos2.getX(), vpos2.getHeight(), (scalar)hpos2.getZ());

					if( rayQuery.mLocalRay.intersect(vtx0,vtx1,vtx2,distance,rayQuery.mSide) && distance < rayQuery.mDistance )
					{
						//if( rayQuery.mTriangle != NULL )
						{
							rayQuery.mTriangle[0] = vtx0;
							rayQuery.mTriangle[1] = vtx1;
							rayQuery.mTriangle[2] = vtx2;
						}

						result = true;
						//set the new limit to check if there's nearer triangle
						rayQuery.mDistance = distance;
					}
				}
			}
			return result;
		}
		else
			return false;
	}


	//////////////////////////////////////////////////////////////////////////
	bool			BlockQueryDataCompact::intersect(const QueryIndexQuad* quad, SBlockVolumeQueryCompact& qvolume)
	{
		AABB localAABB( (scalar)mX, mMinY.getFloat(), (scalar)mZ, (scalar)(mX+mSize), mMaxY.getFloat(), (scalar)(mZ+mSize) );
		scalar distance = qvolume.mQuery.getDistanceLimit();
		POS_VOL pos = qvolume.mQuery.intersect(localAABB, distance);
		if( pos == PV_OUTSIDE)
			return false;

		bool result = false;
		if( pos == PV_INSIDE || quad->mSubQuad == NULL )
			//add all triangles and stop recursion
		{
			//skip triangle intersection check, put it in result directly
			bool skipTriangle = PV_INSIDE || !qvolume.mQuery.needTriangles();
			if( skipTriangle )
				result = true;

			index_t indexStart = quad->mIndicesByLODDiff[ qvolume.mDiffIndex ].mStartIndex;
			size_t indexEnd = indexStart + quad->mIndicesByLODDiff[ qvolume.mDiffIndex ].mTriangleCount*3;
			for(index_t index = indexStart; index < indexEnd; index += 3)
			{
				const TERRAIN_POSITION_DATA_XZ& hpos0 = qvolume.mHorizVertices[ qvolume.mIndices[index] ];
				const TERRAIN_POSITION_DATA_XZ& hpos1 = qvolume.mHorizVertices[ qvolume.mIndices[index+1] ];
				const TERRAIN_POSITION_DATA_XZ& hpos2 = qvolume.mHorizVertices[ qvolume.mIndices[index+2] ];
				const TERRAIN_POSITION_DATA_Y& vpos0 = qvolume.mVertVertices[ qvolume.mIndices[index] ];
				const TERRAIN_POSITION_DATA_Y& vpos1 = qvolume.mVertVertices[ qvolume.mIndices[index+1] ];
				const TERRAIN_POSITION_DATA_Y& vpos2 = qvolume.mVertVertices[ qvolume.mIndices[index+2] ];

				Vector3 vtx0((scalar)hpos0.getX(), vpos0.getHeight(), (scalar)hpos0.getZ());
				Vector3 vtx1((scalar)hpos1.getX(), vpos1.getHeight(), (scalar)hpos1.getZ());
				Vector3 vtx2((scalar)hpos2.getX(), vpos2.getHeight(), (scalar)hpos2.getZ());

				vtx0 += qvolume.mTranslation;
				vtx1 += qvolume.mTranslation;
				vtx2 += qvolume.mTranslation;

				if( skipTriangle || qvolume.mQuery.intersect(vtx0,vtx1,vtx2,distance) )
				{
					qvolume.mQuery.addTriangle(vtx0,vtx1,vtx2);
					result = result || true;
				}
			}

		}
		else
		{
			for(size_t i = 0; i < 4; ++i)
			{
				if( qvolume.mQueryDataPool[mSubLeafIndex+i].intersect(&quad->mSubQuad[i],qvolume) )
					result = true;
			}
		}
		return result;
	}

	//////////////////////////////////////////////////////////////////////////
	void			BlockQueryDataCompact::updateHeight(BlockQueryDataCompact* pool,const QueryIndexQuad* quad,
		const TERRAIN_POSITION_DATA_Y* vposition, const TERRAIN_POSITION_DATA_XZ* hposition, const IndexBufferHelper& indices)
	{
		if(quad->mSubQuad != NULL )
		{
			mMinY = FLT_MAX;
			mMaxY = FLT_MIN;
			for(size_t i = 0; i < 4; ++i)
			{
				BlockQueryDataCompact& sub = pool[ mSubLeafIndex+i ];
				sub.updateHeight(pool, &quad->mSubQuad[i],vposition, hposition ,indices);

				if( mMinY > sub.mMinY )
					mMinY = sub.mMinY;
				if( mMaxY < sub.mMaxY )
					mMaxY = sub.mMaxY;
			}
		}
		else
		{
			mMinY = FLT_MAX;
			mMaxY = FLT_MIN;
			index_t indexStart = quad->mIndicesByLODDiff[LODDI_ALL].mStartIndex;
			size_t indexEnd = indexStart + quad->mIndicesByLODDiff[LODDI_ALL].mTriangleCount*3;
			for(index_t index = indexStart; index < indexEnd; ++index)
			{
				const TERRAIN_POSITION_DATA_Y& vpos = vposition[ indices[index] ];
				scalar height = vpos.getHeight();

				if( mMinY > height)
					mMinY = height;
				if( mMaxY < height)
					mMaxY = height;
			}
		}
	}


	/************************************************************************/
	/*                                                                      */
	/************************************************************************/


	//////////////////////////////////////////////////////////////////////////
	BlockQueryDataRootCompact::BlockQueryDataRootCompact()
		:mIndexQuad(NULL)
		,mQueryDataPool(NULL)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	BlockQueryDataRootCompact::~BlockQueryDataRootCompact()
	{
		BLADE_DELETE[] mQueryDataPool;
	}

	//////////////////////////////////////////////////////////////////////////
	void			BlockQueryDataRootCompact::initialize(const QueryIndexQuad* quad,
		const TERRAIN_POSITION_DATA_Y* vposition, const TERRAIN_POSITION_DATA_XZ* hposition, const IndexBufferHelper& indices)
	{
		mIndexQuad = quad;

		//could be null now
		//assert(quad->mSubQuad[0] != NULL );

		//calculate total leaf count
		const QueryIndexQuad* depthQuad = quad;
		size_t leafCount = 1;
		size_t totalCount = 0;
		for(;;)
		{
			depthQuad = &(depthQuad->mSubQuad[0]);
			if( depthQuad == NULL )
				break;

			leafCount *= 4;
			totalCount += leafCount;
		};

		mQueryDataPool = BLADE_NEW BlockQueryDataCompact[totalCount];

		uint16 leafIndex = 0;
		//mSubLeafIndex = leafIndex;
		//leafIndex += 4;
		//for(size_t i = 0; i < 4; ++i)
		//{
		//	mQueryDataPool[ mSubLeafIndex+i ].initialize(leafIndex,mQueryDataPool,quad->mSubQuad[i],vertices,indices);
		//}
		this->BlockQueryDataCompact::initialize(leafIndex,mQueryDataPool,quad,vposition, hposition, indices);

		assert(leafIndex == totalCount );
	}

	//////////////////////////////////////////////////////////////////////////
	bool			BlockQueryDataRootCompact::raycastPoint(SBlockRayQueryCompact& rayQuery)
	{
		//bool result = false;
		//rayQuery.mQueryDataPool = mQueryDataPool;

		//for(size_t i = 0; i < 4; ++i)
		//{
		//	if( mQueryDataPool[ mSubLeafIndex+i ].raycastPoint(mIndexQuad->mSubQuad[i],rayQuery) )
		//		result = true;
		//}

		//return result;

		rayQuery.mQueryDataPool = mQueryDataPool;
		return this->BlockQueryDataCompact::raycastPoint(mIndexQuad,rayQuery);
	}

	//////////////////////////////////////////////////////////////////////////
	bool			BlockQueryDataRootCompact::intersect(SBlockVolumeQueryCompact& qvolume, POS_VOL contentPos)
	{
		bool result = false;
		qvolume.mQueryDataPool = mQueryDataPool;

		if( contentPos == PV_INSIDE || mIndexQuad->mSubQuad == NULL )
			//add all triangles and stop recursion
		{
			if( contentPos == PV_INSIDE || !qvolume.mQuery.needTriangles() )
			{
				result = true;
				index_t indexStart = mIndexQuad->mIndicesByLODDiff[ qvolume.mDiffIndex ].mStartIndex;
				size_t indexEnd = indexStart + mIndexQuad->mIndicesByLODDiff[ qvolume.mDiffIndex ].mTriangleCount*3;
				for(index_t index = indexStart; index < indexEnd; index += 3)
				{
					const TERRAIN_POSITION_DATA_XZ& hpos0 = qvolume.mHorizVertices[ qvolume.mIndices[index] ];
					const TERRAIN_POSITION_DATA_XZ& hpos1 = qvolume.mHorizVertices[ qvolume.mIndices[index+1] ];
					const TERRAIN_POSITION_DATA_XZ& hpos2 = qvolume.mHorizVertices[ qvolume.mIndices[index+2] ];
					const TERRAIN_POSITION_DATA_Y& vpos0 = qvolume.mVertVertices[ qvolume.mIndices[index] ];
					const TERRAIN_POSITION_DATA_Y& vpos1 = qvolume.mVertVertices[ qvolume.mIndices[index+1] ];
					const TERRAIN_POSITION_DATA_Y& vpos2 = qvolume.mVertVertices[ qvolume.mIndices[index+2] ];

					Vector3 vtx0((scalar)hpos0.getX(), vpos0.getHeight(), (scalar)hpos0.getZ());
					Vector3 vtx1((scalar)hpos1.getX(), vpos1.getHeight(), (scalar)hpos1.getZ());
					Vector3 vtx2((scalar)hpos2.getX(), vpos2.getHeight(), (scalar)hpos2.getZ());

					vtx0 += qvolume.mTranslation;
					vtx1 += qvolume.mTranslation;
					vtx2 += qvolume.mTranslation;

					qvolume.mQuery.addTriangle(vtx0,vtx1,vtx2);
				}
			}
			else
			{
				index_t indexStart = mIndexQuad->mIndicesByLODDiff[ qvolume.mDiffIndex ].mStartIndex;
				size_t indexEnd = indexStart + mIndexQuad->mIndicesByLODDiff[ qvolume.mDiffIndex ].mTriangleCount*3;
				for(index_t index = indexStart; index < indexEnd; index += 3)
				{
					const TERRAIN_POSITION_DATA_XZ& hpos0 = qvolume.mHorizVertices[ qvolume.mIndices[index] ];
					const TERRAIN_POSITION_DATA_XZ& hpos1 = qvolume.mHorizVertices[ qvolume.mIndices[index+1] ];
					const TERRAIN_POSITION_DATA_XZ& hpos2 = qvolume.mHorizVertices[ qvolume.mIndices[index+2] ];
					const TERRAIN_POSITION_DATA_Y& vpos0 = qvolume.mVertVertices[ qvolume.mIndices[index] ];
					const TERRAIN_POSITION_DATA_Y& vpos1 = qvolume.mVertVertices[ qvolume.mIndices[index+1] ];
					const TERRAIN_POSITION_DATA_Y& vpos2 = qvolume.mVertVertices[ qvolume.mIndices[index+2] ];

					Vector3 vtx0((scalar)hpos0.getX(), vpos0.getHeight(), (scalar)hpos0.getZ());
					Vector3 vtx1((scalar)hpos1.getX(), vpos1.getHeight(), (scalar)hpos1.getZ());
					Vector3 vtx2((scalar)hpos2.getX(), vpos2.getHeight(), (scalar)hpos2.getZ());

					vtx0 += qvolume.mTranslation;
					vtx1 += qvolume.mTranslation;
					vtx2 += qvolume.mTranslation;
					scalar distance;
					if( qvolume.mQuery.intersect(vtx0,vtx1,vtx2, distance) )
					{
						result = true;
						qvolume.mQuery.addTriangle(vtx0,vtx1,vtx2);
					}
				}
			}

		}
		else
		{
			for(size_t i = 0; i < 4; ++i)
			{
				if( mQueryDataPool[ mSubLeafIndex+i ].intersect(&mIndexQuad->mSubQuad[i], qvolume) )
					result = true;
			}
		}
		return result;
	}

	//////////////////////////////////////////////////////////////////////////
	void			BlockQueryDataRootCompact::updateHeight(const TERRAIN_POSITION_DATA_Y* vposition, const TERRAIN_POSITION_DATA_XZ* hposition, const IndexBufferHelper& indices)
	{
		this->BlockQueryDataCompact::updateHeight(mQueryDataPool,mIndexQuad,vposition, hposition, indices);
	}
	

}//namespace Blade


