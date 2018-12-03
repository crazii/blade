/********************************************************************
	created:	2013/10/13
	filename: 	PreciseIndexGenerator.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "PreciseIndexGenerator.h"
#include <interface/public/graphics/IGraphicsResourceManager.h>
#include "../TerrainBufferManager.h"


namespace Blade
{
	
	//////////////////////////////////////////////////////////////////////////
	TerrainIndexGroup*	PreciseIndexGenerator::createTileIndexBuffer()
	{
		size_t BlockSize = TerrainConfigManager::getSingleton().getTerrainBlockSize();
		size_t MaxLod = TerrainConfigManager::getSingleton().getMaxLODLevel();
		size_t CurLod = TerrainConfigManager::getSingleton().getLODLevel();
		IGraphicsResourceManager* GResMan = TerrainConfigManager::getSingleton().getBatchCombiner()->getResourceManager();
		IGraphicsBuffer::USAGE GBU = TerrainConfigManager::getSingleton().getBatchCombiner()->getIndexBufferUsage();
		size_t MaxBlockIndexCount = 0;

		TerrainIndexGroup* group = BLADE_NEW TerrainIndexGroup();
		group->mBuffers.resize(MaxLod+1,TerrainIndexGroup::DiffBufferList());
		mMaxIndexCount.resize(MaxLod+1);

		size_t IndexCount	= BlockSize*BlockSize*6;
		uint32 IndexStride	= 1;
		//const size_t Size	= TerrainSize + TerrainSize/BlockSize;
		const size_t Size	= BlockSize + 1;
		const size_t VertexCount	= (Size)*(Size);

		//get index type
		IIndexBuffer::EIndexType indexType = IndexBufferHelper::calcIndexType(VertexCount);

		for( size_t i = 0; i <= CurLod; ++i )
		{
			TerrainIndexGroup::DiffBufferList& LevelBuffer = group->mBuffers[i];

			// 2 triangles
			if( i == MaxLod )
			{
				assert( IndexCount == 6 );

				HIBUFFER& indexbuffer = LevelBuffer[0];

				void* ibData = BLADE_TMP_ALLOC(IndexBufferHelper::calcIndexSize(indexType)* IndexCount);
				IndexBufferHelper ibuffer(ibData, indexType);

				uint32 p1 = (uint32)(BlockSize*Size);
				uint32 p2 = p1+(uint32)BlockSize;
				uint32 p3 = (uint32)BlockSize;

				//		0		p3
				//		+-------+
				//		| \     |
				//		|  \   	|
				//		|   \   |
				//		|    \	|
				//		|  	  \	|
				//  	+-------+
				//		p1		p2

				ibuffer[0] = 0;
				ibuffer[1] = p1;
				ibuffer[2] = p2;

				ibuffer[3] = 0;
				ibuffer[4] = p2;
				ibuffer[5] = p3;

				indexbuffer = GResMan->createIndexBuffer(ibData, indexType, IndexCount, GBU);
				BLADE_TMP_FREE(ibData);
			}
			else
			{
				//build difference adaptive buffer
				for( index_t diffIdx = 0; diffIdx < 15; ++diffIdx )
				{
					// level default buffer
					HIBUFFER& indexbuffer = LevelBuffer[diffIdx];
					LODDiff diff = LODDiff::generateLODDifference((LOD_DI)diffIdx);
					size_t diffCount = diff.getDiffSideCount();

					//remove LOD index
					size_t CurIndexCount =  IndexCount - diffCount*3*( (BlockSize/IndexStride)/2 );
					if( CurIndexCount > MaxBlockIndexCount )
						MaxBlockIndexCount = CurIndexCount;
					if( CurIndexCount > mMaxIndexCount[i] )
						mMaxIndexCount[i] = CurIndexCount;

					//TotalIndexCount = CurIndexCount;
					void* ibData = BLADE_TMP_ALLOC(IndexBufferHelper::calcIndexSize(indexType)* CurIndexCount);
					IndexBufferHelper ibuffer(ibData, indexType);
					index_t	IndexSub		= 0;
					const uint32 RowStride	= (uint32)Size*IndexStride*2;
					const uint32 ColStride	= IndexStride*2;
					const uint32 RowCount	= (uint32)(BlockSize/IndexStride)/2;
					const uint32 ColCount	= (uint32)(BlockSize/IndexStride)/2;
					uint32	RowA			= 0;
					uint32	RowB			= RowStride/2;
					uint32	RowC			= RowStride;
					uint32	Col0			= 0;
					uint32	Col1			= IndexStride;
					uint32	Col2			= IndexStride*2;

					//
					for( uint32 z = 0; z < RowCount; ++z)
					{
						for( uint32 x = 0; x < ColCount; ++x)
						{
							//		x   x+1
							//		0	1	2
							//A		+---+---+
							//		| \ |  /|
							//		|  \| /	|
							//B		+---+---+
							//		|  /|\	|
							//		| /	| \	|
							//C		+---+---+

							if( diff.hasLevelDifference() )
							{
								assert( i != MaxLod );
								assert( BlockSize/IndexStride > 1 );
							}

							if( diff.isUpDiff() && z == 0 )
							{
								//remove up triangle
								//0A,1B,2A
								ibuffer[IndexSub++] = RowA + Col0;
								ibuffer[IndexSub++] = RowB + Col1;
								ibuffer[IndexSub++] = RowA + Col2;
							}
							else
							{
								//0A,1B,1A
								ibuffer[IndexSub++] = RowA + Col0;
								ibuffer[IndexSub++] = RowB + Col1;
								ibuffer[IndexSub++] = RowA + Col1;
								//1A,1B,2A
								ibuffer[IndexSub++] = RowA + Col1;
								ibuffer[IndexSub++] = RowB + Col1;
								ibuffer[IndexSub++] = RowA + Col2;
							}

							if( diff.isLeftDiff() && x == 0 )
							{
								//0A,0C,1B
								ibuffer[IndexSub++] = RowA + Col0;
								ibuffer[IndexSub++] = RowC + Col0;
								ibuffer[IndexSub++] = RowB + Col1;
							}
							else
							{
								//0A,0B,1B
								ibuffer[IndexSub++] = RowA + Col0;
								ibuffer[IndexSub++] = RowB + Col0;
								ibuffer[IndexSub++] = RowB + Col1;
								//0B,0C,1B
								ibuffer[IndexSub++] = RowB + Col0;
								ibuffer[IndexSub++] = RowC + Col0;
								ibuffer[IndexSub++] = RowB + Col1;
							}

							if( diff.isDownDiff() && z == RowCount -1 )
							{
								//1B,0C,2C
								ibuffer[IndexSub++] = RowB + Col1;
								ibuffer[IndexSub++] = RowC + Col0;
								ibuffer[IndexSub++] = RowC + Col2;
							}
							else
							{
								//1B,0C,1C
								ibuffer[IndexSub++] = RowB + Col1;
								ibuffer[IndexSub++] = RowC + Col0;
								ibuffer[IndexSub++] = RowC + Col1;
								//1B,1C,2C
								ibuffer[IndexSub++] = RowB + Col1;
								ibuffer[IndexSub++] = RowC + Col1;
								ibuffer[IndexSub++] = RowC + Col2;
							}

							if( diff.isRightDiff() && x == ColCount -1 )
							{
								//2A,1B,2C
								ibuffer[IndexSub++] = RowA + Col2;
								ibuffer[IndexSub++] = RowB + Col1;
								ibuffer[IndexSub++] = RowC + Col2;
							}
							else
							{
								//2A,1B,2B
								ibuffer[IndexSub++] = RowA + Col2;
								ibuffer[IndexSub++] = RowB + Col1;
								ibuffer[IndexSub++] = RowB + Col2;
								//1B,2C,2B
								ibuffer[IndexSub++] = RowB + Col1;
								ibuffer[IndexSub++] = RowC + Col2;
								ibuffer[IndexSub++] = RowB + Col2;
							}

							Col0 += ColStride;
							Col1 += ColStride;
							Col2 += ColStride;
						}

						//clear col index
						Col0			= 0;
						Col1			= IndexStride;
						Col2			= IndexStride*2;

						//move to next row
						RowA += RowStride;
						RowB +=	RowStride;
						RowC +=	RowStride;

					}//for each block
					assert( IndexSub == CurIndexCount );

					indexbuffer = GResMan->createIndexBuffer(ibData, indexType, CurIndexCount, GBU);
					BLADE_TMP_FREE(ibData);
				}// for each diff level

			}////build difference adaptive buffer

			if( i > 0 )
				group->mBuffers[i-1][15] = LevelBuffer[0];

			IndexCount /= 4;
			IndexStride *= 2;

		}//for each LOD level

		//
		TERRAIN_INFO info = TerrainConfigManager::getSingleton().getTerrainInfo();
		info.mMaxBlockIndexCount = MaxBlockIndexCount;
		TerrainConfigManager::getSingleton().updateGlobalTerrainInfo(info);

		return group;
	}

	//////////////////////////////////////////////////////////////////////////
	TerrainQueryIndexGroup*	PreciseIndexGenerator::createBlockQueryIndexBuffer()
	{
		size_t	BlockSize = TerrainConfigManager::getSingleton().getTerrainBlockSize();
		size_t	MaxLod = TerrainConfigManager::getSingleton().getMaxLODLevel();
		//size_t	CurLod = TerrainConfigManager::getSingleton().getLODLevel();
		IGraphicsResourceManager* GResMan = IGraphicsResourceManager::getOtherSingletonPtr(BTString("Software"));

		TerrainQueryIndexGroup* group = BLADE_NEW TerrainQueryIndexGroup(MaxLod);
		group->mBuffers.resize(MaxLod+1,TerrainIndexGroup::DiffBufferList());

		size_t IndexCount	= BlockSize*BlockSize*6;
		uint32 IndexStride	= 1;
		const size_t Size	= BlockSize + 1;
		const size_t VertexCount	= (Size)*(Size);

		//get index type
		IIndexBuffer::EIndexType indexType = IndexBufferHelper::calcIndexType(VertexCount);

		//the top 2 level is simple and could use the Hardware index buffer instead,
		//so we don't build query buffer for the highest 2 levels
		for( size_t i = 0; i < MaxLod; ++i )
		{
			TerrainIndexGroup::DiffBufferList& LevelBuffer = group->mBuffers[i];

			//build difference adaptive buffer
			for( index_t diffIdx = 0; diffIdx < 16; ++diffIdx )
			{
				// level default buffer
				HIBUFFER& indexbuffer = LevelBuffer[diffIdx];

				LODDiff diff = LODDiff::generateLODDifference((LOD_DI)diffIdx);

				size_t diffCount = diff.getDiffSideCount();

				//remove index for LOD adaption
				size_t CurIndexCount =  IndexCount - diffCount*3*( (BlockSize/IndexStride)/2 );

				void* ibData = BLADE_TMP_ALLOC(IndexBufferHelper::calcIndexSize(indexType)* CurIndexCount);
				IndexBufferHelper ibuffer(ibData, indexType);
				const uint32 RowStride	= (uint32)Size*IndexStride*2;
				const uint32 ColStride	= IndexStride*2;
				const uint32 RowCount	= (uint32)(BlockSize/IndexStride)/2;
				const uint32 ColCount	= (uint32)(BlockSize/IndexStride)/2;

				SQueryIndexData data(ibuffer);
				data.LODLevel = i;
				data.ColCount = ColCount;
				data.RowCount = RowCount;
				data.RowStride = RowStride;
				data.ColStride = ColStride;
				data.Diff = diff;
				data.DiffIndex = (LOD_DI)diffIdx;
				data.IndexCount = 0;

#ifdef DEBUG_DEPTH
				this->buildQueryIndex(MaxLod-i,0,0,RowCount,group->mQueryQuads[i],data);
#else
				this->buildQueryIndex(0,0,RowCount,group->mQueryQuads[i],data);
#endif

				assert( data.IndexCount == CurIndexCount );

				indexbuffer = GResMan->createIndexBuffer(ibData, indexType, CurIndexCount, IGraphicsBuffer::GBU_STATIC);
				BLADE_TMP_FREE(ibData);
			}////build difference adaptive buffer

			IndexCount /= 4;
			IndexStride *= 2;

		}//for each LOD level


		//manual build for max LOD
		{
			TerrainIndexGroup::DiffBufferList& LevelBuffer = group->mBuffers[MaxLod];
			HIBUFFER& indexbuffer = LevelBuffer[0];
			mMaxIndexCount[0] = 6;

			void* ibData = BLADE_TMP_ALLOC(IndexBufferHelper::calcIndexSize(indexType)* 6);
			IndexBufferHelper ibuffer(ibData, indexType);
			uint32 p1 = (uint32)(BlockSize*Size);
			uint32 p2 = p1+(uint32)BlockSize;
			uint32 p3 = (uint32)BlockSize;
			//		0		p3
			//		+-------+
			//		| \     |
			//		|  \   	|
			//		|   \   |
			//		|    \	|
			//		|  	  \	|
			//  	+-------+
			//		p1		p2

			ibuffer[0] = 0;
			ibuffer[1] = p1;
			ibuffer[2] = p2;

			ibuffer[3] = 0;
			ibuffer[4] = p2;
			ibuffer[5] = p3;

			indexbuffer = GResMan->createIndexBuffer(ibData, indexType, 6, IGraphicsBuffer::GBU_STATIC);
			BLADE_TMP_FREE(ibData);
			for( index_t i = 1; i < 16; ++i)
				LevelBuffer[i] = LevelBuffer[0];

			for( index_t diffIdx = 0; diffIdx < 16; ++diffIdx )
			{
				group->mQueryQuads[MaxLod]->mIndicesByLODDiff[diffIdx].mStartIndex = 0;
				group->mQueryQuads[MaxLod]->mIndicesByLODDiff[diffIdx].mTriangleCount = 2;
			}
		}

		return group;
	}

	//////////////////////////////////////////////////////////////////////////
	TerrainFixedIndexGroup*	PreciseIndexGenerator::createFixedIndexBuffer()
	{
		TerrainConfigManager& tcm = TerrainConfigManager::getSingleton();
		ILODComparator* cmp = this;
		size_t flatLOD = tcm.getFlatLODLevel();
		size_t cliffLOD = tcm.getCliffLODLevel();

		size_t LODs[] =
		{
			cliffLOD,
			flatLOD,
		};

		size_t	BlockSize = tcm.getTerrainBlockSize();
		const size_t Size = BlockSize + 1;
		const size_t VertexCount = (Size)*(Size);
		IIndexBuffer::EIndexType indexType = IndexBufferHelper::calcIndexType(VertexCount);

		IGraphicsResourceManager* GResMan = tcm.getBatchCombiner()->getResourceManager();
		IGraphicsBuffer::USAGE GBU = TerrainConfigManager::getSingleton().getBatchCombiner()->getIndexBufferUsage();
		TerrainFixedIndexGroup* group = BLADE_NEW TerrainFixedIndexGroup();

		index_t curLOD = tcm.getLODLevel();
		index_t maxLOD = tcm.getMaxLODLevel();

		for (size_t i = 0; i < countOf(LODs); ++i)
		{
			index_t LOD = LODs[i];
			assert(LOD > 0 && LOD <= curLOD);
			TerrainFixedIndexGroup::LODBuffer& buffer = (LOD == tcm.getCliffLODLevel()) ? group->mCliffBuffer : group->mFlatBuffer;

			size_t maxDiff = (size_t)std::max(std::abs(int(0 - LOD)), std::abs(int(curLOD - LOD)));
			size_t IndexCount = BlockSize * BlockSize * 6;
			uint32 IndexStride = 1u << std::min(LOD, maxLOD - 1);	//max LOD is 2 triangle2 that cannot break, use max LOD-1
			uint32 diffCount = uint32(4u * maxDiff);	//4 sides
			size_t maxIndexCount = IndexCount + diffCount * 6 * ((BlockSize / IndexStride) / 2);

			for (index_t l = 0; l <= curLOD; ++l)
			{
				for (index_t t = 0; t <= curLOD; ++t)
				{
					for (index_t r = 0; r <= curLOD; ++r)
					{
						for (index_t b = 0; b <= curLOD; ++b)
						{
							indexdiff_t ldiff = cmp->compareLOD(l, LOD);
							indexdiff_t tdiff = cmp->compareLOD(t, LOD);
							indexdiff_t rdiff = cmp->compareLOD(r, LOD);
							indexdiff_t bdiff = cmp->compareLOD(b, LOD);

							FixedLODDiff diff((int8)ldiff, (int8)tdiff, (int8)rdiff, (int8)bdiff);
							if (!diff.isValid())
								continue;

							HIBUFFER& indexBuffer = buffer[diff];

							//maxLOD level is 2 triangle and share the same LOD connection as maxLOD-1
							if (LOD == maxLOD)
							{
								if(ldiff <= -1)
									ldiff += 1;
								if(tdiff <= -1)
									tdiff += 1;
								if(rdiff <= -1)
									rdiff += 1;
								if(bdiff <= -1)
									bdiff += 1;
							}

							TempBuffer tempIBuffer;
							tempIBuffer.reserve(maxIndexCount);	//reserve enough space
							IndexBufferHelper ibuffer(tempIBuffer.getData(), indexType);

							const uint32 RowStride = (uint32)Size*IndexStride * 2;
							const uint32 ColStride = IndexStride * 2;
							const uint32 RowCount = (uint32)(BlockSize / IndexStride) / 2;
							const uint32 ColCount = (uint32)(BlockSize / IndexStride) / 2;
							index_t	IndexSub = 0;

							if (LOD != maxLOD)
							{
								uint32	RowA = 0;
								uint32	RowB = RowStride / 2;
								uint32	RowC = RowStride;
								uint32	Col0 = 0;
								uint32	Col1 = ColStride / 2;
								uint32	Col2 = ColStride;

								//		x   x+1
								//		0	1	2
								//A		+-------+
								//		| \    /|
								//		|  \  /	|
								//B		|   +   |
								//		|  / \	|
								//		| /	  \	|
								//C		+-------+

								for (uint32 z = 0; z < RowCount; ++z)
								{
									for (uint32 x = 0; x < ColCount; ++x)
									{
										//0A,1B,2A
										if ((z != 0 || diff.t == 0)
											&& (x != 0 || diff.l < 1) && (x != ColCount - 1 || diff.r < 1) && (z != RowCount - 1 || diff.b < 1))
										{
											//0A,1B,1A
											ibuffer[IndexSub++] = RowA + Col0;
											ibuffer[IndexSub++] = RowB + Col1;
											ibuffer[IndexSub++] = RowA + Col1;
											//1A,1B,2A
											ibuffer[IndexSub++] = RowA + Col1;
											ibuffer[IndexSub++] = RowB + Col1;
											ibuffer[IndexSub++] = RowA + Col2;
										}
										//0A,0C,1B
										if ((x != 0 || diff.l == 0)
											&& (z != 0 || diff.t < 1) && (z != RowCount - 1 || diff.b < 1) && (x != ColCount - 1 || diff.r < 1))
										{
											//0A,0B,1B
											ibuffer[IndexSub++] = RowA + Col0;
											ibuffer[IndexSub++] = RowB + Col0;
											ibuffer[IndexSub++] = RowB + Col1;
											//0B,0C,1B
											ibuffer[IndexSub++] = RowB + Col0;
											ibuffer[IndexSub++] = RowC + Col0;
											ibuffer[IndexSub++] = RowB + Col1;
										}
										//1B,0C,2C
										if ((z != RowCount - 1 || diff.b == 0)
											&& (x != 0 || diff.l < 1) && (x != ColCount - 1 || diff.r < 1) && (z != 0 || diff.t < 1))
										{
											//1B,0C,1C
											ibuffer[IndexSub++] = RowB + Col1;
											ibuffer[IndexSub++] = RowC + Col0;
											ibuffer[IndexSub++] = RowC + Col1;
											//1B,1C,2C
											ibuffer[IndexSub++] = RowB + Col1;
											ibuffer[IndexSub++] = RowC + Col1;
											ibuffer[IndexSub++] = RowC + Col2;
										}
										//2A,1B,2C
										if ((x != ColCount - 1 || diff.r == 0)
											&& (z != 0 || diff.t < 1) && (z != RowCount - 1 || diff.b < 1) && (x != 0 || diff.l < 1))
										{
											//2A,1B,2B
											ibuffer[IndexSub++] = RowA + Col2;
											ibuffer[IndexSub++] = RowB + Col1;
											ibuffer[IndexSub++] = RowB + Col2;
											//1B,2C,2B
											ibuffer[IndexSub++] = RowB + Col1;
											ibuffer[IndexSub++] = RowC + Col2;
											ibuffer[IndexSub++] = RowB + Col2;
										}
										Col0 += ColStride;
										Col1 += ColStride;
										Col2 += ColStride;
									}

									//clear col index
									Col0 = 0;
									Col1 = IndexStride;
									Col2 = IndexStride * 2;

									//move to next row
									RowA += RowStride;
									RowB += RowStride;
									RowC += RowStride;

								}//for inner triangles
							}

							//left side
							{
								uint32 leftRowStride = ldiff > 0 ? RowStride << (ldiff) : RowStride >> (-ldiff);

								if (ldiff <= 0)
								{
									leftRowStride /= 2;
									uint32	Col0 = 0;
									uint32	Col1 = ColStride / 2;

									if (LOD == maxLOD && ldiff == 0)
									{
										ibuffer[IndexSub++] = 0 + Col0;
										ibuffer[IndexSub++] = RowStride + Col0;
										ibuffer[IndexSub++] = RowStride/2 + Col1;
									}
									else
									for (uint32 z = (tdiff>=1 ? 1u : 0); z < (bdiff>=1 ? RowCount-1 : RowCount); ++z)
									{
										uint32 Row0 = z * RowStride;
										uint32 Row1 = Row0 + ((z == 0 && tdiff > 1) ? RowStride : ((z == RowCount - 1 && bdiff > 1) ? 0 : RowStride / 2));
										for (uint32 z1 = 0; z1 < (2u << -ldiff); ++z1)
										{
											if (ldiff != 0 || z == 0 || z == RowCount - 1)
											{
												ibuffer[IndexSub++] = Row0 + z1 * leftRowStride + Col0;
												ibuffer[IndexSub++] = Row0 + (z1 + 1) * leftRowStride + Col0;
												ibuffer[IndexSub++] = Row1 + Col1;
											}
										}
									}
								}
								else if (ldiff >= 1)
								{
									uint32 leftRowCount = RowCount >> (ldiff);
									bool minDetail = leftRowCount == 0;
									if (minDetail)//hard fix for max(min detail) LOD level (2 triangles)
									{
										leftRowCount = 1;
										leftRowStride = RowStride*RowCount;
									}

									uint32	Col0 = 0;
									uint32	Col1 = ColStride;

									for (uint32 n = 0; n < leftRowCount; ++n)
									{
										uint32 Row0 = n * leftRowStride;
										uint32 Row1 = Row0 + leftRowStride / 2;
										uint32 Row2 = Row0 + leftRowStride;

										for (uint32 z1 = (n == 0 && tdiff >= 1) ? RowStride : Row0; z1 < Row1; z1 += RowStride / 2)
										{
											ibuffer[IndexSub++] = Row0 + Col0;
											ibuffer[IndexSub++] = (z1 + RowStride / 2) + Col1;
											ibuffer[IndexSub++] = z1 + Col1;
										}

										for (uint32 z1 = (n == leftRowCount-1 && bdiff >= 1) ? (RowCount - 1)*RowStride : Row2; z1 > Row1; z1 -= RowStride / 2)
										{
											ibuffer[IndexSub++] = Row2 + Col0;
											ibuffer[IndexSub++] = z1 + Col1;
											ibuffer[IndexSub++] = (z1 - RowStride / 2) + Col1;
										}

										if (minDetail)
										{
											ibuffer[IndexSub++] = Row0 + Col0;
											ibuffer[IndexSub++] = Row2 + Col0;
											ibuffer[IndexSub++] = Row1 + Col1;
										}
										else
										{
											ibuffer[IndexSub++] = Row0 + Col0;
											ibuffer[IndexSub++] = Row1 + Col0;
											ibuffer[IndexSub++] = Row1 + Col1;

											ibuffer[IndexSub++] = Row1 + Col0;
											ibuffer[IndexSub++] = Row2 + Col0;
											ibuffer[IndexSub++] = Row1 + Col1;
										}
									}
								}
							}

							//top side
							{
								uint32 topColStride = tdiff > 0 ? ColStride << (tdiff) : ColStride >> (-tdiff);

								if (tdiff <= 0)
								{
									topColStride /= 2;
									uint32	Row0 = 0;
									uint32	Row1 = RowStride / 2;

									if (LOD == maxLOD && tdiff == 0)
									{
										ibuffer[IndexSub++] = Row0 + ColStride;
										ibuffer[IndexSub++] = Row0 + 0;
										ibuffer[IndexSub++] = Row1 + ColStride / 2;
									}
									else
									for (uint32 x = (ldiff>=1 ? 1u : 0); x < (rdiff>=1 ? ColCount -1 : ColCount); ++x)
									{
										uint32 Col0 = x * ColStride;
										uint32 Col1 = Col0 + ((x == 0 && ldiff > 1) ? ColStride : ((x == ColCount - 1 && rdiff > 1) ? 0 : ColStride / 2));
										for (uint32 x1 = 0; x1 < (2u << -tdiff); ++x1)
										{
											if (tdiff != 0 || x == 0 || x == ColCount - 1)
											{
												ibuffer[IndexSub++] = Row0 + Col0 + x1 * topColStride;
												ibuffer[IndexSub++] = Row1 + Col1;
												ibuffer[IndexSub++] = Row0 + Col0 + (x1 + 1) * topColStride;
											}
										}
									}
								}
								else if (tdiff >= 1)
								{
									uint32 topColCount = ColCount >> (tdiff);
									bool minDetial = topColCount == 0;
									if (minDetial)//hard fix for max LOD level (2 triangles)
									{
										topColCount = 1;
										topColStride = ColStride*ColCount;
									}

									uint32	Row0 = 0;
									uint32	Row1 = RowStride;
									
									for (uint32 n = 0; n < topColCount; ++n)
									{
										uint32 Col0 = n * topColStride;
										uint32 Col1 = Col0 + topColStride / 2;
										uint32 Col2 = Col0 + topColStride;

										for (uint32 x1 = (n == 0 && ldiff >= 1) ? ColStride : Col0; x1 < Col1; x1 += ColStride / 2)
										{
											ibuffer[IndexSub++] = Row0 + Col0;
											ibuffer[IndexSub++] = Row1 + x1;
											ibuffer[IndexSub++] = Row1 + x1 + ColStride / 2;
										}

										for (uint32 x1 = (n == topColCount-1 && rdiff >= 1) ? (ColCount - 1)*ColStride : Col2; x1 > Col1; x1 -= ColStride / 2)
										{
											ibuffer[IndexSub++] = Row0 + Col2;
											ibuffer[IndexSub++] = Row1 + x1 - ColStride / 2;
											ibuffer[IndexSub++] = Row1 + x1;
										}

										if (minDetial)
										{
											ibuffer[IndexSub++] = Row0 + Col0;
											ibuffer[IndexSub++] = Row1 + Col1;
											ibuffer[IndexSub++] = Row0 + Col2;
										}
										else
										{
											ibuffer[IndexSub++] = Row0 + Col0;
											ibuffer[IndexSub++] = Row1 + Col1;
											ibuffer[IndexSub++] = Row0 + Col1;

											ibuffer[IndexSub++] = Row0 + Col1;
											ibuffer[IndexSub++] = Row1 + Col1;
											ibuffer[IndexSub++] = Row0 + Col2;
										}
									}
								}
							}

							//right side (triangle winding order diff from left side)
							{
								uint32 rightRowStride = rdiff > 0 ? RowStride << (rdiff) : RowStride >> (-rdiff);

								if (rdiff <= 0)
								{
									rightRowStride /= 2;
									uint32	Col0 = (ColCount - 1) * ColStride + ColStride / 2;
									uint32	Col1 = Col0 + ColStride / 2;

									if (LOD == maxLOD && rdiff == 0)
									{
										ibuffer[IndexSub++] = RowStride + Col1;
										ibuffer[IndexSub++] = 0 + Col1;
										ibuffer[IndexSub++] = RowStride / 2 + Col0;
									}
									else
									for (uint32 z = (tdiff>=1 ? 1u : 0); z < (bdiff>=1 ? RowCount-1 : RowCount); ++z)
									{
										uint32 Row0 = z * RowStride;
										uint32 Row1 = Row0 + ((z == 0 && tdiff > 1) ? RowStride : ((z == RowCount - 1 && bdiff > 1) ? 0 : RowStride / 2));
										for (uint32 z1 = 0; z1 < (2u << -rdiff); ++z1)
										{
											if (rdiff != 0 || z == 0 || z == RowCount - 1)
											{
												ibuffer[IndexSub++] = Row0 + z1 * rightRowStride + Col1;
												ibuffer[IndexSub++] = Row1 + Col0;
												ibuffer[IndexSub++] = Row0 + (z1 + 1) * rightRowStride + Col1;
											}
										}
									}
								}
								else if (rdiff >= 1)
								{
									uint32 rightRowCount = RowCount >> (rdiff);
									bool minDetail = rightRowCount == 0;
									if (minDetail)//hard fix for max LOD level (2 triangles)
									{
										rightRowCount = 1;
										rightRowStride = RowStride*RowCount;
									}

									uint32	Col0 = (ColCount - 1) * ColStride;
									uint32	Col1 = Col0 + ColStride;

									for (uint32 n = 0; n < rightRowCount; ++n)
									{
										uint32 Row0 = n * rightRowStride;
										uint32 Row1 = Row0 + rightRowStride / 2;
										uint32 Row2 = Row0 + rightRowStride;

										for (uint32 z1 = (n == 0 && tdiff >= 1) ? RowStride : Row0; z1 < Row1; z1 += RowStride / 2)
										{
											ibuffer[IndexSub++] = Row0 + Col1;
											ibuffer[IndexSub++] = z1 + Col0;
											ibuffer[IndexSub++] = (z1 + RowStride / 2) + Col0;
										}

										for (uint32 z1 = (n == rightRowCount-1 && bdiff >= 1) ? (RowCount - 1)*RowStride : Row2; z1 > Row1; z1 -= RowStride / 2)
										{
											ibuffer[IndexSub++] = Row2 + Col1;
											ibuffer[IndexSub++] = z1 - RowStride / 2 + Col0;
											ibuffer[IndexSub++] = z1 + Col0;
										}

										if (minDetail)
										{
											ibuffer[IndexSub++] = Row0 + Col1;
											ibuffer[IndexSub++] = Row1 + Col0;
											ibuffer[IndexSub++] = Row2 + Col1;
										}
										else
										{
											ibuffer[IndexSub++] = Row0 + Col1;
											ibuffer[IndexSub++] = Row1 + Col0;
											ibuffer[IndexSub++] = Row1 + Col1;

											ibuffer[IndexSub++] = Row1 + Col1;
											ibuffer[IndexSub++] = Row1 + Col0;
											ibuffer[IndexSub++] = Row2 + Col1;
										}
									}
								}
							}

							//bottom side
							{
								uint32 bottomColStride = bdiff > 0 ? ColStride << (bdiff) : ColStride >> (-bdiff);

								if (bdiff <= 0)
								{
									bottomColStride /= 2;

									uint32	Row0 = (RowCount - 1) * RowStride + RowStride / 2;
									uint32	Row1 = Row0 + RowStride / 2;

									if (LOD == maxLOD && bdiff == 0)
									{
										ibuffer[IndexSub++] = Row1 + 0;
										ibuffer[IndexSub++] = Row1 + ColStride;
										ibuffer[IndexSub++] = Row0 + ColStride/2;
									}
									else
									for (uint32 x = (ldiff >=1 ? 1u : 0); x < (rdiff >=1 ? ColCount-1 : ColCount); ++x)
									{
										uint32 Col0 = x * ColStride;
										uint32 Col1 = Col0 + ((x == 0 && ldiff > 1) ? ColStride : ((x == ColCount-1 && rdiff > 1) ? 0 : ColStride / 2));
										for (uint32 x1 = 0; x1 < (2u << -bdiff); ++x1)
										{
											if (bdiff != 0 || x == 0 || x == ColCount - 1)
											{
												ibuffer[IndexSub++] = Row1 + Col0 + x1 * bottomColStride;
												ibuffer[IndexSub++] = Row1 + Col0 + (x1 + 1) * bottomColStride;
												ibuffer[IndexSub++] = Row0 + Col1;
											}
										}
									}
								}
								else if (bdiff >= 1)
								{
									uint32 bottomColCount = ColCount >> (bdiff);
									bool minDetail = bottomColCount == 0;
									if (minDetail)//hard fix for max LOD level (2 triangles)
									{
										bottomColCount = 1;
										bottomColStride = ColStride*ColCount;
									}

									uint32	Row0 = (RowCount - 1) * RowStride;
									uint32	Row1 = Row0 + RowStride;
									
									for (uint32 n = 0; n < bottomColCount; ++n)
									{
										uint32 Col0 = n * bottomColStride;
										uint32 Col1 = Col0 + bottomColStride / 2;
										uint32 Col2 = Col0 + bottomColStride;

										for (uint32 x1 = (n == 0 && ldiff >= 1) ? ColStride : Col0; x1 < Col1; x1 += ColStride / 2)
										{
											ibuffer[IndexSub++] = Row1 + Col0;
											ibuffer[IndexSub++] = Row0 + x1 + ColStride / 2;
											ibuffer[IndexSub++] = Row0 + x1;
										}

										for (uint32 x1 = (n == bottomColCount-1 && rdiff >= 1) ? (ColCount - 1)*ColStride : Col2; x1 > Col1; x1 -= ColStride / 2)
										{
											ibuffer[IndexSub++] = Row1 + Col2;
											ibuffer[IndexSub++] = Row0 + x1;
											ibuffer[IndexSub++] = Row0 + x1 - ColStride / 2;
										}

										if (minDetail)
										{
											ibuffer[IndexSub++] = Row1 + Col0;
											ibuffer[IndexSub++] = Row1 + Col2;
											ibuffer[IndexSub++] = Row0 + Col1;
										}
										else
										{
											ibuffer[IndexSub++] = Row1 + Col0;
											ibuffer[IndexSub++] = Row1 + Col1;
											ibuffer[IndexSub++] = Row0 + Col1;

											ibuffer[IndexSub++] = Row1 + Col1;
											ibuffer[IndexSub++] = Row1 + Col2;
											ibuffer[IndexSub++] = Row0 + Col1;
										}
									}
								}
							}

							assert(IndexSub <= maxIndexCount);
							indexBuffer = GResMan->createIndexBuffer(tempIBuffer.getData(), indexType, IndexSub, GBU);
						}// 
					}
				}
			}

		}
		return group;
	}

	//////////////////////////////////////////////////////////////////////////
	int16				PreciseIndexGenerator::getLowLODHeight(const int16* tileHeightaMap, size_t& LOD,
		index_t blockX, index_t blockZ, index_t x, index_t z) const
	{
		const size_t BlockSize = TerrainConfigManager::getSingleton().getTerrainBlockSize();
		const size_t TileSize = TerrainConfigManager::getSingleton().getTerrainTileSize();
		const size_t SourceSize = TileSize + 1;
		const int16* blockVPos = tileHeightaMap + (blockZ*BlockSize/*+z*/)*SourceSize + blockX*BlockSize/*+x*/;

		LOD = TerrainConfigManager::getSingleton().getMaxLODLevel()-1;
		if( (x == 0 || x == BlockSize ) && (z == 0 || z == BlockSize) )
			return blockVPos[z*SourceSize+x];
		else if(x == BlockSize/2 && z == BlockSize/2)
			//lowest LOD level: (p0+p2)/2
			return ((int32)blockVPos[0] + (int32)blockVPos[BlockSize*SourceSize+BlockSize])/2;

		size_t quadSize = BlockSize;
		index_t quadTop = 0;
		index_t quadBottom = BlockSize;
		index_t quadLeft = 0;
		index_t quadRight = BlockSize;

		bool cornerLeft = true;
		bool cornerTop = true;

		while( quadSize > 2 )
		{
			--LOD;
			assert( (int)LOD >= 0 );
			assert( quadSize > 4 || LOD == 0);
			assert( quadRight - quadLeft == quadSize );
			assert( quadBottom - quadTop == quadSize );

			//	        A
			//	   G+---+---+-------+H
			//		| \ |  /|     / |
			//		|  \| /	|    /  |
			//     D+---+---+B  /   +F
			//		|  /|\	|  /    |
			//		| /	| \	| /     |
			//      +---+---+-------+
			//      |   C  /|\      |
			//      |     / | \     |
			//      |   /   |   \   |
			//      |  /    |    \  |
			//      | /     |     \ |
			//      +---+---+-------+
			//		I	E           J

			//next level top/bottom border center point (A,C,E,...)
			if( (x == quadLeft+quadSize/4 || x == quadLeft+quadSize*3/4)
				&& (z == quadTop || z == quadTop+quadSize/2 || z == quadBottom) )
				return ((int32)((int32)blockVPos[z*SourceSize+x-quadSize/4] + (int32)blockVPos[z*SourceSize+x+quadSize/4]))/2;
			//next level left/right border center point (B,D,F,...)
			if( (z == quadTop+quadSize/4 || z == quadTop+quadSize*3/4)
				&& (x == quadLeft || x == quadLeft+quadSize/2 || x == quadRight) )
				return ((int32)((int32)blockVPos[(z-quadSize/4)*SourceSize+x] + (int32)blockVPos[(z+quadSize/4)*SourceSize+x]))/2;

			if (quadSize == 4)
			{
				//G,H,I,J
				if ((x == quadLeft || x == quadRight) && (z == quadTop || z == quadBottom))
					return blockVPos[z*SourceSize + x];
			}

			if( x < quadLeft+quadSize/2 )
			{
				quadRight -= quadSize/2;
				cornerLeft = true;
			}
			else
			{
				quadLeft += quadSize/2;
				cornerLeft = false;
			}

			if( z < quadTop+quadSize/2 )
			{
				quadBottom -= quadSize/2;
				cornerTop = true;
			}
			else
			{
				quadTop += quadSize/2;
				cornerTop = false;
			}

			//next level center point
			if( (x == quadLeft+quadSize/4 || x == quadLeft+quadSize*3/4) && (z == quadTop+quadSize/4 || z == quadTop+quadSize*3/4) )
			{
				if( (cornerLeft && cornerTop) || (!cornerLeft && !cornerTop) )
					return ((int32)((int32)blockVPos[(z-quadSize/4)*SourceSize+(x-quadSize/4)] + (int32)blockVPos[(z+quadSize/4)*SourceSize+(x+quadSize/4)]))/2;
				else
					return ((int32)((int32)blockVPos[(z-quadSize/4)*SourceSize+(x+quadSize/4)] + (int32)blockVPos[(z+quadSize/4)*SourceSize+(x-quadSize/4)]))/2;
			}

			quadSize /= 2;
		}
		assert(false);
		return 0;
	}

	//////////////////////////////////////////////////////////////////////////
	//recursive method
