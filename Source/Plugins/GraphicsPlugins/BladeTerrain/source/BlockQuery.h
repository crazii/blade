/********************************************************************
	created:	2014/11/06
	filename: 	BlockQuery.h
	author:		Crazii
	purpose:	terrain block query without extra data to save memory.
				use marching steps for ray cast query
				use point intersection for volume query (ranged points by volume AAB)
*********************************************************************/
#ifndef __Blade_BlockQuery_h__
#define __Blade_BlockQuery_h__
#include <math/Half.h>
#include <math/BladeMath.h>
#include <math/AxisAlignedBox.h>
#include <math/Ray.h>
#include <math/Matrix44.h>
#include <interface/public/graphics/SpaceQuery.h>
#include "terrain_interface/ITerrainTile.h"

namespace Blade
{
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	struct BlockVolumeQuery : public NonAssignable
	{
	public:
		BlockVolumeQuery(const Vector3& translation, SpaceQuery& query, uint8 LOD, LOD_DI loddif)
			:mTranslation(translation)
			,mQuery(query)
			,mTargetLOD(LOD)
			,mDiffIndex(loddif)
		{

		}
		~BlockVolumeQuery()	{}

		const Vector3&		mTranslation;
		SpaceQuery&			mQuery;
		scalar				mDistance;
		const uint8			mTargetLOD;
		const LOD_DI		mDiffIndex;
	};

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class BlockQuery : public Allocatable
	{
	public:
		BlockQuery()
			:mHeight(NULL)
			,mHorizontalPos(NULL)
		{
			mMinY = -FLT_MAX;
			mMaxY = FLT_MAX;
		}

		~BlockQuery() {}

		/** @brief  */
		void			initialize(const TERRAIN_POSITION_DATA_Y* vposition, const TERRAIN_POSITION_DATA_XZ* hposition, 
			const TerrainQueryIndexGroup* queryIndex, index_t blockX, index_t blockZ)
		{
			mHeight = vposition;
			mHorizontalPos = hposition;
			mQueryIndex = queryIndex;
			mBlockX = (scalar)blockX;
			mBlockZ = (scalar)blockZ;
		}

		/** @brief  */
		void	setAABB(const AABB& aab)
		{
			mMinY = aab.getMinPoint().y;
			mMaxY = aab.getMaxPoint().y;
		}

		/** @brief  */
		bool	raycastPoint(BlockVolumeQuery& rayQuery) const;

		/** @brief  */
		bool	intersect(BlockVolumeQuery& qvolume, POS_VOL contentPos) const;

	protected:

		/** @brief  */
		scalar	getInterpolatedHeight(int x, int z) const;


		class RayCastContext : public NonAssignable
		{
		public:
			const  IndexBufferHelper&	indices;
			BlockVolumeQuery&			query;
			const QueryIndexQuad*		quad;
			scalar						x;
			scalar						z;
			scalar						size;

			inline RayCastContext(const  IndexBufferHelper& ih, BlockVolumeQuery& q)
				:indices(ih), query(q)	{quad = NULL;}

			inline void	modify(const QueryIndexQuad* qd, scalar _x, scalar _z, scalar _size)
			{
				assert(qd != NULL); quad = qd; x = _x; z = _z; size = _size;
			}
		};

		/** @brief  */
		bool	raycastPointRecursive(RayCastContext& context) const;


		class  VolumeQueryConext : public NonAssignable
		{
		public:
			const IndexBufferHelper&	indices;
			BlockVolumeQuery&			query;
			const QueryIndexQuad*		quad;
			scalar						x;
			scalar						z;
			scalar						size;

			inline VolumeQueryConext(const  IndexBufferHelper& ih, BlockVolumeQuery& q)
				:indices(ih), query(q)	{quad = NULL;}

			inline void	modify(const QueryIndexQuad* qd, scalar _x, scalar _z, scalar _size)
			{
				assert(qd != NULL); quad = qd; x = _x; z = _z; size = _size;
			}
		};

		/** @brief  */
		bool	volumeQueryRecurisve(VolumeQueryConext& context, POS_VOL contentPos) const;


		const TERRAIN_POSITION_DATA_Y*	mHeight;
		const TERRAIN_POSITION_DATA_XZ*	mHorizontalPos;
		const TerrainQueryIndexGroup*	mQueryIndex;

		scalar				mBlockX;
		scalar				mBlockZ;
		scalar				mMinY;
		scalar				mMaxY;
	};

}//namespace Blade


#endif//__Blade_BlockQuery_h__
