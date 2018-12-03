/********************************************************************
	created:	2011/06/02
	filename: 	BlockQueryDataCompact.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_BlockQueryDataCompact_h__
#define __Blade_BlockQueryDataCompact_h__
#include <math/Half.h>
#include <math/BladeMath.h>
#include <math/AxisAlignedBox.h>
#include <math/Ray.h>
#include <interface/public/graphics/SpaceQuery.h>
#include "terrain_interface/ITerrainTile.h"

namespace Blade
{

	class BlockQueryDataCompact;

	struct SBlockRayQueryCompact
	{
		SBlockRayQueryCompact(const TERRAIN_POSITION_DATA_Y* vposition, const TERRAIN_POSITION_DATA_XZ* hposition,const IndexBufferHelper& indices,LOD_DI diffIdx,
			const Ray& localray,POS_SIDE side,scalar limit)
			:mHorizVertices(hposition)
			,mVertVertices(vposition)
			,mIndices(indices)
			,mDiffIndex(diffIdx)
			,mLocalRay(localray),mSide(side),mDistance(limit)
		{
			mTriangle[0] = mTriangle[1] = mTriangle[2] = POINT3::ZERO;
		}

		const TERRAIN_POSITION_DATA_XZ*	mHorizVertices;
		const TERRAIN_POSITION_DATA_Y*	mVertVertices;
		const IndexBufferHelper&		mIndices;
		LOD_DI		mDiffIndex;
		Ray			mLocalRay;
		POS_SIDE	mSide;		//valid side : both,positive,negative
		scalar		mDistance;
		POINT3		mTriangle[3];
		BlockQueryDataCompact*	mQueryDataPool;
	private:
		SBlockRayQueryCompact& operator=(const SBlockRayQueryCompact&);
	};

	struct SBlockVolumeQueryCompact
	{
		SBlockVolumeQueryCompact(const TERRAIN_POSITION_DATA_Y* vposition, const TERRAIN_POSITION_DATA_XZ* hposition,const IndexBufferHelper& indices,LOD_DI diffIdx,
			const Vector3& translation, SpaceQuery& query)
			:mHorizVertices(hposition)
			,mVertVertices(vposition)
			,mIndices(indices)
			,mDiffIndex(diffIdx)
			,mTranslation(translation)
			,mQuery(query)	{}

		const TERRAIN_POSITION_DATA_XZ*	mHorizVertices;
		const TERRAIN_POSITION_DATA_Y*	mVertVertices;
		const IndexBufferHelper&		mIndices;
		LOD_DI							mDiffIndex;
		const Vector3&					mTranslation;
		SpaceQuery&						mQuery;
		BlockQueryDataCompact*			mQueryDataPool;
	private:
		SBlockVolumeQueryCompact&	operator=(const SBlockVolumeQueryCompact&);
	};


	//using simple quad tree to boost query process
	class BlockQueryDataCompact : public ResourceAllocatable
	{
	public:
		BlockQueryDataCompact();
		~BlockQueryDataCompact();

		/** @brief  */
		void			initialize(uint16& index,BlockQueryDataCompact* pool,const QueryIndexQuad* quad,
			const TERRAIN_POSITION_DATA_Y* vposition, const TERRAIN_POSITION_DATA_XZ* hposition, const IndexBufferHelper& indices);

		/** @brief  */
		bool			raycastPoint(const QueryIndexQuad* quad,SBlockRayQueryCompact& rayQuery);

		/** @brief  */
		bool			intersect(const QueryIndexQuad* quad,SBlockVolumeQueryCompact& qvolume);

		/** @brief  */
		void			updateHeight(BlockQueryDataCompact* pool,const QueryIndexQuad* quad,
			const TERRAIN_POSITION_DATA_Y* vposition, const TERRAIN_POSITION_DATA_XZ* hposition, const IndexBufferHelper& indices);

	protected:
		//note there's no empty sub tree, except the leaf
		uint16		mSubLeafIndex;
		uint16		mX;
		uint16		mZ;
		uint16		mSize;
		fp16		mMinY;
		fp16		mMaxY;
	};//class BlockQueryData
	static_assert( sizeof(BlockQueryDataCompact) == 12, "size/alignment error");

	class BlockQueryDataRootCompact : public BlockQueryDataCompact
	{
	public:

		BlockQueryDataRootCompact();
		~BlockQueryDataRootCompact();

		/** @brief  */
		void			initialize(const QueryIndexQuad* quad,
			const TERRAIN_POSITION_DATA_Y* vposition, const TERRAIN_POSITION_DATA_XZ* hposition, const IndexBufferHelper& indices);

		/** @brief  */
		bool			raycastPoint(SBlockRayQueryCompact& rayQuery);

		/** @brief  */
		bool			intersect(SBlockVolumeQueryCompact& qvolume, POS_VOL contentPos);

		/** @brief  */
		void			updateHeight(const TERRAIN_POSITION_DATA_Y* vposition, const TERRAIN_POSITION_DATA_XZ* hposition, const IndexBufferHelper& indices);

	protected:
		const QueryIndexQuad*	mIndexQuad;
		BlockQueryDataCompact*	mQueryDataPool;
	};
	

}//namespace Blade



#endif // __Blade_BlockQueryDataCompact_h__