#if BLADE_DEBUG && DEBUG_DEPTH
	void	PreciseIndexGenerator::buildQueryIndex(size_t depth,size_t x,size_t z,size_t size,QueryIndexQuad* quad,const SQueryIndexData& data)
#else
	void	PreciseIndexGenerator::buildQueryIndex(size_t x,size_t z,size_t size,QueryIndexQuad* quad,const SQueryIndexData& data)
#endif
	{
		size_t& IndexSub = data.IndexCount;
		IndexBufferHelper& ib = data.IndexBuffer;

		assert(quad != NULL );

#if BLADE_DEBUG && DEBUG_DEPTH
		if( depth == 1 )
			//final
		{
			//minimal block
			assert(size == 1 );
#else
		if( size == 1)
		{

#endif
			//all null
			assert(quad->mSubQuad == NULL );
			SQueryIndices& indices = quad->mIndicesByLODDiff[data.DiffIndex];
			indices.mStartIndex = (uint16)IndexSub;
			indices.mTriangleCount = 0;

			uint32	RowA			= uint32(z * data.RowStride);
			uint32	RowB			= RowA + data.RowStride/2;
			uint32	RowC			= RowA + data.RowStride;
			uint32	Col0			= uint32(x * data.ColStride);
			uint32	Col1			= Col0 + data.ColStride/2;
			uint32	Col2			= Col0 + data.ColStride;

			if( !(data.Diff.isUpDiff() && z == 0) )
			{
				//0A,1B,1A
				ib[IndexSub++] = RowA + Col0;
				ib[IndexSub++] = RowB + Col1;
				ib[IndexSub++] = RowA + Col1;
				//1A,1B,2A
				ib[IndexSub++] = RowA + Col1;
				ib[IndexSub++] = RowB + Col1;
				ib[IndexSub++] = RowA + Col2;

				indices.mTriangleCount += 2;
			}
			else
			{
				//remove one triangle
				//0A,1B,2A
				ib[IndexSub++] = RowA + Col0;
				ib[IndexSub++] = RowB + Col1;
				ib[IndexSub++] = RowA + Col2;

				indices.mTriangleCount += 1;
			}

			if( !(data.Diff.isLeftDiff() && x == 0) )
			{
				//0A,0B,1B
				ib[IndexSub++] = RowA + Col0;
				ib[IndexSub++] = RowB + Col0;
				ib[IndexSub++] = RowB + Col1;
				//0B,0C,1B
				ib[IndexSub++] = RowB + Col0;
				ib[IndexSub++] = RowC + Col0;
				ib[IndexSub++] = RowB + Col1;

				indices.mTriangleCount += 2;
			}
			else
			{
				//0A,0C,1B
				ib[IndexSub++] = RowA + Col0;
				ib[IndexSub++] = RowC + Col0;
				ib[IndexSub++] = RowB + Col1;

				indices.mTriangleCount += 1;
			}

			if( !(data.Diff.isDownDiff() && z == data.RowCount -1) )
			{
				//1B,0C,1C
				ib[IndexSub++] = RowB + Col1;
				ib[IndexSub++] = RowC + Col0;
				ib[IndexSub++] = RowC + Col1;
				//1B,1C,2C
				ib[IndexSub++] = RowB + Col1;
				ib[IndexSub++] = RowC + Col1;
				ib[IndexSub++] = RowC + Col2;


				indices.mTriangleCount += 2;

			}
			else
			{
				//1B,0C,2C
				ib[IndexSub++] = RowB + Col1;
				ib[IndexSub++] = RowC + Col0;
				ib[IndexSub++] = RowC + Col2;

				indices.mTriangleCount += 1;
			}

			if( !(data.Diff.isRightDiff() && x == data.ColCount -1) )
			{
				//2A,1B,2B
				ib[IndexSub++] = RowA + Col2;
				ib[IndexSub++] = RowB + Col1;
				ib[IndexSub++] = RowB + Col2;
				//1B,2C,2B
				ib[IndexSub++] = RowB + Col1;
				ib[IndexSub++] = RowC + Col2;
				ib[IndexSub++] = RowB + Col2;


				indices.mTriangleCount += 2;
			}
			else
			{
				//2A,1B,2C
				ib[IndexSub++] = RowA + Col2;
				ib[IndexSub++] = RowB + Col1;
				ib[IndexSub++] = RowC + Col2;

				indices.mTriangleCount += 1;
			}
		}
		else
		{
			size_t subSize = size/2;
#if BLADE_DEBUG && DEBUG_DEPTH
			buildQueryIndex(depth-1,x			,z			,subSize,quad->mSubQuad[0],data);
			buildQueryIndex(depth-1,x+subSize	,z			,subSize,quad->mSubQuad[1],data);
			buildQueryIndex(depth-1,x			,z+subSize	,subSize,quad->mSubQuad[2],data);
			buildQueryIndex(depth-1,x+subSize	,z+subSize	,subSize,quad->mSubQuad[3],data);
#else
			buildQueryIndex(x			,z			,subSize,&quad->mSubQuad[0],data);
			buildQueryIndex(x+subSize	,z			,subSize,&quad->mSubQuad[1],data);
			buildQueryIndex(x			,z+subSize	,subSize,&quad->mSubQuad[2],data);
			buildQueryIndex(x+subSize	,z+subSize	,subSize,&quad->mSubQuad[3],data);
#endif

			SQueryIndices& indices = quad->mIndicesByLODDiff[data.DiffIndex];
			indices.mStartIndex = quad->mSubQuad[0].mIndicesByLODDiff[data.DiffIndex].mStartIndex;
			indices.mTriangleCount = 0;
			for(size_t i = 0; i < 4; ++i)
				indices.mTriangleCount += quad->mSubQuad[i].mIndicesByLODDiff[data.DiffIndex].mTriangleCount;
		}
	}

}//namespace Blade