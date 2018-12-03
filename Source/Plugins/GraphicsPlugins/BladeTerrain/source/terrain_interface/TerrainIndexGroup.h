/********************************************************************
	created:	2011/05/31
	filename: 	IndexGroup.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_IndexGroup_h__
#define __Blade_IndexGroup_h__
#include <interface/public/graphics/IIndexBuffer.h>
#include <utility/BladeContainer.h>
#include <BladeTerrain.h>
#include <interface/ITerrainConfigManager.h>

namespace Blade
{
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class TerrainIndexGroup : public Allocatable, public NonCopyable, public NonAssignable
	{
	public:
		typedef FixedArray<HIBUFFER, LODDI_COUNT>	DiffBufferList;

		TerrainIndexGroup();
		~TerrainIndexGroup();

		/** @brief  */
		/* @param IndexStart [out] return the index start position in the index buffer */
		/* @param IndexCount [out] return the index count in the index buffer */
		inline const HIBUFFER&	getIndexBuffer(index_t LODLevel, LOD_DI diffIndex, size_t blockIndex, uint32& IndexStart, uint32& IndexCount) const
		{
#if BLADE_DEBUG
			size_t blocks = ITerrainConfigManager::getSingleton().getBlocksPerTile();
			if (LODLevel >= mBuffers.size() || diffIndex > 16 || blockIndex >= blocks)
			{
				assert(false);
				return HIBUFFER::EMPTY;
			}
#else
			BLADE_UNREFERENCED(blockIndex);
#endif
			const TerrainIndexGroup::DiffBufferList& DiffBuffer = mBuffers[LODLevel];
			const HIBUFFER& tibuffer = DiffBuffer[diffIndex];

			IndexStart = 0;
			IndexCount = 0;
			return tibuffer;
		}

	public:
		typedef Vector<TerrainIndexGroup::DiffBufferList>				LODBuffer;

		LODBuffer	mBuffers;
		RefCount	mCount;
	};//class TerrainIndexGroup
	
	
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	struct SQueryIndices
	{
		//record the position and count in software index buffer
		//start position in query index buffer
		uint16	mStartIndex;
		//triangle count(index count/3)
		uint16	mTriangleCount;
	};//struct SQueryIndices

	class QueryIndexQuad : public Allocatable
	{
	public:
		QueryIndexQuad()	{}
		~QueryIndexQuad()	
		{
			BLADE_DELETE[] mSubQuad;
		}

		QueryIndexQuad*	mSubQuad;
		SQueryIndices	mIndicesByLODDiff[LODDI_COUNT];	//LOD difference
	};//class QueryIndexQuad

	class TerrainQueryIndexGroup : public TerrainIndexGroup
	{
	public:
		TerrainQueryIndexGroup(index_t LODLevel);
		~TerrainQueryIndexGroup();

		/** @brief  */
		const QueryIndexQuad*	getQueryIndexbyLOD(index_t LODLevel) const;

	public:

		typedef Vector<QueryIndexQuad*>				LODQueryList;
		LODQueryList	mQueryQuads;

	protected:
		/** @brief  */
		void		initQuad(QueryIndexQuad* quad,index_t depth);
	};//class TerrainQueryIndexGroup

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	///Fixed LOD may have neighbors whose LOD diff > 1
	struct FixedLODDiff
	{
		union
		{
			struct
			{
				int8 l;
				int8 t;
				int8 r;
				int8 b;
			};
			int32 v;
		};


		FixedLODDiff(int8 _l, int8 _t, int8 _r, int8 _b) :l(_l), t(_t), r(_r), b(_b) {}
		FixedLODDiff() { v = 0; }

		inline bool operator<(const FixedLODDiff& rhs) const
		{
			return v < rhs.v;
		}

		inline bool isValid() const { return std::abs(l) >= 1 || std::abs(t) >= 1 || std::abs(r) >= 1 || std::abs(b) >= 1; }
	};

	///index group for fixed LOD blocks, used to weld seams for fixed LOD (flat terrain or cliff)
	class TerrainFixedIndexGroup : public Allocatable
	{
	public:
		typedef Map<FixedLODDiff, HIBUFFER> LODBuffer;
		TerrainFixedIndexGroup() {}
		~TerrainFixedIndexGroup() {}

		/** @brief  */
		inline const HIBUFFER&	getIndexBuffer(ITerrainConfigManager& itcm, index_t LODLevel, FixedLODDiff diff, size_t blockIndex, uint32& IndexStart, uint32& IndexCount) const
		{
#if BLADE_DEBUG
			size_t blocks = itcm.getBlocksPerTile();
			if ((LODLevel != itcm.getCliffLODLevel() && LODLevel != itcm.getFlatLODLevel())
				|| blockIndex >= blocks)
			{
				assert(false);
				return HIBUFFER::EMPTY;
			}
#else
			BLADE_UNREFERENCED(blockIndex);
#endif
			const LODBuffer& buffer = (LODLevel == itcm.getCliffLODLevel()) ? mCliffBuffer : mFlatBuffer;
			IndexStart = 0;
			IndexCount = 0;

			LODBuffer::const_iterator i = buffer.find(diff);
			if (i != buffer.end())
				return i->second;
			return HIBUFFER::EMPTY;
		}

		LODBuffer	mCliffBuffer;
		LODBuffer	mFlatBuffer;
		RefCount	mCount;
	};

}//namespace Blade



#endif // __Blade_IndexGroup_h__